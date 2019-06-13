#ifndef PINS_H_
#define PINS_H_
#include <msp430.h>
#include "main.h"
#define LED_R BIT1
#define LED_G BIT3
#define LED_B BIT5

void setup_pins(void);
void set_gpio_p1_high(unsigned int gpio_pin);
void set_gpio_p1_low(unsigned int gpio_pin);
void turn_on_led(unsigned int led_pin);
void turn_off_led(unsigned int led_pin);
void toggle_led(unsigned int led_pin);

#endif
