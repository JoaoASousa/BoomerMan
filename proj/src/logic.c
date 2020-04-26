#include "logic.h"
#include "graphics.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "xpms.h"
#include <lcom/lcf.h>
#include <math.h>

static uint high_score = 0;
static uint high_year = 0;
static uint high_month = 0;
static uint high_day = 0;
uint current_stage = 1;
static uint bombs_placed = 0;
static uint enemies_alive = 0;
uint player_lives;
uint player_score;
uint time_left;
static uint time_interval = 60;

static xpm_image_t tileset_img;
static uint8_t *tileset_sprite;
static xpm_image_t img0, img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12, img13, img14;
static uint8_t *xpms[15];

// static Entity ** all_entities;
static Entity **all_entities;
static Tilemap tilemap;

int read_highscore_file() {
  char txt[50];
  sprintf(txt, "/home/lcom/labs/proj/src/highscore.txt");

  FILE *fp = fopen(txt, "r");

  if (!fp) {
    printf("Can't open file\n");
    return 1;
  }

  char buf[1024];
  fgets(buf, 1024, fp);
  char *field = strtok(buf, "/");
  uint64_t num;
  size_t index = 0;

  while (field != NULL) {
    // Convert to number (string to long int)
    num = strtol(field, NULL, 10);
    if (num == UINT64_MAX && errno == ERANGE) {
      printf("Could not convert!");
      return 1;
    }

    switch (index) {
      case 0:
        high_score = num;
        break;
      case 1:
        high_day = num;
        break;
      case 2:
        high_month = num;
        break;
      case 3:
        high_year = num;
        break;
      default:
        break;
    }
    index++;

    field = strtok(NULL, "/");
  }

  fclose(fp);
  return 0;
}

int update_highscore() {
  char txt[50];
  sprintf(txt, "/home/lcom/labs/proj/src/temp.txt");

  FILE *fp = fopen(txt, "w");

  if (!fp) {
    printf("Can't open file\n");
    return 1;
  }

  fprintf(fp, "%d/%d/%d/%d", high_score, high_day, high_month, high_year);

  remove("/home/lcom/labs/proj/src/highscore.txt");
  rename("/home/lcom/labs/proj/src/temp.txt", "/home/lcom/labs/proj/src/highscore.txt");

  fclose(fp);
  return 0;
}

void insertEnemy(Vector2D tile_pos, uint64_t identifier) {

  for (size_t i = 0; i < ARRAY_SIZE; i++) {

    if (all_entities[i]->type == NO_TYPE) {
      all_entities[i]->type = ENEMY;
      all_entities[i]->controls_disabled = false;
      all_entities[i]->dead = false;
      all_entities[i]->firepass = false;
      all_entities[i]->tile_pos = tile_pos;
      all_entities[i]->entity_pos.x = tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X + TILESIZE / 2;
      all_entities[i]->entity_pos.y = tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y + TILESIZE / 2;
      all_entities[i]->sprite = xpms[identifier + ENEMY_XPM_STARTING];
      all_entities[i]->img = &img5; // Enemies xpms all have same atributes;
      all_entities[i]->linear_speed = (0.5 * identifier + 1) * 0.5 + 1;
      if (identifier == 3)
        all_entities[i]->wallpass = true;
      else
        all_entities[i]->wallpass = false;
      all_entities[i]->animation_state = (Vector2D){0, 0};
      all_entities[i]->animation_cooldown = ENEM_COOLDOWN;

      if (all_entities[i]->tile_pos.y == 2 || all_entities[i]->tile_pos.y == 6 || all_entities[i]->tile_pos.y == 10)
        all_entities[i]->current_facing = NORTH;

      else if (all_entities[i]->tile_pos.y == 4 || all_entities[i]->tile_pos.y == 8)
        all_entities[i]->current_facing = SOUTH;

      else if (all_entities[i]->tile_pos.y == 1 || all_entities[i]->tile_pos.y == 5 || all_entities[i]->tile_pos.y == 9 || all_entities[i]->tile_pos.y == 13)
        all_entities[i]->current_facing = EAST;

      else if (all_entities[i]->tile_pos.y == 3 || all_entities[i]->tile_pos.y == 7 || all_entities[i]->tile_pos.y == 11 || all_entities[i]->tile_pos.y == 15)
        all_entities[i]->current_facing = WEST;

      all_entities[i]->score = pow(2, identifier - 1) * 100;

      break;
    }

    // enemies_alive++;
  }
  enemies_alive++;
}

