#include "gdt.h"

static struct gdt_entry gdt[5];
static struct gdt_ptr   gdtp;

static void gdt_set_gate(int num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_init(void) {
    gdtp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdtp.base  = (uint64_t)&gdt;

    // Null
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // カーネルコードセグメント（64ビット）
    // アクセス権: 0x9A = 10011010 (Present, Ring0, Code, Executable, Readable)
    // グラニュラリティ: 0xAF = 10101111 (64-bit, 4KB granularity)
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xAF);
    
    // カーネルデータセグメント
    // アクセス権: 0x92 = 10010010 (Present, Ring0, Data, Writable)
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xAF);
    
    // ユーザーコード
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xAF);
    
    // ユーザーデータ
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xAF);

    // GDTをロード
    __asm__ volatile (
        "lgdt %0\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        // 長いリターンでCSを0x08に設定
        "pushq $0x08\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        :
        : "m"(gdtp)
        : "rax", "memory"
    );
}