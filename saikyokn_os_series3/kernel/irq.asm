; ISRスタブ - 割り込みゲート用の低レベルハンドラ

extern _isr_handler
extern _irq_handler

; ===== 例外ハンドラ（エラーコードなし）=====
global _isr0
_isr0:
    push qword 0
    push qword 0
    jmp isr_common_stub

global _isr1
_isr1:
    push qword 0
    push qword 1
    jmp isr_common_stub

global _isr2
_isr2:
    push qword 0
    push qword 2
    jmp isr_common_stub

global _isr3
_isr3:
    push qword 0
    push qword 3
    jmp isr_common_stub

global _isr4
_isr4:
    push qword 0
    push qword 4
    jmp isr_common_stub

global _isr5
_isr5:
    push qword 0
    push qword 5
    jmp isr_common_stub

global _isr6
_isr6:
    push qword 0
    push qword 6
    jmp isr_common_stub

global _isr7
_isr7:
    push qword 0
    push qword 7
    jmp isr_common_stub

; ===== 例外ハンドラ（エラーコードあり）=====
global _isr8
_isr8:
    push qword 8
    jmp isr_common_stub

global _isr9
_isr9:
    push qword 0
    push qword 9
    jmp isr_common_stub

global _isr10
_isr10:
    push qword 10
    jmp isr_common_stub

global _isr11
_isr11:
    push qword 11
    jmp isr_common_stub

global _isr12
_isr12:
    push qword 12
    jmp isr_common_stub

global _isr13
_isr13:
    push qword 13
    jmp isr_common_stub

global _isr14
_isr14:
    push qword 14
    jmp isr_common_stub

global _isr15
_isr15:
    push qword 0
    push qword 15
    jmp isr_common_stub

global _isr16
_isr16:
    push qword 0
    push qword 16
    jmp isr_common_stub

global _isr17
_isr17:
    push qword 17
    jmp isr_common_stub

global _isr18
_isr18:
    push qword 0
    push qword 18
    jmp isr_common_stub

global _isr19
_isr19:
    push qword 0
    push qword 19
    jmp isr_common_stub

global _isr20
_isr20:
    push qword 0
    push qword 20
    jmp isr_common_stub

global _isr21
_isr21:
    push qword 0
    push qword 21
    jmp isr_common_stub

global _isr22
_isr22:
    push qword 0
    push qword 22
    jmp isr_common_stub

global _isr23
_isr23:
    push qword 0
    push qword 23
    jmp isr_common_stub

global _isr24
_isr24:
    push qword 0
    push qword 24
    jmp isr_common_stub

global _isr25
_isr25:
    push qword 0
    push qword 25
    jmp isr_common_stub

global _isr26
_isr26:
    push qword 0
    push qword 26
    jmp isr_common_stub

global _isr27
_isr27:
    push qword 0
    push qword 27
    jmp isr_common_stub

global _isr28
_isr28:
    push qword 0
    push qword 28
    jmp isr_common_stub

global _isr29
_isr29:
    push qword 0
    push qword 29
    jmp isr_common_stub

global _isr30
_isr30:
    push qword 30
    jmp isr_common_stub

global _isr31
_isr31:
    push qword 0
    push qword 31
    jmp isr_common_stub

; ===== IRQハンドラ =====
global _irq0
_irq0:
    push qword 0
    push qword 32
    jmp irq_common_stub

global _irq1
_irq1:
    push qword 0
    push qword 33
    jmp irq_common_stub

global _irq2
_irq2:
    push qword 0
    push qword 34
    jmp irq_common_stub

global _irq3
_irq3:
    push qword 0
    push qword 35
    jmp irq_common_stub

global _irq4
_irq4:
    push qword 0
    push qword 36
    jmp irq_common_stub

global _irq5
_irq5:
    push qword 0
    push qword 37
    jmp irq_common_stub

global _irq6
_irq6:
    push qword 0
    push qword 38
    jmp irq_common_stub

global _irq7
_irq7:
    push qword 0
    push qword 39
    jmp irq_common_stub

global _irq8
_irq8:
    push qword 0
    push qword 40
    jmp irq_common_stub

global _irq9
_irq9:
    push qword 0
    push qword 41
    jmp irq_common_stub

global _irq10
_irq10:
    push qword 0
    push qword 42
    jmp irq_common_stub

global _irq11
_irq11:
    push qword 0
    push qword 43
    jmp irq_common_stub

global _irq12
_irq12:
    push qword 0
    push qword 44
    jmp irq_common_stub

global _irq13
_irq13:
    push qword 0
    push qword 45
    jmp irq_common_stub

global _irq14
_irq14:
    push qword 0
    push qword 46
    jmp irq_common_stub

global _irq15
_irq15:
    push qword 0
    push qword 47
    jmp irq_common_stub

; ===== 共通ISRスタブ =====
isr_common_stub:
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
    
    mov rcx, rsp
    sub rsp, 32
    call _isr_handler
    add rsp, 32
    
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
    
    add rsp, 16
    iretq

; ===== 共通IRQスタブ =====
irq_common_stub:
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
    
    mov rcx, rsp
    sub rsp, 32
    call _irq_handler
    add rsp, 32
    
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
    
    add rsp, 16
    iretq