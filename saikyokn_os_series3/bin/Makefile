# ===== パス =====
W64_PATH = D:/w64devkit/w64devkit/bin
QEMU     = D:/saikyokn_os/qemu/qemu-system-x86_64.exe
OVMF_CODE = D:/OVMF_CODE.fd
OVMF_VARS = D:/OVMF_VARS.fd

# ===== ツール =====
CC  = x86_64-w64-mingw32-gcc.exe
TARGET = saikyokn.efi

# ===== ソース（🔥ここ重要）=====
SRC = ../kernel/main.c \
      ../kernel/shell.c \
      ../kernel/console.c \
      ../kernel/font.c \
      ../kernel/hankaku.c

# ===== オプション =====
CFLAGS = -ffreestanding -mno-red-zone -fno-stack-protector -fshort-wchar -m64 \
         -march=x86-64 -mno-mmx -fno-builtin -I.

LDFLAGS = -shared -nostdlib -e EfiMain \
          -Wl,--subsystem,10 \
          -Wl,--image-base,0x100000 \
          -Wl,--section-alignment,0x1000

# ===== ビルド =====
all: clean $(TARGET)

$(TARGET): $(SRC)
	@echo [Building Saikyokn OS: SHELL MODE]
	cmd /c "set PATH=$(W64_PATH);%PATH% && $(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)"

# ===== 実行 =====
run: all
	@if not exist fs\EFI\BOOT mkdir fs\EFI\BOOT
	@copy /y $(TARGET) fs\EFI\BOOT\BOOTX64.EFI
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	        -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	        -drive file=fat:rw:fs,format=raw \
	        -vga std \
	        -serial stdio

# ===== クリーン =====
clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist fs rmdir /s /q fs