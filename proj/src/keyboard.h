#ifndef _LCOM_KEYBOARD_H_
#define _LCOM_KEYBOARD_H_

#include <stdbool.h>
#include <stdint.h>

/** @defgroup keyboard keyboard
 * @{
 *
 * Keyboard related functions.
 */

#define NUM_TRIES 2 /**< @brief Number of tries */

/**
 * @brief Keyboard key make and break codes
 * 
 */
enum KB_KEY {
  TWO_BYTE = 0,     /**< enum Two bytes */
  ESC_BREAK = 2,    /**< enum Esc breakcode */
  NO_KEY = 1,       /**< enum No key pressed or invalid key*/
  W_MAKE = 3,       /**< enum W makecode */
  S_MAKE = 4,       /**< enum S makecode*/
  A_MAKE = 5,       /**< enum A makecode*/
  D_MAKE = 6,       /**< enum D makecode*/
  W_BREAK = 7,      /**< enum W breakcode*/
  S_BREAK = 8,      /**< enum S breakcode*/
  A_BREAK = 9,      /**< enum A breakcode*/
  D_BREAK = 10,     /**< enum D breakcode*/
  SPACE_BREAK = 11, /**< enum Space breakcode*/
  ENTER_BREAK = 12  /**< enum Enter breakcode*/
};

/**
 * @brief Increments sys_inb
 * 
 */
void sys_inb_increment();

/**
 * @brief Reads status and output buffers
 * 
 * @param status 
 * @return int 
 */
int read_status_and_output_buffer_kbc(uint8_t *status);

/**
 * @brief Gets pressed key scancodes and returns appropriate enum value
 * 
 */
enum KB_KEY (get_key_pressed)(void);

/**
 * @brief Returns appropriate enum value from given scancode
 * 
 */
enum KB_KEY (check_key)(uint8_t scancode, bool is_two_b);

/**
 * @brief Keyboard interrupt handler
 * 
 */
void (kbc_ih)(void);

/**
 * @brief Reenables keyboard
 * 
 */
int (reenable_keyboard_int());

/**
 * @brief Subscribes keyboard interrupts
 * 
 */
int (keyboard_subscribe_int) (uint8_t *bit_n);

/**
 * @brief Unsubscribes keyboard interrupts
 * 
 */
int (keyboard_unsubscribe_int) ();

/**
 * @brief Discards makecode if there were errors
 * 
 * @return uint8_t 
 */
uint8_t discard_make_codes(void);

#endif
