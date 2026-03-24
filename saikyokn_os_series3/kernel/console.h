#ifndef CONSOLE_H
#define CONSOLE_H

// ===== 初期化 =====
void console_init(unsigned int* vram, unsigned int stride);

// ===== 描画 =====
void console_putc(char c);
void console_write(const char* s);
void console_render(void);

// ===== 制御 =====
void console_clear(void);
void console_set_color(unsigned int color);

// ===== UI（追加機能）=====
void console_ui_render(void);

#endif