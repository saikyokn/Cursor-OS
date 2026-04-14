#include "interrupt.h"
#include "pic.h"
#include "idt.h"

volatile unsigned int kbd_int_count = 0;
volatile unsigned long long ticks = 0;

#define KBD_BUFFER_SIZE 32
static volatile uint8_t kbd_buffer[KBD_BUFFER_SIZE];
static volatile int kbd_head = 0, kbd_tail = 0;

void kbd_buffer_put(uint8_t sc) {
    int next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_tail) {
        kbd_buffer[kbd_head] = sc;
        kbd_head = next;
    }
}

int kbd_buffer_empty(void) { return kbd_head == kbd_tail; }

uint8_t kbd_buffer_get(void) {
    if (kbd_buffer_empty()) return 0;
    uint8_t sc = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return sc;
}

// 割り込みハンドラ
void timer_handler(void) {
    ticks++;
    pic_send_eoi(0);
}

void keyboard_handler(void) {
    uint8_t sc = inb(0x60);
    kbd_buffer_put(sc);
    kbd_int_count++;
    pic_send_eoi(1);
}

// ★ すべての例外ハンドラを定義（不足分を追加）
#define DEFINE_EXCEPTION(name) void name##_handler(void) { while(1) __asm__ volatile ("hlt"); }

DEFINE_EXCEPTION(divide_error)
DEFINE_EXCEPTION(debug)
DEFINE_EXCEPTION(nmi)
DEFINE_EXCEPTION(breakpoint)
DEFINE_EXCEPTION(overflow)
DEFINE_EXCEPTION(bound)
DEFINE_EXCEPTION(invalid_opcode)
DEFINE_EXCEPTION(device_not_available)
DEFINE_EXCEPTION(double_fault)
DEFINE_EXCEPTION(coprocessor_segment_overrun)  // ベクタ9
DEFINE_EXCEPTION(invalid_tss)
DEFINE_EXCEPTION(segment_not_present)
DEFINE_EXCEPTION(stack_fault)
DEFINE_EXCEPTION(general_protection)
DEFINE_EXCEPTION(page_fault)
DEFINE_EXCEPTION(reserved)                     // ベクタ15
DEFINE_EXCEPTION(fpu_error)
DEFINE_EXCEPTION(alignment_check)
DEFINE_EXCEPTION(machine_check)
DEFINE_EXCEPTION(simd_exception)

// アセンブリスタブの外部宣言
extern void exception_divide_error(void);
extern void exception_debug(void);
extern void exception_nmi(void);
extern void exception_breakpoint(void);
extern void exception_overflow(void);
extern void exception_bound(void);
extern void exception_invalid_opcode(void);
extern void exception_device_not_available(void);
extern void exception_double_fault(void);
extern void exception_coprocessor_segment_overrun(void);
extern void exception_invalid_tss(void);
extern void exception_segment_not_present(void);
extern void exception_stack_fault(void);
extern void exception_general_protection(void);
extern void exception_page_fault(void);
extern void exception_reserved(void);
extern void exception_fpu_error(void);
extern void exception_alignment_check(void);
extern void exception_machine_check(void);
extern void exception_simd_exception(void);
extern void irq_timer(void);
extern void irq_keyboard(void);

void interrupt_init(void) {
    // 例外ハンドラ登録 (0-31)
    reg_irq(0,  (irq_handler)exception_divide_error);
    reg_irq(1,  (irq_handler)exception_debug);
    reg_irq(2,  (irq_handler)exception_nmi);
    reg_irq(3,  (irq_handler)exception_breakpoint);
    reg_irq(4,  (irq_handler)exception_overflow);
    reg_irq(5,  (irq_handler)exception_bound);
    reg_irq(6,  (irq_handler)exception_invalid_opcode);
    reg_irq(7,  (irq_handler)exception_device_not_available);
    reg_irq(8,  (irq_handler)exception_double_fault);
    reg_irq(9,  (irq_handler)exception_coprocessor_segment_overrun);
    reg_irq(10, (irq_handler)exception_invalid_tss);
    reg_irq(11, (irq_handler)exception_segment_not_present);
    reg_irq(12, (irq_handler)exception_stack_fault);
    reg_irq(13, (irq_handler)exception_general_protection);
    reg_irq(14, (irq_handler)exception_page_fault);
    reg_irq(15, (irq_handler)exception_reserved);
    reg_irq(16, (irq_handler)exception_fpu_error);
    reg_irq(17, (irq_handler)exception_alignment_check);
    reg_irq(18, (irq_handler)exception_machine_check);
    reg_irq(19, (irq_handler)exception_simd_exception);
    // 20-31 は予約済み（必要に応じて追加）
    
    // IRQハンドラ登録
    reg_irq(32, (irq_handler)irq_timer);      // IRQ0
    reg_irq(33, (irq_handler)irq_keyboard);  // IRQ1
    
    install_idt();
    pic_init();
}