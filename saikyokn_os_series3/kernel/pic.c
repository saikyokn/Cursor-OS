#include "pic.h"

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_init(void) {
    // 1. 全IRQをマスク（初期化中の誤動作防止）
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    // 2. ICW1: 初期化開始（ICW4が必要、カスケードあり）
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    // 3. ICW2: 割り込みベクタのリマップ
    // PIC1: 0x20-0x27 (32-39)
    // PIC2: 0x28-0x2F (40-47)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // 4. ICW3: マスターとスレーブの接続
    outb(PIC1_DATA, 0x04); // PIC1のIRQ2にPIC2が接続
    outb(PIC2_DATA, 0x02); // PIC2はスレーブID=2

    // 5. ICW4: 8086モード
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // 6. ★ すべてのIRQをマスクしたまま（まだ割り込みは許可しない）
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_enable_irq(uint8_t irq) {
    uint16_t port = PIC1_DATA;
    if (irq >= 8) {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t mask = inb(port) & ~(1 << irq);
    outb(port, mask);
}

void pic_disable_irq(uint8_t irq) {
    uint16_t port = PIC1_DATA;
    if (irq >= 8) {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t mask = inb(port) | (1 << irq);
    outb(port, mask);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}