int read_map(void) {
  char csv[50];
  sprintf(csv, "/tmp/resources/map%d.csv", current_stage);

  FILE *fp = fopen(csv, "r");

  if (!fp) {
    printf("Can't open file\n");
    return 1;
  }

  char buf[1024];
  int row_count = 0;
  int field_count = 0;

  // Resetting enemies
  enemies_alive = 0;

  // put in buf a line
  while (fgets(buf, 1024, fp)) {
    field_count = 0;
    row_count++;

    // CSV Header skip
    if (row_count == 1) {
      continue;
    }

    // Separate elements of a string using "," as delimiter
    char *field = strtok(buf, ",");

    while (field) {

      // Parsing map tiles
      if (row_count >= MAP_TILE_INFO_START_ROW && row_count < MAP_ENT_INFO_START_ROW) {

        // Convert to number (string to long int)
        uint64_t num = strtol(field, NULL, 10);
        if (num == UINT64_MAX && errno == ERANGE) {
          printf("Could not convert!");
          return 1;
        }
        else {
          tilemap.tiles[row_count - MAP_TILE_INFO_START_ROW][field_count] = num;
        }

        // Parsing map collisions

        BoundingBox bbox;
        Vector2D p1;
        Vector2D p2;

        p1.x = (field_count) *TILESIZE + MAP_RENDER_OFFSET_X;
        p1.y = (row_count - MAP_TILE_INFO_START_ROW) * TILESIZE + MAP_RENDER_OFFSET_Y;

        p2.x = (field_count + 1) * TILESIZE + MAP_RENDER_OFFSET_X;
        p2.y = (row_count - MAP_TILE_INFO_START_ROW + 1) * TILESIZE + MAP_RENDER_OFFSET_Y;

        bbox.upper_left = p1;
        bbox.lower_right = p2;

        // Has collision
        if (num == 1) {
          bbox.active = true;
          bbox.permanent = false;
        }
        else if (num == 0) {
          bbox.active = true;
          bbox.permanent = true;
        }
        else {
          bbox.active = false;
          bbox.permanent = true;
        }
        tilemap.collisions[row_count - MAP_TILE_INFO_START_ROW][field_count] = bbox;
      }
      // Parsing map entities

      else if (row_count >= MAP_ENT_INFO_START_ROW && row_count < MAP_ENEMY_INFO_START_ROW) {

        // Convert to number (string to long int)
        uint64_t num = strtol(field, NULL, 10);
        if (num == UINT64_MAX && errno == ERANGE) {
          printf("Could not convert!");
          return 1;
        }
        else {
          tilemap.map_entities[row_count - MAP_ENT_INFO_START_ROW][field_count] = num;
        }
      }

      // Parsing map enemies
      else {
        // Convert to number (string to long int)
        uint64_t num = strtol(field, NULL, 10);
        if (num == UINT64_MAX && errno == ERANGE) {
          printf("Could not convert!");
          return 1;
        }
        else {
          if (num != 0)
            insertEnemy((Vector2D){field_count, row_count - MAP_ENEMY_INFO_START_ROW}, num);
        }
      }

      field = strtok(NULL, ",");
      field_count++;
    }

    load_map(tileset_xpm);
  }

  // for (int y = 0; y < 13; y++) {
  //   for (int x = 0; x < 15; x++) {
  //     printf("%d, ", tilemap.tiles[y][x]);
  //   }
  //   printf("\n");
  // }

  fclose(fp);
  return 0;
}

void load_map(xpm_map_t xpm) {
  tileset_sprite = xpm_load(xpm, XPM_8_8_8, &tileset_img);
}

void load_xpms() {

  xpms[0] = xpm_load(Bomberman_xpm, XPM_8_8_8, &img0);

  xpms[1] = xpm_load(Bomb_xpm, XPM_8_8_8, &img1);

  xpms[2] = xpm_load(Explosions_xpm, XPM_8_8_8, &img2);

  xpms[3] = xpm_load(Walls_xpm, XPM_8_8_8, &img3);

  xpms[4] = xpm_load(Entities_xpm, XPM_8_8_8, &img4);

  xpms[5] = xpm_load(Enemy_1_xpm, XPM_8_8_8, &img5);

  xpms[6] = xpm_load(Enemy_2_xpm, XPM_8_8_8, &img6);

  xpms[7] = xpm_load(Enemy_3_xpm, XPM_8_8_8, &img7);

  xpms[8] = xpm_load(Enemy_4_xpm, XPM_8_8_8, &img8);

  xpms[9] = xpm_load(Info_xpm, XPM_8_8_8, &img9);

  xpms[10] = xpm_load(Numbers_xpm, XPM_8_8_8, &img10);

  xpms[11] = xpm_load(TitleScreen1_xpm, XPM_8_8_8, &img11);

  xpms[12] = xpm_load(TitleScreen2_xpm, XPM_8_8_8, &img12);

  xpms[13] = xpm_load(Level_screen_xpm, XPM_8_8_8, &img13);

  xpms[14] = xpm_load(Game_end_xpm, XPM_8_8_8, &img14);
}

void free_entities() {
  free(all_entities);
}

void load_entities(Entity * oldPlayer) {

  all_entities = malloc(ARRAY_SIZE * sizeof(Entity));

  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    all_entities[i] = malloc(sizeof(Entity));
    all_entities[i]->type = NO_TYPE;
  }

  if (oldPlayer != NULL) all_entities[0] = oldPlayer;
}

void load_player(bool new_game) {
  Entity *player = all_entities[0];
  player->sprite = xpms[0];
  player->img = &img0;
  player->animation_state = (Vector2D){0, 0};
  player->current_facing = NO_DIR;
  player->animation_cooldown = ANIM_COOLDOWN;

  if (new_game){
    player->max_bombs = 1;
    player->max_range = 1;
    player->bombpass = false;
    player->wallpass = false;
    player->firepass = false;
    player->remote_control = false;
    player->linear_speed = 2;
    player->invincibility_cooldown = 0;

  }

  player->controls_disabled = false;
  player->dead = false;

  player->type = PLAYER;
  // To center its coordinates
  player->entity_pos.x = 1 * TILESIZE + MAP_RENDER_OFFSET_X + TILESIZE / 2;
  player->entity_pos.y = 3 * TILESIZE + MAP_RENDER_OFFSET_Y + TILESIZE / 2;
}

void draw_game_over_screen(){
  vg_clear_screen();
  vg_draw_xpm(xpms[14], &img14, 128, 64);
  draw_number(player_score, 128 + 230, 256 + 64, false);
  double_buf();

  sleep(4);
}

void draw_level_screen(){
  vg_clear_screen();
  vg_draw_xpm(xpms[13], &img13, 128, 64);
  draw_number(current_stage, 128 + 316, 64 + 240, false);
  double_buf();
  
  sleep(2);
}

void reset_stage() {
  bombs_placed = 0;
  time_left = 200;
  enemies_alive = 0;
  load_entities(all_entities[0]);
  load_player(false);
  read_map();

  draw_level_screen();
}

