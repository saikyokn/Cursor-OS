#pragma once
#include <stdint.h>

void console_init(uint32_t* vram, uint32_t stride);
void console_putc(char c);
void console_write(const char* s);
void console_render();

void console_set_color(uint32_t color);
void console_clear(void);   // ← clear用