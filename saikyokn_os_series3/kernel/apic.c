#include "apic.h"

// ★ outb を直接定義（PICを使わないのでpic.hは不要）
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#define LAPIC_BASE  0xFEE00000
#define IOAPIC_BASE 0xFEC00000

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_ENABLE (1 << 11)

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = val & 0xFFFFFFFF;
    uint32_t high = val >> 32;
    __asm__ volatile ("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t*)((uintptr_t)LAPIC_BASE + reg);
}

void lapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)((uintptr_t)LAPIC_BASE + reg) = val;
}

static uint32_t ioapic_read(uint32_t reg) {
    *(volatile uint32_t*)((uintptr_t)IOAPIC_BASE) = reg;
    return *(volatile uint32_t*)((uintptr_t)IOAPIC_BASE + 0x10);
}

static void ioapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)((uintptr_t)IOAPIC_BASE) = reg;
    *(volatile uint32_t*)((uintptr_t)IOAPIC_BASE + 0x10) = val;
}

void lapic_eoi(void) {
    lapic_write(LAPIC_EOI, 0);
}

void apic_init(void) {
    // 1. PICを完全に無効化
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    
    // 2. Local APICを有効化（MSR経由）
    uint64_t apic_base = rdmsr(IA32_APIC_BASE_MSR);
    apic_base |= IA32_APIC_BASE_ENABLE;
    wrmsr(IA32_APIC_BASE_MSR, apic_base);
    
    // 3. Spurious Interrupt Vector Registerを設定
    lapic_write(LAPIC_SVR, lapic_read(LAPIC_SVR) | 0x1FF);
    
    // 4. I/O APICの初期化（バージョン確認）
    // uint32_t ioapic_ver = ioapic_read(IOAPIC_VERSION);  // 未使用のためコメントアウト
    
    // 5. キーボード割り込み（IRQ1）をRedirection Tableに設定
    uint32_t redir_low = 0x21;           // ベクタ番号
    redir_low |= (0 << 8);               // 送信先モード：物理
    redir_low |= (1 << 11);              // 送信ステータス
    redir_low |= (1 << 13);              // ポラリティ：High active
    redir_low |= (0 << 15);              // トリガーモード：エッジ
    redir_low |= (0 << 16);              // マスク：解除
    
    uint32_t redir_high = 0;             // 送信先Local APIC ID = 0
    
    uint32_t reg = IOAPIC_REDTBL_START + 2 * 1;  // IRQ1
    ioapic_write(reg, redir_low);
    ioapic_write(reg + 1, redir_high);
    
    // 6. LINT0/LINT1を無効化
    lapic_write(LAPIC_LVT_LINT0, 1 << 16);
    lapic_write(LAPIC_LVT_LINT1, 1 << 16);
    
    // エラーハンドラ
    lapic_write(LAPIC_LVT_ERROR, 0xFE);
}