void draw_number(int n, int16_t x, int16_t y, bool must_be_two_digits) {
  int digits[9];
  size_t counter = 0;

  // Wont draw numbers too long or negative numbers
  if (n < 0 || n > 999999999) {
    vg_draw_tile(xpms[10], img10, x, y, 0, 0, TILESIZE / 2);
    return;
  }

  if (n == 0) {
    digits[0] = 0;
    counter++;
  }

  while (n > 0) {
    digits[counter] = n % 10;
    n /= 10;
    counter++;
  }

  for (size_t i = counter; i > 0; i--) {
    
    if (counter == 1 && must_be_two_digits) {
      vg_draw_tile(xpms[10], img10, x, y, 0, 0, TILESIZE / 2);
      vg_draw_tile(xpms[10], img10, x + TILESIZE / 2, y, digits[0], 0, TILESIZE / 2);
    }
    
    else vg_draw_tile(xpms[10], img10, x + ((counter - i) * TILESIZE / 2), y, digits[i - 1], 0, TILESIZE / 2);
  }
}

void draw_ui() {

  if (time_interval == 0) {
    time_left--;
    time_interval = 60;
  }
  else
    time_interval--;

  // Time
  vg_draw_tile(xpms[9], img9, MAP_RENDER_OFFSET_X + 16, MAP_RENDER_OFFSET_Y - 48, 0, 0, TILESIZE * 2);
  draw_number(time_left, MAP_RENDER_OFFSET_X + 16 + 80, MAP_RENDER_OFFSET_Y - TILESIZE + 7, false);

  // Score

  draw_number(player_score, MAP_RENDER_OFFSET_X + 352, MAP_RENDER_OFFSET_Y - TILESIZE + 7, false);

  // Lives left
  vg_draw_tile(xpms[9], img9, MAP_RENDER_OFFSET_X + 608, MAP_RENDER_OFFSET_Y - 48, 1, 0, TILESIZE * 2);
  draw_number(player_lives, MAP_RENDER_OFFSET_X + 608 + 80, MAP_RENDER_OFFSET_Y - TILESIZE + 7, false);
}

void draw_map(uint16_t offset_x, uint16_t offset_y) {

  for (size_t y = 0; y < MAP_TILE_HEIGHT; y++) {
    for (size_t x = 0; x < MAP_TILE_WIDTH; x++) {

      uint16_t tile_x = tilemap.tiles[y][x] % MAP_TILESET_WIDTH;
      uint16_t tile_y = tilemap.tiles[y][x] / MAP_TILESET_WIDTH;

      if (tilemap.tiles[y][x] == 1) {
        // Entity before wall

        vg_draw_tile(tileset_sprite, tileset_img, x * TILESIZE + offset_x, y * TILESIZE + offset_y, tile_x, tile_y, TILESIZE);
      }
      else {
        // Wall before entity
        vg_draw_tile(tileset_sprite, tileset_img, x * TILESIZE + offset_x, y * TILESIZE + offset_y, tile_x, tile_y, TILESIZE);

        if (tilemap.map_entities[y][x] != NO_ENT)
          vg_draw_tile(xpms[4], img4, x * TILESIZE + offset_x, y * TILESIZE + offset_y, tilemap.map_entities[y][x], 0, TILESIZE);
      }
    }
  }
}

bool Vector2DEqual(Vector2D l, Vector2D r) {
  return (l.x == r.x && l.y == r.y);
}

bool boxesIntersect(Vector2D a, Vector2D b, int aSize, int bSize) {
  return ((abs(a.x - b.x) * 2 < (aSize + bSize)) && (abs(a.y - b.y) * 2 < (aSize + bSize)));
}

bool tilesIntersect(Vector2D a, Vector2D b, int aSize, int bSize) {
  Vector2D this;
  this.x = a.x * TILESIZE + MAP_RENDER_OFFSET_X;
  this.y = a.y * TILESIZE + MAP_RENDER_OFFSET_Y;
  Vector2D that;
  that.x = b.x * TILESIZE + MAP_RENDER_OFFSET_X;
  that.y = b.y * TILESIZE + MAP_RENDER_OFFSET_Y;
  return boxesIntersect(this, that, aSize, bSize);
}

void exploded(Entity *this) {
  this->dead = true;
  this->controls_disabled = true;
  this->entity_vel = (Vector2D){0, 0};
}

void placeBrokenWalls(Vector2D walls_broken[4]) {
  // Go through possible broken walls;
  for (size_t j = 0; j < 4; j++) {
    if (!Vector2DEqual(walls_broken[j], (Vector2D){0, 0})) {

      for (size_t k = 0; k < ARRAY_SIZE; k++) {

        // If there is already a broken wall entity on that tile, won't place
        if (all_entities[k]->type == BROKEN_WALL && Vector2DEqual(all_entities[k]->tile_pos, walls_broken[j]))
          break;

        // Create broken wall entity
        if (all_entities[k]->type == NO_TYPE) {

          // Remove wall from tilemap
          tilemap.tiles[walls_broken[j].y][walls_broken[j].x] = 2;

          all_entities[k]->type = BROKEN_WALL;
          all_entities[k]->img = &img3;
          all_entities[k]->sprite = xpms[3];
          all_entities[k]->animation_state = (Vector2D){0, 0};
          all_entities[k]->animation_cooldown = WALL_BREAK_COOLDOWN;
          all_entities[k]->tile_pos = walls_broken[j];
          break;
        }
      }
    }
  }
}

void powerupCollisions(Entity *player) {
  if (player->type != PLAYER) return;
  for (size_t i = 0; i < MAP_TILE_HEIGHT; i++) {
    for (size_t j = 0; j < MAP_TILE_WIDTH; j++) {
      Vector2D tile = {j, i};
      if (tilesIntersect(player->tile_pos, tile, ENTITY_BB_SIZE, TILESIZE) && tilemap.tiles[tile.y][tile.x] == 2) {

        switch (tilemap.map_entities[i][j]) {
          case BOMB_UP:
            player->max_bombs++;
            break;
          case FIRE_UP:
            player->max_range++;
            break;
          case SPEED_UP:
            player->linear_speed += SPEED_INCREMENT;
            break;
          case WALL_PASS:
            player->wallpass = true;
            break;
          case REMOTE_CONTROL:
            player->remote_control = true;
            break;
          case BOMB_PASS:
            player->bombpass = true;
            break;
          case FIRE_PASS:
            player->firepass = true;
            break;
          case QUESTION_MARK:
            player->invincibility_cooldown = INVINCIBILITY_COOLDOWN;
            break;
          case EXIT:
            //printf("ENEMIES: %d", enemies_alive);
            if (enemies_alive == 0) {
              current_stage++;
              if (current_stage > LAST_LEVEL)
                return;
              player_lives++;
              if (current_stage <= LAST_LEVEL)
                reset_stage();
            }
            break;
          default:
            break;
        }

        // If player picks up power, remove it
        if (tilemap.map_entities[i][j] != NO_ENT && tilemap.map_entities[i][j] != EXIT) {
          tilemap.map_entities[i][j] = NO_ENT;
        }
      }
    }
  }
}

