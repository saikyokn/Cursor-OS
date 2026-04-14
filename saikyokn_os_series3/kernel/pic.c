#include "pic.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_init(void) {
    // ICW1: 初期化開始（ICW4が必要、カスケードあり）
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    
    // ICW2: ベクタベース（PIC1=0x20, PIC2=0x28）
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    
    // ICW3: カスケード設定
    outb(PIC1_DATA, 0x04);  // PIC1のIRQ2にPIC2が接続
    outb(PIC2_DATA, 0x02);  // PIC2はスレーブID=2
    
    // ICW4: 8086モード
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // マスク設定：IRQ0（タイマー）とIRQ1（キーボード）のみ許可
    outb(PIC1_DATA, 0xFC);  // 11111100
    outb(PIC2_DATA, 0xFF);  // すべてマスク
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}