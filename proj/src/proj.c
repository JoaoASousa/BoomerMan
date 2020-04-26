// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

// Any header files included below this line should have been created by you

#include "rtc.h"
#include "rtc_macros.h"

#include "graph_macros.h"
#include "graphics.h"
#include "i8042.h"
#include "i8254.h"
#include "keyboard.h"
#include "logic.h"
#include "mouse.h"
#include "timer.c"
#include "utils.h"
#include "xpms.h"

uint16_t packet[3]; // declared as extern in mouse.c
uint8_t byte_data; // declared as extern in mouse.c
struct packet packet_data;  // declared as extern in mouse.c

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

/*
static int print_usage() {
  printf("Usage: <mode - hex>\n");

  return 1;
}
*/


extern uint player_lives;
extern uint player_score;
extern uint time_left;
extern uint current_stage;
bool on_menu = true;

int(proj_main_loop)(int argc, char *argv[]) {

  sys_enable_iop(SELF);

  rtc_enable_alarm();

  graphics_init(D_600);

  load_xpms();
  read_highscore_file();

  uint8_t timer_bit_no = 0;
  uint8_t keyboard_bit_no = 1;
  uint8_t mouse_bit_no;
  uint8_t rtc_bit_no = 8;

  mouse_enable_data_reporting();

  timer_subscribe_int(&timer_bit_no);
  keyboard_subscribe_int(&keyboard_bit_no);
  mouse_subscribe_int(&mouse_bit_no);
  rtc_subscribe_int(&rtc_bit_no);

  uint8_t irq_set_timer = BIT(0);
  uint8_t irq_set_kb = BIT(1);
  uint8_t irq_set_rtc = 8;
  int irq_set_mouse = BIT(mouse_bit_no);


  // general input
  enum KB_KEY pressed_k = NO_KEY;
  enum MS_KEY pressed_m = NO_CLICK;

  message msg;
  int ipc_status, r;

  int selected_menu;

  player_lives = 2;
  player_score = 0;
  time_left = 200;

  load_entities(NULL);
  load_player(true);
  read_map();

  selected_menu = 1;
  rtc_set_alarm(2);

  while (pressed_k != ESC_BREAK) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

          if (msg.m_notify.interrupts & irq_set_kb) {
            pressed_k = get_key_pressed();
            handle_input_kb(pressed_k);
          }
          if (msg.m_notify.interrupts & irq_set_mouse) {
            mouse_ih();
            pressed_m = check_click();
            handle_input_ms(pressed_m);
            // printf("\nPRESSED: %d\n", pressed_m);
          }

          if (msg.m_notify.interrupts & irq_set_rtc) {
            rtc_ih();

            if (on_menu) {
              switch (selected_menu) {
                case 1:
                  selected_menu = 2;
                  break;
                case 2:
                  selected_menu = 1;
                  break;
                default:
                  break;
              }
              draw_menu(selected_menu);

            }
          }

          if (msg.m_notify.interrupts & irq_set_timer) {
            //death_counter++;
            //if (death_counter == 300) exit_game(false);
            
            if (!on_menu) {
              
              if (player_lives == 0) {
                draw_game_over_screen();
                on_menu = true;

                selected_menu = 1;
                rtc_set_alarm(2);

                break;
              }

              handle_animations();
              handle_physics();

              if (current_stage > LAST_LEVEL) {
                break;
              }

              render();

            }
            else if (on_menu){
              if (pressed_k == ENTER_BREAK) {
                draw_level_screen();
                on_menu = false;
                pressed_k = NO_KEY;

                player_lives = 2;
                player_score = 0;
                time_left = 200;

                load_entities(NULL);
                load_player(true);
                read_map();
              }
            }
          }

          break;
      }

      if (current_stage > LAST_LEVEL) {
        break;
      }
    }
  }

  exit_game(current_stage > LAST_LEVEL);

  return 0;
}

//
// if you're interested, try to extend the command line options so that the usage becomes:
// <mode - hex> <minix3 logo  - true|false> <grayscale - true|false> <delay (secs)>
//

/*
  bool const minix3_logo = false;
  bool const grayscale = false;
  uint8_t const delay = 5;
  uint16_t mode;

  if (argc != 1)
    return print_usage();

  // parse mode info (it has some limitations for the sake of simplicity)
  if (sscanf(argv[0], "%hx", &mode) != 1) {
    printf("%s: invalid mode (%s)\n", __func__, argv[0]);

    return print_usage();
  }

  return proj_demo(mode, minix3_logo, grayscale, delay);
  */
