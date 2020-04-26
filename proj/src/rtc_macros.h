#ifndef RTC_MACROS_H
#define RTC_MACROS_H

/** @defgroup rtc_macros rtc_macros
 * @{
 *
 * Constants for programming the Real Time Clock.
 */


#define RTC_IRQ         8       /**< @brief RTC Irq */
#define RTC_HOOK_ID     3       /**< @brief RTC Hook id */

#define RTC_ADDR_REG    0x70    /**< @brief To load with the address of the RTC register to be accessed */
#define RTC_DATA_REG    0x71    /**< @brief To use to transfer the data to/from the RTC's register accessed */

// Time of the Day, Alarm and Date Registers
#define SECONDS         0
#define SECONDS_ALARM   1
#define MINUTES         2
#define MINUTES_ALARM   3
#define HOURS           4
#define HOURS_ALARM     5
#define DAY_OF_WEEK     6
#define DAY_OF_MONTH    7
#define MONTH           8
#define YEAR            9

#define DONT_CARE_VALUE 0xC0  /**< @brief Don't care value for alarm register */

// Registers
#define RTC_REG_A       0x0A  /**< @brief RTC Register A */
#define RTC_REG_B       0x0B  /**< @brief RTC Register B */
#define RTC_REG_C       0x0C  /**< @brief RTC Register C */
#define RTC_REG_D       0x0D  /**< @brief RTC Register D */


// Register A
#define UIP BIT(7)    /**< @brief If set to 1, update in progress. Do not access time/date registers */
#define DV2 BIT(6)    /**< @brief Control the couting chain */
#define DV1 BIT(5)    /**< @brief Control the couting chain */
#define DV0 BIT(4)    /**< @brief Control the couting chain */
#define RS3 BIT(3)    /**< @brief Rate selector – for periodic interrupts and square wave output */
#define RS2 BIT(2)    /**< @brief Rate selector – for periodic interrupts and square wave output */
#define RS1 BIT(1)    /**< @brief Rate selector – for periodic interrupts and square wave output */
#define RS0 BIT(0)    /**< @brief Rate selector – for periodic interrupts and square wave output */

// Register B
#define SET BIT(7)      /**< @brief Set to 1 to inhibit updates of time/date registers */
#define PIE BIT(6)      /**< @brief Set to 1 to enable the corresponding interrupt source */
#define AIE BIT(5)      /**< @brief Set to 1 to enable the corresponding interrupt source */
#define UIE BIT(4)      /**< @brief Set to 1 to enable the corresponding interrupt source */
#define SQWE BIT(3)     /**< @brief Set to 1 to enable square-wave generation */
#define DM BIT(2)       /**< @brief Set to 1 to set time, alarm and date registers in binary. Set to 0, for BCD */
#define _24_12_ BIT(1)  /**< @brief Set to 1 to set hours range from 0 to 23, and to 0 to range from 1 to 12 */
#define DSE BIT(0)      /**< @brief Set to 1 to enable Daylight Savings Time, and to 0 to disable (Useless) */

// Register C
#define IRQF BIT(7)     /**< @brief IRQ line active */
#define PF  BIT(6)      /**< @brief Periodic interrupt pending */
#define AF  BIT(5)      /**< @brief Alarm interrupt pending */
#define UF  BIT(4)      /**< @brief Update interrupt pending */

// Register D
#define VRT BIT(7)      /**< @brief Valid RAM/time – set to 0 when the battery runs out */


#endif
