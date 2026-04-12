#include "interrupt.h"
#include "idt.h"
#include "pic.h"
#include "console.h"

volatile unsigned int sw_int_count = 0;

extern unsigned int* vram_global;
extern unsigned int stride_global;

struct regs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

void isr_handler(struct regs *r) {
    // ソフトウェア割り込みカウンタを増やす
    sw_int_count++;
    
    // デバッグ用：赤い四角を表示
    if (vram_global) {
        unsigned int color = 0x00FF0000 + (r->int_no * 0x100);
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 8; x++) {
                vram_global[y * stride_global + x] = color;
            }
        }
    }
}

unsigned char pic_get_mask(int pic_num) {
    if (pic_num == 1) return inb(PIC1_DATA);
    else return inb(PIC2_DATA);
}

void interrupt_init(void) {
    idt_init();
    pic_init();  // ★ PIC初期化を追加
}