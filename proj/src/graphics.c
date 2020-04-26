#include <lcom/lcf.h>

#include "graph_macros.h"
#include "graphics.h"

static char *video_mem; /* Process (virtual) address to which VRAM is mapped */
static char *video_buff;

static unsigned h_res;          /* Horizontal resolution in pixels */
static unsigned v_res;          /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

static vbe_mode_info_t vbe_info;

void double_buf(){
  if (bits_per_pixel == 15) {
    memcpy(video_mem, video_buff, h_res * v_res * 2 );
  }
  else {
    memcpy(video_mem, video_buff, h_res * v_res * (bits_per_pixel / 8) );
  }
}

void free_buf() {
  free(video_buff);
}

// only works for RGB888 mode
void vg_draw_tile(uint8_t* tileset_sprite, xpm_image_t tileset_img, uint16_t x, uint16_t y, uint16_t tile_x, uint16_t tile_y, uint tilesize) {

  // xpm_image_t img;
  // uint8_t* tileset = xpm_load(xpm, XPM_8_8_8, &img);

  int current_pixel = tile_x * tilesize * 3 + tile_y * tileset_img.width * tilesize * 3;
  int pixel_offset = tile_x * tilesize * 3 + tile_y * tileset_img.width * tilesize * 3;
  for (uint i = 0; i < tilesize; i++) {
    for (uint j = 0; j < tilesize; j++) {

      uint32_t r = tileset_sprite[current_pixel + 2];
      uint32_t g = tileset_sprite[current_pixel + 1];
      uint32_t b = tileset_sprite[current_pixel];

      uint32_t color = ((r << 16) & 0xFF0000) | ((g << 8) & 0xFF00) | (b & 0xFF);

      if(xpm_transparency_color(XPM_8_8_8) != color) {
        vg_draw_pixel(x + j, y + i, color);
      }

      current_pixel += 3;
    }

    current_pixel = i * tileset_img.width * 3 + pixel_offset;
  }
}


void *(graphics_init)(uint16_t mode) {

  vbe_get_mode_info(mode, &vbe_info);
  // our_vbe_get_mode_info(mode, &vbe_info);

  // Initializing h_res, v_res and bits_per_pixel with mode info
  h_res = vbe_info.XResolution;
  v_res = vbe_info.YResolution;
  bits_per_pixel = vbe_info.BitsPerPixel;

  struct minix_mem_range mr;

  uint32_t vram_size;

  // If 15 bits per pixel, allocate as if it had 16 bits since the MSB is unused
  // but present
  if (bits_per_pixel == 15) {
    vram_size = (bits_per_pixel * h_res * v_res);
  } else vram_size = h_res * v_res * (bits_per_pixel / 8);

  int r;

  /* Allow memory mapping */
  mr.mr_base = vbe_info.PhysBasePtr;
  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  /* Map memory */
  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
  
  if (video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  // malloc of video_buff
  video_buff = (char *) malloc(vram_size);

  struct reg86
      regStruct; // only works with reg86 instead of the reg86_t in the slides

  memset(&regStruct, 0, sizeof(regStruct));

  regStruct.ax = VBE_FUNC_02;
  regStruct.bx = (1 << 14) | mode;
  regStruct.intno = VIDEO_CARD_BIOS;

  if (sys_int86(&regStruct) != OK) {
    return NULL;
  }

  // If the VBE function called is supported, the AL register is set with value
  // 0x4F
  if (regStruct.al != VBE_FUNC_SUP) {
    return NULL;
  }

  // If the VBE function completed successfully, value 0x00 is returned in the
  // AH
  if (regStruct.ah != SUCCESS_CALL) {
    return NULL;
  }

  return video_mem;
}
/////////////////
/////////////////
/////////////////
int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {

  if (x > h_res || y > v_res || x + len > h_res) {
    return 1;
  }

  if (bits_per_pixel == 8) {
    memset(video_buff + h_res * y + x, color, len);
  } else if (bits_per_pixel == 15) {
    for (size_t i = 0; i < len; i++) {
      memcpy(video_buff + h_res * y * 2 + (x + i) * 2, &color, 2);
    }
  } else {
    for (size_t i = 0; i < len; i++) {
      memcpy(video_buff + h_res * y * (bits_per_pixel / 8) +
                 (x + i) * (bits_per_pixel / 8),
             &color, (bits_per_pixel / 8));
    }
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       uint32_t color) {

  if (bits_per_pixel == 8 && color > 255) {
    return 1;
  }

  for (size_t i = 0; i < height; i++) {
    vg_draw_hline(x, y + i, width, color);
  }

  return 0;
}

int vg_draw_square_outline(uint16_t x, uint16_t y, uint16_t size, uint32_t color) {
  vg_draw_hline(x, y, size, color);
  for (int i = 1 ; i < size - 1 ; i++){
    vg_draw_pixel(x, y + i, color);
    vg_draw_pixel(x + size - 1, y + i, color);
  }
  vg_draw_hline(x, y + size - 1, size, color);
  return 0;
}

uint32_t get_indexed_color(uint16_t row, uint16_t col, uint32_t first,
                           uint8_t step, uint8_t no_rectangles) {
  return (first + (row * no_rectangles + col) * step) % (1 << bits_per_pixel);
}

uint32_t getColorMask(uint32_t mask_size, uint32_t mask_position) {

  uint32_t mask = 0;
  for (size_t i = 0; i < mask_size; i++) {
    mask |= (1 << mask_position) << i;
  }

  return mask;
}

uint32_t get_direct_color(uint16_t row, uint16_t col, uint32_t first,
                          uint8_t step) {

  // Compute mask based on channel size and position
  uint32_t r_mask =
      getColorMask(vbe_info.RedMaskSize, vbe_info.RedFieldPosition);
  uint32_t g_mask =
      getColorMask(vbe_info.GreenMaskSize, vbe_info.GreenFieldPosition);
  uint32_t b_mask =
      getColorMask(vbe_info.BlueMaskSize, vbe_info.BlueFieldPosition);

  // Get the first color's separate channels shifted right so they start at the
  // LSB
  uint32_t r_first = (first & r_mask) >> vbe_info.RedFieldPosition;
  uint32_t g_first = (first & g_mask) >> vbe_info.GreenFieldPosition;
  uint32_t b_first = (first & b_mask) >> vbe_info.BlueFieldPosition;

  // Apply formulas
  uint32_t r = (r_first + col * step) % (1 << vbe_info.RedMaskSize);
  uint32_t g = (g_first + row * step) % (1 << vbe_info.GreenMaskSize);
  uint32_t b = (b_first + (col + row) * step) % (1 << vbe_info.BlueMaskSize);

  // Reshift individual channels
  r = r << vbe_info.RedFieldPosition;
  g = g << vbe_info.GreenFieldPosition;
  b = b << vbe_info.BlueFieldPosition;

  // Binary OR the channels after AND operation to make sure no garbage is
  // present
  uint32_t result = ((r & r_mask) | (g & g_mask) | (b & b_mask));
  return result;
}

int vg_draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint32_t width = h_res / no_rectangles;
  uint32_t height = v_res / no_rectangles;

  uint32_t color;
  uint16_t current_x, current_y;

  for (int i = 0; i < no_rectangles; i++) {
    current_y = i * height;
    for (int j = 0; j < no_rectangles; j++) {
      current_x = j * width;

      if (bits_per_pixel == 8) {
        color = get_indexed_color(i, j, first, step, no_rectangles);
      } else {
        color = get_direct_color(i, j, first, step);
      }


      vg_draw_rectangle(current_x, current_y, width, height, color);
    }
  }

  return 0;
}

int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x > h_res || y > v_res) {
    return 1;
  }

  if (bits_per_pixel == 8) {
    memset(video_buff + h_res * y + x, color, 1);
  }

  else if (bits_per_pixel == 15) {
    memcpy(video_buff + h_res * y * 2 + x * 2, &color, 2);
  }

  else {
    memcpy(video_buff + h_res * y * (bits_per_pixel / 8) +
               x * (bits_per_pixel / 8),
           &color, (bits_per_pixel / 8));
  }

  return 0;
}

