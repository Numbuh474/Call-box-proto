#ifndef PINS_H_
#define PINS_H_
#include <msp430.h>

void set_gpio_p1_high(unsigned int gpio_pin);
void set_gpio_p1_low(unsigned int gpio_pin);
void turn_on_p1_led(unsigned int led_pin);
void turn_off_p1_led(unsigned int led_pin);
void toggle_p1_led(unsigned int led_pin);

#endif
