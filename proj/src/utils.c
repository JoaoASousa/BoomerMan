#include <lcom/lcf.h>

#include <stdint.h>

#include <minix/sysutil.h>

#include "keyboard.h"
#include "utils.h"

uint32_t sys_inb_counter = 0;

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {

  if (lsb == NULL) return 1;

  *lsb = (uint8_t) val;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {

  if (msb == NULL) return 1;
  
  uint16_t noLsb = val >> 8;
  
  *msb = (uint8_t) noLsb;
  
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  // printf("\nAt util_sys_inb\n");

  /*
  #ifdef LAB3
  sys_inb_counter++;
  #endif*/
  #ifdef LAB3
  sys_inb_increment();
  #endif

  if (value == NULL) return 1;

  uint32_t temp; 

  if (sys_inb(port, &temp) == 0) {

    *value = temp;
    return 0;
  }

  else return 1;
}
