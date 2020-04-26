#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"

/** @defgroup mouse mouse
 * @{
 *
 * Mouse related functions.
 */

#define MOUSE_TOLERANCE 3 /**< Mouse movement tolerance */

/**
 * @brief Mouse movement and clicks
 * 
 */
enum MS_KEY {
  NO_CLICK = 1, /**< enum No Click Detected */

  UP = 3,       /**< enum Upwards Movement */

  DOWN = 4,     /**< enum Downwards Movement */

  LEFT = 5,     /**< enum Left Movement */

  RIGHT = 6,    /**< enum Right Movement */

  LEFT_CLICK = 7, /**< enum Left Click Detected */

  RIGHT_CLICK = 8 /**< enum Right Click Movement */
};

/**
 * @brief Returns appropriate enum value from give packet
 * 
 */
enum MS_KEY (check_click)(void);

/**
 * @brief Handling of mouse interrupts
 * 
 */
void (mouse_ih)(void);

/**
 * @brief Processes a mouse packet
 * 
 */
void (parse_packet)(void);

/**
 * @brief Subscribe Mouse Interrupts
 * 
 */
int (mouse_subscribe_int)(uint8_t *bit_n);

/**
 * @brief Unsubscribe Mouse Interrupts
 * 
 */
int (mouse_unsubscribe_int)(void);

/**
 * @brief (Tries to) Read the output buffer
 * 
 * @return int16_t 
 */
int16_t read_output_buf();

/**
 * @brief Reads data from command byte
 * 
 * @return int Returns the Data that was read or -1 in case of error 
 */
int read_cmd_byte();

/**
 * @brief Writes command byte
 * 
 * @param cmd 
 * @return int 
 */
int write_cmd_byte(uint32_t cmd);

/**
 * @brief (Tries to) Write the received parameter 
 * 
 * @param arg 
 * @return int 
 */
int write_kbc_arg(uint32_t arg);

/**
 * @brief Sends the command in the received parameter to be written to the mouse
 * 
 * @param cmd 
 * @return int 
 */
int send_cmd_to_mouse(uint32_t cmd);

/**
 * @brief (Tries to) Write the received parameter (to the mouse)
 * 
 * @param cmd 
 * @param port 
 * @return int 
 */
int write_to_mouse(uint32_t cmd, uint32_t port);

/**
 * @brief Disables Data Reporting, sets stream mode, writes the default command byte
 * 
 * @return int 
 */
int reset_mouse();

/**
 * @brief Convert packet to action
 * 
 * @param p 
 * @return struct mouse_ev* 
 */
struct mouse_ev* mouse_get_event(struct packet *p);

/**
 * @brief Detection of the mouse gesture
 * 
 * @param ev 
 * @param x_len 
 * @param tolerance 
 */
void mouse_detect_gesture(struct mouse_ev * ev, uint8_t x_len, uint8_t tolerance);

/**
 * @brief logical "and" parts
 * 
 */
enum logical_and{
  init, /**< enum Start of the gesture */
  first_leg, /**< enum First leg of the logical and */
  in_between, /**< enum Point between the first and second leg */
  second_leg /**< enum Second leg of the logical and */
};


#endif /* _MOUSE_H_ */
