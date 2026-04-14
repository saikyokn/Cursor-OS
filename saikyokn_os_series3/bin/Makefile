# ===== パス設定 =====
W64_PATH   = D:\w64devkit\w64devkit\bin
QEMU       = D:\qemu\qemu-system-x86_64.exe
OVMF_CODE  = D:/qemu/share/edk2-x86_64-code.fd
OVMF_VARS  = D:/OVMF_VARS.fd

# ===== ツール =====
CC      = gcc.exe
TARGET  = saikyokn.efi

# ===== ソースファイル =====
C_SRC   = ../kernel/boot.c \
          ../kernel/main.c \
          ../kernel/console.c \
          ../kernel/font.c \
          ../kernel/hankaku.c \
          ../kernel/gdt.c \
          ../kernel/error.c \
          ../kernel/idt.c \
          ../kernel/pic.c \
          ../kernel/interrupt.c \
          ../kernel/mouse.c \
          ../kernel/gui.c \
          ../kernel/isr.S      # ★ .S を C_SRC に含める

ASM_SRC = ../kernel/entry.s

# ===== オブジェクトファイル =====
C_OBJ   = $(notdir $(C_SRC:.c=.o))
C_OBJ   := $(C_OBJ:.S=.o)      # .S も .o に変換
ASM_OBJ = $(notdir $(ASM_SRC:.s=.o))
OBJ     = $(C_OBJ) $(ASM_OBJ)

# ===== コンパイルオプション =====
CFLAGS = -ffreestanding \
         -mno-red-zone \
         -fno-stack-protector \
         -fshort-wchar \
         -m64 \
         -march=x86-64 \
         -mno-mmx \
         -fno-builtin \
         -I../kernel \
         -Wall \
         -O2 \
         -mgeneral-regs-only

LDFLAGS = -shared \
          -nostdlib \
          -e EfiMain \
          -Wl,--subsystem,10 \
          -Wl,--image-base,0x100000 \
          -Wl,--section-alignment,0x1000

# ===== 環境変数 =====
ENV = set PATH=$(W64_PATH);%PATH% &&

# ===== ビルドルール =====
all: $(TARGET)

%.o: ../kernel/%.c
	@echo [CC] $<
	@$(ENV) $(CC) $(CFLAGS) -c $< -o $@

%.o: ../kernel/%.S
	@echo [AS] $<
	@$(ENV) $(CC) $(CFLAGS) -c $< -o $@

%.o: ../kernel/%.s
	@echo [AS] $<
	@$(ENV) $(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	@echo [LINK] $@
	@$(ENV) $(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# ===== 実行 =====
run: clean all
	@if not exist fs\EFI\BOOT mkdir fs\EFI\BOOT
	@copy /y $(TARGET) fs\EFI\BOOT\BOOTX64.EFI
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	        -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	        -drive file=fat:rw:fs,format=raw \
	        -vga std \
	        -display sdl

# ===== クリーン =====
clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist *.o del *.o
	@if exist fs rmdir /s /q fs

.PHONY: all run clean