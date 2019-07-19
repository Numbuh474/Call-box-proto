#ifndef PINS_H_
#define PINS_H_
#include <msp430.h>
#include "main.h"

//////////////////////////////////////////////////////////
#ifdef __MSP430G2553
#define LED_R BIT1
#define LED_G BIT3
#define LED_B BIT5
//GPIO definitions
//INPUTS
#define GPIO_BUTTON(x)           (BIT((int)x<<1))//2.0, 2.2, 2.4, and 2.6
//OUTPUTS
//#define GPIO_AUDIO_REC1_ENABLE   BIT3  //P1.4 >> NONE
//#define GPIO_AUDIO_CHAN1_ENABLE  BIT2  //P1.7 >> NONE
#define GPIO_STATUS_LED          LED_R  //BIT6  //P1.6 >> P2.1 (R)
#define GPIO_RF_ACTIVITY_LED     LED_G  //BIT0  //P1.0 >> P2.3 (G)
#define GPIO_ERROR_LED           LED_B  //P2.5

//SPECIAL
#define GPIO_RF_INPUT            BIT1  //P1.1 ((must stay))
#define GPIO_USCI_SS             BIT4  //P1.4
#define GPIO_USCI_CLK            BIT5  //P1.5
#define GPIO_USCI_MISO           BIT6  //P1.6
#define GPIO_USCI_MOSI           BIT7  //P1.7
#endif //__MSP430G2553
//////////////////////////////////////////////////////////

#ifdef __msp430fr2355_H__
#define LED_R BIT0
#define LED_G BIT0
#define LED_B BIT0
//PORT 1
#define GPIO_STATUS_LED         BIT0    //p1.0
#define GPIO_RF_ACTIVITY_LED    BIT0
#define GPIO_ERROR_LED          BIT0
#define GPIO_USCI_CLK           BIT1    //p1.1
#define GPIO_USCI_MOSI          BIT2    //p1.2
#define GPIO_USCI_MISO          BIT3    //p1.3
#define GPIO_USCI_SS            BIT4    //p1.4
//#define GPIO_INTERRUPT          BIT5    //p1.5 TODO:
#define GPIO_RF_INPUT           BIT6    //p1.6
#define
//PORT 2
#define GPIO_BUTTON(x)          (BIT4<<x) //p2.4,2.5,2.6,2.7

#endif
///////////////////////////////////////////////////////////

void setup_pins(void);
void set_gpio_p1_high(unsigned int gpio_pin);
void set_gpio_p1_low(unsigned int gpio_pin);
void turn_on_led(unsigned int led_pin);
void turn_off_led(unsigned int led_pin);
void toggle_led(unsigned int led_pin);



#endif
