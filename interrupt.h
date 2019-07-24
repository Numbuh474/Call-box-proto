#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "main.h"

#ifdef __MSP430G2553
void TimerA01(void);
void TimerA00(void);
void TIMERA11(void);
#endif

#ifdef __msp430fr2355_H__
void TimerB01(void);
void TimerB00(void);
void TIMERB11(void);
void ISDisr(void);
#endif

void ISDtxRdy(void);
void ISDrxRdy(void);

#endif /* INTERRUPT_H_ */
