#ifndef TIMER_H_
#define TIMER_H_

#include "main.h"

#define TIMER_SYN_MAX_LEN (32*500)
#define TIMER_SYN_MIN_LEN (32*300)
#define TIMER_RCV_BIT_LEN 24
#define TIMER_RCV_TELEGRAM 3
#define TIMER_HALF_PULSE (timer_rcv_rate>>3)//half pulse
#define TIMER_PULSE (timer_rcv_rate>>2)//pulse
#define TIMER_RCV_ZERO    0b1000
#define TIMER_RCV_ONE     0b1110

typedef enum
{
    off,
    idle,
    syn,
    read,
    flag
} timer_state_t;

char timer_rcv_decode [TIMER_RCV_BIT_LEN];
char timer_rcv_buffer [TIMER_RCV_BIT_LEN];


unsigned long timer_rcv_transmission;

volatile timer_state_t timer_state;
volatile unsigned int timer_rcv_index;
volatile unsigned int timer_poll_count;
unsigned int timer_rcv_rate; //4 pulses
unsigned int timer_rcv_periods;

void timera_init(void);
inline void start_timera();
void stop_timera();

void timer_push();
int timer_decode();


#endif