bool mapCollisions(Vector2D entity_coords, bool wallpass) {
  for (int j = 0; j < MAP_TILE_HEIGHT; j++) {
    for (int k = 0; k < MAP_TILE_WIDTH; k++) {
      BoundingBox current = tilemap.collisions[j][k];
      if (current.active) {

        int16_t boxX = current.upper_left.x + TILESIZE / 2;
        int16_t boxY = current.upper_left.y + TILESIZE / 2;
        Vector2D currentBox = {boxX, boxY};

        if (boxesIntersect(currentBox, entity_coords, TILESIZE, ENTITY_BB_SIZE)) {
          if (current.permanent || (!current.permanent && !wallpass)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool explosionCollision(Vector2D this_tile, Entity *explosion) {
  // Center
  if (tilesIntersect(this_tile, explosion->tile_pos, ENTITY_BB_SIZE, TILESIZE))
    return true;

  size_t max_cycle;

  // Left
  if (explosion->hit_l == 0)
    max_cycle = explosion->max_range;
  else
    max_cycle = explosion->hit_l;
  for (size_t i = 1; i <= max_cycle; i++) {
    if (tilesIntersect(this_tile, (Vector2D){explosion->tile_pos.x - i, explosion->tile_pos.y}, ENTITY_BB_SIZE, TILESIZE))
      return true;
  }

  // Right
  if (explosion->hit_r == 0)
    max_cycle = explosion->max_range;
  else
    max_cycle = explosion->hit_r;
  for (size_t i = 1; i <= max_cycle; i++) {
    if (tilesIntersect(this_tile, (Vector2D){explosion->tile_pos.x + i, explosion->tile_pos.y}, ENTITY_BB_SIZE, TILESIZE))
      return true;
  }

  // Up
  if (explosion->hit_u == 0)
    max_cycle = explosion->max_range;
  else
    max_cycle = explosion->hit_u;
  for (size_t i = 1; i <= max_cycle; i++) {
    if (tilesIntersect(this_tile, (Vector2D){explosion->tile_pos.x, explosion->tile_pos.y - i}, ENTITY_BB_SIZE, TILESIZE))
      return true;
  }

  // Down
  if (explosion->hit_d == 0)
    max_cycle = explosion->max_range;
  else
    max_cycle = explosion->hit_d;
  for (size_t i = 1; i <= max_cycle; i++) {
    if (tilesIntersect(this_tile, (Vector2D){explosion->tile_pos.x, explosion->tile_pos.y + i}, ENTITY_BB_SIZE, TILESIZE))
      return true;
  }

  return false;
}

void explosionWithMapCollision(Entity *explosion) {

  // Initialize and fill with default values vector to hold broken wall
  Vector2D walls_broken[4] = {(Vector2D){0, 0}, (Vector2D){0, 0}, (Vector2D){0, 0}, (Vector2D){0, 0}};

  for (size_t i = 1; i <= explosion->max_range; i++) {
    BoundingBox current_box;

    // Horizontal

    // Left
    current_box = tilemap.collisions[explosion->tile_pos.y][explosion->tile_pos.x - i];

    if (explosion->hit_l == 0) {
      if (current_box.active && !current_box.permanent) {
        walls_broken[0] = (Vector2D){explosion->tile_pos.x - i, explosion->tile_pos.y};
        explosion->hit_l = i;
      }
      else if (current_box.active && current_box.permanent)
        explosion->hit_l = i;
    }

    // Right
    current_box = tilemap.collisions[explosion->tile_pos.y][explosion->tile_pos.x + i];

    if (explosion->hit_r == 0) {
      if (current_box.active && !current_box.permanent) {
        walls_broken[1] = (Vector2D){explosion->tile_pos.x + i, explosion->tile_pos.y};
        explosion->hit_r = i;
      }
      else if (current_box.active && current_box.permanent)
        explosion->hit_r = i;
    }

    // Vertical

    // Up
    current_box = tilemap.collisions[explosion->tile_pos.y - i][explosion->tile_pos.x];

    if (explosion->hit_u == 0) {
      if (current_box.active && !current_box.permanent) {
        walls_broken[2] = (Vector2D){explosion->tile_pos.x, explosion->tile_pos.y - i};
        explosion->hit_u = i;
      }
      else if (current_box.active && current_box.permanent)
        explosion->hit_u = i;
    }

    // Down
    current_box = tilemap.collisions[explosion->tile_pos.y + i][explosion->tile_pos.x];

    if (explosion->hit_d == 0) {
      if (current_box.active && !current_box.permanent) {
        walls_broken[3] = (Vector2D){explosion->tile_pos.x, explosion->tile_pos.y + i};
        explosion->hit_d = i;
      }
      else if (current_box.active && current_box.permanent)
        explosion->hit_d = i;
    }
  }

  placeBrokenWalls(walls_broken);
}

bool entityCollisions(Entity *this) {
  bool result = false;
  for (size_t i = 0; i < ARRAY_SIZE; i++) {

    if (all_entities[i] == this) {
      continue;
    }

    if (this->type == PLAYER) {
      // Hit enemy -> Die
      if (all_entities[i]->type == ENEMY && this->invincibility_cooldown == 0) {
        if (boxesIntersect(this->entity_pos, all_entities[i]->entity_pos, ENTITY_BB_SIZE, ENTITY_BB_SIZE) && all_entities[i]->animation_state.y < 2 && this->current_facing != DEAD) {
          // Die
          exploded(this);
          result = true;
        }
      }

      // Hit bomb -> Check if has powerup and if is active -> Go back
      if (all_entities[i]->type == BOMB) {
        result = (tilesIntersect(this->tile_pos, all_entities[i]->tile_pos, TILESIZE, ENTITY_BB_SIZE) && !this->bombpass && all_entities[i]->box_active);
      }

      powerupCollisions(this);
    }

    if (this->type == EXPLOSION) {

      // Hit destructable wall -> Break
      if (this->explosion_active)
        explosionWithMapCollision(this);

      this->explosion_active = false;

      // Hit player -> Die
      if (all_entities[i]->type == PLAYER && all_entities[i]->current_facing != DEAD && all_entities[i]->invincibility_cooldown == 0) {
        if (explosionCollision(all_entities[i]->tile_pos, this)) {
          // Die
          if (!all_entities[i]->firepass){
            exploded(all_entities[i]);
            result = true;
          }
          else result = false;
        }
      }
      // Hit enemy -> Die
      if (all_entities[i]->type == ENEMY && all_entities[i]->current_facing != DEAD) {
        if (explosionCollision(all_entities[i]->tile_pos, this)) {
          // Die
          if (!all_entities[i]->firepass){
            exploded(all_entities[i]);
            result = true;
          }
          else result = false;
        }
      }
      // Hit bomb -> bomb_ticks = 1;
      if (all_entities[i]->type == BOMB) {
        if (explosionCollision(all_entities[i]->tile_pos, this)) {
          all_entities[i]->bomb_tick = 1;
          result = true;
        }
      }
    }

    if (this->type == ENEMY) {
      if (all_entities[i]->type == BOMB) {
        result = (tilesIntersect(this->tile_pos, all_entities[i]->tile_pos, ENTITY_BB_SIZE, TILESIZE));
      }
    }
  }

  return result;
}

void detonate() {
  printf("HEHRHRHA");
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
      if (all_entities[0]->remote_control && all_entities[i]->type == BOMB) {
        all_entities[i]->bomb_tick = 0;
        break;
      }
    }
}

void placeBomb(Vector2D tile_coords) {
  if (bombs_placed < all_entities[0]->max_bombs && tilemap.tiles[tile_coords.y][tile_coords.x] == 2) {

    for (size_t i = 0; i < ARRAY_SIZE; i++) {

      // Stops player from placing bombs on tile with an entity
      if (all_entities[i]->type != PLAYER && all_entities[i]->type != NO_TYPE && Vector2DEqual(all_entities[i]->tile_pos, tile_coords))
        break;

      // Found empty spot on array
      if (all_entities[i]->type == NO_TYPE) {
        bombs_placed++;
        all_entities[i]->box_active = false;
        all_entities[i]->sprite = xpms[1];
        all_entities[i]->img = &img1;
        all_entities[i]->type = BOMB;
        all_entities[i]->bomb_tick = BOMB_TICK;
        all_entities[i]->tile_pos = tile_coords;
        all_entities[i]->animation_state = (Vector2D){0, 0};
        break;
      }
    }
  }
}

void placeExplosion(Entity *this) {
  bombs_placed--;
  this->type = EXPLOSION;
  this->max_range = all_entities[0]->max_range;
  this->sprite = xpms[2];
  this->img = &img2;
  this->animation_state = (Vector2D){0, 0};
  this->animation_cooldown = EXPLOSION_COOLDOWN;
  this->explosion_active = true;
  this->hit_d = 0;
  this->hit_l = 0;
  this->hit_r = 0;
  this->hit_u = 0;
  //printf("TILECOORDS: %d, %d", this->tile_pos.x, this->tile_pos.y);
}

void handle_animations() {
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    Entity *this = all_entities[i];

    if (this->type == PLAYER) {

      // Reset animation cycle
      if (this->animation_state.x > 3) {
        this->animation_state.x = 0;
      }

      if (this->animation_cooldown > 0) {
        this->animation_cooldown--;
      }

      if (this->dead) {
        this->animation_state = (Vector2D){0, 4};
        this->animation_cooldown = DEAD_FACE_COOLDOWN;
        this->current_facing = DEAD;
        this->dead = false;
      }

      switch (this->current_facing) {
        case WEST:
          if (this->animation_state.y == 0) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = PLAYER_ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 0};
          break;

        case SOUTH:
          if (this->animation_state.y == 1) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = PLAYER_ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 1};
          break;

        case EAST:
          if (this->animation_state.y == 2) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = PLAYER_ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 2};
          break;

        case NORTH:
          if (this->animation_state.y == 3) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = PLAYER_ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 3};
          break;

        case NO_DIR:
          if (this->animation_state.x != 1)
            this->animation_state.x = 1;
          break;

        case DEAD:
          if (this->animation_cooldown == 0) {
            this->animation_state.x++;
            this->animation_cooldown = DEAD_STEP_COOLDOWN;

            if (this->animation_state.x > 3) {
              sleep(1);
              
              this->type = NO_TYPE;
              player_lives--;
              if (player_lives != 0) reset_stage(); // Wont load level if its game over

              if (player_score > high_score) {
                high_score = player_score;
                
                uint8_t nothing;
                uint8_t day ;
                uint8_t month;
                uint8_t year;

                get_date(&nothing, &day, &month, &year);

                high_day = day;
                high_month = month;   
                high_year = 2000 + year;
                
              }
            }
          }
          break;

        default:
          break;
      }
    }

    if (this->type == ENEMY) {
      // Reset animation cycle
      if (this->animation_state.x > 3) {
        this->animation_state.x = 0;
      }

      if (this->animation_cooldown > 0) {
        this->animation_cooldown--;
      }

      if (this->dead) {
        this->animation_state = (Vector2D){0, 2};
        this->animation_cooldown = DEAD_FACE_COOLDOWN;
        this->current_facing = DEAD;
        this->dead = false;
      }

      switch (this->current_facing) {
        case EAST:
          if (this->animation_state.y == 0) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 0};
          break;

        case WEST:
          if (this->animation_state.y == 1) {
            if (this->animation_cooldown == 0) {
              this->animation_state.x++;
              this->animation_cooldown = ANIM_COOLDOWN;
            }
          }
          else
            this->animation_state = (Vector2D){0, 1};
          break;

        case NORTH:
        case SOUTH:
          if (this->animation_cooldown == 0) {
            this->animation_state.x++;
            this->animation_cooldown = ANIM_COOLDOWN;
          }
          break;

        case DEAD:
          if (this->animation_cooldown == 0) {
            this->animation_state.x++;
            this->animation_cooldown = DEAD_STEP_COOLDOWN;

            if (this->animation_state.x > 3) {
              this->animation_state = (Vector2D){0, 3};
              this->animation_cooldown = SCORE_COOLDOWN;
              this->current_facing = SCORE;
            }
          }
          break;

        case SCORE:
          player_score += this->score / SCORE_COOLDOWN;
          if (this->animation_cooldown == 0) {
            enemies_alive--;
            this->type = NO_TYPE;
          }

        default:
          break;
      }
    }

    if (this->type == BOMB) {

      // Player exited bomb tile
      if (!tilesIntersect(this->tile_pos, all_entities[0]->tile_pos, TILESIZE, ENTITY_BB_SIZE))
        this->box_active = true;

      // Reset animation
      if (this->animation_state.x > 3)
        this->animation_state.x = 0;

      if (this->animation_cooldown > 0) {
        this->animation_cooldown--;
      }

      if (this->animation_cooldown == 0) {
        this->animation_state.x++;
        this->animation_cooldown = BOMB_COOLDOWN;
        this->bomb_tick--;
      }

      // Not here ??
      // Explode bomb
      if (this->bomb_tick == 0) {
        placeExplosion(this);
      }
    }

    if (this->type == EXPLOSION) {
      if (this->animation_state.x > 4) {
        this->type = NO_TYPE;
      }

      if (this->animation_cooldown > 0) {
        this->animation_cooldown--;
      }

      if (this->animation_cooldown == 0) {
        this->animation_state.x++;
        this->animation_cooldown = EXPLOSION_COOLDOWN;
        // printf("Incremented animation\n");
      }
    }

    if (this->type == BROKEN_WALL) {
      if (this->animation_state.x > 4) {
        this->type = NO_TYPE;
        tilemap.collisions[this->tile_pos.y][this->tile_pos.x].active = false;
        tilemap.collisions[this->tile_pos.y][this->tile_pos.x].permanent = true;
      }
      if (this->animation_cooldown > 0) {
        this->animation_cooldown--;
      }
      if (this->animation_cooldown == 0) {
        this->animation_state.x++;
        this->animation_cooldown = WALL_BREAK_COOLDOWN;
      }
    }
  }
}

