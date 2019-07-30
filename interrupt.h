#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "main.h"

#ifdef __MSP430G2553
void TimerA01(void);
void TimerA00(void);
void TIMERA11(void);
void ISDtxRdy(void);
void ISDrxRdy(void);
#endif

#ifdef __msp430fr2355_H__
void TimerB0(void);
void TIMERB1(void);
void ISDisr(void);
#endif



#endif /* INTERRUPT_H_ */
