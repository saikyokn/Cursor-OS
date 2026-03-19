# entry.s
.intel_syntax noprefix
.section .text
.global asm_entry

asm_entry:
    # 呼び出し規約(ms_abi)に合わせて、efi_main を叩く
    # 第1引数: RCX, 第2引数: RDX
    sub rsp, 32       # シャドウスペース確保
    call efi_main
    add rsp, 32
    ret