#include "console.h"
#include "font.h"

#define COLS 80
#define ROWS 25

static char buf[ROWS][COLS];

static UINT32 cur_x = 0;
static UINT32 cur_y = 0;

static UINT32 *fb;
static UINT32 fb_stride;

void console_init(UINT32 *vram, UINT32 stride){
    fb = vram;
    fb_stride = stride;
    console_clear();
}

void console_clear(){
    for(int y=0;y<ROWS;y++)
        for(int x=0;x<COLS;x++)
            buf[y][x] = ' ';

    cur_x = 0;
    cur_y = 0;
}

static void scroll(){
    for(int y=1;y<ROWS;y++)
        for(int x=0;x<COLS;x++)
            buf[y-1][x] = buf[y][x];

    for(int x=0;x<COLS;x++)
        buf[ROWS-1][x] = ' ';

    cur_y = ROWS-1;
}

void console_putc(char c){

    if(c=='\n'){
        cur_x=0;
        cur_y++;
    }
    else if(c=='\b'){
        if(cur_x>0){
            cur_x--;
            buf[cur_y][cur_x]=' ';
        }
    }
    else{
        buf[cur_y][cur_x]=c;
        cur_x++;

        if(cur_x>=COLS){
            cur_x=0;
            cur_y++;
        }
    }

    if(cur_y>=ROWS){
        scroll();
    }
}

void console_write(const char *s){
    while(*s) console_putc(*s++);
}

void console_render(){
    for(int y=0;y<ROWS;y++){
        for(int x=0;x<COLS;x++){
            draw_char(fb, fb_stride, x*8, y*16, 0xFFFFFF, buf[y][x]);
        }
    }
}