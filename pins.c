#include <msp430.h>
#include <pins.h>

#ifdef __MSP430G2553
void setup_pins(void)
{
     // set P1 GPIOs
    P1SEL &= ~(GPIO_AUDIO_CHAN1_ENABLE | GPIO_AUDIO_REC1_ENABLE);
    // intialize output pins
    P1OUT |= GPIO_USCI_SS;
    P1OUT &= ~(GPIO_AUDIO_CHAN1_ENABLE | GPIO_AUDIO_REC1_ENABLE);
    // set up output pins
    P1DIR |= GPIO_AUDIO_CHAN1_ENABLE | GPIO_AUDIO_REC1_ENABLE | GPIO_USCI_SS ;
    //set P1 input pins
    P1DIR &= ~(GPIO_RF_INPUT);
    //set USCI to rst
    UCB0CTL1 = UCSWRST;
    //set GPIO_RF_INPUT to TA0, GPIO_USCIx to UCB0
    P1SEL |= GPIO_RF_INPUT | GPIO_USCI_CLK | GPIO_USCI_MISO | GPIO_USCI_MOSI;
    P1SEL2 |= GPIO_USCI_CLK | GPIO_USCI_MISO | GPIO_USCI_MOSI;

    // set P2 GPIOs
    P2SEL &= ~(GPIO_BUTTON(0) | GPIO_BUTTON(1) | GPIO_BUTTON(2) | GPIO_BUTTON(3) | LED_R | LED_G | LED_B);
    // set up output pins
    P2DIR |= LED_R | LED_G | LED_B;
    // intialize output pins
    P2OUT &= ~(LED_R | LED_G | LED_B);
    //set input pins
    P2DIR &= ~(GPIO_BUTTON(0) | GPIO_BUTTON(1) | GPIO_BUTTON(2) | GPIO_BUTTON(3));
    //set pull downs
    P2REN &= ~(GPIO_BUTTON(0) | GPIO_BUTTON(1) | GPIO_BUTTON(2) | GPIO_BUTTON(3));
    //setup interrupt
    //P2IES &= ~(GPIO_PROGRAM_BUTTON); // rising Edge 0 -> 1
    //P2IE |= (GPIO_PROGRAM_BUTTON);
    //P2IFG &= ~(GPIO_PROGRAM_BUTTON);

    //disable port 3 for now
    P3DIR = 0xFF;
    P3OUT &= 0x00;

}

void set_gpio_p1_high(unsigned int gpio_pin)
{
    //set output pin high
    P1OUT |= gpio_pin;
}
void set_gpio_p1_low(unsigned int gpio_pin)
{
    //set output pin low
    P1OUT &= ~gpio_pin;
}

void turn_on_led(unsigned int led_pin)
{
    // set pin output
    P2OUT |= led_pin;
}

void turn_off_led(unsigned int led_pin)
{
    // clear pin output
    P2OUT &= ~led_pin;
}

void toggle_led(unsigned int led_pin)
{
    // toggle pin of P1
    P2OUT ^= led_pin;
}
#endif

//////////////////////////////////////////////////
#ifdef __msp430fr2355_H__
void setup_pins(void)
{
    //PORT 1
    P1OUT   = GPIO_USCI_SS | GPIO_USCI_MOSI;//OUTPUTS HIGH
  //P1OUT   &= ~(GPIO_STATUS_LED | GPIO_INTERR);//OUTPUTS LOW
    P1DIR   = GPIO_USCI_SS | GPIO_USCI_MOSI | GPIO_USCI_CLK | GPIO_STATUS_LED;//SELECT OUTPUTS
  //P1DIR   &= ~(GPIO_USCI_MISO | GPIO_RF_INPUT);//SELECT INPUTS
    P1SEL0   = GPIO_USCI_CLK | GPIO_USCI_MOSI | GPIO_USCI_MISO;
    P1SEL1  = GPIO_RF_INPUT;
    P1REN   &=  ~(GPIO_USCI_MISO | GPIO_RF_INPUT);
    //PORT 2
    P2OUT   = 0;//outputs high
    P2DIR   = 0;//select outputs
  //P2DIR   &= GPIO_BUTTON(0) | GPIO_BUTTON(1) | GPIO_BUTTON(2) | GPIO_BUTTON(3);//select inputs
    P2SEL0   = 0;
    P2SEL1  = 0;
    P2REN   &= ~(GPIO_BUTTON(0) | GPIO_BUTTON(1) | GPIO_BUTTON(2) | GPIO_BUTTON(3));
}
void set_gpio_p1_high(unsigned int gpio_pin)
{
    P1OUT |= gpio_pin;
}
void set_gpio_p1_low(unsigned int gpio_pin)
{
    P1OUT &= ~gpio_pin;
}
void turn_on_led(unsigned int led_pin)
{
    P1OUT |= led_pin;
}
void turn_off_led(unsigned int led_pin)
{
    P1OUT &= ~led_pin;

}
void toggle_led(unsigned int led_pin)
{
    P1OUT ^= led_pin;
}
#endif
