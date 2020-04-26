#include <lcom/lcf.h>

#include "mouse.h"


int mouse_hook_id = MOUSE_IRQ;
extern uint16_t packet[3];
extern uint8_t byte_data;
extern struct packet packet_data;
int b_counter = 0;
bool gesture = false;

int read_kbc_status(uint8_t *status);
int mouse_read_status(uint8_t * status);

int16_t read_output_buf() {
  uint8_t status;
  uint8_t data;

  int8_t tries = 3;
  while (tries > 0) {
    if (util_sys_inb(STAT_REG, &status) != 0) {
      return -1;
    }
    if (status & OBF) {
      if (util_sys_inb(OUTPUT_B, &data)) {
        return -1;
      }
      tickdelay(micros_to_ticks(DELAY_US));
      return data;
    }
    tickdelay(micros_to_ticks(DELAY_US));
    tries--;
  }
  return -1;
}

int mouse_read_status(uint8_t * status){
  // Reads the status register
  if (util_sys_inb(STAT_REG, status) != 0) {
    return 1;
  }

  // Check output buffer and if there was a parity / timeout error
  else if ((*status & (PAR | TIMEOUT)) != 0) {
    return 1;
  }

  // if there wasn't any error
  
  // does the NULL work as a way to discard the byte read ?
  if (util_sys_inb(OUTPUT_B, &byte_data) != 0) {
    return 1;
  }

  return 0;
}

// Handling of mouse interrupts
void (mouse_ih)(void) {
  
  // Reads the status register and the output buffer (OB).
  // If there was some error, the byte read from the OB should be discarded.

  bool readError = false; // if true it means there was an error reading the status register
  bool outputParityTimeout = false;
  uint8_t status;

  // printf("\nPRE STATUS REG READING\n");

  // Reads the status register
  if (util_sys_inb(STAT_REG, &status) != 0) {
    // printf("\nREG READING ERROR\n");
    readError = true;
  }

  

  // Check output buffer and if there was a parity / timeout error
  else if ((status & (PAR | TIMEOUT)) != 0) {
    // printf("\nSOME ERROR\n");
    outputParityTimeout = true;
  }

  // if there wasn't any error
  if (!readError && !outputParityTimeout) {
    // printf("\nNO ERRORS\n");
    // does the NULL work as a way to discard the byte read ?
    if (util_sys_inb(OUTPUT_B, &byte_data) != 0) {
      return ;
    }
  }
}

void parse_packet() {

  // Fill Data Field Bytes
  for (int i = 0; i < 3; i++) {
    packet_data.bytes[i] = packet[i];
  }

  // Fill Data Field rb
  packet_data.rb = (packet[0] & RB) ? true : false;

  // Fill Data Field mb
  packet_data.mb = (packet[0] & MB) ? true : false;

  // Fill Data Field lb
  packet_data.lb = (packet[0] & LB) ? true : false;

  // Fill Data Field delta_x
  packet_data.delta_x = packet[1];
  if (packet[0] & MSB_X_DELTA) packet_data.delta_x -= 256;

  // Fill Data Field delta_y
  packet_data.delta_y = packet[2];
  if (packet[0] & MSB_Y_DELTA) packet_data.delta_y -= 256;

  // Fill Data Field x_ov
  packet_data.x_ov = (packet[0] & X_OVF) ? true : false;

  // Fill Data Field y_ov
  packet_data.y_ov = (packet[0] & Y_OVF) ? true : false;

}

enum MS_KEY (check_click)(void) {
  // bool lb = packet_data.lb;
  // uint8_t status;

  // if (mouse_read_status(&status) != 0) {
  //   return NO_CLICK;
  // }

  switch (b_counter) {
    case 0:
      if (byte_data & BIT3_ON) {
        packet[0] = byte_data;
        b_counter++;
      }
      break;

    case 1:
      packet[1] = byte_data;
      b_counter++;
      break;

    case 2:
      packet[2] = byte_data;

      parse_packet();

      b_counter = 0;
      break;
  }

  if (packet_data.lb) return LEFT_CLICK;

  if (packet_data.rb) return RIGHT_CLICK;

  uint16_t delta_x_abs = abs(packet_data.delta_x);
  uint16_t delta_y_abs = abs(packet_data.delta_y);

  if (delta_x_abs > delta_y_abs) {
    if (packet_data.delta_x > MOUSE_TOLERANCE) {
      return RIGHT;
    }
    else if (packet_data.delta_x < -MOUSE_TOLERANCE) {
      return LEFT;
    }
    else {
      return NO_CLICK;
    }
  }

  else if (delta_y_abs > delta_x_abs) {
    if (packet_data.delta_y > MOUSE_TOLERANCE) {
      return UP;
    }
    else if (packet_data.delta_y < -MOUSE_TOLERANCE) {
      return DOWN;
    }
    else {
      return NO_CLICK;
    }

  }

