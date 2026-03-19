W64_PATH = D:/w64devkit/w64devkit/bin
QEMU     = D:/saikyokn_os/qemu/qemu-system-x86_64.exe
OVMF_CODE = D:/OVMF_CODE.fd
OVMF_VARS = D:/OVMF_VARS.fd

CC       = x86_64-w64-mingw32-gcc.exe
TARGET   = saikyokn.efi
SRC      = ../kernel/main.c

CFLAGS   = -ffreestanding -mno-red-zone -fno-stack-protector -fshort-wchar -m64
LDFLAGS  = -shared -nostdlib -e EfiMain -Wl,--subsystem,10 -Wl,--image-base,0x100000 -Wl,--section-alignment,0x1000

all: clean $(TARGET)

$(TARGET): $(SRC)
	@echo [Building UEFI Application: Saikyokn OS Full Single-File...]
	cmd /c "set PATH=$(W64_PATH);%PATH% && $(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)"

run: all
	@if not exist fs\EFI\BOOT mkdir fs\EFI\BOOT
	@copy /y $(TARGET) fs\EFI\BOOT\BOOTX64.EFI
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	        -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	        -drive file=fat:rw:fs,format=raw -vga std -serial stdio

clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist fs rmdir /s /q fs