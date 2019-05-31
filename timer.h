#ifndef TIMER_H_
#define TIMER_H_

#include "main.h"

#define TIMER_SYN_MAX_LEN (32*500)
#define TIMER_SYN_MIN_LEN (32*300)
#define TIMER_RCV_BIT_LEN 24
#define TIMER_RCV_TELEGRAM 3
#define TIMER_HALF_STEP (timer_poll_rate>>3)
#define TIMER_STEP (timer_poll_rate>>2)

typedef enum
{
    off,
    idle,
    syn,
    read,
    flag
} timer_state_t;
typedef enum
{
    error = 0,
    zero =  0b1000,
    one =   0b1110,
} rcv_bit_t;

rcv_bit_t timer_rcv_decode [TIMER_RCV_BIT_LEN];
rcv_bit_t timer_rcv_buffer [TIMER_RCV_BIT_LEN];


unsigned long timer_rcv_transmission;

timer_state_t timer_state;
unsigned int timer_poll_rate; // pulse width/2
unsigned int timer_poll_mod;
unsigned int timer_poll_count;
unsigned int timer_rcv_periods;
unsigned int timer_rcv_index;

void timera_init(void);
void start_timera();
void stop_timera();

unsigned long timer_get_transmission();
void timer_push();
int timer_decode();


#endif