void handle_input_ms(enum MS_KEY input) {

  switch (input) {

    case UP:
      all_entities[0]->entity_vel.y = -1;
      all_entities[0]->entity_vel.x = 0;
      all_entities[0]->current_facing = NORTH;
      break;

    // down
    case DOWN:
      all_entities[0]->entity_vel.y = 1;
      all_entities[0]->entity_vel.x = 0;
      all_entities[0]->current_facing = SOUTH;
      break;

    // left
    case LEFT:
      all_entities[0]->entity_vel.x = -1;
      all_entities[0]->entity_vel.y = 0;
      all_entities[0]->current_facing = WEST;
      break;

    // right
    case RIGHT:
      all_entities[0]->entity_vel.x = 1;
      all_entities[0]->entity_vel.y = 0;
      all_entities[0]->current_facing = EAST;
      break;

    case NO_CLICK:
      all_entities[0]->entity_vel = (Vector2D){0, 0};
      all_entities[0]->current_facing = NO_DIR;
      break;
      
    case LEFT_CLICK:
      placeBomb(all_entities[0]->tile_pos);
      break;
    
    case RIGHT_CLICK:
      detonate();
      break;
    
    default:
      break;
  }
}

void handle_input_kb(enum KB_KEY input) {

  if (!all_entities[0]->controls_disabled) {
    // keyboard or mouse input
    switch (input) {
      // up
      case W_MAKE:
        all_entities[0]->entity_vel.y = -1;
        all_entities[0]->current_facing = NORTH;
        break;
      // down
      case S_MAKE:
        all_entities[0]->entity_vel.y = 1;
        all_entities[0]->current_facing = SOUTH;
        break;
      // left
      case A_MAKE:
        all_entities[0]->entity_vel.x = -1;
        all_entities[0]->current_facing = WEST;
        break;
      // right
      case D_MAKE:
        all_entities[0]->entity_vel.x = 1;
        all_entities[0]->current_facing = EAST;
        break;
      case W_BREAK:
        all_entities[0]->entity_vel.y = 0;
        all_entities[0]->current_facing = NO_DIR;

        break;
      case S_BREAK:
        all_entities[0]->entity_vel.y = 0;
        all_entities[0]->current_facing = NO_DIR;

        break;
      case A_BREAK:
        all_entities[0]->entity_vel.x = 0;
        all_entities[0]->current_facing = NO_DIR;

        break;
      case D_BREAK:
        all_entities[0]->entity_vel.x = 0;
        all_entities[0]->current_facing = NO_DIR;

        break;
      case SPACE_BREAK:

        // Place bomb on player tile coords
        placeBomb(all_entities[0]->tile_pos);
        break;

      case ENTER_BREAK:

        // Try detonating
        detonate();
        break;
      default:
        break;
    }
  }
}

