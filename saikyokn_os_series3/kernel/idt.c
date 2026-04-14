#include "idt.h"

static struct idt_entry idt_entries[IDT_MAX_INTERRUPTS];

static struct idt_ptr idt_ptr = {
    .limit = (sizeof(struct idt_entry) * IDT_MAX_INTERRUPTS) - 1,
    .base = (uint64_t)&idt_entries
};

int reg_irq(int irq_line, irq_handler hndl) {
    if (irq_line < 0 || irq_line >= IDT_MAX_INTERRUPTS)
        return -1;
    
    uint64_t handler_addr = (uint64_t)hndl;
    
    idt_entries[irq_line].offset_low  = handler_addr & 0xFFFF;
    idt_entries[irq_line].offset_mid  = (handler_addr >> 16) & 0xFFFF;
    idt_entries[irq_line].offset_high = (handler_addr >> 32) & 0xFFFFFFFF;
    
    idt_entries[irq_line].selector = 0x08;  // カーネルコードセグメント
    idt_entries[irq_line].ist      = 0;     // IST未使用
    idt_entries[irq_line].attr     = IDT_ATTR_INTERRUPT_GATE;
    idt_entries[irq_line].reserved = 0;
    
    return 0;
}

int install_idt(void) {
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
    return 0;
}