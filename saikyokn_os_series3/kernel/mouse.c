#include "mouse.h"
#include "pic.h"

#define MOUSE_DATA   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_CMD    0x64

// マウス状態
static volatile mouse_state_t mouse = {400, 300, 0, 0, 0};
static volatile uint8_t mouse_cycle = 0;
static volatile uint8_t mouse_bytes[3];

// 割り込み用バッファ
#define MOUSE_BUF_SIZE 16
static volatile uint8_t mouse_buf[MOUSE_BUF_SIZE];
static volatile int mouse_buf_head = 0;
static volatile int mouse_buf_tail = 0;

static void mouse_write(uint8_t val) {
    outb(MOUSE_CMD, 0xD4);
    outb(MOUSE_DATA, val);
    while ((inb(MOUSE_STATUS) & 1) == 0);
    inb(MOUSE_DATA); // ACK読み捨て
}

static void mouse_buffer_put(uint8_t data) {
    int next = (mouse_buf_head + 1) % MOUSE_BUF_SIZE;
    if (next != mouse_buf_tail) {
        mouse_buf[mouse_buf_head] = data;
        mouse_buf_head = next;
    }
}

static int mouse_buffer_get(uint8_t *data) {
    if (mouse_buf_head == mouse_buf_tail) return 0;
    *data = mouse_buf[mouse_buf_tail];
    mouse_buf_tail = (mouse_buf_tail + 1) % MOUSE_BUF_SIZE;
    return 1;
}

// 割り込みハンドラ (IRQ12)
void mouse_handler(void) {
    while (inb(MOUSE_STATUS) & 1) {
        uint8_t data = inb(MOUSE_DATA);
        mouse_buffer_put(data);
    }
    pic_send_eoi(12);
}

// パケット処理 (割り込み/ポーリング共用)
static void mouse_process_packet(uint8_t data) {
    switch (mouse_cycle) {
        case 0:
            if (!(data & 0x08)) break;
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

            uint8_t state = mouse_bytes[0];
            mouse.buttons = state & 0x07;

            int dx = (int)mouse_bytes[1];
            int dy = (int)mouse_bytes[2];
            if (state & 0x10) dx -= 0x100;
            if (state & 0x20) dy -= 0x100;
            dy = -dy;

            if (!(state & 0x40) && !(state & 0x80)) {
                if (dx > 64) dx = 64;
                if (dx < -64) dx = -64;
                if (dy > 64) dy = 64;
                if (dy < -64) dy = -64;

                mouse.dx += dx;
                mouse.dy += dy;
                mouse.x += dx;
                mouse.y += dy;

                if (mouse.x < 0) mouse.x = 0;
                if (mouse.y < 0) mouse.y = 0;
                if (mouse.x > 1023) mouse.x = 1023;
                if (mouse.y > 767) mouse.y = 767;
            }
            break;
    }
}

// 割り込みバッファからデータを処理
static void mouse_process_interrupt(void) {
    uint8_t data;
    while (mouse_buffer_get(&data)) {
        mouse_process_packet(data);
    }
}

// ポーリングによる直接読み取り
void mouse_poll(void) {
    while (inb(MOUSE_STATUS) & 1) {
        uint8_t data = inb(MOUSE_DATA);
        mouse_process_packet(data);
    }
}

void mouse_init(void) {
    // 8042コントローラの割り込み有効化 (効果がなくても害はない)
    outb(MOUSE_CMD, 0x20);
    uint8_t conf = inb(MOUSE_DATA);
    conf |= 0x02;
    conf &= ~0x20;
    outb(MOUSE_CMD, 0x60);
    outb(MOUSE_DATA, conf);

    // マウスポート有効化
    outb(MOUSE_CMD, 0xA8);

    // マウス初期化
    mouse_write(0xF6);
    mouse_write(0xF4);

    // PICマスク解除 (IRQ12)
    uint8_t mask = inb(0xA1);
    outb(0xA1, mask & ~(1 << 4));

    mouse.x = 400;
    mouse.y = 300;
    mouse.buttons = 0;
    mouse_cycle = 0;
    mouse_buf_head = mouse_buf_tail = 0;
}

void mouse_get_state(mouse_state_t *state) {
    // 割り込みバッファにデータがあれば処理
    mouse_process_interrupt();
    *state = mouse;
}