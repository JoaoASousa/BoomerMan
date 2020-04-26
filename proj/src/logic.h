#ifndef _LCOM_LOGIC_H_
#define _LCOM_LOGIC_H_

#include "keyboard.h" // For KB_KEY enum
#include "mouse.h"    // For MS_KEY enum

/** @defgroup logic logic
 * @{
 *
 * Game logic functions.
 */

#define ARRAY_SIZE 32 /**< @brief Size of all_entities array */

#define ENTITY_BB_SIZE 24 /**< @brief Size of entity bounding box */

#define ENEMY_XPM_STARTING 4 /**< @brief Starting index of enemies xpms on xpms array */

#define LAST_LEVEL 3 /**< @brief Last level available */

#define INVINCIBILITY_COOLDOWN 300
#define ANIM_COOLDOWN 8 /**< @brief Default animation cooldown */
#define PLAYER_ANIM_COOLDOWN 4 /**< @brief Player animation cooldown */
#define ENEM_COOLDOWN 12 /**< @brief Enemy animation cooldown */
#define BOMB_COOLDOWN 15 /**< @brief Bomb animation cooldown */
#define EXPLOSION_COOLDOWN 8 /**< @brief Explosion animation cooldown */
#define DEAD_FACE_COOLDOWN 92 /**< @brief First step of death animations' cooldown */
#define DEAD_STEP_COOLDOWN 15 /**< @brief Remaining steps of death animations' cooldown */
#define WALL_BREAK_COOLDOWN 10 /**< @brief Wall break animation cooldown */
#define SCORE_COOLDOWN 50 /**< @brief Score animation cooldown */
#define BOMB_TICK 10;/**< @brief Duration of bomb fuse */

#define SPEED_INCREMENT 1 /**< @brief Speed increment of speed powerup */

/**
 * @brief Type of entity
 * 
 */
typedef enum entity_type {
  NO_TYPE,
  PLAYER,
  ENEMY,
  BOMB,
  EXPLOSION,
  BROKEN_WALL
} EntityType;

/**
 * @brief Vector with x and y coordinates
 * 
 */
typedef struct vector2d {
  int16_t x;
  int16_t y;
} Vector2D;

/**
 * @brief Indicates direction the entity is facing/moving ; also used to indicate death state and score display state
 * 
 */
typedef enum current_facing_direction{
  NORTH,
  SOUTH,
  WEST,
  EAST,
  NO_DIR,
  DEAD,
  SCORE
} Facing;

/**
 * @brief Entity attributes
 * 
 */
typedef struct entity {
  EntityType type; /**< Type of entity*/
  bool controls_disabled; /**< If controls are disabled*/
  bool dead; /**< If entity is dead*/
  uint max_bombs; /**< Max number of placeable bombs */
  uint max_range; /**< Bombs max range */
  bool wallpass; /**< If entity can walk through breakable walls */
  bool bombpass; /**< If entity can walk through placed bombs */
  bool firepass; /**< If entity can walk through explosions */
  bool remote_control; /**< If entity can remote detonate bombs */
  uint invincibility_cooldown; /**< Time until invincibility runs out */

  uint8_t* sprite; /**< entity sprite */
  xpm_image_t* img; /**< entity sprites' image stats */
  
  Facing current_facing; /**< current facing direction */
  Vector2D animation_state; /**< current animation state */
  uint animation_cooldown; /**< current animation cooldown */

  Vector2D entity_pos; /**< current screen coordinates */
  Vector2D entity_vel; /**< current velocity vector */
  Vector2D tile_pos; /**< current tile coordinates */
  int linear_speed; /**< entities' speed attribute */

  uint bomb_tick; /**< bombs current fuse time */
  bool box_active; /**< if bombs' bounding box is active */

  bool explosion_active; /**< if explosions' collision with walls is active (in order to only do map collision checks once, improves performance) */
  int hit_u; /**< distance at which bombs upper arm hit an obstacle, 0 if didn't encounter any */
  int hit_d; /**< distance at which bombs lower arm hit an obstacle, 0 if didn't encounter any */
  int hit_l; /**< distance at which bombs left arm hit an obstacle, 0 if didn't encounter any */
  int hit_r; /**< distance at which bombs right arm hit an obstacle, 0 if didn't encounter any */

  uint score; /**< score value of enemy */

} Entity;

