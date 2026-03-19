#include "graphics.h"

// 指定した範囲を色で塗りつぶす（基本中の基本！）
void fill_rect(unsigned int *vram, unsigned int stride, int x, int y, int w, int h, unsigned int color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vram[i * stride + j] = color;
        }
    }
}

// OSっぽさを出す「ウィンドウ」を描画する
void draw_window(unsigned int *vram, unsigned int stride, int x, int y, int w, int h, char *title) {
    // 1. ウィンドウの影（ちょっとリッチに見える）
    fill_rect(vram, stride, x + 2, y + 2, w, h, 0x00000000); 
    // 2. ウィンドウ本体
    fill_rect(vram, stride, x, y, w, h, 0x00C6C6C6); 
    // 3. タイトルバー（Saikyokn Dark Blue）
    fill_rect(vram, stride, x + 3, y + 3, w - 6, 20, 0x00000080); 
    // 4. タイトル文字（白）
    draw_string(vram, stride, x + 10, y + 5, title, 0x00FFFFFF);
    // 5. 右上の「×」ボタン（赤）
    fill_rect(vram, stride, x + w - 22, y + 5, 16, 16, 0x00FF0000); 
}