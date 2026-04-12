#include "idt.h"

// 割り込みスタブの外部宣言
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();

static struct idt_entry idt[256];
static struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_mid  = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector  = sel;
    idt[num].ist       = 0;
    idt[num].flags     = flags;
    idt[num].zero      = 0;
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint64_t)&idt;

    // 全エントリをクリア
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // ソフトウェア割り込みテスト用 (0x20-0x2F)
    // int $0x20 〜 int $0x2F が使えるように登録
    idt_set_gate(0x20, (uint64_t)isr0,  0x08, 0x8E);
    idt_set_gate(0x21, (uint64_t)isr1,  0x08, 0x8E);
    idt_set_gate(0x22, (uint64_t)isr2,  0x08, 0x8E);
    idt_set_gate(0x23, (uint64_t)isr3,  0x08, 0x8E);
    idt_set_gate(0x24, (uint64_t)isr4,  0x08, 0x8E);
    idt_set_gate(0x25, (uint64_t)isr5,  0x08, 0x8E);
    idt_set_gate(0x26, (uint64_t)isr6,  0x08, 0x8E);
    idt_set_gate(0x27, (uint64_t)isr7,  0x08, 0x8E);
    idt_set_gate(0x28, (uint64_t)isr8,  0x08, 0x8E);
    idt_set_gate(0x29, (uint64_t)isr9,  0x08, 0x8E);
    idt_set_gate(0x2A, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(0x2B, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(0x2D, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(0x2E, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(0x2F, (uint64_t)isr15, 0x08, 0x8E);

    // IDTをロード
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}