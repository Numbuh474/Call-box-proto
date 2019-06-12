#ifndef TIMER_H_
#define TIMER_H_

#include "main.h"

#define TIMER_SYN_MAX_LEN (32*500)
#define TIMER_SYN_MIN_LEN (32*300)
#define TIMER_RCV_BIT_LEN 24
#define TIMER_RCV_TELEGRAM 3
#define TIMER_HALF_PULSE (timer_rcv_rate>>3)
#define TIMER_PULSE (timer_rcv_rate>>2)
#define TIMER_RCV_ZERO    0b1000
#define TIMER_RCV_ONE     0b1110
#define TIMER_RESOURCE    16

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
unsigned int timer_usec;
unsigned int timer_msec [TIMER_RESOURCE];

unsigned long timer_rcv_transmission;

volatile timer_state_t timer_state;
volatile unsigned int timer_rcv_index;
volatile unsigned int timer_poll_count;
unsigned int timer_rcv_rate; //4 pulses
unsigned int timer_rcv_periods;
volatile unsigned int timer_enable;
volatile unsigned int timer_sem;

void timera_init(void);
inline void start_timera();
void stop_timera();

void timer_push();
int timer_decode();

void start_timera1();
void timer_delay(unsigned int usec);
int timer_begin();
unsigned int timer_check(int id);
unsigned int timer_release(int id);
int timer_release_at(int id, unsigned int msec);


#endif
