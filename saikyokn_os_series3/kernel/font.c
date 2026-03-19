#include "graphics.h"

// 8x16ピクセル フォントデータ (hankaku.txt 準拠)
unsigned char hankaku[4096] = {
    // 0x20: ' ' (Space)
    [0x20*16+0]=0x00, [0x20*16+1]=0x00, [0x20*16+2]=0x00, [0x20*16+3]=0x00,
    // 0x41: 'A'
    [0x41*16+0]=0x18, [0x41*16+1]=0x18, [0x41*16+2]=0x24, [0x41*16+3]=0x24, [0x41*16+4]=0x42, [0x41*16+5]=0x42, [0x41*16+6]=0x7E, [0x41*16+7]=0x42,
    // 0x53: 'S' (SaikyoknのS!)
    [0x53*16+0]=0x3C, [0x53*16+1]=0x66, [0x53*16+2]=0x42, [0x53*16+3]=0x1C, [0x53*16+4]=0x06, [0x53*16+5]=0x42, [0x53*16+6]=0x3C,
    // 0x57: 'W' (WindowのW!)
    [0x57*16+0]=0x42, [0x57*16+1]=0x42, [0x57*16+2]=0x42, [0x57*16+3]=0x42, [0x57*16+4]=0x5A, [0x57*16+5]=0x5A, [0x57*16+6]=0x24,
    // ... 必要な文字をここに足していく ...
};

void draw_char(unsigned int *vram, unsigned int stride, int x, int y, unsigned char c, unsigned int color) {
    unsigned char *font = &hankaku[c * 16];
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 8; dx++) {
            if ((font[dy] << (7 - dx)) & 0x01) { // ビット判定
                vram[(y + dy) * stride + (x + dx)] = color;
            }
        }
    }
}

void draw_string(unsigned int *vram, unsigned int stride, int x, int y, char *s, unsigned int color) {
    while (*s) {
        draw_char(vram, stride, x, y, (unsigned char)*s, color);
        s++;
        x += 8;
    }
}