void handle_physics() {
  if (time_left == 0) exploded(all_entities[0]);

  for (size_t i = 0; i < ARRAY_SIZE; ++i) {
    Entity *this = all_entities[i];

    if (this->type == PLAYER) {
      if (this->invincibility_cooldown > 0) this->invincibility_cooldown--;

      int diffx = this->entity_vel.x * this->linear_speed;
      int diffy = this->entity_vel.y * this->linear_speed;

      Vector2D old_pos = this->entity_pos;

      // checking collisions, on all entities

      // Try moving x
      if (!this->controls_disabled) {
        this->entity_pos.x += diffx;
      }

      // Update possible tile
      this->tile_pos.x = (this->entity_pos.x - MAP_RENDER_OFFSET_X) / TILESIZE;

      if (mapCollisions(this->entity_pos, this->wallpass) || entityCollisions(this)) {
        this->entity_pos.x = old_pos.x;
      }

      // Then y
      if (!this->controls_disabled) {
        this->entity_pos.y += diffy;
      }

      // Update possible tile
      this->tile_pos.y = (this->entity_pos.y - MAP_RENDER_OFFSET_Y) / TILESIZE;

      if (mapCollisions(this->entity_pos, this->wallpass) || entityCollisions(this)) {
        this->entity_pos.y = old_pos.y;
      }

      // Making sure tile is correct
      this->tile_pos.x = (this->entity_pos.x - MAP_RENDER_OFFSET_X) / TILESIZE;
      this->tile_pos.y = (this->entity_pos.y - MAP_RENDER_OFFSET_Y) / TILESIZE;
    }

    else if (this->type == ENEMY) {

      // Update velocity
      switch (this->current_facing) {
        case NORTH:
          this->entity_vel = (Vector2D){0, -1};
          break;
        case SOUTH:
          this->entity_vel = (Vector2D){0, 1};
          break;
        case EAST:
          this->entity_vel = (Vector2D){1, 0};
          break;
        case WEST:
          this->entity_vel = (Vector2D){-1, 0};
        default:
          break;
      }

      int diffx = this->entity_vel.x * this->linear_speed;
      int diffy = this->entity_vel.y * this->linear_speed;

      Vector2D old_pos = this->entity_pos;

      // Try moving x
      if (!this->controls_disabled) {
        this->entity_pos.x += diffx;
      }

      // Update possible tile
      this->tile_pos.x = (this->entity_pos.x - MAP_RENDER_OFFSET_X) / TILESIZE;

      if (mapCollisions(this->entity_pos, this->wallpass) || entityCollisions(this)) {
        this->entity_pos.x = old_pos.x;

        // If it collides with map, switch direction
        switch (this->current_facing) {
          case EAST:
            this->current_facing = WEST;
            break;
          case WEST:
            this->current_facing = EAST;
            break;
          default:
            break;
        }
      }

      // Then y
      if (!this->controls_disabled) {
        this->entity_pos.y += diffy;
      }

      // Update possible tile
      this->tile_pos.y = (this->entity_pos.y - MAP_RENDER_OFFSET_Y) / TILESIZE;

      if (mapCollisions(this->entity_pos, this->wallpass) || entityCollisions(this)) {
        this->entity_pos.y = old_pos.y;

        // If it collides with map, switch direction
        switch (this->current_facing) {
          case NORTH:
            this->current_facing = SOUTH;
            break;
          case SOUTH:
            this->current_facing = NORTH;
            break;
          default:
            break;
        }
      }

      // Making sure tile is correct
      this->tile_pos.x = (this->entity_pos.x - MAP_RENDER_OFFSET_X) / TILESIZE;
      this->tile_pos.y = (this->entity_pos.y - MAP_RENDER_OFFSET_Y) / TILESIZE;
    }

    //FIXME Dont understand why, but bombs dont work without this
    else {
      int diffx = all_entities[i]->entity_vel.x * all_entities[i]->linear_speed;
      int diffy = all_entities[i]->entity_vel.y * all_entities[i]->linear_speed;

      Vector2D old_pos = all_entities[i]->entity_pos;

      // checking collisions, on all entities

      // Try moving x
      if (!all_entities[i]->controls_disabled) {
        all_entities[i]->entity_pos.x += diffx;
      }

      if (mapCollisions(all_entities[i]->entity_pos, all_entities[i]->wallpass) || entityCollisions(all_entities[i])) {
        all_entities[i]->entity_pos.x = old_pos.x;
      }

      // Then y
      if (!all_entities[i]->controls_disabled) {
        all_entities[i]->entity_pos.y += diffy;
      }

      if (mapCollisions(all_entities[i]->entity_pos, all_entities[i]->wallpass) || entityCollisions(all_entities[i])) {
        all_entities[i]->entity_pos.y = old_pos.y;
      }
    }
  }
}

