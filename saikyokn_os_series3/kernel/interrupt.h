#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

void interrupt_init(void);

// ソフトウェア割り込みカウンタ（main.cで定義）
extern volatile unsigned int sw_int_count;

#endif