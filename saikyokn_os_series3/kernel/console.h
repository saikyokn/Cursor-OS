#pragma once

typedef unsigned int UINT32;

void console_init(UINT32 *vram, UINT32 stride);
void console_clear();
void console_putc(char c);
void console_write(const char *s);
void console_render();