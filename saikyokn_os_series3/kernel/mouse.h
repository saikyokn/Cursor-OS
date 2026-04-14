#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x, y;           // 絶対座標
    int buttons;        // ボタン状態（1:左, 2:右, 4:中）
    int dx, dy;         // 累積移動量（前回取得からの差分）
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);          // ★ ポーリング関数の宣言を追加
void mouse_get_state(mouse_state_t *state);

// 割り込みハンドラ（interrupt.cから呼ばれる）
void mouse_handler(void);

#endif