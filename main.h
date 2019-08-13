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

typedef enum
{
    noError,
    undefinedError,
    memoryReadError,
    memoryWriteError,
    isdTransmitError,
    isdTimeoutError,
    isdStallError,
    timerError,
    timerRFError,
} error_t;

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
void play_audio(unsigned int audio_channel);
void halt(error_t error);


#endif
