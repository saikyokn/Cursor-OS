#include "console.h"

static unsigned int* vram;
static unsigned int stride;

static int cursor_x = 0;
static int cursor_y = 0;

static unsigned int current_color = 0x00FFFFFF;

// 🔥 これが重要（font.cと一致させる）
extern void draw_char(unsigned int *vram, unsigned int stride, int x, int y, unsigned int color, unsigned char c);

// サイズ
#define CHAR_W 8
#define CHAR_H 16
#define SCREEN_W 1024
#define SCREEN_H 768

void console_set_color(unsigned int color){
    current_color = color;
}

void console_clear(){
    for(int y = 0; y < SCREEN_H; y++){
        for(int x = 0; x < SCREEN_W; x++){
            vram[y * stride + x] = 0x00000000;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void console_init(unsigned int* fb, unsigned int s){
    vram = fb;
    stride = s;
    cursor_x = 0;
    cursor_y = 0;

    console_clear();
}

void console_putc(char c){

    if(c == '\n'){
        cursor_x = 0;
        cursor_y++;
        return;
    }

    if(c == '\b'){
        if(cursor_x > 0) cursor_x--;
        return;
    }

    // 🔥 ここが修正ポイント
    draw_char(vram, stride,
              cursor_x * CHAR_W,
              cursor_y * CHAR_H,
              current_color,
              (unsigned char)c);

    cursor_x++;

    if(cursor_x >= (SCREEN_W / CHAR_W)){
        cursor_x = 0;
        cursor_y++;
    }
}

void console_write(const char* s){
    while(*s){
        console_putc(*s++);
    }
}

void console_render(){
}