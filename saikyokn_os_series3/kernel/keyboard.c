#include "font.h"

typedef unsigned char UINT8;

static inline UINT8 inb(unsigned short port){
    UINT8 r;
    __asm__ volatile ("inb %1,%0":"=a"(r):"Nd"(port));
    return r;
}

static inline void outb(unsigned short port, UINT8 val){
    __asm__ volatile ("outb %0,%1"::"a"(val),"Nd"(port));
}

extern unsigned int* vram_global;
extern unsigned int stride_global;

int cursor_x = 10;

__attribute__((naked))
void irq1_handler(){
    __asm__ volatile(
        "push %rax\n"
        "push %rcx\n"
        "push %rdx\n"

        "call keyboard_handler\n"

        "mov $0x20, %al\n"
        "out %al, $0x20\n"

        "pop %rdx\n"
        "pop %rcx\n"
        "pop %rax\n"
        "iretq\n"
    );
}

void keyboard_handler(){
    UINT8 sc = inb(0x60);

    char c = '?';

    if(sc == 0x1E) c='A';
    if(sc == 0x30) c='B';
    if(sc == 0x2E) c='C';

    char str[2] = {c,0};

    draw_string(vram_global,stride_global,cursor_x,200,0xFFFFFF,str);
    cursor_x += 8;
}