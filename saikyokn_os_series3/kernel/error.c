#include "font.h"

typedef unsigned int UINT32;

void panic(UINT32* vram, UINT32 stride,
           UINT32 width, UINT32 height,
           const char* msg){

    __asm__ volatile("cli");

    // 黒画面
    for(UINT32 y=0;y<height;y++)
        for(UINT32 x=0;x<width;x++)
            vram[y*stride+x]=0x000000;

    draw_string(vram,stride,50,50,0xFF0000,"KERNEL PANIC");
    draw_string(vram,stride,50,80,0xFFFFFF,msg);

    while(1){
        __asm__ volatile("hlt");
    }
}