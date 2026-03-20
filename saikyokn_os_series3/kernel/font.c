#include "font.h"

// hankaku.c で定義されている配列を外部参照する
extern unsigned char hankaku[4096];

void draw_char(unsigned int *vram, unsigned int stride, int x, int y, unsigned int color, unsigned char c) {
    // 1文字16バイトなので、文字コード c に 16 を掛けて開始位置を決める
    unsigned char *font = &hankaku[c * 16];
    
    for (int i = 0; i < 16; i++) {
        unsigned char row = font[i];
        for (int j = 0; j < 8; j++) {
            // 1ビットずつ見て、1なら色を置く
            if ((row << j) & 0x80) {
                vram[(y + i) * stride + (x + j)] = color;
            }
        }
    }
}

void draw_string(unsigned int *vram, unsigned int stride, int x, int y, unsigned int color, const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        draw_char(vram, stride, x + (i * 8), y, color, (unsigned char)s[i]);
    }
}