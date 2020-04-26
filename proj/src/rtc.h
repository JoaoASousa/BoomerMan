#ifndef RTC_H
#define RTC_H

#include "stdint.h"

/** @defgroup rtc rtc
 * @{
 *
 * RTC related functions.
 */

/**
 * @brief Assembly Function that "enables" the detection of interrupts
 * 
 * @return int 
 */
int _ena_interrupts(void);

/**
 * @brief Assembly Function that "disables" the detection of interrupts
 * 
 * @return int 
 */
int _dis_interrupts(void);

/**
 * @brief Subscribe RTC Interrupts
 * 
 */
int (rtc_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribe RTC Interrupts
 * 
 */
int (rtc_unsubscribe)();

/**
 * @brief Write to the register (param reg) the information in the param data
 * 
 * @param reg Register to be written the information to 
 * @param data Information to be written
 * @return int 0 in case of success
 */
int rtc_write_reg(uint32_t reg, uint32_t data);

/**
 * @brief 
 * 
 * @param reg Register to read from
 * @param data Where the information read is written
 * @return int 0 in case of success
 */
int rtc_read_reg(uint32_t reg, uint8_t *data);

/**
 * @brief RTC Interrupt Handler
 * 
 */
void rtc_ih(void);

/**
 * @brief Writes to the param second the current number of seconds of the time
 * 
 * @param second 
 * @return int 0 in case of success
 */
int get_seconds(uint8_t *second);

/**
 * @brief Writes to the param minute the current number of minutes of the time
 * 
 * @param minute 
 * @return int 0 in case of success
 */
int get_minutes(uint8_t *minute);

/**
 * @brief Writes to the param hour the current number of hours of the time
 * 
 * @param hour 
 * @return int 0 in case of success
 */
int get_hours(uint8_t *hour);

/**
 * @brief Writes to the params the current time
 * 
 * @param second 
 * @param minute 
 * @param hour 
 * @return int 0 in case of success
 */
int get_time(uint8_t *second, uint8_t *minute, uint8_t *hour);

/**
 * @brief Displays the time (human friendly way)
 * 
 */
void show_time();

/**
 * @brief Checks if register A can be accessed (does not have its UIP (Update In Progress) flag set to 1)
 * 
 * @return int 0 if register A is not updating
 */
int check_status_regA();

/**
 * @brief Writes to the param dayOfWeek the number correspondent to the current day of the week
 * 
 * @param dayOfWeek 
 * @return int 0 in case of success
 */
int get_day_of_week(uint8_t *dayOfWeek);

/**
 * @brief Writes to the param dayOfMonth the number correspondent to the current day of the month
 * 
 * @param dayOfMonth 
 * @return int 0 in case of success
 */
int get_day_of_month(uint8_t *dayOfMonth);

/**
 * @brief Writes to the param month the number correspondent to the current month
 * 
 * @param month 
 * @return int 0 in case of success
 */
int get_month(uint8_t *month);

/**
 * @brief Writes to the param year the number correspondent to the current year
 * 
 * @param year 
 * @return int 0 in case of success
 */
int get_year(uint8_t *year);

/**
 * @brief Writes to the params the current date
 * 
 * @param dayOfWeek 
 * @param dayOfMonth 
 * @param month 
 * @param year 
 * @return int 0 in case of success
 */
int get_date(uint8_t *dayOfWeek, uint8_t *dayOfMonth, uint8_t *month, uint8_t *year);

/**
 * @brief Displays the date (human friendly way)
 * 
 */
void show_date();

/**
 * @brief Enables RTC Alarms (Sets Bit 5 to 1)
 * 
 * @return int 0 in case of success
 */
int rtc_enable_alarm();

/**
 * @brief Disables RTC Alarms (Sets Bit 5 to 0)
 * 
 * @return int 0 in case of success
 */
int rtc_disable_alarm();

/**
 * @brief Sets an alarm for number_seconds (param) after the current time
 * 
 * @param number_seconds 
 * @return int 0 in case of success
 */
int rtc_set_alarm(uint number_seconds);

/**
 * @brief Conversion from binary coded decimal to decimal
 * 
 * @param seconds 
 * @return uint32_t 
 */
uint32_t convert_sec_BCD_to_DEC(uint32_t seconds);

/**
 * @brief Conversion from decimal to binary coded decimal
 * 
 * @param seconds 
 * @return uint32_t 
 */
uint32_t convert_sec_DEC_to_BCD(uint32_t seconds);

/**
 * @brief Conversion from decimal to hexadecimal representation
 * 
 * @param n 
 * @return uint 
 */
uint decToHexa(uint8_t n);


#endif
