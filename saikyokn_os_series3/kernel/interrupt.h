#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

void interrupt_init(void);

// ソフトウェア割り込みカウンタ
extern volatile unsigned int sw_int_count;

// PICマスクレジスタ読み取り用
unsigned char pic_get_mask(int pic_num);

#endif