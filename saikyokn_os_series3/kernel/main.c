#include "console.h"
#include "gdt.h"
#include "interrupt.h"

unsigned int* vram_global;
unsigned int stride_global;
unsigned int w_global, h_global;

// ★ interrupt.c で定義されている変数を extern 宣言
extern volatile unsigned int sw_int_count;

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int keyboard_ready(void) { return inb(0x64) & 1; }
unsigned char keyboard_read(void) { return inb(0x60); }

static const char keytable_base[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' '
};
static const char keytable_shift[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,' '
};

static char input_buf[256];
static int input_len = 0;

int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

void itoa10(unsigned int n, char* buf) {
    int i = 0;
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    buf[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char t = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = t;
    }
}

void keyboard_process_key(uint8_t scancode) {
    static int shift = 0;
    if (scancode == 0x2A || scancode == 0x36) { shift = 1; return; }
    if (scancode == 0xAA || scancode == 0xB6) { shift = 0; return; }
    if (scancode & 0x80) return;
    if (scancode >= 128) return;

    char c = shift ? keytable_shift[scancode] : keytable_base[scancode];
    if (!c) return;

    if (c == '\n') {
        input_buf[input_len] = '\0';
        console_putc('\n');
        
        if (input_len > 0) {
            if (strcmp(input_buf, "clear") == 0) {
                console_clear_screen();
                console_write("SAIKYOKN OS - Step1 (IDT only)\n");
            }
            else if (strcmp(input_buf, "test") == 0) {
                console_write("Executing INT 0x20...\n");
                __asm__ volatile ("int $0x20");
                console_write("Returned\n");
            }
            else if (strcmp(input_buf, "sw") == 0) {
                char buf[16];
                itoa10(sw_int_count, buf);
                console_write("SW count: ");
                console_write(buf);
                console_write("\n");
            }
            else if (strcmp(input_buf, "help") == 0) {
                console_write("Commands: clear, test, sw, help\n");
            }
            else {
                console_write("Unknown: ");
                console_write(input_buf);
                console_write("\n");
            }
        }
        console_write("> ");
        input_len = 0;
    } else if (c == '\b') {
        if (input_len > 0) { input_len--; console_putc('\b'); }
    } else if (input_len < 255) {
        input_buf[input_len++] = c;
        console_putc(c);
    }
    console_render();
}

void kernel_main(unsigned int* vram, unsigned int stride,
                 unsigned int width, unsigned int height) {
    vram_global = vram; stride_global = stride; w_global = width; h_global = height;

    // VRAMテスト
    for (unsigned int y = 0; y < height; y++)
        for (unsigned int x = 0; x < width; x++)
            vram[y * stride + x] = 0xFF888888;
    for (unsigned int y = 0; y < 100 && y < height; y++)
        for (unsigned int x = 0; x < 100 && x < width; x++)
            vram[y * stride + x] = 0xFFFFFFFF;
    for (volatile int i = 0; i < 300000000; i++);

    console_init(vram, stride, width, height);
    console_set_color(0x00FFFFFF);
    console_clear_screen();
    console_write("SAIKYOKN OS - Step1 (IDT only)\n");
    
    // IDT初期化
    interrupt_init();
    console_write("IDT initialized\n");
    console_write("Try 'test' command\n> ");
    console_render();

    while (1) {
        if (keyboard_ready()) {
            keyboard_process_key(keyboard_read());
        }
        for (volatile int i = 0; i < 10000; i++);
    }
}