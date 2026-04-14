#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

void interrupt_init(void);

extern volatile unsigned int kbd_int_count;
extern volatile unsigned long long ticks;

void kbd_buffer_put(uint8_t sc);
int kbd_buffer_empty(void);
uint8_t kbd_buffer_get(void);

#endif