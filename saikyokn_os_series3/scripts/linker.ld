ENTRY(_start)
SECTIONS
{
    . = 0x1000;
    .text : {
        kernel_entry.o(.text)  /* これを一番上に書くのがコツや！ */
        *(.text)
    }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}