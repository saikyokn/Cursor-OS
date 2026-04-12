/* isr.S - ソフトウェア割り込みテスト用スタブ */
.intel_syntax noprefix
.section .text

.extern isr_handler

.macro ISR_STUB num
.global isr\num
isr\num:
    push 0
    push \num
    jmp isr_common_stub
.endm

ISR_STUB 0
ISR_STUB 1
ISR_STUB 2
ISR_STUB 3
ISR_STUB 4
ISR_STUB 5
ISR_STUB 6
ISR_STUB 7
ISR_STUB 8
ISR_STUB 9
ISR_STUB 10
ISR_STUB 11
ISR_STUB 12
ISR_STUB 13
ISR_STUB 14
ISR_STUB 15

isr_common_stub:
    // レジスタをすべて保存
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

    // 第1引数にスタックポインタを渡す
    mov rdi, rsp
    call isr_handler

    // レジスタを復元
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

    // エラーコードと割り込み番号を捨てる
    add rsp, 16
    iretq