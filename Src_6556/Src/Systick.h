#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void systick_init(uint32_t ticks);
void print_uart(const char *str);
void led_init(void);
void led_on(void);
void led_off(void);
void delay(uint32_t count);
#endif
