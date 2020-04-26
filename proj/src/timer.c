#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

#define first_byte 0x0f

uint32_t counter = 0;

// hook_id is used by the kernel when there's an interrupt.
int timer_hook_id = TIMER0_IRQ;

int (timer_get_conf)(uint8_t timer, uint8_t *st);

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  // timer number check
  if (timer < 0 || timer > 2) {
    return 1;
  }
  
  // first condition avoids overflow of thisFrequency (declared below)
  // second condition assures thisFrequency >= 1
  if (freq < 19 || freq > TIMER_FREQ) {
    return 1;
  }


  uint32_t thisFrequency = TIMER_FREQ / freq;
  uint8_t conf; 
  
  
  if(timer_get_conf(timer, &conf) != 0) {
    return 1;
  }
  
  uint8_t lsb;
  uint8_t msb;
  
  util_get_MSB(thisFrequency, &msb);
  util_get_LSB(thisFrequency, &lsb);

  conf = (conf & first_byte) | TIMER_LSB_MSB;

  switch (timer) {
    case 0:
      conf |= TIMER_SEL0;
      break;
    case 1:
      conf |= TIMER_SEL1;
      break;
    case 2:
      conf |= TIMER_SEL2;
      break;

    default:
      return 1;
  }

  // check if sys_outb works as intended
  if (sys_outb(TIMER_CTRL, conf) != 0) {
    // printf("\nsys_out returned != 0\n");
    return 1;
  }

  switch (timer) {
    case 0:
      if (sys_outb(TIMER_0, lsb) != 0) {
        return 1;
      }
			if (sys_outb(TIMER_0, msb) != 0) {
        return 1;
      }
    
    case 1:
      if (sys_outb(TIMER_1, lsb) != 0) {
        return 1;
      }
		  if (sys_outb(TIMER_1, msb) != 0) {
        return 1;
      }
    
    case 2:
      if (sys_outb(TIMER_2, lsb) != 0) {
        return 1;
      }
		  if (sys_outb(TIMER_2, msb) != 0) {
        return 1;
      }
    
    default:
      return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  // setting the bit 0 to 1
  *bit_no = BIT(timer_hook_id);

  // sys_irqsetpolicy changes the value at &hook_id
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id) != OK) {
    return 1;
  }

  return 0;
}

int (timer_unsubscribe_int)() {

  if (sys_irqrmpolicy(&timer_hook_id) != OK) {
    return 1;
  }
  
  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  // printf("\ntimer_get_conf called\n");
  if (timer < 0 || timer > 2) {
    return 1;
  }

  uint32_t readBack = (TIMER_RB_COUNT_ | TIMER_RB_SEL(timer) | TIMER_RB_CMD);

  if (sys_outb(TIMER_CTRL, readBack) != 0){
    return 1;
  }

  if (util_sys_inb(TIMER_0 + timer, st) != 0) {
    return 1;
  }
  // printf("\n%x\n", *st);
  
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  
  union timer_status_field_val val;

  switch (field) {

    case tsf_all:
      // printf("\nAt tsf_all\n");
      val.byte = st;
      break;
    
    case tsf_initial:
      // printf("\nAt tsf_initial\n");
      if ((st & TIMER_LSB) && (st & TIMER_MSB)) {
        val.in_mode = MSB_after_LSB;
      }
      else if ((st & TIMER_LSB) && !(st & TIMER_MSB)) {
        val.in_mode = LSB_only;
      }
      else if (!(st & TIMER_LSB) && (st & TIMER_MSB)) {
        val.in_mode = MSB_only;
      }
      else {
        val.in_mode = INVAL_val;
      }
      break;

    case tsf_mode:
      // printf("\nAt tsf_mode\n");

      // dividing by 2 works on most of cases
      val.count_mode = ( (st & (TIMER_SQR_WAVE | BIT(3))) >> 1);

      if (val.count_mode >= 6) {
        // to the cases that the division by 2 doesn't work
        // a possible solution is to "eliminate" the 1 in the 3rd bit (BIT(2))
        val.count_mode &= (BIT(1) | BIT(0));
      }
      break;
    
    case tsf_base:
      // printf("\nAt tsf_base\n");
      val.bcd = st & BIT(0);
      // printf("\n%d\n", val.bcd);
      break;

    default:
      // printf("\nDefault Case\n");
      return 1;
  }

  
  
  if (timer_print_config(timer, field, val) != 0) {
    // printf("\ntimer_print_config returned != 0");
    return 1;
  }
  else {
    // printf("\ntimer_print_config worked as intended\n");
    return 0;
  }
  
}
