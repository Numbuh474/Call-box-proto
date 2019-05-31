#ifndef MAIN_H_
#define MAIN_H_

#include <msp430.h>
#include <flash.h>
#include <pins.h>
#include <timer.h>
#include <stddef.h>
#include "queue.h"

#define MINIMUM_DECODE_DELTA 52
#define DECODE_ARRAY_SIZE 160

//Audio Channel Defines
//TODO: allow for multiple audio channels
#define AUDIO_CHANNEL_NONE 0
#define AUDIO_CHANNEL_1 1
#define AUDIO_CHANNEL_2 1
#define AUDIO_CHANNEL_3 1
#define AUDIO_CHANNEL_4 1
#define AUDIO_CHANNEL_TOTAL 4
#define BUTTON_ID_INVALID 0xFF000000

//GPIO definitions
#define GPIO_RF_ACTIVITY_LED     BIT0  //P1.0
#define GPIO_RF_INPUT            BIT1  //P1.1
#define GPIO_AUDIO_REC1_ENABLE   BIT4  //P1.4
#define GPIO_STATUS_LED          BIT6  //P1.6
#define GPIO_AUDIO_CHAN1_ENABLE  BIT7  //P1.7
#define GPIO_PROGRAM_BUTTON      BIT0  //P2.0
#define GPIO_PROGRAM_BUTTON_1    BIT1  //P2.1
#define GPIO_ERASE_BUTTONS       BIT2  //P2.2
#define GPIO_CHAN1_REC_BUTTON    BIT4  //P2.4

/*volatile unsigned int new_cap=0;
volatile unsigned int old_cap=0;
volatile unsigned int cap_diff=0;
unsigned int decode_array[DECODE_ARRAY_SIZE] = {0};
unsigned int decode_array_head = 0;
unsigned int decode_last_timestamp = 0;
unsigned int decode_data_available = 0;*/
unsigned int toggle_led_index;
unsigned int p2_gpio_int_state;
unsigned int program_mode_active;
unsigned int program_button_active;
unsigned int program_button_target;

unsigned int button_1_programmed;

//unsigned int message_length;

flash_data_struct_t button_id_list;
struct Queue id_queue;
queue_t id_queue_array [8];


void init_clk();
void setup_pins(void);
unsigned int add_decode_transition(unsigned int new_timestamp);
unsigned long get_call_button_id(void);
unsigned int get_next_decoded_bit(unsigned int decode_index);
void reset_decoder(void);
unsigned long call_button_received(void);
void run_program(void);
void play_audio(unsigned int audio_channel);
void inline_delay(unsigned int delay_cycle);
unsigned int get_audio_channel(unsigned long button_id);
unsigned int get_num_active_chan(void);
void load_button_ids(void);
void write_button_ids(void);
void erase_button_ids(void);
int add_button_id(unsigned long new_button, unsigned int index);
void handle_receiver_inputs(void);
void handle_receiver_inputs_alt(void);
void handle_user_inputs(void);
void handle_user_inputs_alt(void);
void init_globals(void);
unsigned long get_multiple_call_button_ids(unsigned int num_of_ids);
void add_to_queue(unsigned long button_id);
void play_from_queue();


#endif
