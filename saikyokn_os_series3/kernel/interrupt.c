#include "interrupt.h"
#include "idt.h"
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
    // ★ 割り込み番号を画面に直接表示（デバッグ用）
    if (vram_global) {
        // 割り込み番号を16進数で表示するための簡易テーブル
        char hex[] = "0123456789ABCDEF";
        unsigned int num = r->int_no;
        
        // 画面左上に番号を表示（赤い四角の下）
        // 1桁目（16の位）
        unsigned char high = (num >> 4) & 0x0F;
        // 2桁目（1の位）
        unsigned char low = num & 0x0F;
        
        // 簡易的に数字を描画（ドット絵）
        // ここでは省略して、赤い四角の色を番号で変える
        unsigned int color = 0x00FF0000 + (num * 0x100);
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 8; x++) {
                vram_global[y * stride_global + x] = color;
            }
        }
    }
    
    // ソフトウェア割り込みカウンタを増やす（条件を緩く）
    sw_int_count++;  // ★ すべての割り込みでカウントアップ
}

void interrupt_init(void) {
    idt_init();
}