#ifndef MAIN_H_
#define MAIN_H_

#include <msp430.h>
#include <flash.h>
#include <pins.h>
#include <timer.h>
#include <stddef.h>
#include "queue.h"
#include "isd.h"

#define COLOR_  0
#define RED     0b00000111
#define GREEN   0b00111000
#define BLUE    0b11000000
#define BIT(x) (0x1<<(x))
#define FLAG(x,y) ((x&BIT(y))!=0)
#define SET_FLAG(x,y) x|=BIT(y)
#define CLEAR_FLAG(x,y) x&=~BIT(y)

//Audio Channel Defines
//TODO: allow for multiple audio channels
#define AUDIO_CHANNEL_NONE 0
#define AUDIO_CHANNEL_1 1
#define AUDIO_CHANNEL_2 2
#define AUDIO_CHANNEL_3 3
#define AUDIO_CHANNEL_4 4
#define AUDIO_CHANNEL(x) (x)
#define AUDIO_CHANNEL_TOTAL 4
#define BUTTON_ID_INVALID 0xFF000000

//GPIO definitions
//INPUTS
//#define GPIO_PROGRAM_BUTTON      BIT0  //P2.0
//#define GPIO_ERASE_BUTTONS       BIT2  //P2.2
//#define GPIO_CHAN1_REC_BUTTON    BIT4  //P2.4
//#define GPIO_PROGRAM_BUTTON_1    BIT6  //P2.1 >> P2.6
#define GPIO_BUTTON(x)           (BIT((int)x<<1))
//OUTPUTS
#define GPIO_AUDIO_REC1_ENABLE   BIT3  //P1.4 >> NONE
#define GPIO_AUDIO_CHAN1_ENABLE  BIT2  //P1.7 >> NONE
#define GPIO_STATUS_LED          LED_R  //BIT6  //P1.6 >> P2.1 (R)
#define GPIO_RF_ACTIVITY_LED     LED_G  //BIT0  //P1.0 >> P2.3 (G)
#define GPIO_ERROR_LED           LED_B  //P2.5

//SPECIAL
#define GPIO_RF_INPUT            BIT1  //P1.1 ((must stay))
#define GPIO_USCI_SS             BIT4  //P1.4
#define GPIO_USCI_CLK            BIT5  //P1.5
#define GPIO_USCI_MISO           BIT6  //P1.6
#define GPIO_USCI_MOSI           BIT7  //P1.7

unsigned int toggle_led_index;
unsigned int p2_gpio_int_state;
unsigned int program_mode_active;
unsigned int program_button_active;
unsigned int program_button_target;
unsigned int button_1_programmed;
unsigned int message_length;
unsigned int can_play;

flash_data_struct_t button_id_list;
struct Queue id_queue;
queue_t id_queue_array [8];


void init_clk();
unsigned int add_decode_transition(unsigned int new_timestamp);
unsigned long get_call_button_id(void);
unsigned int get_next_decoded_bit(unsigned int decode_index);
void reset_decoder(void);
unsigned long call_button_received(void);
void run_program(void);
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
void play_audio(unsigned int audio_channel)
void halt();


#endif
