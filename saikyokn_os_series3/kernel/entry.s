.intel_syntax noprefix
.section .text
.global asm_entry

asm_entry:
    sub rsp, 32
    call EfiMain
    add rsp, 32
    ret