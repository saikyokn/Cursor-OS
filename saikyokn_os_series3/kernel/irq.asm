.global irq1_wrapper
.extern kb_push

irq1_wrapper:
    pushq %rax

    inb $0x60, %al
    mov %al, %dil
    call kb_push

    movb $0x20, %al
    outb %al, $0x20

    popq %rax
    iretq