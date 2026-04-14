#include "gui.h"
#include "font.h"
#include "mouse.h"

static unsigned int *vram;
static unsigned int stride;
static unsigned int screen_w, screen_h;
static int gui_active = 1;
static int desktop_drawn = 0;

#define TASKBAR_HEIGHT 30

static button_t console_btn;
static button_t test_btn;
static button_t paint_btn;   // お絵かき起動ボタン
static button_t back_btn;    // お絵かきから戻るボタン（左上）

// 時計表示用
static unsigned long long last_sec = 0;
static char clock_str[16] = "00:00:00";

// マウス背景保存用
static uint32_t mouse_backup[16][16];
static int backup_x = 0, backup_y = 0;
static int backup_valid = 0;

// お絵かきモード
static int paint_mode = 0;
static int prev_mouse_x = 0, prev_mouse_y = 0;
static int mouse_was_pressed = 0;

extern void console_activate(void);

static void console_btn_click(void) {
    gui_switch_to_console();
}

static void test_btn_click(void) {
    // テスト用（何もしない）
}

static void paint_btn_click(void) {
    paint_mode = 1;
    desktop_drawn = 0; // 再描画を促す
}

static void back_btn_click(void) {
    paint_mode = 0;
    desktop_drawn = 0; // デスクトップ再描画
    mouse_was_pressed = 0;
}

void gui_init(unsigned int *fb, unsigned int s, unsigned int w, unsigned int h) {
    vram = fb;
    stride = s;
    screen_w = w;
    screen_h = h;

    console_btn.x = 50;
    console_btn.y = 50;
    console_btn.w = 150;
    console_btn.h = 40;
    console_btn.text = "Console";
    console_btn.color = 0x00408080;
    console_btn.hover_color = 0x0040A0A0;
    console_btn.on_click = console_btn_click;

    test_btn.x = 50;
    test_btn.y = 110;
    test_btn.w = 150;
    test_btn.h = 40;
    test_btn.text = "Test";
    test_btn.color = 0x00808040;
    test_btn.hover_color = 0x00A0A040;
    test_btn.on_click = test_btn_click;

    paint_btn.x = 50;
    paint_btn.y = 170;
    paint_btn.w = 150;
    paint_btn.h = 40;
    paint_btn.text = "Paint";
    paint_btn.color = 0x00804080;
    paint_btn.hover_color = 0x00A060A0;
    paint_btn.on_click = paint_btn_click;

    // ★ 左上に配置
    back_btn.x = 10;
    back_btn.y = 10;
    back_btn.w = 50;
    back_btn.h = 30;
    back_btn.text = "X";
    back_btn.color = 0x00FF0000;
    back_btn.hover_color = 0x00FF4040;
    back_btn.on_click = back_btn_click;

    gui_active = 1;
    desktop_drawn = 0;
    backup_valid = 0;
    last_sec = 0;
    paint_mode = 0;
    mouse_was_pressed = 0;
}

static void fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx, py = y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h)
                vram[py * stride + px] = color;
        }
    }
}

void gui_draw_desktop(void) {
    if (paint_mode) {
        // お絵かきモードの背景（黒）
        fill_rect(0, 0, screen_w, screen_h, 0x00000000);
        gui_draw_button(&back_btn);
    } else {
        fill_rect(0, 0, screen_w, screen_h - TASKBAR_HEIGHT, 0x00202020);
        gui_draw_button(&console_btn);
        gui_draw_button(&test_btn);
        gui_draw_button(&paint_btn);
        gui_draw_taskbar();
    }
    desktop_drawn = 1;
}

void gui_draw_taskbar(void) {
    if (paint_mode) return;
    int y = screen_h - TASKBAR_HEIGHT;
    fill_rect(0, y, screen_w, TASKBAR_HEIGHT, 0x00101010);
}

static void draw_clock(void) {
    if (paint_mode) return;
    int y = screen_h - TASKBAR_HEIGHT + 8;
    int text_width = 8 * 8;
    int x = screen_w - text_width - 20;
    fill_rect(x - 5, screen_h - TASKBAR_HEIGHT, text_width + 10, TASKBAR_HEIGHT, 0x00101010);
    draw_string(vram, stride, x, y, 0x00FFFFFF, clock_str);
}