void draw_menu(int selected_menu) {
  rtc_set_alarm(2);
  vg_clear_screen();

  // Select title screen
  // Draw title screen
  // Draw highscore and timestamp

  if (selected_menu == 1) {
    vg_draw_xpm(xpms[11], &img11, 128, 64);

    draw_number(high_score, 326 + 128, 336 + 64, false);

    draw_number(high_day, 135 + 128, 336 + 64, true);
    draw_number(high_month, 189 + 128, 336 + 64, true);
    draw_number(high_year, 241 + 128, 336 + 64, true);
  }

  else {
    vg_draw_xpm(xpms[12], &img12, 128, 64);

    draw_number(high_score, 326 + 128, 336 + 64, false);

    draw_number(high_day, 135 + 128, 336 + 64, true);
    draw_number(high_month, 189 + 128, 336 + 64, true);
    draw_number(high_year, 241 + 128, 336 + 64, true);
  }

  double_buf();
}

void draw_explosion(Entity *ent) {

  // Center
  vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ent->animation_state.x, ent->animation_state.y, TILESIZE);
  ent->animation_state.y++;

  bool hit_l = false, hit_r = false, hit_u = false, hit_d = false;
  // Extensions
  for (size_t i = 1; i < ent->max_range; i++) {

    // Horizontal

    // Left
    if (!hit_l) {
      if (!tilemap.collisions[ent->tile_pos.y][ent->tile_pos.x - i].active) {
        vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X - TILESIZE * i, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ent->animation_state.x, ent->animation_state.y, TILESIZE);
      }
      else
        hit_l = true;
    }

    // Right
    if (!hit_r) {
      if (!tilemap.collisions[ent->tile_pos.y][ent->tile_pos.x + i].active) {
        vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X + TILESIZE * i, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ent->animation_state.x, ent->animation_state.y, TILESIZE);
      }
      else
        hit_r = true;
    }

    ent->animation_state.y++;

    // Vertical

    // Up
    if (!hit_u) {
      if (!tilemap.collisions[ent->tile_pos.y - i][ent->tile_pos.x].active) {
        vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y - TILESIZE * i, ent->animation_state.x, ent->animation_state.y, TILESIZE);
      }
      else
        hit_u = true;
    }

    // Down
    if (!hit_d) {
      if (!tilemap.collisions[ent->tile_pos.y + i][ent->tile_pos.x].active) {
        vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y + TILESIZE * i, ent->animation_state.x, ent->animation_state.y, TILESIZE);
      }
      else
        hit_d = true;
    }

    ent->animation_state.y--;
  }
  ent->animation_state.y++;
  ent->animation_state.y++;

  //Left arm
  if (!hit_l) {
    if (!tilemap.collisions[ent->tile_pos.y][ent->tile_pos.x - ent->max_range].active) {
      vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X - TILESIZE * ent->max_range, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ent->animation_state.x, ent->animation_state.y, TILESIZE);
    }
  }
  ent->animation_state.y++;

  // Right arm
  if (!hit_r) {
    if (!tilemap.collisions[ent->tile_pos.y][ent->tile_pos.x + ent->max_range].active) {
      vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X + TILESIZE * ent->max_range, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ent->animation_state.x, ent->animation_state.y, TILESIZE);
    }
  }
  ent->animation_state.y++;

  // Upper arm
  if (!hit_u) {
    if (!tilemap.collisions[ent->tile_pos.y - ent->max_range][ent->tile_pos.x].active) {
      vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y - TILESIZE * ent->max_range, ent->animation_state.x, ent->animation_state.y, TILESIZE);
    }
  }
  ent->animation_state.y++;

  // Lower arm
  if (!hit_d) {
    if (!tilemap.collisions[ent->tile_pos.y + ent->max_range][ent->tile_pos.x].active) {
      vg_draw_tile(ent->sprite, *ent->img, ent->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, ent->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y + TILESIZE * ent->max_range, ent->animation_state.x, ent->animation_state.y, TILESIZE);
    }
  }

  ent->animation_state.y = 0;
}

