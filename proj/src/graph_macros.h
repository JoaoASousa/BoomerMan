/** @defgroup graph_macros graph_macros
 * @{
 *
 * Constants for programming the graphics.
 */

#define VBE_FUNC_00       0x4F00    /**< @brief VBE Function 00 */
#define VBE_FUNC_01       0x4F01    /**< @brief VBE Function 01 */
#define VBE_FUNC_02       0x4F02    /**< @brief VBE Function 02, set VBE mode */

// AH Register
#define SUCCESS_CALL      0x00      /**< @brief Function call successful */
#define FAILED_FUNC_CALL  0x01	    /**< @brief Function call failed */
#define FUNC_NOT_SUP      0x02	    /**< @brief Function is not supported in current HW */configuration
#define FUNC_INV_CUR_MODE 0x03	    /**< @brief Function is invalid in current video mode */

// AL Register
#define VBE_FUNC_SUP      0x4F      /**< @brief If the VBE function called is supported */

// Graphics Modes                   //          Screen Res   Color Mode    Bits per pixel (R:G:B)
#define I_768             0x105     /**< @brief 1024x768	    Indexed	      8 */
#define D_480             0x110     /**< @brief 640x480	    Direct color	15((1:)5:5:5) */
#define D_600             0x115     /**< @brief 800x600	    Direct color	24 (8:8:8) */
#define D_1024            0x11A     /**< @brief 1280x1024	  Direct color	16 (5:6:5) */
#define D_864             0x14C     /**< @brief 1152x864	  Direct color	32 ((8:)8:8:8) */

// Text Mode
#define CGA_MODE          0x03      /**< @brief standard CGA mode */

// BIOS Calls       Interrupt vector    Service
#define VIDEO_CARD_BIOS   0x10      /**< @brief video card */
#define PC_CONFIG_BIOS    0x11      /**< @brief PC configuration */
#define MEM_CONFIG_BIOS   0x12      /**< @brief memory configuration */
#define KEYBOARD_BIOS     0x16     /**< @brief keyboard */