void gui_update_clock(unsigned long long ticks) {
    unsigned long long sec = ticks / 18;
    if (sec == last_sec) return;
    last_sec = sec;

    unsigned long long s = sec % 60;
    unsigned long long m = (sec / 60) % 60;
    unsigned long long h = (sec / 3600) % 24;

    clock_str[0] = '0' + (h / 10); clock_str[1] = '0' + (h % 10);
    clock_str[2] = ':';
    clock_str[3] = '0' + (m / 10); clock_str[4] = '0' + (m % 10);
    clock_str[5] = ':';
    clock_str[6] = '0' + (s / 10); clock_str[7] = '0' + (s % 10);
    clock_str[8] = '\0';

    if (gui_active && !paint_mode) {
        draw_clock();
    }
}

void gui_draw_button(button_t *btn) {
    mouse_state_t m;
    mouse_get_state(&m);
    int hover = (m.x >= btn->x && m.x < btn->x + btn->w &&
                 m.y >= btn->y && m.y < btn->y + btn->h);
    uint32_t col = hover ? btn->hover_color : btn->color;
    fill_rect(btn->x, btn->y, btn->w, btn->h, col);
    draw_string(vram, stride, btn->x + 10, btn->y + 10, 0x00FFFFFF, btn->text);
}

static void restore_mouse_background(void) {
    if (!backup_valid) return;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = backup_x + dx, py = backup_y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h) {
                vram[py * stride + px] = mouse_backup[dy][dx];
            }
        }
    }
    backup_valid = 0;
}

static void save_mouse_background(int x, int y) {
    backup_x = x;
    backup_y = y;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = x + dx, py = y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h) {
                mouse_backup[dy][dx] = vram[py * stride + px];
            } else {
                mouse_backup[dy][dx] = 0;
            }
        }
    }
    backup_valid = 1;
}

void gui_draw_mouse(int x, int y) {
    if (x < 0 || y < 0 || x + 16 >= (int)screen_w || y + 16 >= (int)screen_h) return;
    restore_mouse_background();
    save_mouse_background(x, y);

    uint32_t cur_color = 0x00FFFFFF;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = x + dx, py = y + dy;
            if (dx == 0 && dy < 10) vram[py * stride + px] = cur_color;
            if (dy == 0 && dx < 10) vram[py * stride + px] = cur_color;
            if (dx == dy && dx < 8) vram[py * stride + px] = cur_color;
        }
    }
}

// 整数のみで線を描く Bresenham アルゴリズム
static void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;

    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;

    int err = dx - dy;
    int x = x0, y = y0;

    while (1) {
        if (x >= 0 && x < (int)screen_w && y >= 0 && y < (int)screen_h)
            vram[y * stride + x] = color;

        if (x == x1 && y == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void gui_handle_click(int x, int y) {
    if (paint_mode) {
        if (x >= back_btn.x && x < back_btn.x + back_btn.w &&
            y >= back_btn.y && y < back_btn.y + back_btn.h) {
            back_btn.on_click();
        }
        return;
    }

    if (x >= console_btn.x && x < console_btn.x + console_btn.w &&
        y >= console_btn.y && y < console_btn.y + console_btn.h) {
        console_btn.on_click();
    }
    else if (x >= test_btn.x && x < test_btn.x + test_btn.w &&
             y >= test_btn.y && y < test_btn.y + test_btn.h) {
        test_btn.on_click();
    }
    else if (x >= paint_btn.x && x < paint_btn.x + paint_btn.w &&
             y >= paint_btn.y && y < paint_btn.y + paint_btn.h) {
        paint_btn.on_click();
    }
}

void gui_run(void) {
    if (!gui_active) return;

    static int last_buttons = 0;
    mouse_state_t m;
    mouse_get_state(&m);

    if (!desktop_drawn) {
        gui_draw_desktop();
    }

    // お絵かきモードの描画処理
    if (paint_mode) {
        if (m.buttons & 1) {
            if (mouse_was_pressed) {
                draw_line(prev_mouse_x, prev_mouse_y, m.x, m.y, 0x00FFFFFF);
            }
            prev_mouse_x = m.x;
            prev_mouse_y = m.y;
            mouse_was_pressed = 1;
        } else {
            mouse_was_pressed = 0;
        }
        gui_draw_button(&back_btn);
    }

    gui_draw_mouse(m.x, m.y);

    if ((m.buttons & 1) && !(last_buttons & 1)) {
        gui_handle_click(m.x, m.y);
    }
    last_buttons = m.buttons;
}

void gui_switch_to_console(void) {
    gui_active = 0;
    desktop_drawn = 0;
    backup_valid = 0;
    paint_mode = 0;
    console_activate();
}

void gui_activate(void) {
    gui_active = 1;
    desktop_drawn = 0;
    backup_valid = 0;
    paint_mode = 0;
}

int gui_is_active(void) {
    return gui_active;
}