void render() {
  vg_clear_screen();

  // draw background and powerups
  draw_map(MAP_RENDER_OFFSET_X, MAP_RENDER_OFFSET_Y);

  draw_ui();

  // Draw map hitboxes
  // for (int j = 0; j < MAP_TILE_HEIGHT; j++) {
  //   for (int k = 0; k < MAP_TILE_WIDTH; k++) {
  //     BoundingBox current = tilemap.collisions[j][k];
  //     uint32_t color;
  //     if (current.permanent)
  //       color = 0xFF0000;
  //     else
  //       color = 0x00FF00;
  //     if (current.active)
  //       vg_draw_square_outline(current.upper_left.x, current.upper_left.y, TILESIZE, color);
  //   }
  // }

  // Draw bman hitbox
  //vg_draw_square_outline(all_entities[0]->entity_pos.x - ENTITY_BB_SIZE / 2, all_entities[0]->entity_pos.y - ENTITY_BB_SIZE / 2, ENTITY_BB_SIZE, 0xFFFFFF);

  // draw entities
  for (int i = ARRAY_SIZE - 1; i >= 0; --i) {
    Entity *this = all_entities[i];

    if (this->type == NO_TYPE)
      continue;

    else if (this->type == EXPLOSION)
      draw_explosion(all_entities[i]);

    else if (this->type == ENEMY)
      vg_draw_tile(this->sprite, *this->img, this->entity_pos.x - TILESIZE / 2, this->entity_pos.y - TILESIZE / 2, this->animation_state.x, this->animation_state.y, TILESIZE);

    else if (this->type == PLAYER)
      vg_draw_tile(this->sprite, *this->img, this->entity_pos.x - TILESIZE / 2, this->entity_pos.y - TILESIZE / 2, this->animation_state.x, this->animation_state.y, TILESIZE);

    else if (this->type == BOMB)
      vg_draw_tile(this->sprite, *this->img, this->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, this->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, this->animation_state.x, this->animation_state.y, TILESIZE);

    else if (this->type == BROKEN_WALL)
      vg_draw_tile(this->sprite, *this->img, this->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, this->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, this->animation_state.x, this->animation_state.y, TILESIZE);

    //vg_draw_square_outline(this->entity_pos.x - ENTITY_BB_SIZE / 2, this->entity_pos.y - ENTITY_BB_SIZE / 2, ENTITY_BB_SIZE, 0xFFFFFF);

    //vg_draw_square_outline(this->tile_pos.x * TILESIZE + MAP_RENDER_OFFSET_X, this->tile_pos.y * TILESIZE + MAP_RENDER_OFFSET_Y, ENTITY_BB_SIZE, 0xFFFFFF);
  }

  double_buf();
}

void exit_game(bool won){
  update_highscore();

  free_entities();

  // discarding garbage
  read_output_buf();
  send_cmd_to_mouse(DISABLE_REP);

  keyboard_unsubscribe_int();
  timer_unsubscribe_int();
  mouse_unsubscribe_int();
  rtc_unsubscribe();

  vg_exit();

  free_buf();
  read_output_buf();
  if (won) printf("YOU WON\n");
}
