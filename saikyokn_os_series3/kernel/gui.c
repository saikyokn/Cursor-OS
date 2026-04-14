#include "gui.h"
#include "font.h"
#include "mouse.h"

static unsigned int *vram;
static unsigned int stride;
static unsigned int screen_w, screen_h;
static int gui_active = 1;
static int desktop_drawn = 0;

static button_t console_btn;
static button_t test_btn;

// マウス背景保存用
static uint32_t mouse_backup[16][16];
static int backup_x = 0, backup_y = 0;
static int backup_valid = 0;

extern void console_activate(void);

static void console_btn_click(void) {
    gui_switch_to_console();
}

static void test_btn_click(void) {
    // テスト用
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

    gui_active = 1;
    desktop_drawn = 0;
    backup_valid = 0;
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
    fill_rect(0, 0, screen_w, screen_h, 0x00202020);
    gui_draw_button(&console_btn);
    gui_draw_button(&test_btn);
    desktop_drawn = 1;
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

void gui_handle_click(int x, int y) {
    if (x >= console_btn.x && x < console_btn.x + console_btn.w &&
        y >= console_btn.y && y < console_btn.y + console_btn.h) {
        console_btn.on_click();
    }
    else if (x >= test_btn.x && x < test_btn.x + test_btn.w &&
             y >= test_btn.y && y < test_btn.y + test_btn.h) {
        test_btn.on_click();
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