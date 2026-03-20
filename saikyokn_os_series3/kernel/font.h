#ifndef FONT_H
#define FONT_H

void draw_char(unsigned int *vram, unsigned int stride, int x, int y, unsigned int color, unsigned char c);
void draw_string(unsigned int *vram, unsigned int stride, int x, int y, unsigned int color, const char *s);

#endif