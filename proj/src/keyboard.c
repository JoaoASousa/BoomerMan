#include <lcom/lcf.h>

#include "i8042.h"
#include "keyboard.h"
#include "utils.h"
#include "logic.h"

#include <minix/sysutil.h>

extern uint32_t sys_inb_counter;
uint8_t scancodes[2];
uint8_t scancode;
static int kbd_hook_id = KEYBOARD_IRQ;
bool is_two_bytes = false;

void (sys_inb_increment()) {
  sys_inb_counter++;
}

int read_status_and_output_buffer_kbc(uint8_t *status){
    // ler status

  if (util_sys_inb(STAT_REG, status) != 0) {
    printf("Keyboard: Error reading status");
    return 1;
  }

  // check for errors
  if (*status & (PAR | TIMEOUT | AUX)) {
    printf("Keyboard: Parity / Timeout error");
    return 1;
  }

  // check BFO
  if ((*status & OBF) != 1) {
    printf("Keyboard: Output buffer error");
    return 1;
  }
  // ler scancode
  if (util_sys_inb(OUTPUT_B, &scancode) != 0) {
    printf("Keyboard: Error reading scancode");
    return 1;
  }

  return 0;
}


uint8_t discard_make_codes(void) {

  uint8_t garbage;
  uint8_t i = NUM_TRIES;
  while (i > 0) {
    util_sys_inb(OUTPUT_B, &garbage);
    i--;
    if (garbage & BIT(7)) return garbage;
  }

  return 0;
}

enum KB_KEY (check_key)(uint8_t scancode, bool is_two_b) {

  if (is_two_b) {

    switch(scancode) {
      default:
      return NO_KEY;
      break;
    }
  } else {

    switch (scancode) {
      case ESC_BREAKCODE:
        return ESC_BREAK;
        break;
      case W_MAKECODE:
        return W_MAKE;
        break;
      case S_MAKECODE:
        return S_MAKE;
        break;
      case A_MAKECODE:
        return A_MAKE;
        break;
      case D_MAKECODE:
        return D_MAKE;
        break;
      case W_BREAKCODE:
        return W_BREAK;
        break;
      case S_BREAKCODE:
        return S_BREAK;
        break;
      case A_BREAKCODE:
        return A_BREAK;
        break;
      case D_BREAKCODE:
        return D_BREAK;
        break;
      case SPACE_BREAKCODE:
        return SPACE_BREAK;
        break;
      case ENTER_BREAKCODE:
        return ENTER_BREAK;
        break;
      default:
        return NO_KEY;
        break;
    }
  } 
}

enum KB_KEY (get_key_pressed)(void) {

  uint8_t status;
  
  if (read_status_and_output_buffer_kbc(&status) != 0){
    return NO_KEY;
  }

  uint8_t discarded = discard_make_codes();
  if (discarded != 0) scancode = discarded;

  // Check scancode
  if (scancode == 0xE0) {
    is_two_bytes = true;
    scancodes[0] = scancode;
    return TWO_BYTE;
  }

  else if (is_two_bytes) {
    is_two_bytes = false;
    scancodes[1] = scancode;
    return check_key(scancodes[1], true);
  } 
  
  else {
    scancodes[0] = scancode;
    return check_key(scancodes[0], false);
  }
}

void (kbc_ih)(void) {

  // ler status
  uint8_t status;

  if (read_status_and_output_buffer_kbc(&status) != 0) {
    return ;
  }

  // escrever scancode
  if (scancode == 0xe0) {
    is_two_bytes = true;
    scancodes[0] = scancode;
  }

  else if (is_two_bytes) {
    is_two_bytes = false;
    scancodes[1] = scancode;
  }
  
  else {
    scancodes[0] = scancode; 
    // (semelhante ao que está no lab3.c)
  }

}

int (reenable_keyboard_int()) {

  if (sys_outb(STAT_REG, READ_CMD_BYTE) != 0) {
    return 1;
  }

  uint8_t command;

  if (util_sys_inb(OUTPUT_B, &command) != 0) {
    return 1;
  }

  if (sys_outb(STAT_REG, OUTPUT_B) != 0) {
    return 1;
  }

  command |= ENABLE_KBD_OBF_INT;

  if (sys_outb(OUTPUT_B, command) != 0) {
    return 1;
  }

  return 0;
}

int (keyboard_subscribe_int) (uint8_t *bit_n) {
  *bit_n = BIT(kbd_hook_id);

  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_ENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != OK) {
    return 1;
  }

  return 0;
}

int (keyboard_unsubscribe_int)() {
  
  if (sys_irqrmpolicy(&kbd_hook_id) != OK) {
    return 1;
  }

  return 0;

}



