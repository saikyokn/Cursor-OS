QEMU development — CursorOS
Primary day-to-day target. Real ThinkPad E495 is separate (E495.md).
Quick start
```powershell
cd bin
mingw32-make build
mingw32-make run          # virtio-vga, e1000, AC97 audio, NVMe
```
Guest audio check (host speakers via SDL audiodev):
```text
sound              # expect ac97: ready
sound tone 440     # PCM sine — should hear a beep
beep               # uses PCM when AC97 present
wav MEDIA/MEGALOVA.WAV   # PCM WAVE from guest FS (Esc to stop)
```
Default host backend is `sdl`. `dsound` often fails on Windows with
`Could not initialize DirectSoundCapture` when no capture device is present —
use `AUDIODEV=sdl` (default) or `AUDIODEV=wav` / `AUDIODEV=none` instead.

`make qemu-help` lists targets and variables.
Run targets
Target	What
`make run`	Default — `virtio-vga`, ~60 FPS present path
`make run-vga`	std VGA only — use for `gpu nogop` (DISPI BAR)
`make run-gl`	VirGL attempt + Windows-safe fallback (`run_gl.py`)
`make run-usb`	USB keyboard + relative USB mouse (in-tree xHCI HID)
`make run-snap`	Same as `run` + `-snapshot` (disk writes discarded)
`make run-fresh`	Reset OVMF vars, then `run`
Useful variables
```powershell
mingw32-make run              # default MEM=2G
mingw32-make run MEM=4G       # more RAM (GGUF / stress)
mingw32-make run USB_KBD=1 USB_MOUSE=1
mingw32-make run HOSTFWD=1          # host :18080 → guest :80
mingw32-make run SNAPSHOT=1
mingw32-make run RUN_EXTRA="-monitor vc"
```
Var	Effect
`MEM`	Guest RAM (default `2G`; `4G` YouTube/GUI; `8G` for large GGUF up to ~3.5 GiB)
`USB_KBD=1`	Add `usb-kbd`
`USB_MOUSE=1`	Add `usb-mouse` (relative HID)
`HOSTFWD=1`	Usernet port forward `18080→80`
`SNAPSHOT=1`	Discard IDE/NVMe writes on exit
`DISPLAY`	QEMU display backend (default `gtk`)
`RUN_EXTRA`	Raw extra QEMU args
`AUDIODEV`	Host audiodev driver (default `sdl`; avoid `dsound` if Capture init fails)
Troubleshooting: `cannot set up guest memory 'pc.ram': Invalid argument`
Windows often refuses a large contiguous guest RAM mapping after a heavy build or when commit charge is high. Fix:
```powershell
mingw32-make run-vga MEM=2G
# or: MEM=1536M
```
Do not use `8GB` / `8192MB` spellings — QEMU wants `2G`, `2048`, `4G`, etc.
Default `run` already has `qemu-xhci` + `usb-tablet` (absolute pointer).  
`run-usb` adds relative `usb-mouse` + `usb-kbd` for stressing the guest xHCI stack.
Disks (FAT16 boot + exFAT data)
Image	Bus	Format	Role
`fs.img`	IDE	FAT16	UEFI ESP + writable system / `MODELS` / configs
`nvme.img`	NVMe	exFAT	Data volume → guest `EXFAT/` (read-only + stream)
```powershell
mingw32-make prepare-nvme              # rebuild nvme.img (default 64 MiB)
mingw32-make prepare-nvme EXFAT_MIB=256
# optional: put ASCII files under bin/exfat_fs/ before prepare-nvme
```
Boot message should show `exFAT data (RO): … imported under EXFAT/`.  
`ls EXFAT` / `type EXFAT/TEST.TXT` — large GGUF/WAV on exFAT use stream reads.
Guest checklist (after boot)
```
ver
gpu                 # expect virtio-gpu on make run
gpu bench           # target >= 55 FPS, failures=0
gpu anim
gpu nogop           # virtio present path (or DISPI on run-vga)
mouse               # IRQs / pos; tablet or PS/2
xhci                # after run-usb / USB_* 
gui
net                 # NIC status (e1000 on QEMU)
```
Power
```
shutdown     # QEMU ACPI power-off (also: poweroff)
reboot
```
Ring 3 apps — `.CSR` (CursorOS)
Official app extension is `.CSR` (Cursor). On the Windows toolchain the shipped image is a flat loadable binary (`objcopy -O binary`); the kernel also accepts real ELF64 `ET_EXEC` if present. Legacy `.BIN` / `.ELF` still resolve as fallbacks.
```
run FILETEST.CSR    # open/read HELLO.TXT + gettime
run NETTEST.CSR     # TCP client GET 1.1.1.1:80 (needs DHCP)
run ECHOSRV.CSR     # TCP echo server on :80 (use HOSTFWD=1)
run SPAWNTEST.CSR   # pipe + spawn CHILD.CSR + wait exit 42
run SNAKE           # extension defaults to .CSR
```
Notepad (GUI): Ctrl+C copy all / Ctrl+V paste (system clipboard).  
Ring3: `sys_clip_*`, `sys_mmap_anon` / `sys_munmap`, `sys_bind` / `sys_listen` / `sys_accept` (R3V59+).
Browser default URL is YouTube Lite. Open `https://www.youtube.com/watch?v=…` (needs DHCP)
for a watch page that tries H.264 stream (InnerTube + z264); falls back to keyframe stills.
H.264: `play MEDIA/SAMPLE.264` or Files → open `.264` / `.mp4` (avc1). Decoder is z264
(Main/CABAC). YouTube downloads are capped (~2.5 MiB / Range) so long videos play a prefix.
Display notes — 描画標準化 (QEMU only)
Standard present path (`make run`):
Guest CPU paints the virtio-gpu guest framebuffer (WB).
Damage goes through `gpu_damage_rect` / `gfx2d` → `virtio_gpu_mark_dirty`.
Dirty area ≥ ~50% of the screen (or collapse overflow) → one full-frame transfer.
~60 FPS `virtio_gpu_present` (TRANSFER_TO_HOST_2D + FLUSH). Transient present failures
keep virtio live; only ~30 consecutive fails (~0.5 s) revert to VGA.
Resolution: host `GET_DISPLAY_INFO` when available, else 1280×720 (max 1920×1080).
Modeset: `表示とデスクトップ` or `gpu mode 1920x1080` → `virtio_gpu_set_mode`, then
desktop/windows/cursor redraw + full present (never DISPI while virtio is live).
Do not write the UEFI GOP LFB as the live scanout after virtio bind.  
Do not pair `-vga std` with `virtio-vga` (dual scanout / split FB).  
VirGL (`make run-gl`) and double-buffer are not the default GUI path.
Target	Role
`make run`	Canonical — `-device virtio-vga` present + virtio modeset
`make run-vga` + `gpu nogop`	Regression — Bochs DISPI BAR LFB (no virtio)
ThinkPad E495	Out of scope here — firmware GOP + `gpu_shadow`
Prefer `make run` (virtio). Avoid `-vga none` alone → blank “Display output is not active.”
`gpu nogop` on `run` = virtio (no GOP writes). On `run-vga` = Bochs DISPI BAR LFB.
Acceptance: `gpu bench` ≥55 FPS, `failures=0` at 1280×720 / 1920×1080.
Windows VirGL often AVs; `run-gl` falls back automatically.
Input notes
Host device	Guest path
PS/2 keyboard (default)	i8042
`usb-tablet` (default)	xHCI absolute HID
`run-usb` / `USB_MOUSE=1`	xHCI relative mouse
`USB_KBD=1`	xHCI keyboard
Network
Usernet + e1000. With `HOSTFWD=1`, guest `:80` is reachable from the host as `:18080`:
```powershell
# guest: run ECHOSRV.CSR
# host:
curl http://127.0.0.1:18080/
# or:  echo hello | nc 127.0.0.1 18080
```
Safe destructive tests
```powershell
mingw32-make run-snap
# guest: install wipe YES   — NVMe image restored on QEMU exit
```
Paths
Edit `bin/Makefile` if needed:
`QEMU` — `qemu-system-x86_64.exe`
`OVMF_CODE` — EDK2 code FD
`W64_PATH` — w64devkit `bin`
vs real E495
	QEMU	E495
GPU	virtio / DISPI	GOP + shadow only
Mouse	PS/2 + USB tablet	PS/2 flaky bit5; prefer USB later
Net	e1000 usernet	Realtek RTL8168 (`rtl8169`)
