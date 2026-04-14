#ifndef APIC_H
#define APIC_H

#include <stdint.h>

// Local APICレジスタオフセット
#define LAPIC_ID      0x0020
#define LAPIC_VERSION 0x0030
#define LAPIC_TPR     0x0080
#define LAPIC_EOI     0x00B0
#define LAPIC_SVR     0x00F0
#define LAPIC_ICR0    0x0300
#define LAPIC_ICR1    0x0310
#define LAPIC_LVT_TIMER    0x0320
#define LAPIC_LVT_LINT0    0x0350
#define LAPIC_LVT_LINT1    0x0360
#define LAPIC_LVT_ERROR    0x0370

// I/O APICレジスタ
#define IOAPIC_ID       0x00
#define IOAPIC_VERSION  0x01
#define IOAPIC_REDTBL_START 0x10

void apic_init(void);
void lapic_eoi(void);
uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t val);

#endif