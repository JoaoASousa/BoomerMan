#include <lcom/lcf.h>

#include <stdint.h>
#include <minix/sysutil.h>

#include "rtc_macros.h"
#include "rtc.h"
#include "utils.h"

#include <string.h>

int rtc_hook_id = RTC_HOOK_ID;

int (rtc_subscribe_int)(uint8_t *bit_no) {

    *bit_no = BIT(rtc_hook_id);

    if (sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &rtc_hook_id) != OK) {
        return 1;
    }
    printf("\nSubscribe\n");

    return 0;
}


int (rtc_unsubscribe)() {
    
    if (sys_irqrmpolicy(&rtc_hook_id) != OK) {
        return 1;
    }

    return 0;

}


int rtc_write_reg(uint32_t reg, uint32_t data) {
    printf("\nWriting Reg 1");
    if (sys_outb(RTC_ADDR_REG, reg) != 0) {
        printf("\nWriting Reg 2");
        return 1;
    }
    printf("\nWriting Reg 3");
    if (sys_outb(RTC_DATA_REG, data) != 0) {
        printf("\nWriting Reg 4");
        return 1;
    }
    printf("\nWriting Reg 5");
    return 0;
}


int rtc_read_reg(uint32_t reg, uint8_t *data) {

    if (sys_outb(RTC_ADDR_REG, reg) != 0) {
        return 1;
    }

    if (util_sys_inb(RTC_DATA_REG, data) != 0) {
        return 1;
    }


    return 0;
}


void rtc_ih() {
    
    // printf("RTC Interrupt");

    int cause;
    uint32_t reg;

    sys_outb(RTC_ADDR_REG, RTC_REG_C);
    cause = sys_inb(RTC_DATA_REG, &reg);
    
    // printf("\nAfter SYS_INB\n");

    // // Alarm interrupts
    // if (cause & AF)
    //     printf("Alarm Interrupt");

}

int get_seconds(uint8_t *second) {

    if (rtc_read_reg(SECONDS, second) != 0) {
        return 1;
    }

    return 0;
}

int get_minutes(uint8_t *minute) {

    if (rtc_read_reg(MINUTES, minute) != 0) {
        return 1;
    }

    return 0;
}

int get_hours(uint8_t *hour) {

    if (rtc_read_reg(HOURS, hour) != 0) {
        return 1;
    }

    return 0;
}

int get_time(uint8_t *second, uint8_t *minute, uint8_t *hour) {
    
    int check;

    do {
        check = check_status_regA();
    } while (check != 0);

    if (get_seconds(second) != 0) {
        printf("Seconds Error");
        return 1;
    }
    else if (get_minutes(minute) != 0) {
        printf("Minutes Error");
        return 1;
    }
    else if (get_hours(hour) != 0) {
        printf("Hours Error");
        return 1;
    }

    return 0;
}


void show_time() {

    uint8_t second, minute, hour;

    if (get_time(&second, &minute, &hour) != 0) {
        printf("Time Error");
    }
    else {
        printf("%02X:%02X:%02X", hour, minute, second);
    }    
   
}


int check_status_regA() {

    uint8_t regA;

    if (rtc_read_reg(RTC_REG_A, &regA) != 0) {
        return 1;
    }


    if ((regA & UIP) != 0) {
        return 1;
	}
    

    return 0;
}


int get_day_of_week(uint8_t *dayOfWeek) {

    if (rtc_read_reg(DAY_OF_WEEK, dayOfWeek) != 0) {
        return 1;
    }
    uint8_t temp = *dayOfWeek;
    *dayOfWeek = decToHexa(temp);
    return 0;
}

int get_day_of_month(uint8_t *dayOfMonth) {

    if (rtc_read_reg(DAY_OF_MONTH, dayOfMonth) != 0) {
        return 1;
    }
    uint8_t temp = *dayOfMonth;
    *dayOfMonth = decToHexa(temp);
    return 0;
}

int get_month(uint8_t *month) {

    if (rtc_read_reg(MONTH, month) != 0) {
        return 1;
    }
    uint8_t temp = *month;
    *month = decToHexa(temp);
    return 0;
}

int get_year(uint8_t *year) {

    if (rtc_read_reg(YEAR, year) != 0) {
        return 1;
    }
    uint8_t temp = *year;
    *year = decToHexa(temp);
    return 0;
}

