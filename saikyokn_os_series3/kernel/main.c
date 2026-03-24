#include "console.h"

// shell
extern void shell_run(char* input);

// I/O
static inline unsigned char in8(unsigned short port){
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int keyboard_ready(){
    return in8(0x64) & 1;
}

unsigned char keyboard_read(){
    return in8(0x60);
}

// ===== キー =====
char keytable[128] = {
0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,
'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
0,'a','s','d','f','g','h','j','k','l',';',39,'`',
0,'\\','z','x','c','v','b','n','m',',','.','/',0,
'*',0,' '
};

// ===== 入力 =====
#define INPUT_MAX 128
char input[INPUT_MAX];
int input_len = 0;

// ===== カーネル =====
void kernel_main(unsigned int* vram, unsigned int stride){

    console_init(vram, stride);
    console_write("SAIKYOKN OS KERNEL\n> ");
    console_render();

    while(1){

        if(keyboard_ready()){

            unsigned char sc = keyboard_read();

            if(sc & 0x80) continue;

            char c = keytable[sc];
            if(!c) continue;

            if(c == '\n'){
                input[input_len] = 0;
                shell_run(input);
                input_len = 0;
                console_write("> ");
                console_render();
                continue;
            }

            if(c == 8){
                if(input_len > 0){
                    input_len--;
                    console_putc('\b');
                    console_render();
                }
                continue;
            }

            if(input_len < INPUT_MAX-1){
                input[input_len++] = c;
                console_putc(c);
                console_render();
            }
        }
    }
}