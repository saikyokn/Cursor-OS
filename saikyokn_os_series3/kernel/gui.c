#include "gui.h"
#include "font.h"
#include "mouse.h"
#include <stddef.h>   // NULL の定義に必要

static unsigned int *vram;
static unsigned int stride;
static unsigned int screen_w, screen_h;
static int gui_active = 1;
static int desktop_drawn = 0;

#define TASKBAR_HEIGHT 30
#define MAX_WINDOWS 10

// ----- ウィンドウ構造体 -----
typedef struct window_t {
    int x, y, width, height;
    const char *title;
    int is_visible;
    void (*draw_content)(struct window_t *win);
    // お絵かき用データ
    int prev_mouse_x, prev_mouse_y;
    int mouse_was_pressed;
} window_t;

static window_t windows[MAX_WINDOWS];
static int window_count = 0;

static button_t console_btn;
static button_t test_btn;
static button_t paint_btn;

// 時計表示用
static unsigned long long last_sec = 0;
static char clock_str[16] = "00:00:00";

// マウス背景保存用
static uint32_t mouse_backup[16][16];
static int backup_x = 0, backup_y = 0;
static int backup_valid = 0;

// ドラッグ中のウィンドウ
static window_t *dragging_window = NULL;
static int drag_offset_x, drag_offset_y;

extern void console_activate(void);

// ----- ボタンコールバック -----
static void console_btn_click(void) {
    gui_switch_to_console();
}

static void test_btn_click(void) {
    // 何もしない
}

static void paint_btn_click(void);

// ----- ウィンドウ操作 -----
static window_t *gui_create_window(const char *title, int x, int y, int w, int h) {
    if (window_count >= MAX_WINDOWS) return NULL;
    window_t *win = &windows[window_count++];
    win->x = x; win->y = y; win->width = w; win->height = h;
    win->title = title;
    win->is_visible = 1;
    win->draw_content = NULL;
    win->prev_mouse_x = win->prev_mouse_y = 0;
    win->mouse_was_pressed = 0;
    return win;
}

// 未使用関数はコメントアウト
/*
static void gui_destroy_window(window_t *win) {
    win->is_visible = 0;
}
*/

static void fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx, py = y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h)
                vram[py * stride + px] = color;
        }
    }
}

void gui_draw_window(window_t *win) {
    if (!win || !win->is_visible) return;

    uint32_t border_color = 0x00505050;
    uint32_t title_color = 0x00808080;
    uint32_t client_color = 0x00C0C0C0;

    // ウィンドウ枠
    fill_rect(win->x, win->y, win->width, win->height, border_color);
    // タイトルバー
    fill_rect(win->x + 2, win->y + 2, win->width - 4, 20, title_color);
    // クライアント領域
    fill_rect(win->x + 2, win->y + 24, win->width - 4, win->height - 26, client_color);
    // タイトル文字
    draw_string(vram, stride, win->x + 6, win->y + 5, 0x00FFFFFF, win->title);

    if (win->draw_content) {
        win->draw_content(win);
    }
}

// ----- お絵かきウィンドウ -----
static window_t *paint_win = NULL;

