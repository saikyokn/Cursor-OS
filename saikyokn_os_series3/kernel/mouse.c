#include "mouse.h"

#define MOUSE_DATA   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_CMD    0x64

static mouse_state_t mouse = {400, 300, 0, 0, 0};
static uint8_t mouse_cycle = 0;
static uint8_t mouse_bytes[3];

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void mouse_write(uint8_t val) {
    outb(MOUSE_CMD, 0xD4);
    outb(MOUSE_DATA, val);
}

static uint8_t mouse_read(void) {
    while (!(inb(MOUSE_STATUS) & 1));
    return inb(MOUSE_DATA);
}

void mouse_init(void) {
    // PS/2マウス有効化
    outb(MOUSE_CMD, 0xA8);
    mouse_write(0xF4);  // データ送信開始
    mouse_read();       // ACK読み捨て
    
    mouse.x = 400;
    mouse.y = 300;
    mouse.buttons = 0;
    mouse_cycle = 0;
}

void mouse_poll(void) {
    uint8_t status = inb(MOUSE_STATUS);
    if (!(status & 1)) return;
    if (!(status & 0x20)) return;  // マウスデータではない

    uint8_t data = inb(MOUSE_DATA);

    switch (mouse_cycle) {
        case 0:
            // ビット3は常に1であるべき
            if (!(data & 0x08)) {
                // 同期ずれ：リセット
                mouse_cycle = 0;
                break;
            }
            mouse_bytes[0] = data;
            mouse_cycle = 1;
            break;
            
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;
            
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;

            // ボタン状態
            mouse.buttons = mouse_bytes[0] & 0x07;

            // X移動量（9ビット符号拡張）
            int dx = mouse_bytes[1];
            if (mouse_bytes[0] & 0x10) dx |= 0xFFFFFF00;  // 符号拡張
            
            // Y移動量（9ビット符号拡張）
            int dy = mouse_bytes[2];
            if (mouse_bytes[0] & 0x20) dy |= 0xFFFFFF00;  // 符号拡張
            dy = -dy;  // Y方向反転

            // オーバーフローチェック（異常値なら破棄）
            if (mouse_bytes[0] & 0x40 || mouse_bytes[0] & 0x80) {
                // XまたはYオーバーフロー → このパケットは信頼できない
                dx = 0;
                dy = 0;
            }

            // 急激な移動を抑制（1回の移動量を±64に制限）
            if (dx > 64) dx = 64;
            if (dx < -64) dx = -64;
            if (dy > 64) dy = 64;
            if (dy < -64) dy = -64;

            // 累積移動量として保存（即座に座標更新はしない）
            mouse.dx += dx;
            mouse.dy += dy;
            
            // 座標を更新し、画面範囲内にクランプ
            mouse.x += dx;
            mouse.y += dy;
            
            if (mouse.x < 0) mouse.x = 0;
            if (mouse.y < 0) mouse.y = 0;
            if (mouse.x > 1023) mouse.x = 1023;
            if (mouse.y > 767) mouse.y = 767;
            break;
    }
}

void mouse_get_state(mouse_state_t *state) {
    *state = mouse;
    // 累積移動量はリセットしない（必要に応じてリセットする場合はここで）
    // mouse.dx = 0;
    // mouse.dy = 0;
}