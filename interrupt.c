#include "interrupt.h"

#ifdef __MSP430G2553
//CCR1
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA01(void)
{
    unsigned int taiv = TAIV;
    switch (timer_state)
    {
    case syn:
    {
        timer_state = idle;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2;
        break;
    }
    case read:
    {
        if (timer_rcv_index < TIMER_RCV_BIT_LEN && (taiv && TA0IV_TACCR1))
        {
            timer_push(TACCTL0 & CCI);
            TACCR1 +=TIMER_PULSE;
        }

        else if (timer_rcv_index >= TIMER_RCV_BIT_LEN)
        {
            __enable_interrupt();
            if (timer_decode())
                timer_state = flag;
            else
            {
                timer_state = idle;
                //rising, CCIxA, sync, capture, interrupt
                TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                TACCTL1 = CM_0 | CCIS_0 | SCS;
                TACTL = TASSEL_2 | ID_0 | MC_2;
            }
        }
        break;
    }
    case flag:
    {
        TACTL = MC_0 | TACLR;
        break;
    }
    default:
    {
        timer_state = idle;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2;
        halt(timerRFError);
    }
    }
}

//TACCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA00(void)
{
    switch (timer_state)
    {
    case idle:
    {
        TACTL = TASSEL_2 | ID_0 | MC_1 | TAIE | TACLR;
        timer_state = syn;
        TACCR0 = TIMER_SYN_MAX_LEN;
        //rising edge, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        break;
    }
    case syn:
    {
        timer_rcv_rate  = TACCR0;
        if (timer_rcv_rate >= TIMER_SYN_MIN_LEN)
        {
            TACTL = TASSEL_2 | ID_0 | MC_2 | TACLR | TAIE;
            timer_rcv_rate = timer_rcv_rate >> 3;
            TACCR1 = TIMER_HALF_PULSE;
            //rising, CCIxA, sync, capture, interrupt
            TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            //no capture, CCIxA, sync, compare, interrupt
            TACCTL1 = CM_3 | CCIS_0 | SCS |/*| CAP |*/ CCIE;
            timer_rcv_index = 0;
            timer_poll_count = 0;
            timer_state = read;
        }
        else
        {
            timer_state = idle;
            //rising, CCIxA, sync, capture, interrupt
            TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            TACCTL1 = CM_0 | CCIS_0 | SCS;
            TACTL = TASSEL_2 | ID_0 | MC_2;
        }
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        break;
    }
    case read:
    {
        timer_rcv_rate = TACCR0;
        TACCR0 = 0xffff;
        TACTL |= TACLR;
        TACCR1 = TIMER_HALF_PULSE;
        break;
    }
    case flag:
    {
        TACTL = MC_0 | TACLR;
        break;
    }
    case off:
    {
        break;
    }
    default:
    {
        timer_state = idle;
        TACTL = MC_0 | TACLR;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2 | TACLR;
        halt(timerRFError);
    }
    }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMERA11 (void)
{
    __enable_interrupt();
    unsigned int taiv = TA1IV;
    switch (taiv)
    {
    case TA1IV_TAIFG:
    {
        unsigned int i;
        if (FLAG(timer_enable,0))
        {
            if (timer_msec[0]==0)
            {
                TA1CCTL1 = CM_0 | CCIS_2 | CCIE;
                TA1CTL |= TACLR;
                TA1CTL = TASSEL_2 | ID_0 | MC_1 | TAIE;
            }
            if (timer_msec[0])
                timer_msec[0]--;
        }

        for (i = 1; i < TIMER_RESOURCE; i++)
        {
            if (FLAG(timer_enable,i))
                timer_msec[i]++;
        }
        break;
    }
    case TA1IV_TACCR1:
    {
        CLEAR_FLAG(timer_enable,0);
        TA1CCTL1 &= ~(CCIE);
        break;
    }
    default:
    {
        halt(timerError);
    }
    }
}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void ISDtxRdy (void)
{
        UCB0TXBUF = isd_tx[isd_tx_index++];
        if (isd_tx_index >= isd_cmd_len)
        {
            UCB0IE &= ~UCTXIE;
            if (!(UCB0IE & UCRXIE))
                P1OUT |= GPIO_USCI_SS;
        }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void ISDrxRdy (void)
{
    isd_rx[isd_rx_index++] = UCB0RXBUF;
    if (isd_rx_index >= isd_cmd_len)
    {
        UCB0IE &= ~UCRXIE;
        if (!(UCB0IE & UCTXIE))
            P1OUT |= GPIO_USCI_SS;
    }
}
#endif


///////////////////////////////////////////////////////////////////////////////
#ifdef __msp430fr2355_H__

#pragma vector = TIMER0_B0_VECTOR
__interrupt void TimerB0T(void)
{
    halt(timerRFError);
}
#pragma vector = TIMER0_B1_VECTOR
__interrupt void TimerB0(void)
{
    unsigned int tbiv = TB0IV & 0x0F;
    switch (timer_state)
    {
    case idle:
    {
        //rising edge found.
        //set overflow value
            //TB0CCR0 = TIMER_SYN_MAX_LEN;
        //reset TBR and set r1 to capture next signal
        TB0CTL = MC_0;
        TB0CTL |= TBCLR;
        TB0CTL = TBSSEL_2 | ID_0 | MC_1 | TBIE;
        //go to syn mode.
        timer_state = syn;
        break;
    }
    case syn:
    {
        TB0CTL = MC_0;
        TB0CTL |= TBCLR;
        TB0CTL = TBSSEL_2 | ID_0 | MC_1 | TBIE;
        //capture read before overflow.
        if (tbiv == TBIV__TBCCR1)
        {
            //if timestamp is long enough, go to read mode.
            if (TB0CCR1 >= TIMER_SYN_MIN_LEN)
            {
                //get pulse length measurement. timer_rcv_rate = 32/8 = 4 pulses.
                timer_rcv_rate = TB0CCR1 >> 3;
                //Rising edges mark the start of a new bit.
                //TB0CCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                //set counter to 1+half a pulse (push), begin counting for compare event
                TB0CCR2 = TIMER_HALF_PULSE+TIMER_PULSE;
                TB0CCTL2 = CM_0 | OUTMOD_0 | CCIE;

                //set variables
                timer_rcv_index = 0;
                timer_poll_count = 0;
                timer_rcv_buffer[0] = 0;

                timer_state = read;
                timer_push(1);
            }
            //if too short, tb0ctl reset at top causes this routine to run again on next ovf/capture
        }

        //overflow detected. Reset to idle here.
        else if (tbiv == TBIV__TBIFG)
        {
            TB0CTL = MC_0;
            TB0CTL |= TBCLR;
            TB0CTL = TBSSEL_2 | ID_0 | MC_1;
            timer_state = idle;
        }
        break;
    }
    case read:
    {
        //on compare event, take sample and advance counter by 1 pulse
        if (tbiv == TBIV__TBCCR2)
        {
            timer_push(TB0CCTL1 & CCI);
            TB0CCR2 += TIMER_PULSE;
            /*if (timer_poll_count == 0)
            {
                TB0CCR2 = 0xFFFF;
            }*/
        }
        //on rising edge, reset timer and sampling rate, and set half pulse again
        else if (    tbiv == TBIV__TBCCR1
                /*&& TB0CCR1 >= TIMER_SYN_MIN_LEN/32*4
                && TB0CCR1 <= TIMER_SYN_MIN_LEN/32*4    */)
        {
            TB0CTL = MC_0;
            TB0CTL |= TBCLR;
            TB0CTL = TBSSEL_2 | ID_0 | MC_1 | TBIE;

            //timer_rcv_rate = (TB0CCR1+timer_rcv_rate)>>1;

            if (timer_poll_count == TIMER_RCV_SAMPLES-1)
                timer_push(0);

            timer_push(1);
            TB0CCR2 = TIMER_PULSE+TIMER_HALF_PULSE;
        }
        //on overflow save data and reset to idle
        else if (tbiv == TBIV__TBIFG)
        {
            timer_decode();
            TB0CTL = MC_0;
            TB0CTL |= TBCLR;
            TB0CTL = TBSSEL_2 | ID_0 | MC_1;
            TB0CCTL2 &= ~(CCIE);
            timer_state = idle;
        }

        //if buffer is full, check if valid bit sequence.
        if (timer_rcv_index >= TIMER_RCV_BIT_LEN)
        {
            //__enable_interrupt();
            if (timer_decode())
            {
                timer_state = flag;//success
            }
            else
            {
                TB0CTL = MC_0;
                TB0CTL |= TBCLR;
                TB0CTL = TBSSEL_2 | ID_0 | MC_1;
                timer_state = idle;//failure
            }
        }
        break;
    }
    //stop timer
    case flag:
    case off:
    {
        //stop timer
        TB0CTL = MC_0;
        break;
    }

    default:
    {
        //reset to idle.
        TB0CTL = MC_0;
        TB0CTL |= TBCLR;
        TB0CTL = TBSSEL_2 | ID_0 | MC_1;
        TB0CCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TB0CCTL2 = CM_0;
        timer_state = idle;
    }
    }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMERB1 (void)
{
    //__enable_interrupt();
    unsigned int tbiv = TB1IV & 0x0F;
    if (tbiv == TBIV__TBCCR1)
    {
        CLEAR_FLAG(timer_enable,0);
        TB1CCTL1 &= ~(CCIE);
    }

    else if (tbiv == TBIV__TBIFG)
    {
        unsigned int i;
        if (FLAG(timer_enable,0))
        {
            if (timer_msec[0]==0)
            {
                TB1CCTL1 = CM_0 | CCIS_2 | CCIE;
                TB1CTL |= TBCLR;
                TB1CTL = TBSSEL_2 | ID_0 | MC_1 | TBIE;
            }
            if (timer_msec[0])
                timer_msec[0]--;
        }

        for (i = 1; i < TIMER_RESOURCE; i++)
        {
            if (FLAG(timer_enable,i))
                timer_msec[i]++;
        }
    }
    else
    {
        //halt(timerError);
    }
}
//#pragma vector = USCIAB0TX_VECTOR
//__interrupt

#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISDisr (void)
{
    unsigned int flag = UCB0IFG;
    //rx get
    if (flag & UCRXIFG)
    {
        isd_rx[isd_rx_index++] = UCB0RXBUF;
        if (isd_rx_index >= isd_cmd_len)
        {
            UCB0IE &= ~UCRXIE;
            if (!(UCB0IE & UCTXIE))
                P1OUT |= GPIO_USCI_SS;
        }
    }
    //tx ready
    if (flag & UCTXIFG)
    {
        UCB0TXBUF = isd_tx[isd_tx_index++];
        if (isd_tx_index >= isd_cmd_len)
        {
            UCB0IE &= ~UCTXIE;
            if (!(UCB0IE & UCRXIE))
                P1OUT |= GPIO_USCI_SS;
        }
    }
}
#endif