#define MAP_TILE_WIDTH 23 /**< @brief Tile width of map */
#define MAP_TILE_HEIGHT 17 /**< @brief Tile height of map */

#define MAP_TILE_INFO_START_ROW 3 /**< @brief Csv's tile info starting line */
#define MAP_ENT_INFO_START_ROW 21 /**< @brief Csv's entity info starting line */
#define MAP_ENEMY_INFO_START_ROW 39 /**< @brief Csv's enemy info starting line */

#define MAP_TILESET_WIDTH 2 /**< @brief Width of map tileset xpm */
#define MAP_TILESET_HEIGHT 2 /**< @brief Height of map tileset xpm */

/**
 * @brief Type of map entiity
 * 
 */
typedef enum map_entity {
  BOMB_UP,
  FIRE_UP,
  SPEED_UP,
  WALL_PASS,
  REMOTE_CONTROL,
  BOMB_PASS,
  FIRE_PASS,
  QUESTION_MARK,
  EXIT,
  NO_ENT
} MapEntity;

/**
 * @brief Bounding box struct
 * 
 */
typedef struct bbox {
  Vector2D upper_left; /**< upper left corner coordinates */
  Vector2D lower_right; /**< lower right corner coordinates */
  bool active; /**< If bounding box is currently active */
  bool permanent; /**< If bounding box is permanent (used to diferentiate between breakable and unbreakable walls' boxes) */
} BoundingBox;

/**
 * @brief Tilemap struct
 * 
 */
typedef struct tilemap {
  uint8_t tiles[MAP_TILE_HEIGHT][MAP_TILE_WIDTH]; /**< Holds the type of each tile */
  BoundingBox collisions[MAP_TILE_HEIGHT][MAP_TILE_WIDTH]; /**< Holds the bounding boxes of each tile */
  MapEntity map_entities[MAP_TILE_HEIGHT][MAP_TILE_WIDTH]; /**< Holds the map entities on each tile */
} Tilemap;

/**
 * @brief Reads the highscore.txt file and saves its info
 * 
 */
int read_highscore_file();

/**
 * @brief Saves current highscore and corresponding date on a new highscore.txt file
 * 
 */
int update_highscore();

/**
 * @brief Inserts an enemy of type identifier on tile coordinates tile_pos on the map
 * 
 */
void insertEnemy(Vector2D tile_pos, uint64_t identifier);

/**
 * @brief Reads the currently selected map file, storing the tile, collision, enemy and powerup information in their corresponding places
 * 
 */
int read_map(void);

/**
 * @brief Sets the current maps tileset_sprite after reading the tileset_xpm
 * 
 */
void load_map(xpm_map_t tileset_xpm);

/**
 * @brief Loads all the games' xpms and stores them in the static array xpms[] ; also makes sure to save each xpms info on a xpm_image_t variable
 * 
 */
void load_xpms();

/**
 * @brief Frees memory allocated for the entity array
 * 
 */
void free_entities();

/**
 * @brief Allocates memory for the entity erray, overwritting previous space. If it is a new game, player entity is also overwritten
 * 
 */
void load_entities(Entity * oldPlayer);

/**
 * @brief Loads player entity, resetting powerups if its the start of a new game ; Otherwise, resets position, animation state, graphic details and reenables controls
 * 
 */
void load_player(bool new_game);

/**
 * @brief Draws the game over screen
 * 
 */
void draw_game_over_screen();

/**
 * @brief Draws the current level screen
 * 
 */
