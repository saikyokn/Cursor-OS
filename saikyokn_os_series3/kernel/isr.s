/* isr.S - 64ビット割り込みスタブ（マウス対応版・コメント修正） */
.intel_syntax noprefix
.section .text

.extern timer_handler
.extern keyboard_handler
.extern mouse_handler
.extern divide_error_handler
.extern debug_handler
.extern nmi_handler
.extern breakpoint_handler
.extern overflow_handler
.extern bound_handler
.extern invalid_opcode_handler
.extern device_not_available_handler
.extern double_fault_handler
.extern coprocessor_segment_overrun_handler
.extern invalid_tss_handler
.extern segment_not_present_handler
.extern stack_fault_handler
.extern general_protection_handler
.extern page_fault_handler
.extern reserved_handler
.extern fpu_error_handler
.extern alignment_check_handler
.extern machine_check_handler
.extern simd_exception_handler

.macro SAVE_REGS
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
.endm

.macro RESTORE_REGS
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
.endm

.macro EXCEPTION_NOERR num name
.global exception_\name
exception_\name:
    push 0
    push \num
    SAVE_REGS
    call \name\()_handler
    RESTORE_REGS
    add rsp, 16
    iretq
.endm

.macro EXCEPTION_ERR num name
.global exception_\name
exception_\name:
    push \num
    SAVE_REGS
    call \name\()_handler
    RESTORE_REGS
    add rsp, 16
    iretq
.endm

.macro IRQ_HANDLER num name handler_func
.global irq_\name
irq_\name:
    push 0
    push \num
    SAVE_REGS
    call \handler_func
    RESTORE_REGS
    add rsp, 16
    iretq
.endm

EXCEPTION_NOERR 0  divide_error
EXCEPTION_NOERR 1  debug
EXCEPTION_NOERR 2  nmi
EXCEPTION_NOERR 3  breakpoint
EXCEPTION_NOERR 4  overflow
EXCEPTION_NOERR 5  bound
EXCEPTION_NOERR 6  invalid_opcode
EXCEPTION_NOERR 7  device_not_available
EXCEPTION_ERR   8  double_fault
EXCEPTION_NOERR 9  coprocessor_segment_overrun
EXCEPTION_ERR   10 invalid_tss
EXCEPTION_ERR   11 segment_not_present
EXCEPTION_ERR   12 stack_fault
EXCEPTION_ERR   13 general_protection
EXCEPTION_ERR   14 page_fault
EXCEPTION_NOERR 15 reserved
EXCEPTION_NOERR 16 fpu_error
EXCEPTION_ERR   17 alignment_check
EXCEPTION_NOERR 18 machine_check
EXCEPTION_NOERR 19 simd_exception

IRQ_HANDLER 32 timer   timer_handler
IRQ_HANDLER 33 keyboard keyboard_handler
IRQ_HANDLER 44 mouse   mouse_handler