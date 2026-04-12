#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

void pic_init(void);
void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);
void pic_send_eoi(uint8_t irq);

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

#endif