#ifndef ERROR_H
#define ERROR_H

void panic(unsigned int* vram, unsigned int stride,
           unsigned int width, unsigned int height,
           const char* msg);

#endif