  return NO_CLICK;
}

// To subscribe mouse interrupts
int mouse_subscribe_int (uint8_t *bit_n) {
  *bit_n = mouse_hook_id;
  
  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_ENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != OK) {
    printf("Mouse subscribe: Failed to set policy");
    return 1;
  }

  return 0;
}

// To unsubscribe mouse interrupts
int mouse_unsubscribe_int () {

  if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
    return 1;
  }
  
  return 0;
}

int send_cmd_to_mouse(uint32_t cmd) {

  if (write_to_mouse(WRITE_BYTE_MOUSE, KBC_CMD_REG) == -1) {
    return -1;
  }

  if (write_to_mouse(cmd, OUTPUT_B) == -1) {
    return -1;
  }

  int16_t answer = read_output_buf();
  if (answer != ACK) {
    // printf("Mouse yikes");
    return -1;
  }

  // printf("SENT 0x%02X\n", cmd);
  return 0;
}

int reset_mouse() {
  send_cmd_to_mouse(DISABLE_REP);
  send_cmd_to_mouse(SET_STREAM_MODE);
  uint32_t dflt = minix_get_dflt_kbc_cmd_byte();

  write_cmd_byte(dflt);

  return 0;
}

int write_to_mouse(uint32_t cmd, uint32_t port) {

  int write_tries = 0;

  uint8_t status;

  // Writing command
  while (write_tries < 3) {
    if (util_sys_inb(STAT_REG, &status) != 0){
      return -1;
    }

    if ((status & IBF) == 0) {
      if (sys_outb(port, cmd) != 0){
        return -1;
      }
      tickdelay(micros_to_ticks(DELAY_US));
      return 0;
    }
    
    write_tries++;
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return -1;
} 




int read_cmd_byte() {
  
  if (sys_outb(KBC_CMD_REG, READ_CMD_BYTE) != 0){
    return -1;
  }

  int16_t read_data;
  read_data = read_output_buf();

  if (read_data == -1) {
    return -1;
  }

  return read_data;
}


int write_cmd_byte(uint32_t cmd) {
  
  if (sys_outb(KBC_CMD_REG, WRITE_CMD_BYTE) != 0){
    return -1;
  }
  if(write_kbc_arg(cmd) == -1){
    return -1;
  }

  return 0;
}



int read_kbc_status(uint8_t *status){

  uint8_t temp;
  if (util_sys_inb(STAT_REG, &temp) != 0){
    return 1;
  }
  
  *status = temp;

  return 0;
  
}

bool kbc_check_ibf () {
  uint8_t status;

  if (read_kbc_status(&status) != OK) {
    return 1;
  }

  return ((status & IBF) == 0);

  /*
  if ((status & IBF) == 0) {
    return true;
  }
  else
    return false;
  */
}

bool kbc_check_obf () {
  uint8_t status;

  if (read_kbc_status(&status) != OK) {
    return 1;
  }

  if (status & OBF) {

    if ((status & (PAR | TIMEOUT)) == 0) {
      return true;
    }

    else {
      return false;
    }
  }
  else 
    return false;
}

int write_kbc_cmd(uint32_t cmd){
  unsigned tries = 0;

  while(tries < 3){
    
    if (kbc_check_ibf()){ // will only write command if ibf is available
      
      if (sys_outb(KBC_CMD_REG,cmd) != 0){
        return 1;
      }

      else return 0;
    }
  
    else {
      tries++;
      tickdelay(micros_to_ticks(DELAY_US));
    }
  }

  // will have timed out all tries, return error
  return 1;
}

int write_kbc_arg(uint32_t arg){
  unsigned tries = 0;

  while(tries < 3){
    
    if (kbc_check_ibf()){ //will only write command if ibf is available
      
      if (sys_outb(WRITE_CMD_BYTE,arg) != 0){
        return -1;
      }

      else return 0;
    }
  
    else {
      tries++;
      tickdelay(micros_to_ticks(DELAY_US));
    }
  }
  // will have timed out all tries, return error
  return -1;
}

int write_mouse_cmd(uint32_t cmd, uint8_t *rsp){

  unsigned tries = 0;

  while(tries < 3){
    
    if (kbc_check_ibf()){

      if (sys_outb(WRITE_CMD_BYTE,cmd) != 0){
        return 1;
      }
      
      if (util_sys_inb(OUTPUT_B, rsp) != 0){
          return 1;
      }

      // printf("rsp: %x\n", *rsp);
    }
    else {
      tries++;
      tickdelay(DELAY_US);
      continue;
    }

    switch (*rsp)
    { 
    case ACK:
      printf("ACK\n");
      return 0;

    case NACK:
      printf("Resend!\n");
      tries++;
      continue;

    case ERROR:
      printf("ERROR!\n");
      return 1;
      break;
    
    default:
      printf("Default!\n");
      break;
    }

  }
  // will have timed out all tries, return error
  printf("\nTimeout\n");
  return 1;
}

int mouse_enable(){

  if (kbc_check_ibf() != 0) {
    return (sys_outb(KBC_CMD_REG,ENABLE_MOUSE));
    
  }
  else return 0;
}

int mouse_disable(){

  if (kbc_check_ibf() != 0) {

    return (sys_outb(KBC_CMD_REG,DISABLE_REP));
  }
  else return 0;
}




//function to determine type of event;
struct mouse_ev* mouse_get_event(struct packet *p){
  static struct mouse_ev result;
  static struct packet previous ;

  static bool init = true;

  uint8_t x = p->bytes[0];
  
  if (init){ //É draw
    result.type = BUTTON_EV;
    init = false;
  }

  else {

    if ((x & LB) == LB && (previous.bytes[0] & LB) == 0)
      result.type = LB_PRESSED;

    else if ((x & RB) == RB && (previous.bytes[0] & RB) == 0)
      result.type = RB_PRESSED;

    else if ((x & MB) == MB && (previous.bytes[0] & MB) == 0)
      result.type = BUTTON_EV;


    else if (((x & LB) == 0) && (previous.bytes[0] & LB) == LB )
      result.type = LB_RELEASED;

    else if (((x & RB) == 0) && (previous.bytes[0] & LB) == LB )
      result.type = RB_RELEASED;

    else if (((x & MB) == 0) && (previous.bytes[0] & LB) == LB )
      result.type = BUTTON_EV;

    else {
        result.type = MOUSE_MOV;
    result.delta_x = p->delta_x;
    result.delta_y = p->delta_y;
    }
  }

  previous = * p;
  return &result;
}


void mouse_detect_gesture(struct mouse_ev * ev, uint8_t x_len, uint8_t tolerance){
  enum logical_and st = init;
  int total_x = 0;

  switch (st) {
    case init:
      if (ev->type == LB_PRESSED) {
        st = first_leg;
      }

      else {
        st = init;
      }

      break;

    case first_leg:
      if (ev->type == BUTTON_EV) {
        int k = ev->delta_y / ev->delta_x;
        if ((ev->delta_x < tolerance) || (ev->delta_y < tolerance) || k < 1) {
          st = init;
        }

        else {
          st = first_leg;
          total_x += ev->delta_x;

          if (total_x >= x_len) {
            st = init;
          }
        }
      }

      else if (ev->type == LB_RELEASED) {
        st = in_between;
      }

      else {
        st = init;
      }
      break;

    case in_between:
      if (ev->type == RB_PRESSED) {
        st = second_leg;
      }

      else {
        st = init;
      }
      break;

    case second_leg:
      if (ev->type == BUTTON_EV) {
        int k = ev->delta_y / ev->delta_x;
        if ((ev->delta_x < tolerance) || (ev->delta_y > tolerance) || k >= -1) {
          st = init;
        }

        else {
          st = second_leg;
          total_x += ev->delta_x;

          if (total_x >= x_len) {
            st = init;
          }
        }
      }

      else if (ev->type == RB_RELEASED) {
        total_x = 0;
        gesture = true;
      }

      else {
        st = init;
      }

      break;
  }
  ///
}
/*

//function to determine type of event;
struct mouse_ev mouse_get_event(struct packet *p){
  static struct packet previous;
  struct mouse_ev result;
  
  uint8_t x = p->bytes[0];
  
  if (p->bytes[1] != 0 || p->bytes[2] != 0) {
    result.type = MOUSE_MOV;
    result.delta_x = p->delta_x;
    result.delta_y = p->delta_y;
  }

  else{
    / *
    switch(x){
      case LB:
      result.type = LB_PRESSED;
      break;
      case RB:
      result.type = RB_PRESSED;
      break;
      case MB:
      result.type = BUTTON_EV;    Ha forma de fazer com switch??
      break;
      case !LB:
      result.type = LB_RELEASED;
      break;
      case !RB:
      result.type = RB_RELEASED;
      break;
      case !MB:
      result.type = BUTTON_EV;
      break;
    }
    * /

    if ((x & LB) == LB) result.type = LB_PRESSED;

    else if ((x & RB) == RB) result.type = RB_PRESSED;

    else if ((x & MB) == MB) result.type = BUTTON_EV;

    else if ((x & LB) != LB) result.type = LB_RELEASED;

    else if ((x & RB) != RB) result.type = RB_RELEASED;

    else if ((x & MB) != MB) result.type = BUTTON_EV;

  }
}

struct mouse_ev* detect_mouse_event	(	struct packet * 	pp	)	{
  
}


*/
