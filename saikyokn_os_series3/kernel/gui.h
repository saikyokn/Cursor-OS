#ifndef GUI_H
#define GUI_H

#include <stdint.h>

typedef struct {
    int x, y, w, h;
    const char *text;
    uint32_t color;
    uint32_t hover_color;
    void (*on_click)(void);
} button_t;

void gui_init(unsigned int *vram, unsigned int stride, unsigned int w, unsigned int h);
void gui_draw_desktop(void);
void gui_draw_button(button_t *btn);
void gui_draw_mouse(int x, int y);
void gui_handle_click(int x, int y);
void gui_run(void);
void gui_switch_to_console(void);
void gui_activate(void);          // ★ 追加
int gui_is_active(void);

#endif