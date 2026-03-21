# パス
W64_PATH = D:/w64devkit/w64devkit/bin
QEMU     = D:/saikyokn_os/qemu/qemu-system-x86_64.exe
OVMF_CODE = D:/OVMF_CODE.fd
OVMF_VARS = D:/OVMF_VARS.fd

CC   = x86_64-w64-mingw32-gcc.exe
AS   = x86_64-w64-mingw32-as.exe

TARGET = saikyokn.efi

# ★追加：asmとinput.c
SRC = ../kernel/main.c ../kernel/font.c ../kernel/hankaku.c ../kernel/input.c
OBJ = irq.o

CFLAGS = -ffreestanding -mno-red-zone -fno-stack-protector -fshort-wchar -m64 \
         -march=x86-64 -mno-mmx -mno-sse -mno-sse2 -mno-avx -fno-builtin -I.

LDFLAGS = -shared -nostdlib -e EfiMain \
          -Wl,--subsystem,10 \
          -Wl,--image-base,0x100000 \
          -Wl,--section-alignment,0x1000

all: clean $(TARGET)

# ★ asmビルド
irq.o:
	cmd /c "set PATH=$(W64_PATH);%PATH% && $(AS) ../kernel/irq.asm -o irq.o"

$(TARGET): $(SRC) irq.o
	@echo [Building Saikyokn OS: TRUE CORE EDITION]
	cmd /c "set PATH=$(W64_PATH);%PATH% && $(CC) $(CFLAGS) $(SRC) irq.o -o $(TARGET) $(LDFLAGS)"

run: all
	@if not exist fs\EFI\BOOT mkdir fs\EFI\BOOT
	@copy /y $(TARGET) fs\EFI\BOOT\BOOTX64.EFI
	@echo [Starting QEMU with Debug Serial...]
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	        -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	        -drive file=fat:rw:fs,format=raw \
	        -vga std \
	        -device qemu-xhci \
	        -device usb-mouse \
	        -serial stdio

clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist irq.o del irq.o
	@if exist fs rmdir /s /q fs