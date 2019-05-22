#include <msp430.h>
#include <pins.h>

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

void turn_on_p1_led(unsigned int led_pin)
{
    // set pin output
    P1OUT |= led_pin;
}

void turn_off_p1_led(unsigned int led_pin)
{
    // clear pin output
    P1OUT &= ~led_pin;
}

void toggle_p1_led(unsigned int led_pin)
{
    // toggle pin of P1
    P1OUT ^= led_pin;
}