// 線描画 (Bresenham) - ウィンドウのクライアント領域相対座標で描画
static void draw_line_relative(window_t *win, int x0, int y0, int x1, int y1, uint32_t color) {
    // ウィンドウ内の絶対座標に変換
    int base_x = win->x + 2;
    int base_y = win->y + 24;

    int cx0 = base_x + x0, cy0 = base_y + y0;
    int cx1 = base_x + x1, cy1 = base_y + y1;

    // クライアント領域の範囲内にクランプ
    if (cx0 < base_x) cx0 = base_x;
    if (cy0 < base_y) cy0 = base_y;
    if (cx1 < base_x) cx1 = base_x;
    if (cy1 < base_y) cy1 = base_y;
    int max_x = base_x + win->width - 4;
    int max_y = base_y + win->height - 26;
    if (cx0 >= max_x) cx0 = max_x - 1;
    if (cy0 >= max_y) cy0 = max_y - 1;
    if (cx1 >= max_x) cx1 = max_x - 1;
    if (cy1 >= max_y) cy1 = max_y - 1;

    int dx = cx1 - cx0, dy = cy1 - cy0;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;
    int err = dx - dy;
    int x = cx0, y = cy0;

    while (1) {
        if (x >= 0 && x < (int)screen_w && y >= 0 && y < (int)screen_h)
            vram[y * stride + x] = color;
        if (x == cx1 && y == cy1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

static void paint_window_draw(window_t *win) {
    // 線はすでにフレームバッファに直接描画されるため、ここでは何もしない
    // 実際にはオフスクリーンバッファが必要だが、今回は簡易実装
}

static void paint_btn_click(void) {
    if (paint_win) {
        paint_win->is_visible = 0;
    }
    paint_win = gui_create_window("Paint", 200, 150, 400, 350);
    if (paint_win) {
        paint_win->draw_content = paint_window_draw;
    }
    desktop_drawn = 0;
}

// ----- GUI初期化 -----
void gui_init(unsigned int *fb, unsigned int s, unsigned int w, unsigned int h) {
    vram = fb; stride = s; screen_w = w; screen_h = h;

    console_btn.x = 50; console_btn.y = 50; console_btn.w = 150; console_btn.h = 40;
    console_btn.text = "Console"; console_btn.color = 0x00408080;
    console_btn.hover_color = 0x0040A0A0; console_btn.on_click = console_btn_click;

    test_btn.x = 50; test_btn.y = 110; test_btn.w = 150; test_btn.h = 40;
    test_btn.text = "Test"; test_btn.color = 0x00808040;
    test_btn.hover_color = 0x00A0A040; test_btn.on_click = test_btn_click;

    paint_btn.x = 50; paint_btn.y = 170; paint_btn.w = 150; paint_btn.h = 40;
    paint_btn.text = "Paint"; paint_btn.color = 0x00804080;
    paint_btn.hover_color = 0x00A060A0; paint_btn.on_click = paint_btn_click;

    gui_active = 1; desktop_drawn = 0; backup_valid = 0;
    last_sec = 0; dragging_window = NULL; window_count = 0; paint_win = NULL;
}

// ----- デスクトップ描画 -----
void gui_draw_desktop(void) {
    // 背景
    fill_rect(0, 0, screen_w, screen_h - TASKBAR_HEIGHT, 0x00202020);
    gui_draw_button(&console_btn);
    gui_draw_button(&test_btn);
    gui_draw_button(&paint_btn);
    gui_draw_taskbar();

    // 全ウィンドウを描画
    for (int i = 0; i < window_count; i++) {
        if (windows[i].is_visible) {
            gui_draw_window(&windows[i]);
        }
    }
    desktop_drawn = 1;
}

void gui_draw_taskbar(void) {
    int y = screen_h - TASKBAR_HEIGHT;
    fill_rect(0, y, screen_w, TASKBAR_HEIGHT, 0x00101010);
}

static void draw_clock(void) {
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

    if (gui_active) draw_clock();
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

// ----- マウス描画 (背景保存/復元) -----
static void restore_mouse_background(void) {
    if (!backup_valid) return;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = backup_x + dx, py = backup_y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h)
                vram[py * stride + px] = mouse_backup[dy][dx];
        }
    }
    backup_valid = 0;
}

static void save_mouse_background(int x, int y) {
    backup_x = x; backup_y = y;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = x + dx, py = y + dy;
            if (px >= 0 && px < (int)screen_w && py >= 0 && py < (int)screen_h)
                mouse_backup[dy][dx] = vram[py * stride + px];
            else mouse_backup[dy][dx] = 0;
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

// ----- イベント処理 -----
void gui_handle_click(int x, int y) {
    // ボタンのクリック判定
    if (x >= console_btn.x && x < console_btn.x + console_btn.w &&
        y >= console_btn.y && y < console_btn.y + console_btn.h)
        console_btn.on_click();
    else if (x >= test_btn.x && x < test_btn.x + test_btn.w &&
             y >= test_btn.y && y < test_btn.y + test_btn.h)
        test_btn.on_click();
    else if (x >= paint_btn.x && x < paint_btn.x + paint_btn.w &&
             y >= paint_btn.y && y < paint_btn.y + paint_btn.h)
        paint_btn.on_click();
}

void gui_run(void) {
    if (!gui_active) return;

    static int last_buttons = 0;
    mouse_state_t m;
    mouse_get_state(&m);

    // ---- ウィンドウのドラッグ処理 ----
    if (dragging_window) {
        if (m.buttons & 1) {
            int new_x = m.x - drag_offset_x;
            int new_y = m.y - drag_offset_y;
            if (new_x < 0) new_x = 0;
            if (new_y < 0) new_y = 0;
            if (new_x + dragging_window->width > (int)screen_w) new_x = screen_w - dragging_window->width;
            if (new_y + dragging_window->height > (int)screen_h - TASKBAR_HEIGHT)
                new_y = screen_h - TASKBAR_HEIGHT - dragging_window->height;
            dragging_window->x = new_x;
            dragging_window->y = new_y;
            desktop_drawn = 0;
        } else {
            dragging_window = NULL;
        }
    }

    // ---- お絵かきウィンドウ内の描画 ----
    if (paint_win && paint_win->is_visible && !dragging_window) {
        window_t *win = paint_win;
        int rel_x = m.x - (win->x + 2);
        int rel_y = m.y - (win->y + 24);
        int in_client = (rel_x >= 0 && rel_x < win->width - 4 &&
                         rel_y >= 0 && rel_y < win->height - 26);
        if (in_client) {
            if (m.buttons & 1) {
                if (win->mouse_was_pressed) {
                    draw_line_relative(win, win->prev_mouse_x, win->prev_mouse_y, rel_x, rel_y, 0x00000000);
                }
                win->prev_mouse_x = rel_x;
                win->prev_mouse_y = rel_y;
                win->mouse_was_pressed = 1;
            } else {
                win->mouse_was_pressed = 0;
            }
        } else {
            win->mouse_was_pressed = 0;
        }
    }

    // ---- クリック開始 (ウィンドウ移動またはボタン) ----
    if ((m.buttons & 1) && !(last_buttons & 1)) {
        // まずウィンドウのタイトルバーをチェック (後ろから = 手前優先)
        int handled = 0;
        for (int i = window_count - 1; i >= 0; i--) {
            window_t *win = &windows[i];
            if (!win->is_visible) continue;
            if (m.x >= win->x + 2 && m.x < win->x + win->width - 2 &&
                m.y >= win->y + 2 && m.y < win->y + 24) {
                dragging_window = win;
                drag_offset_x = m.x - win->x;
                drag_offset_y = m.y - win->y;
                handled = 1;
                break;
            }
        }
        if (!handled) {
            gui_handle_click(m.x, m.y);
        }
    }

    // ---- 描画 ----
    if (!desktop_drawn) {
        gui_draw_desktop();
    }

    gui_draw_mouse(m.x, m.y);
    last_buttons = m.buttons;
}

void gui_switch_to_console(void) {
    gui_active = 0;
    desktop_drawn = 0;
    backup_valid = 0;
    console_activate();
}

void gui_activate(void) {
    gui_active = 1;
    desktop_drawn = 0;
    backup_valid = 0;
}

int gui_is_active(void) {
    return gui_active;
}