#include "timer.h"

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
void start_timera()
{
    timer_state = idle;
    //rising, CCIxA, sync, capture, interrupt
    TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
    TACTL = TASSEL_2 | ID_0 | MC_2;
}

void stop_timera(void)
{
    timer_state = off;
    TACTL = MC_0 | TACLR;
    TACCTL0 &= ~(CCIE);
    //TACCTL1 &= ~(CCIE);
}

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
     turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
     toggle_led_index++;
    }
    else//if (!signal)
    {
     turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
     toggle_led_index++;
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
    int i = 0, parity_fail = 0;
    timer_rcv_transmission = 0;
    for(i = 0; i<=TIMER_RCV_BIT_LEN; i++)
    {
        //if not new data, then old data is incomplete.
        //try parity check.
        if(timer_rcv_periods > 0)
        {
            if(timer_rcv_decode[i]==zero )
                if (timer_rcv_buffer[i]!=zero)
                    parity_fail = 1;
            else if (timer_rcv_decode[i]==one)
                if (timer_rcv_buffer[i]!=one)
                    parity_fail = 1;
            else//if (timer_rcv_decode==error or otherwise)
                timer_rcv_decode[i]=timer_rcv_buffer[i];
        }
        //if first data, copy info
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
    for(i = 0; i<=TIMER_RCV_BIT_LEN; i++)
    {
        if (timer_rcv_decode[i]==zero)
            timer_rcv_transmission = (timer_rcv_transmission << 1);
        else if (timer_rcv_decode[i]==one)
            timer_rcv_transmission = (timer_rcv_transmission << 1) + 1;
        else//if (timer_rcv_decode[i] != zero && timer_rcv_decode[i] != one)
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
    return 1;
}
unsigned long timer_get_transmission()
{
    unsigned long result = timer_rcv_transmission;
    start_timera();
    return result;
}


