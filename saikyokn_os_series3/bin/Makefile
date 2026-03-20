# 各種パス設定（自分の環境に合わせて調整してな！）
W64_PATH = D:/w64devkit/w64devkit/bin
QEMU     = D:/saikyokn_os/qemu/qemu-system-x86_64.exe
OVMF_CODE = D:/OVMF_CODE.fd
OVMF_VARS = D:/OVMF_VARS.fd

# コンパイラ設定
CC       = x86_64-w64-mingw32-gcc.exe
TARGET   = saikyokn.efi

# ソースファイル一式（ここが分割コンパイルの肝や！）
SRC      = ../kernel/main.c ../kernel/font.c ../kernel/hankaku.c

# UEFI用コンパイルオプション
CFLAGS   = -ffreestanding -mno-red-zone -fno-stack-protector -fshort-wchar -m64 \
           -march=x86-64 -mno-mmx -mno-sse -mno-sse2 -mno-avx -fno-builtin -I.

# リンカオプション（エントリポイントを EfiMain に指定）
LDFLAGS  = -shared -nostdlib -e EfiMain -Wl,--subsystem,10 -Wl,--image-base,0x100000 -Wl,--section-alignment,0x1000

all: clean $(TARGET)

$(TARGET): $(SRC)
	@echo [Building Saikyokn OS: Realtime Debug Edition]
	cmd /c "set PATH=$(W64_PATH);%PATH% && $(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)"

run: all
	@if not exist fs\EFI\BOOT mkdir fs\EFI\BOOT
	@copy /y $(TARGET) fs\EFI\BOOT\BOOTX64.EFI
	@echo [Starting QEMU with USB Support...]
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	        -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	        -drive file=fat:rw:fs,format=raw \
	        -vga std \
	        -device qemu-xhci \
	        -device usb-mouse \
	        -serial stdio

clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist fs rmdir /s /q fs