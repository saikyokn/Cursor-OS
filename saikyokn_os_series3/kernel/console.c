#include "console.h"
#include "font.h"

static unsigned int* vram;
static unsigned int stride;
static unsigned int screen_w, screen_h;
static int cursor_x = 0, cursor_y = 0;
static unsigned int current_color = 0x00FFFFFF;

#define CHAR_W 8
#define CHAR_H 16

void console_init(unsigned int* fb, unsigned int s, unsigned int w, unsigned int h) {
    vram = fb; stride = s; screen_w = w; screen_h = h;
    cursor_x = cursor_y = 0;
    console_clear_screen();
}

void console_clear_screen(void) {
    for (unsigned int y = 0; y < screen_h; y++)
        for (unsigned int x = 0; x < screen_w; x++)
            vram[y * stride + x] = 0x000000;
    cursor_x = cursor_y = 0;
}

void console_set_color(unsigned int c) { current_color = c; }

static void console_scroll(void) {
    for (unsigned int y = CHAR_H; y < screen_h; y++)
        for (unsigned int x = 0; x < screen_w; x++)
            vram[(y - CHAR_H) * stride + x] = vram[y * stride + x];
    for (unsigned int y = screen_h - CHAR_H; y < screen_h; y++)
        for (unsigned int x = 0; x < screen_w; x++)
            vram[y * stride + x] = 0x000000;
}

void console_putc(char c) {
    if (c == '\n') {
        cursor_x = 0; cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
    } else {
        draw_char(vram, stride, cursor_x * CHAR_W, cursor_y * CHAR_H, current_color, (unsigned char)c);
        cursor_x++;
        if (cursor_x >= (int)(screen_w / CHAR_W)) { cursor_x = 0; cursor_y++; }
    }
    if (cursor_y >= (int)(screen_h / CHAR_H)) {
        console_scroll();
        cursor_y = (screen_h / CHAR_H) - 1;
    }
}

void console_write(const char* s) { while (*s) console_putc(*s++); }
void console_render(void) { }