// only draws 24bit xpms
//void vg_draw_xpm(xpm_map_t xpm, uint16_t x, uint16_t y) {
void vg_draw_xpm(uint8_t* sprite, xpm_image_t* img, uint16_t x, uint16_t y) {
  // xpm_image_t img;
  //  uint8_t* sprite = xpm_load(xpm, XPM_8_8_8, &img);

  //if (x==0 && y == 0) return; // Entity is not placed ; remove later

  int current_pixel = 0;
  for (int i = 0; i < img->height; i++) {
    for (int j = 0; j < img->width; j++) {

      uint32_t r = sprite[current_pixel + 2];
      uint32_t g = sprite[current_pixel + 1];
      uint32_t b = sprite[current_pixel];

      uint32_t color = ((r << 16) & 0xFF0000) | ((g << 8) & 0xFF00) | (b & 0xFF);
      
      if(xpm_transparency_color(XPM_8_8_8) != color) {
        vg_draw_pixel(x + j, y + i, color);
      }

      current_pixel += 3;
    }
  }
}

int check_final_pos(uint16_t xf, uint16_t yf, uint16_t xi, uint16_t yi) {

  // Check if final positions are valid
  if (xf > h_res || yf > v_res || xf < 0 || yf < 0 || (xf != xi && yf != yi)) {
    return 1;
  }
  return 0;
}

// clear screen
void vg_clear_screen() {

  if (bits_per_pixel == 15) {
    memset(video_buff, 0, h_res * v_res * 2);
  } else {
    memset(video_buff, 0, h_res * v_res * (bits_per_pixel / 8));
  }
}

int our_vbe_get_mode_info(uint16_t mode, vbe_mode_info_t *vmi_p) {

  mmap_t map_t;
  struct reg86 regStruct;

  memset(&regStruct, 0, sizeof(regStruct));

  regStruct.intno = VIDEO_CARD_BIOS;

  // Return VBE mode information
  regStruct.ax = VBE_FUNC_01;

  // Mode number
  regStruct.cx = mode;

  // Pointer to ModeInfoBlock structure
  regStruct.es = PB2BASE(map_t.phys);
  regStruct.di = PB2OFF(map_t.phys);

  if (lm_alloc(sizeof(*vmi_p), &map_t) == NULL) {
    return 1;
  }

  if (sys_int86(&regStruct) != OK) {
    return 1;
  }

  memcpy(vmi_p, map_t.virt, sizeof(vbe_mode_info_t));

  lm_free(&map_t);

  return 0;
}
