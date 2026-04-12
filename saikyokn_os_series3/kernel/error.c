#include "error.h"
#include "font.h"

void panic(unsigned int* vram, unsigned int stride,
           unsigned int width, unsigned int height,
           const char* msg) {

    __asm__ volatile("cli");

    // 赤画面
    for(unsigned int y = 0; y < height; y++) {
        for(unsigned int x = 0; x < width; x++) {
            vram[y * stride + x] = 0x00FF0000;
        }
    }

    draw_string(vram, stride, 50, 50, 0xFFFFFFFF, "KERNEL PANIC");
    draw_string(vram, stride, 50, 80, 0xFFFFFFFF, (char*)msg);

    while(1) {
        __asm__ volatile("hlt");
    }
}