#ifndef _LCOM_I8024_H_
#define _LCOM_I8024_H_

#include <lcom/lcf.h>

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the keyboard and mouse.
 */

#define DELAY_US        20000 /**< @brief Time delay */

#define W_MAKECODE      0x11 /**< @brief W key makecode */
#define S_MAKECODE      0x1F /**< @brief S key makecode */
#define A_MAKECODE      0x1E /**< @brief A key makecode */
#define D_MAKECODE      0x20 /**< @brief D key makecode */
#define W_BREAKCODE     0x91 /**< @brief W key breakcode */
#define A_BREAKCODE     0x9E /**< @brief A key breakcode */
#define S_BREAKCODE     0x9F /**< @brief S key breakcode */
#define D_BREAKCODE     0xA0 /**< @brief D key breakcode */
#define SPACE_BREAKCODE 0xb9 /**< @brief Space key breakcode */
#define ENTER_BREAKCODE 0x9c /**< @brief Enter key breakcode */
#define ESC_BREAKCODE   0x81 /**< @brief Esc key breakcode */


#define FIRST_OF_TWO    0xE0 /**< @brief The first byte of two byte scancodes */ 

#define STAT_REG        0x64 /**< @brief Status register */
#define KBC_CMD_REG     0x64 /**< @brief Command register */


#define INPUT_B         0x64 /**< @brief Input buffer */
#define OUTPUT_B        0x60 /**< @brief Output buffer */


// Keyboard-Related KBC Commands (for PC-AT/PS2)
#define READ_CMD_BYTE   0x20 /**< @brief Returns command byte */
#define WRITE_CMD_BYTE  0x60 /**< @brief Takes A: Command byte */
#define CHECK_KBC       0xAA /**< @brief Returns 0x55, if OK, Returns 0xFC, if error */
#define CHECK_KBD_INT   0xAB /**< @brief Returns 0, if OK */
#define DISABLE_KBD_INT 0xAD /**< @brief */
#define ENABLE_KBD_INT  0xAE /**< @brief */

// commands forwarded to the mouse
#define DISABLE_MOUSE     0xA7        /**< @brief Disable Mouse command */
#define ENABLE_MOUSE      0xA8        /**< @brief Enable Mouse command */
#define CHECK_M_INTERFACE 0xA9        /**< @brief Check Mouse Interface command */
#define WRITE_BYTE_MOUSE  0xD4        /**< @brief Write Byte to Mouse Byte (Argument) */

// Status Register
#define OBF BIT(0)      /**< @brief Output buffer full */
#define IBF BIT(1)      /**< @brief Input buffer full */
#define SYS BIT(2)      /**< @brief System flag */
#define A2 BIT(3)       /**< @brief A2 input line */
#define INH BIT(4)      /**< @brief Inhibit flag */
#define AUX BIT(5)      /**< @brief Mouse data */
#define TIMEOUT BIT(6)  /**< @brief Timeout error */
#define PAR BIT(7)      /**< @brief Parity error */

// KBC "Command Byte"
#define ENABLE_KBD_OBF_INT BIT(0)

#define MOUSE_IRQ 12

//commands passed as arguments of 0xD4
#define RESET             0xFF       /**< @brief Mouse reset */
#define RESEND            0xFE       /**< @brief For serial comunications errors */
#define SET_DEF           0xF6       /**< @brief Set default values */
#define DISABLE_REP       0xF5       /**< @brief In stream mode should be sent before any command */
#define ENABLE_REP        0xF4       /**< @brief In stream mode only */
#define SET_SAMPLE_RATE   0xF3       /**< @brief Sets state sampling rate */
#define SET_REMOTE_MODE   0xF0       /**< @brief Send data on request only */
#define READ_DATA         0xEB       /**< @brief Send data packet request */
#define SET_STREAM_MODE   0xEA       /**< @brief Send data on events */
#define STATUS_REQUEST    0xE9       /**< @brief Get mouse config (3 bytes) */
#define SET_RESOLUTION    0xE8       /**< @brief */ 
#define SET_SCALING_2_1   0xE7       /**< @brief Acceleration mode */
#define SET_SCALING_1_1   0xE6       /**< @brief Linear mode */

//Mouse command responses
#define ACK               0xFA  /**< @brief If everything OK */
#define NACK              0xFE  /**< @brief If invalid byte */
#define ERROR             0xFC  /**< @brief If second consecutive invalid bbyte */

// Mouse Bits
#define LB                BIT(0) /**< @brief Left Button */
#define RB                BIT(1) /**< @brief Right Button */
#define MB                BIT(2) /**< @brief Middle Button */
#define BIT3_ON           BIT(3) /**< @brief Is 1 if it is first packet */
#define MSB_X_DELTA       BIT(4) /**< @brief Most significant byte of delta x */
#define MSB_Y_DELTA       BIT(5) /**< @brief Most significant byte of delta y */
#define X_OVF             BIT(6) /**< @brief Indicates overflow of delta x */
#define Y_OVF             BIT(7) /**< @brief Indicates overflow of delta y */

#endif
