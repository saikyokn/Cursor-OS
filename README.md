CursorOS Series 4 — R4V1
Version R4V1 — Ring 3 ELF64 (`PML4[1]` 分離 + W^X/NX) & C SDK、SMP マルチコア並列 AI 推論、Intel HDA / AHCI / VirtIO-Net、GUI ブラウザ・ターミナル・漢字変換 IME を統合した実験的 x86-64 UEFI OS です。
> **⚠️ 実用・セキュリティに関する注意（重要）**  
> CursorOS R4V1 は**実験・検証段階の OS** であり、**まだ実用（日常利用・本番運用）には達していません**。メモリ保護や ELF 検証は強化されていますが、依然として既知のセキュリティ脆弱性や制限（証明書検証の緩和、ブラウザ／Ring 3 の不完全なサンドボックス、一部システムコールやブート処理の未成熟など）が残っています。**メイン PC への直接導入や、信頼できない外部バイナリ／Web ページの実行、消えては困るデータを含むディスクへの書き込みは行わないでください。** 詳細は [KNOWN_ISSUES.md](KNOWN_ISSUES.md) と [RELEASE_NOTES.md](RELEASE_NOTES.md) を参照してください。
What's New in R4V1
Ring 3 ELF64 ダイナミックローダ & C SDK (`sdk/`):
`sdk/crt0.S` / `sdk/csr.h` / `sdk/user.ld` / `sdk/Makefile` を整備し、`make -f Makefile.r3v63 csr` でユーザ空間アプリ（`HELLO_SDK.ELF`, `SYSMON_R3.ELF`, `AI_R3.ELF`, `SNAKE_R3.ELF`）をビルド・VFS 埋め込み可能
ユーザ空間を `PML4[1]`（`0x8000000000` 以降）へ完全分離し、カーネル恒等マップ（`PML4[0]`）と構造的に隔離
W^X（`PF_W | PF_X` 拒否）・`PAGE_NX`（ビット 63）保持・ページ単位 PTE 検証・`spinlock`（irqsave 対応）を適用
ターミナルの `exec <elf>` を `sched_spawn_user_detached()` 化し、GUI ループ（`sched_poll_gui()`）での周期回収に対応
SMP マルチコア & 並列 AI 推論:
ACPI MADT パーサ + `0x8000` トランポリンによる AP 起動、`smp_parallel_for` による Q16.16 行列積の並列化
モダンドライバ & ネットワーク:
Intel HDA オーディオ（AC'97 フォールバック付き）、AHCI SATA DMA、VirtIO-Net + DHCP/DNS リトライ＆キャッシュ
GUI・アプリ・ビルド:
GUI ターミナル、Web ブラウザ（URL バー、`about:home` / VFS / HTTP・HTTPS）、日本語辞書漢字変換 IME（Space 変換 + 候補バー + `MPLUS.TTF` 自動初期化）
ウィンドウドラッグ／多辺リサイズ／Z 順／最小化・最大化、高解像度アイコン＆カーソル描画最適化
`Makefile.r3v63` で `OPT=-O2` ビルドおよび `SMOKE_TEST=1` 自動自己テストに対応
Features (Core)
UEFI boot、64 GiB PMM、4 階層ページング、vnode + pagecache + file-backed mmap（demand paging）
ローカル AI エージェント（`aios` / `llama_os`）+ Ring 3 AI システムコール（`SYS_AI_*`）
FAT16（起動・設定）+ exFAT（NVMe データ、読み取り専用ストリーミング `EXFAT/`）
H.264（z264）・WAV 再生・YouTube Lite
Secure Boot 対応（shim + MOK 署名スクリプト）
Build and Run
```powershell
# SDK + カーネルビルド（Windows w64devkit / MinGW-w64）
cd bin
mingw32-make -f Makefile.r3v63 csr
mingw32-make -f Makefile.r3v63 CC=C:/Users/saikyokn/Downloads/w64devkit/bin/gcc.exe OPT=-O2
```
QEMU での実行やテスト手順は QEMU.md、実機（ThinkPad E495）向け手順は E495.md を参照してください。自動スモークテストは `bin/smoke_test.sh`（`SMOKE_TEST=1` ビルド推奨）で行えます。
> **💾 データ保護の推奨**: テスト時は `mingw32-make run-snap`（ディスク変更を破棄）を使うか、事前に `fs.img` / `nvme.img` および実機ディスクのフルバックアップを取得してください。
Project Layout
```
kernel/
  arch/     UEFI boot, GDT/IDT/TSS, APIC, SMP, interrupt, debugcon
  core/     kernel_main, scheduler, syscall, usermode (ELF64), ipc, error
  mm/       PMM (spinlock), paging (PML4[1] user AS), VMM, pagecache, swap
  fs/       VFS, vnode, fdtable, GPT, FAT16, exFAT, installer
  drivers/  PCI, ATA, AHCI, NVMe, xHCI, mouse, NIC (e1000/rtl8169/virtio-net), HDA/AC97
  gpu/      virtio-gpu, shadow FB, damage tracking, AMD/Intel probes
  gui/      window manager (gui_link), icons, console, TTF/IME
  net/      TCP/IP, BearSSL TLS, Web DOM/CSS/JS/Render, YouTube Lite
  media/    WAV, MP4/AnnexB, H.264 (z264)
  ai/       AIOS, llama_os, lm_infer (SMP parallel)
  apps/     terminal, browser, taskmgr, memmgr, calendar, games, ring3_app
  shims/    CRT/math/Win32 shims
sdk/        Ring 3 C SDK (crt0.S, csr.h, user.ld, sample ELFs)
bin/        Makefile.r3v63, smoke_test.sh, image helpers
```
Documentation
RELEASE_NOTES.md — R4V1 リリースノート（新機能・セキュリティ・データ復旧）
KNOWN_ISSUES.md — 既知の不具合・セキュリティ課題・データ保全と復旧
ROADMAP.md — R4V1 実績と R4V2 計画
QEMU.md / E495.md / THIRD_PARTY_NOTICES.md
