#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_MAX_INTERRUPTS 256

// IDT属性フラグ
#define IDT_ATTR_16BIT  0x06  // 00000110
#define IDT_ATTR_32BIT  0x0E  // 00001110

#define IDT_ATTR_DPL_RING_0  0x00
#define IDT_ATTR_DPL_RING_1  0x20  // 00100000
#define IDT_ATTR_DPL_RING_2  0x40  // 01000000
#define IDT_ATTR_DPL_RING_3  0x60  // 01100000

#define IDT_ATTR_SEGMENT_PRESENT 0x80  // 10000000

// 割り込みゲートの標準属性（32-bit, Ring 0, Present）
#define IDT_ATTR_INTERRUPT_GATE (IDT_ATTR_32BIT | IDT_ATTR_DPL_RING_0 | IDT_ATTR_SEGMENT_PRESENT)

// 64ビットIDTエントリ構造体（OSDev Wiki準拠）
struct idt_entry {
    uint16_t offset_low;   // ISRアドレスの下位16ビット
    uint16_t selector;     // GDTセグメントセレクタ（0x08）
    uint8_t  ist;          // 割り込みスタックテーブル（0でOK）
    uint8_t  attr;         // 属性フラグ
    uint16_t offset_mid;   // ISRアドレスの中間16ビット
    uint32_t offset_high;  // ISRアドレスの上位32ビット
    uint32_t reserved;     // 予約（0）
} __attribute__((packed));

// 64ビットIDTR構造体
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

typedef void (*irq_handler)(void);

int reg_irq(int irq_line, irq_handler hndl);
int install_idt(void);

#endif