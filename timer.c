#include "timer.h"

void timera_init(void)
{
    //SMCLK, SMCLK/1, halt mode, clear TAR and divider, enable interrupt
    //TACTL = TASSEL_2 + ID_0 + MC_0 + TAIE;
    TACTL = TASSEL_2 | ID_0 | MC_0 | TACLR | TAIE;

    //system clock initialization moved to init_clk();
    //set timer overflow register here;
    TACCR0 = 0xFFFF;
    TACCTL0 = CM_0 | CCIS_0 | SCS;
    TACCTL1 = CM_1 | CCIS_0 | SCS | CAP | OUTMOD_0;

    timer_state = off;
    timer_rcv_periods = 0;
}
void start_timera()
{
    timer_state = idle;
    start_timera_capture();
}
void start_timera_capture()
{
    //ENABLE cap interrupt and DISABLE ovf, cont mode
    TACTL &= ~(TAIE);
    TACCTL1 |= CCIE;
    TACTL |= TACLR | MC_2;
}
void start_timera_capturecompare(unsigned int period)
{
    //ENABLE cap and ovf, up mode
    TACCR0 = period;
    TACCTL1 |= CCIE;
    TACTL |= TACLR | TAIE | MC_1;
}
void start_timera_compare(unsigned int period)
{
    //enable ovf and DISABLE cap, up mode
    TACCTL1 &= ~(CCIE);
    TACCR0 = period;
    TACTL |= TACLR | TAIE | MC_1;
}

//TODO:replace with timera_init
void stop_timera(void)
{
    timer_state = off;
    TACTL = TASSEL_2 | ID_0 | MC_0 | TACLR | TAIE;
    TACCTL0 &= ~(CCIE);
    TACCTL1 &= ~(CCIE);
}

void timer_push(unsigned int signal)
{
    //clear index on new word
    if(timer_poll_count==0)
    {
        timer_rcv_buffer[timer_rcv_index]= error;
    }
    //check on every even poll (middle of the pulse)
    if( timer_poll_count%2==0 )
    {
         timer_rcv_buffer[timer_rcv_index] =
                 timer_rcv_buffer[timer_rcv_index] << 1;
         //if signal is true, push set bit. else push clear bit
         if (signal)
         {
             timer_rcv_buffer[timer_rcv_index]++;
             P1OUT |= GPIO_RF_ACTIVITY_LED;
             toggle_led_index++;
         }
         else if (!signal)
         {
             P1OUT &= ~(GPIO_RF_ACTIVITY_LED);
             toggle_led_index++;
         }
    }
    //roll poll count over and advance index on 7.
    if (timer_poll_count>=7)
    {
         timer_poll_count = 0;
         timer_rcv_index++;
    }
    else
    {
        timer_poll_count++;
    }

     //if(!(signal) & timer_poll_count%2==1)//noop
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
            timer_rcv_periods = (timer_rcv_periods+1)%3;
            timer_rcv_transmission = BUTTON_ID_INVALID;
            return 0;
        }
    }//for
    return 1;
}
unsigned long timer_get_transmission()
{
    timer_rcv_periods = 0;
    timer_state = idle;
    start_timera_capture();
    return timer_rcv_transmission;
}


