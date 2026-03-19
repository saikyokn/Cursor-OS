@echo off
mkdir fs\EFI\BOOT
copy ..\bin\saikyokn.efi fs\EFI\BOOT\BOOTX64.EFI
D:\saikyokn_os\qemu\qemu-system-x86_64.exe -drive file=fat:rw:fs,format=raw