void draw_level_screen();

/**
 * @brief Reloads the map and resets some stat variables
 * 
 */
void reset_stage();

/**
 * @brief Separates a multiple digit number and draws it on the screen ; If specified, the number drawn will have an extra leftmost 0 (if n is single digit)
 * 
 */
void draw_number(int n, int16_t x, int16_t y, bool must_be_two_digits);

/**
 * @brief Draws the time remaining, current score and lives left on the top of the screen
 * 
 */
void draw_ui();

/**
 * @brief Draws all of the map tiles with an offset given
 * 
 */
void draw_map(uint16_t offset_x, uint16_t offset_y);

/**
 * @brief Compares 2 Vector2D objects
 * 
 */
bool Vector2DEqual(Vector2D l, Vector2D r);

/**
 * @brief Returns if boxes intersect
 * 
 */ 
bool boxesIntersect(Vector2D a, Vector2D b, int aSize, int bSize);

/**
 * @brief Similar to boxesInterecept, used when there aren't xy coordinates and only tile pos coordinates
 * 
 */
bool tilesIntersect(Vector2D a, Vector2D b, int aSize, int bSize);

/**
 * @brief Kills certain entity by explosion (updating its stats)
 * 
 */
void exploded(Entity * this);

/**
 * @brief Trys to place broken wall objects on said coordinates
 * 
 */
void placeBrokenWalls(Vector2D walls_broken[4]);

/**
 * @brief Check if player collided with a pickup and update its stats
 * 
 */
void powerupCollisions(Entity * player);

/**
 * @brief Checks collision on all static walls of the map for a given entity
 * 
 */
bool mapCollisions(Vector2D entity_coords, bool wallpass);

/**
 * @brief Checks if an entity is colliding with any arm of an explosion
 * 
 */
bool explosionCollision(Vector2D this_tile, Entity * explosion);

/**
 * @brief Checks collision of an explosion with all map tiles
 * 
 */
void explosionWithMapCollision(Entity * bomb);

/**
 * @brief Checks collision on all entities, dealing with specific cases;
 * 
 */
bool entityCollisions(Entity * this);

/**
 * @brief Places bomb object on said coordinates, giving it it's adequate atributes
 * 
 */
void placeBomb(Vector2D tile_coords);

/**
 * @brief Places explosion object on said coordinates
 *
 */
void placeExplosion(Entity * this);

/**
 *  @brief Update animations of entity based on it's current stats
 * 
 */
void handle_animations();

/**
 * @brief Get the input from the keyboard / mouse
 * 
 * @return Array with the input of the player: if index 0 != 0 the keyboard was used, and/or if index 1 != 0 the mouse was used.
 */
int get_input();

/**
 * @brief Get the input from the keyboard / mouse
 * 
 * @param msg
 * @return Array with the input of the player: if index 0 != 0 the keyboard was used, and/or if index 1 != 0 the mouse was used.
 * 
 */
int get_input(message msg);

/**
 * @brief Gets the input from the mouse and updates the player velocity.
*/
void handle_input_ms(enum MS_KEY input);

/**
 * @brief Gets the input from the keyboard and updates the player velocity.
 * 
 */
void handle_input_kb(enum KB_KEY input);

/**
 * @brief Takes the velocity (x and y) of each entity and updates their positions. TODO: check for colisions and boundaries before "accepting" the move.
 * 
 */
void handle_physics();

/**
 * @brief Draws menu
 * 
 */
void draw_menu(int selected_menu);

/**
 * @brief Draws explosion entity on the correct tiles
 * 
 * @param ent Bomb entity
 * 
 */
void draw_explosion(Entity *ent);

/**
 * @brief Renders the background (map) and the entities
 * 
 */
void render();

/**
 * @brief Exits the game, unsubscribing all the devices and leaving graphics mode
 * 
 * @param won 
 */
void exit_game(bool won);

#endif
