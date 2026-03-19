[bits 32]
global _start
extern main

section .text
_start:
    ; boot.asmから渡されたVRAMアドレス(eax)を保持したままmainを呼ぶ
    push eax
    call main
    hlt