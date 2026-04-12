#ifndef CONSOLE_H
#define CONSOLE_H

void console_init(unsigned int* vram, unsigned int stride, unsigned int width, unsigned int height);
void console_putc(char c);
void console_write(const char* s);
void console_render(void);
void console_clear_screen(void);
void console_set_color(unsigned int color);
void draw_string(unsigned int* vram, unsigned int stride, int x, int y, unsigned int color, const char* s);

#endif