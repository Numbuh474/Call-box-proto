#include "timer.h"

void timera_init(void)
{
    //SMCLK, SMCLK/1, halt mode
    TACTL = TASSEL_2 + ID_0 + MC_0 + TAIE;

    //system clock initialization moved to init_clk();
    //TACCR0 = 0;

    //--set cc reg 1
    // Rising edge + CCI0A (P1.1)
    // + Capture Mode
    TACCTL1 = CM_3 + SCS + CCIS_0 + CAP;

    //--set cc reg 1
    //no capture + compare mode

    TACCTL0 = CM_0 | OUTMOD_0 | CCIS_0 | SCS;
    //local vars
    timer_state = off;
    timer_rcv_periods = 0;
}
void start_timera()
{
    timer_state = idle;
    start_timera_capture();
}
void start_timera_capture( void )
{
    //TACTL = TASSEL_2 + ID_0 + MC_2;
    //current settings + up mode
    TAR = 0;
    TACCTL1 |= CCIE;
    TACCTL0 &= ~(CCIE);
    TACTL |= MC_2;
}

void start_timera_compare(unsigned int period)
{
    TACCR0 = period;
    TAR = 0;
    TACCTL1 &= !(CCIE);
    TACCTL0 |= CCIE;
    TACTL |= MC_1;
}

void start_timera_capturecompare(unsigned int period)
{
    TACCR0 = period;
    TAR = 0;
    TACCTL0 |= CCIE;
    TACCTL1 |= CCIE;
    TACTL |= MC_1;
}

//TODO:replace with timera_init
void stop_timera(void)
{
    TACTL = MC_0;
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
    //check on every odd poll (middle of the pulse)
    if( timer_poll_count%2==1 )
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
    //roll poll count over on 7.
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
            else
                timer_rcv_decode[i]=timer_rcv_buffer[i];
        }
        //if first data, copy info
        else
            timer_rcv_decode[i]=timer_rcv_buffer[i];
        //if parity fails, maybe new signal.
        //reset signal and run self.
        if (parity_fail)
        {
            timer_rcv_periods = 0;
            return timer_decode();
        }
    }
    //see if data returns valid.
    //if not, increment periods count
    //if > 3 periods reset period count.
    timer_rcv_transmission = 0;
    for(i = 0; i<=TIMER_RCV_BIT_LEN; i++)
    {
        if (timer_rcv_decode[i] != zero && timer_rcv_decode[i] != one)
        {
            timer_rcv_periods = (timer_rcv_periods+1)%3;
            timer_rcv_transmission = 0xFF000000;
            return 0;
        }
        else if (timer_rcv_decode[i]==zero)
            timer_rcv_transmission = (timer_rcv_transmission << 1);
        else if (timer_rcv_decode[i]==one)
            timer_rcv_transmission = (timer_rcv_transmission << 1) + 1;
    }
    return 1;
}
unsigned long timer_get_transmission()
{
    if (timer_state!=flag)
        return BUTTON_ID_INVALID;
    timer_rcv_periods = 0;
    timer_state = idle;
    timera_init();
    start_timera_capture();
    return timer_rcv_transmission;
}


