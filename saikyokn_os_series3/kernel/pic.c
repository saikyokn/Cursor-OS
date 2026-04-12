#include "pic.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIT_CMD   0x43
#define PIT_CH0   0x40

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_init(void) {
    // ★★★ APICを無効化し、PICを再有効化する手順 ★★★
    
    // 1. まずPICを完全にリセット
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    
    // 2. マスターPICを完全初期化
    outb(PIC1_CMD, 0x11);  // ICW1
    outb(PIC1_DATA, 0x20); // ICW2: ベクタ0x20
    outb(PIC1_DATA, 0x04); // ICW3: IRQ2にスレーブ接続
    outb(PIC1_DATA, 0x01); // ICW4: 8086モード
    
    // 3. スレーブPICを完全初期化
    outb(PIC2_CMD, 0x11);  // ICW1
    outb(PIC2_DATA, 0x28); // ICW2: ベクタ0x28
    outb(PIC2_DATA, 0x02); // ICW3: スレーブID=2
    outb(PIC2_DATA, 0x01); // ICW4: 8086モード
    
    // 4. 全マスク解除（すべての割り込みを許可）
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
    
    // 5. PIT初期化（モード2、約100Hz）
    outb(PIT_CMD, 0x34);
    outb(PIT_CH0, 0xA9);
    outb(PIT_CH0, 0x2E);
}

void pic_enable_irq(uint8_t irq) {
    uint16_t port = PIC1_DATA;
    if (irq >= 8) { port = PIC2_DATA; irq -= 8; }
    uint8_t mask = inb(port) & ~(1 << irq);
    outb(port, mask);
}

void pic_disable_irq(uint8_t irq) {
    uint16_t port = PIC1_DATA;
    if (irq >= 8) { port = PIC2_DATA; irq -= 8; }
    uint8_t mask = inb(port) | (1 << irq);
    outb(port, mask);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}