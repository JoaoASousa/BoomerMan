#include <stdint.h>

/** @defgroup graphics graphics
 * @{
 *
 * Graphics related functions.
 */

#define TILESIZE 32 /**< @brief Size of a map tile */
#define MAP_RENDER_OFFSET_X 32 /**< @brief X axis pixel offset on where the map and entities are drawn on the screen */
#define MAP_RENDER_OFFSET_Y 48 /**< @brief Y axis pixel offset on where the map and entities are drawn on the screen */

/**
 * @brief Software implementation of double buffering (copying "back buffer", video_buff, to video ram)
 * 
 */
void double_buf(); 

/**
 * @brief Frees video_buff ("back buffer")
 * 
 */
void free_buf();

/**
 * @brief Draws a tile of a spritesheet of coords (tile_x, tile_y) on screen coords (x,y)
 * 
 * @param tileset_sprite 
 * @param tileset_img 
 * @param x 
 * @param y 
 * @param tile_x 
 * @param tile_y 
 * @param tilesize 
 */
void vg_draw_tile(uint8_t* tileset_sprite, xpm_image_t tileset_img, uint16_t x, uint16_t y, uint16_t tile_x, uint16_t tile_y, uint tilesize);

/**
 * @brief Initializes video mode
 * 
 * @param mode 
 */
void *(graphics_init)(uint16_t mode);

/**
 * @brief Draws hollow square outline on screen
 * 
 * @param x 
 * @param y 
 * @param size 
 * @param color 
 * @return int 
 */
int vg_draw_square_outline(uint16_t x, uint16_t y, uint16_t size, uint32_t color);

/**
 * @brief Get the indexed color
 * 
 * @param row 
 * @param col 
 * @param first 
 * @param step 
 * @param no_rectangles 
 * @return uint32_t 
 */
uint32_t get_indexed_color(uint16_t row, uint16_t col, uint32_t first, uint8_t step, uint8_t no_rectangles);

/**
 * @brief Returns direct color calculated from the formula
 * 
 * @param row 
 * @param col 
 * @param first 
 * @param step 
 * @return uint32_t 
 */
uint32_t get_direct_color(uint16_t row, uint16_t col, uint32_t first, uint8_t step);

/**
 * @brief Get the Color Mask for an rgb channel
 * 
 * @param mask_size 
 * @param mask_position 
 * @return uint32_t 
 */
uint32_t getColorMask(uint32_t mask_size, uint32_t mask_position);

/**
 * @brief Draws on screen a pattern of color alternating rectagnles
 * 
 * @param no_rectangles 
 * @param first 
 * @param step 
 * @return int 
 */
int vg_draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step);

/**
 * @brief Draws on screen a single pixel
 * 
 * @param x 
 * @param y 
 * @param color 
 * @return int 
 */
int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Draws on screen an xpm
 * 
 * @param sprite 
 * @param img 
 * @param x 
 * @param y 
 */
void vg_draw_xpm(uint8_t* sprite, xpm_image_t* img, uint16_t x, uint16_t y);

/**
 * @brief Checks if final positions are valid
 * 
 * @param xf 
 * @param yf 
 * @param xi 
 * @param yi 
 * @return int 
 */
int check_final_pos(uint16_t xf, uint16_t yf, uint16_t xi, uint16_t yi);

/**
 * @brief Resets video memory, clearing the screen
 * 
 */
void vg_clear_screen();

/**
 * @brief Gets the info of the initialized vbe mode
 * 
 * @param mode 
 * @param vmi_p 
 * @return int 
 */
int our_vbe_get_mode_info(uint16_t mode, vbe_mode_info_t *vmi_p);



