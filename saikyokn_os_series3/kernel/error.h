#pragma once

void panic(unsigned int* vram, unsigned int stride,
           unsigned int width, unsigned int height,
           const char* msg);