#include "error.h"

typedef unsigned long long UINT64;

extern volatile UINT64 ticks;
extern unsigned int* vram_global;
extern unsigned int stride_global, w_global, h_global;

// ================= IRQ0 =================
__attribute__((naked))
void irq0_handler() {
    __asm__ volatile(
        "push %rax\n"
        "push %rcx\n"
        "push %rdx\n"

        "incq ticks(%rip)\n"

        "mov $0x20, %al\n"
        "out %al, $0x20\n"

        "pop %rdx\n"
        "pop %rcx\n"
        "pop %rax\n"
        "iretq\n"
    );
}

// ================= #DE =================
__attribute__((naked))
void isr_de(){
    __asm__ volatile(
        "cli\n"
        "call de_handler\n"
        "hlt\n"
    );
}

void de_handler(){
    panic(vram_global,stride_global,w_global,h_global,"DIVIDE ERROR");
}

// ================= #GP =================
__attribute__((naked))
void isr_gp(){
    __asm__ volatile(
        "cli\n"
        "call gp_handler\n"
        "hlt\n"
    );
}

void gp_handler(){
    panic(vram_global,stride_global,w_global,h_global,"GENERAL PROTECTION");
}

// ================= #PF =================
__attribute__((naked))
void isr_pf(){
    __asm__ volatile(
        "cli\n"
        "call pf_handler\n"
        "hlt\n"
    );
}

void pf_handler(){
    panic(vram_global,stride_global,w_global,h_global,"PAGE FAULT");
}