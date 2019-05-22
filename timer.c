#include <msp430.h>
#include <timer.h>

void timer_a_init(void)
{
    if (CALBC1_1MHZ != 0xFF)   //check for calibration constant
    {    
         DCOCTL = 0;
         BCSCTL1 = CALBC1_1MHZ;
         DCOCTL  = CALDCO_1MHZ;
        
         TACCR0 = 0;
        
         // Rising edge + CCI0A (P1.1)
         // + Capture Mode
         CCTL0 = CM_3 + SCS + CCIS_0 + CAP;

         //SMCLK, SMCLK/1, Continuous mode
         TACTL = TASSEL_2 + ID_0 + MC_2;
    }
}

void start_timera(void)
{
    TACTL = TASSEL_2 + ID_0 + MC_2;
    CCTL0 |= CCIE;
}

void stop_timera(void)
{
    TACTL = MC_0;
    CCTL0 &= ~(CCIE);
}

