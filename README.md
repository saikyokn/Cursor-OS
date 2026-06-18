# CursorOS Series 3

**Version R3V49** — UEFI x86-64 hobby OS with desktop GUI, Ring 3 apps, and a built-in HTML/CSS viewer.

## Features

- UEFI boot, paging, PMM, preemptive scheduler
- Desktop: Console, Files, Notepad, Paint, Calculator, Calendar, Memory, Tasks
- Ring 3 binaries: Tetris, Snake, Rust compute worker (`RCOMPUTE.BIN`)
- **Web Browser**: opens `.html` from the FAT filesystem (DOM + CSS subset + framebuffer render)
- Network stack (e1000), NVMe, USB keyboard/mouse

## Requirements

| Tool | Purpose |
|------|---------|
| [w64devkit](https://github.com/skeeto/w64devkit) or MinGW-w64 GCC | Build EFI kernel |
| Python 3 | `prepare_fsimg.py` |
| QEMU + OVMF | Run in VM |

Edit paths in `bin/Makefile` if your w64devkit/QEMU install differs:

```makefile
W64_PATH   = E:/w64devkit/w64devkit/bin
QEMU       = E:/qemu/qemu-system-x86_64.exe
OVMF_CODE  = E:/qemu/share/edk2-x86_64-code.fd
```

## Build and run

```powershell
cd bin
mingw32-make build    # rebuild kernel + fs.img
mingw32-make run     # QEMU
```

Kernel-only link (CI):

```powershell
mingw32-make ci USE_PATH_GCC=1
```

## First boot demo

1. **Files** → `WEB/APP.HTML` → Web Browser  
2. Click **Click Demo** → `ready` updates to `clicked: …s`  
3. **Tasks** → Task Manager (RAM, scheduler)  
4. **Tetris** / **Rust Proc** → Ring 3 apps  
5. **Console** → `ver`, `help`, `ls`

## Project layout

```
kernel/     OS kernel (Ring 0), web engine, GUI
user/       Ring 3 assembly demos
rust/       Rust user binaries
cpp/        C++ user binaries
fs/         Files copied into fs.img (WEB/, EFI/, …)
bin/        Makefile, QEMU run scripts
```

## Web Browser (honest scope)

- Parses real HTML files and applies a **limited** CSS subset  
- **Japanese** via `FONTS/MPLUS.TTF` (TrueType, GUI + Web Browser)
- **Does not** run page `<script>` tags (R3V49)
- Demo button `#helloBtn` is wired in C for stability  
- ASCII text renders best; CJK is not fully supported  

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md).

## License

MIT — see [LICENSE](LICENSE). Third-party components: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Release notes

[RELEASE_NOTES.md](RELEASE_NOTES.md)
