#include "console.h"

// ===== 外部フォント =====
extern void draw_char(unsigned int*, unsigned int, int, int, unsigned int, unsigned char);
extern void draw_string(unsigned int*, unsigned int, int, int, unsigned int, const char*);

// ===== VRAM =====
static unsigned int* vram;
static unsigned int stride;

// ===== 画面サイズ =====
#define SCREEN_W 1024
#define SCREEN_H 768

#define CHAR_W 8
#define CHAR_H 16

// ===== カーソル =====
static int cursor_x = 0;
static int cursor_y = 0;

// ===== 色 =====
static unsigned int current_color = 0x00FFFFFF;

// ===== 色変更 =====
void console_set_color(unsigned int color){
    current_color = color;
}

// ===== 画面クリア =====
void console_clear(){
    for(int y = 0; y < SCREEN_H; y++){
        for(int x = 0; x < SCREEN_W; x++){
            vram[y * stride + x] = 0x00000000;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

// ===== 初期化 =====
void console_init(unsigned int* fb, unsigned int s){
    vram = fb;
    stride = s;

    cursor_x = 0;
    cursor_y = 0;

    console_clear();
}

// ===== 1文字 =====
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

// ===== 文字列 =====
void console_write(const char* s){
    while(*s){
        console_putc(*s++);
    }
}

// ===== フル塗りつぶし =====
void fill_screen(unsigned int color){
    for(int y = 0; y < SCREEN_H; y++){
        for(int x = 0; x < SCREEN_W; x++){
            vram[y * stride + x] = color;
        }
    }
}

// ===== タスクバー =====
void draw_taskbar(){
    int bar_height = 40;

    for(int y = SCREEN_H - bar_height; y < SCREEN_H; y++){
        for(int x = 0; x < SCREEN_W; x++){
            vram[y * stride + x] = 0x00202020;
        }
    }
}

// ===== UIレンダ =====
void console_ui_render(){
    fill_screen(0x000000FF); // 青
    draw_taskbar();

    draw_string(vram, stride,
                10,
                SCREEN_H - 30,
                0x00FFFFFF,
                "Kyusasu");
}

// ===== render =====
void console_render(){
}