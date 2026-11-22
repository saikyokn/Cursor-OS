# Known Issues & Security Advisories — R4V1

## Security & Practical Readiness (Read First)

**CursorOS R4V1 is experimental and NOT ready for daily or production use.** Despite major hardening in R4V1 (user VA isolation at `PML4[1]`, `PAGE_NX` enforcement, `PF_W | PF_X` rejection, `spinlock` irqsave, `ptr_ok` / `munmap` bounds checks), the following security vulnerabilities and limitations remain:

1. **No forced Ring 3 timer preemption**: A misbehaving or infinite-looping Ring 3 application that does not call `yield()` / `csr_sleep_ms()` can starve the cooperative GUI loop and other tasks.
2. **Weak TLS certificate trust & no browser sandbox**: BearSSL extracts the server public key for handshake completion, but full CA chain / hostname trust verification is still relaxed for demo convenience. The Web Browser runs inside kernel space and must not be used on untrusted HTML/JS.
3. **Legacy `sys_exec` slot-1 path**: While terminal `exec` now uses `sched_spawn_user_detached()`, syscall `61` (`sys_exec`) still calls `usermode_load_bin()` (slot 1 fixed) and can overwrite/destroy slot 1 state on mid-load failure.
4. **Incomplete UEFI boot hardening**: `boot.c` calls `ExitBootServices` once without a `GetMemoryMap` retry loop, and `h.rsdp` is not yet populated from `SystemTable->ConfigurationTable`, which can affect real hardware boot and ACPI MADT discovery depending on firmware.
5. **No per-process filesystem ACLs / quota**: Ring 3 syscalls share a flat kernel VFS/FAT layer without user/group isolation or disk quotas.
6. **128 MiB static VFS preload `.bss`**: Large static buffers remain in `vfs.c` (scheduled for reduction in R4V2).

## Data Safety & Recovery

Because kernel bugs, power loss, or unguarded writes can corrupt FAT16 images or disks:

- **Always back up before testing**: Use `mingw32-make run-snap` on QEMU when you do not need persistent disk writes, and keep copies of `fs.img` / `nvme.img`. On real hardware, test only on dedicated/spare media.
- **exFAT protection**: NVMe exFAT volumes (`EXFAT/`) are mounted **read-only** in the guest to prevent accidental data loss.
- **Recovery steps (Real Hardware UEFI)**:
  - If CursorOS fails to boot, use the firmware boot menu to select your primary OS bootloader.
  - Remove `\EFI\CURSOROS\` from the ESP to clean up without touching other OS loaders.
- **Recovery steps (FAT16 Disk / Image Corruption)**:
  - Make a raw copy of the affected disk/image first.
  - Mount or inspect from Linux/Windows and run `fsck.vfat` / `chkdsk` to repair directory/FAT chains and recover files.

## Ring 3 / SDK

- Ring 3 ELFs must be linked at `0x8000000000` (`sdk/user.ld` with `-mcmodel=large`). Legacy low-address ELFs (`< 0x8000000000`) and `PF_W | PF_X` segments are rejected by `usermode_load_elf()`.
- `SMOKE_TEST=1` executes automated self-tests in `kernel_main`, but full interactive GUI/QEMU verification of R4V1 on real hardware is still ongoing.

## Web Browser & Networking

- CSS/JS remain partial subsets; remote HTTPS pages run without JS execution unless `USE_QUICKJS=1` is explicitly integrated and enabled.
- Network stack supports **Intel e1000**, **Realtek RTL8168/8111**, and **VirtIO-Net** (WiFi unsupported). DHCP/DNS include retry and an 8-entry cache.
- YouTube Lite H.264 playback requires sufficient guest RAM (`MEM=4G` recommended in QEMU) and remains mute until AAC decoding is added.

## Audio & Storage

- **Intel HDA** is probed first with automatic fallback to **AC'97** (8/16-bit PCM). No AAC audio decoder yet.
- **AHCI SATA DMA** and **NVMe** are supported; `install wipe YES` overwrites the entire GPT and must only be used on empty disks.

## Input / IME & Display

- **Japanese IME**: Includes dictionary-based Kana-Kanji conversion (`Space` key + candidate bar) and `MPLUS.TTF` auto-init with tofu fallback, though vocabulary coverage is still compact.
- **Display**: QEMU `virtio-vga` and GOP LFB with `gpu_shadow` WB caching and dirty-rect / cursor-backing optimization; ThinkPad E495 still uses firmware GOP scanout without native AMD DC modeset.

## Reporting

If you hit a kernel panic, note `CR2`, `RIP`, `debugcon` output, and what action triggered it.
