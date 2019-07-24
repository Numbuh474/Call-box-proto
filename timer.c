#include "timer.h"

void timer_push(unsigned int signal)
{
    //clear index on new word
    if(timer_poll_count==0)
    {
        timer_rcv_buffer[timer_rcv_index]= 0;
    }
    timer_rcv_buffer[timer_rcv_index] =
         timer_rcv_buffer[timer_rcv_index] << 1;
    //if signal is true, push set bit. else push clear bit
    if (signal)
    {
        timer_rcv_buffer[timer_rcv_index]++;
        turn_on_led(GPIO_RF_ACTIVITY_LED);
    }
    else//if (!signal)
    {
        turn_off_led(GPIO_RF_ACTIVITY_LED);
    }

    if (timer_poll_count>=TIMER_RCV_TELEGRAM)
    {
         timer_poll_count = 0;
         timer_rcv_index++;
    }
    else
    {
        timer_poll_count++;
    }
}
int timer_decode()
{
    unsigned int i = 0, parity_fail = 0;
    timer_rcv_transmission = 0;
    for(i = 0; i<TIMER_RCV_BIT_LEN; i++)
    {
        //if not new data, then old data is incomplete.
        //try parity check.
        if(timer_rcv_periods > 0)
        {
            if(timer_rcv_buffer[i]==TIMER_RCV_ZERO )
            {
                if (timer_rcv_decode[i]!=TIMER_RCV_ZERO)
                    parity_fail = 1;
            }
            else if (timer_rcv_buffer[i]==TIMER_RCV_ONE)
            {
                if (timer_rcv_decode[i]!=TIMER_RCV_ONE)
                    parity_fail = 1;
            }
        }
        else//if(timer_rcv_periods==0)
            timer_rcv_decode[i]=timer_rcv_buffer[i];
        if (parity_fail)
        {
            //try again as though it were the first signal received
            timer_rcv_periods = 0;
            return timer_decode();
        }
    }//for

    //see if data returns valid.
    //if not, increment periods count
    //if > 3 periods reset period count.
    for(i = 0; i<TIMER_RCV_BIT_LEN; i++)
    {
        if (timer_rcv_decode[i]==TIMER_RCV_ZERO)
            timer_rcv_transmission = (timer_rcv_transmission << 1);
        else if (timer_rcv_decode[i]==TIMER_RCV_ONE)
            timer_rcv_transmission = (timer_rcv_transmission << 1) + 1;
        else//if (timer_rcv_decode[i] != TIMER_RCV_ZERO && timer_rcv_decode[i] != TIMER_RCV_ONE)
        {
            timer_rcv_periods++;
            if (timer_rcv_periods >= TIMER_RCV_TELEGRAM)
            {
                timer_rcv_periods = 0;
            }
            timer_rcv_transmission = BUTTON_ID_INVALID;
            return 0;
        }
    }//for
    timer_rcv_periods = 0;
    return 1;
}
//request a running msec timer. Returns timer id.
int timer_begin( void )
{
    if (!timer_sem)
        return 0;
    unsigned int i;
    int id = 0;
    for (i = 1; id==0 && i<TIMER_RESOURCE; i++)
    {
        if (!FLAG(timer_enable,i))
        {
            timer_sem--;
            id = 100+i;
            timer_msec[i]=0;
            SET_FLAG(timer_enable, i);
        }
    }
    return id;
}
//check on a timer without deactivating it
unsigned int timer_check(int id)
{
    id-=100;
    if (id<0 || id>= TIMER_RESOURCE || !FLAG(timer_enable,id))
        return 0;
    return timer_msec[id];
}
//check on a timer and deactivate it
unsigned int timer_release(int id)
{
    id-=100;
    if (id<0 || id>= TIMER_RESOURCE || !FLAG(timer_enable,id))
        return 0;
    unsigned int result = timer_msec[id];
    CLEAR_FLAG(timer_enable,id);
    timer_msec[id] = 0;
    timer_sem++;
    return result;
}
//wait until a timer reaches x miliseconds, then deactivate it
int timer_release_at(int id, unsigned int msec)
{
    id-=100;
    if (id<0 || id>=TIMER_RESOURCE || !FLAG(timer_enable,id))
        return 0;
    while (timer_msec[id] <= msec)
        ;
    CLEAR_FLAG(timer_enable,id);
    timer_msec[id] = 0;
    timer_sem++;
    return 1;
}
/////////////////////////////////////////////////////////////////////////////
#ifdef __MSP430G2553
//initialize rf module
void timera_init(void)
{
    //SMCLK, SMCLK/1, halt mode, clear TAR and divider, enable interrupt
    //TACTL = TASSEL_2 + ID_0 + MC_0 + TAIE;
    TACTL = TASSEL_2 | ID_0 | MC_0 | TACLR;

    //system clock initialization moved to init_clk();
    //set timer overflow register here;
    TACCR0 = 0xFFFF;
    TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | OUTMOD_0;

    timer_state = off;
    timer_rcv_periods = 0;
}
//start rf module
inline void start_timera()
{
    timer_state = idle;
    //rising, CCIxA, sync, capture, interrupt
    TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
    TACCTL1 = CM_0 | CCIS_0 | SCS;
    TACTL = TASSEL_2 | ID_0 | MC_2;
}
//stop rf module
void stop_timera(void)
{
    timer_state = off;
    TACTL = MC_0 | TACLR;
    TACCTL0 &= ~(CCIE);
}
//wait for x microseconds (inaccurate)
void timer_delay(unsigned int usec)
{
    //timer_msec[0] = 0;
    TA1CTL = TASSEL_2 | ID_0 | MC_0;
    usec += TA1R;
    timer_msec[0] = usec / 1000;
    usec = usec % 1000;
    /*while (usec+TA1R >= 1000)
    {
        usec-=1000;
        timer_msec[0]++;
    }*/
    if (timer_msec[0] == 0)
        TA1CCTL1 = CM_0 | CCIS_2 | CCIE;
    TA1CCR1 = usec;
    SET_FLAG(timer_enable,0);
    TA1CTL = TASSEL_2 | ID_0 | MC_1 | TAIE;
    while (FLAG(timer_enable,0))
        ;
}
//start the msec and usec delay timer.
void start_timera1()
{
    TA1CTL = MC_0 | TACLR;
    timer_enable = 0;
    timer_sem = TIMER_RESOURCE-1;
    TA1CCR0 = 1000-1;
    TA1CTL = TASSEL_2 | ID_0 | MC_1 | TAIE;
    TA1CCTL1 = CM_0 | CCIS_2;
    TA1CCR2 = 0;
    TA1CCTL2 =  CM_0 | CCIS_2;
}
#endif
/////////////////////////////////////////////////////////////////////////////
#ifdef __msp430fr2355_H__
void timera_init(void)
{
    TB0CTL = TBCLGRP_0 | CNTL_0 | TBSSEL_2 | ID_0 | MC_0 | TBCLR | TBIE;
    TB0CCTL0 = CM_1 | CCIS_0 | SCS | CAP | OUTMOD_0;
    TB0CCTL1 = CM_0 | CCIS_0 | SCS;

    timer_state = off;
    timer_rcv_periods = 0;

}
inline void start_timera()
{
    timer_state = idle;
    //rising, CCIxA, sync, capture, interrupt
    TB0CCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
    TB0CCTL1 = CM_0 | CCIS_0 | SCS;
    TB0CTL = TBCLGRP_0 | CNTL_0 | TBSSEL_2 | ID_0 | MC_2;
}
void stop_timera()
{
    TB0CTL = TBCLGRP_0 | CNTL_0 | TBSSEL_2 | ID_0 | MC_0 | TBCLR;
}

void start_timera1()
{
    TB1CTL = MC_0 | TBCLR;
    timer_enable = 0;
    timer_sem = TIMER_RESOURCE-1;
    TB1CCR0 = 1000-1;
    TB1CTL = TBSSEL_2 | CNTL_0 | ID_0 | MC_1 | TBIE;
    TB1CCTL1 = CM_0 | CCIS_2;
    TB1CCR2 = 0;
    TB1CCTL2 =  CM_0 | CCIS_2;
}
void timer_delay(unsigned int usec)
{
     TB1CTL = TBSSEL_2 | ID_0 | MC_0;
     usec += TB1R;
     timer_msec[0] = usec / 1000;
     usec = usec % 1000;
     if (timer_msec[0] == 0)
         TB1CCTL1 = CM_0 | CCIS_2 | CCIE;
     TB1CCR1 = usec;
     SET_FLAG(timer_enable,0);
     TB1CTL = TBSSEL_2 | CNTL_0 | ID_0 | MC_1 | TBIE;
     while (FLAG(timer_enable,0))
         ;
}
#endif