int get_date(uint8_t *dayOfWeek, uint8_t *dayOfMonth, uint8_t *month, uint8_t *year) {
    
    //_dis_interrupts();
    
    int check;

    do {
        check = check_status_regA();
    } while (check != 0);

    if (get_day_of_week(dayOfWeek) != 0) {
        printf("Day of Week Error");
        //_ena_interrupts();
        return 1;
    }
    else if (get_day_of_month(dayOfMonth) != 0) {
        printf("Day of Month Error");
        //_ena_interrupts();
        return 1;
    }
    else if (get_month(month) != 0) {
        printf("Month Error");
        //_ena_interrupts();
        return 1;
    }
    else if (get_year(year) != 0) {
        printf("Year Error");
        //_ena_interrupts();
        return 1;
    }

    //_ena_interrupts();
    
    return 0;

}

void show_date() {

    uint8_t dayOfWeek, dayOfMonth, month, year;

    if (get_date(&dayOfWeek, &dayOfMonth, &month, &year) != 0) {
        printf("Date Error");
    }
    else {

        // https://stackoverflow.com/questions/30260986/how-to-print-a-string-using-printf-without-it-printing-the-trailing-newline
        switch (dayOfWeek) {
            case 0:
                printf("%.*s", 3, "SUN");
                break;
            case 1:
                printf("%.*s", 3, "MON");
                break;
            case 2:
                printf("%.*s", 3, "TUE");
                break;
            case 3:
                printf("%.*s", 3, "WED");
                break;
            case 4:
                printf("%.*s", 3, "THU");
                break;
            case 5:
                printf("%.*s", 3, "FRI");
                break;
            case 6:
                printf("%.*s", 3, "SAT");
                break;
        }


        printf(", %02X/%02X/20%02X\n", dayOfMonth, month, year);
    }
    
}

int rtc_enable_alarm() {

    uint8_t data;

    if (rtc_read_reg(RTC_REG_B, &data) != 0) {
        return 1;
    }

    data |= AIE; // Set Bit 5 to 1 (to enable)

    if (rtc_write_reg(RTC_REG_B, data) != 0) {
        return 1;
    }
    printf("\nEnabled Alarm");
    return 0;
}

int rtc_disable_alarm() {

    uint8_t data;

    if (rtc_read_reg(RTC_REG_B, &data) != 0) {
        return 1;
    }

    // Set Bit 5 to 0 (to disable)
    // 1 << 5 -> 00100000
    // ~(1 << 5) -> 11011111
    data &= ( ~(1 << 5) );

    if (rtc_write_reg(RTC_REG_B, data) != 0) {
        return 1;
    }

    return 0;
}


int rtc_set_alarm(uint number_seconds) {
    
    // Invalid Seconds
    if (number_seconds < 1) {
        return 1;
    }

    
    int check_a;
    do {
        check_a = check_status_regA();
    } while (check_a != 0);

    uint8_t current_seconds;
    get_seconds(&current_seconds);

    uint32_t dec_seconds = convert_sec_BCD_to_DEC(current_seconds);

    // printf("cursec: %d\n", dec_seconds);

    // Seconds at which the alarm goes off need to be between 0 and 59
    uint32_t alarm_seconds = (dec_seconds + number_seconds) % 60;

    // printf("alarm: %d\n", alarm_seconds);

    uint8_t conv_alarm_sec = convert_sec_DEC_to_BCD(alarm_seconds);

    // Set Seconds of the Alarm
    if (rtc_write_reg(SECONDS_ALARM, (uint32_t) conv_alarm_sec) != 0) {
        // printf("\nFailed Writing!");
        return 1;
    }

    // Set Seconds of the Alarm
    if (rtc_write_reg(MINUTES_ALARM, (uint32_t) DONT_CARE_VALUE) != 0) {
        // printf("\nFailed Writing!");
        return 1;
    }

        // Set Seconds of the Alarm
    if (rtc_write_reg(HOURS_ALARM, (uint32_t) DONT_CARE_VALUE) != 0) {
        // printf("\nFailed Writing!");
        return 1;
    }
    
    //printf("\nHi Again 5\n");
    return 0;
}


uint32_t convert_sec_BCD_to_DEC(uint32_t seconds) {
    
    uint convSeconds = ((seconds & 0xF0) >> 4) * 10 + (seconds & 0x0F);

    return convSeconds;
}


uint32_t convert_sec_DEC_to_BCD(uint32_t seconds) {

    uint32_t convSeconds;

    convSeconds = ( (seconds / 10) << 4 );
    convSeconds |= (seconds % 10);

    return convSeconds;
}


uint decToHexa(uint8_t n) {    
    
    uint hexa = 0;

    hexa += (n / 16) * 10;
    hexa += n % 16;

    return hexa;
}


