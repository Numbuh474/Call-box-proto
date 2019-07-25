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
        halt();
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
        halt();
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
        halt();
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
//CCR1
#pragma vector = TIMER0_B1_VECTOR
__interrupt void TimerB01(void)
{
    unsigned int tbiv = TB0IV & 0xf;
    switch (timer_state)
    {
    case idle:
    {
        //rising edge detected on CCI1A
        //count up to ccr0 for ovf
        TB0CCR0 = TIMER_SYN_MAX_LEN;
        TB0CTL = TBSSEL_2 | ID_0 | MC_1 | TBIE | TBCLR;
        timer_state = syn;
        //capture on ccr1, ignore ccr2
        TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TBCCTL2 = CM_0 | CCIS_0 | SCS;
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        break;
    }
    case syn:
    {
        //rising edge detected on CC1A
        if (tbiv & TBIV__TBCCR1)
        {
            //valid sync signal received, move to read state
            if (TB0CCR1 >= TIMER_SYN_MIN_LEN)
            {
                //detect timeout on ccr0
                TB0CTL = TBSSEL_2 | ID_0 | MC_2 | TBCLR | TBIE;
                timer_rcv_rate = TB0CCR0 >> 3;//1*len

                //search for rising edge on ccr1
                TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                //wait for HALF_PULSE in ccr2
                TB0CCR2 = TIMER_HALF_PULSE;
                TBCCTL2 = CM_3 | CCIS_0 | SCS |/*| CAP |*/ CCIE;
                timer_rcv_index = 0;
                timer_poll_count = 0;
                timer_rcv_k[0] = timer_rcv_rate;
                timer_state = read;
            }
            else //short sync signal received, reset
            {
                timer_state = idle;
                //detect rising edges on ccr1, ignore ccr2
                TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                TBCCTL2 = CM_0 | CCIS_0 | SCS;
                TB0CTL = TBSSEL_2 | ID_0 | MC_2;
            }
            turn_off_led(GPIO_RF_ACTIVITY_LED);
        }
        //overflow detected, signal past max allowable time
        else if (tbiv & TBIV__TBIFG)
        {
            timer_state = idle;
            //detect rising edges on ccr1, ignore ccr2
            TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            TBCCTL2 = CM_0 | CCIS_0 | SCS;
            TB0CTL = TBSSEL_2 | ID_0 | MC_2;
        }
        break;
    }
    case read:
    {
        //rising edge detected, new pulse was generated
        if ((tbiv & TBIV__TBCCR1)
                && timer_poll_count == 0)
        {
            TB0CTL |= TBCLR;
            timer_rcv_rate = (timer_rcv_rate +TB0CCR1)>>1;
            timer_rcv_k[timer_rcv_index] = timer_rcv_rate;
            TB0CCR2 = TIMER_HALF_PULSE+1;
        }
        //sampling point identified
        else if (timer_rcv_index < TIMER_RCV_BIT_LEN && (tbiv & TB0IV_TB0CCR2))
        {
            //save current sample and increment HALF_PULSE counter
            TB0CCR2 += TIMER_PULSE;
            timer_push(TBCCTL1 & CCI);
        }
        //overflow detected, reset to idle
        else if (tbiv & TBIV__TBIFG)
        {
            timer_state = idle;
            //detect rising edges on ccr1, ignore ccr2
            TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            TBCCTL2 = CM_0 | CCIS_0 | SCS;
            TB0CTL = TBSSEL_2 | ID_0 | MC_2;
        }
        //timer_rcv_index maxed out
        if (timer_rcv_index >= TIMER_RCV_BIT_LEN)
        {
            __enable_interrupt();
            if (timer_decode())
                timer_state = flag;
            else
            {
                timer_state = idle;
                //detect rising edges on ccr1, ignore ccr2
                TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                TBCCTL2 = CM_0 | CCIS_0 | SCS;
                TB0CTL = TBSSEL_2 | ID_0 | MC_2;
            }
        }
        break;
    }
    case flag:
    {
        TB0CTL = MC_0 | TBCLR;
        break;
    }
    case off:
    {
        TB0CTL = MC_0 | TBCLR;
        break;
    }
    default:
    {
        timer_state = idle;
        //detect rising edges on ccr1, ignore ccr2
        TBCCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TBCCTL2 = CM_0 | CCIS_0 | SCS;
        TB0CTL = TBSSEL_2 | ID_0 | MC_2;
    }
    }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMERB11 (void)
{
    //__enable_interrupt();
    unsigned int tbiv = TB1IV & 0xF;
    switch (tbiv)
    {
    case TB1IV_TBIFG:
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
        break;
    }
    case TB1IV_TBCCR1:
    {
        CLEAR_FLAG(timer_enable,0);
        TB1CCTL1 &= ~(CCIE);
        break;
    }
    default:
    {
        //halt();
    }
    }
}
//#pragma vector = USCIAB0TX_VECTOR
//__interrupt
void ISDtxRdy (void)
{

}

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


