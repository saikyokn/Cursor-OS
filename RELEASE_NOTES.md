CursorOS Series 4 R4V2 — Release Notes
> \*\*位置づけ\*\*: R4V1 で導入した Ring 3 分離・SMP・モダンドライバを基盤に、\*\*強制プリエンプト・厳格 TLS CA 検証・PE32+ (`.exe`)・PNG 画像ビューア・USB Tablet・FAT16 `fsck`・VFS `.bss` 128 MiB 削減\*\* を統合した R4V2 リリースです。
主な新機能・改善
Ring 3 実行基盤 & セキュリティ
Ring 3 強制タイマープリエンプト: `irq\_timer` / `sched\_preempt\_from\_irq\_frame` により CS=Ring 3 の長時間タスクを自動切り替え（`-mgeneral-regs-only` で XMM 破壊なし）。
`usermode\_ring3\_return\_stub`: Ring 3 終了および `#PF` 復帰時に MS x64 ABI の callee-saved レジスタ 8 本を確実に復元（`-O0` でも安全復帰）。
PE32+ (`.exe`) ローダ: `usermode\_load\_pe()` で W^X / 境界検証付きで `HELLO.EXE` 等を実行可能。
SDK アプリ拡充 & マウス対応: `calc\_r3`, `files\_r3`, `paint\_r3`, `edit\_r3`, `crash\_r3` を追加し、`×` ボタンによるタスク回収とマウスクリック（syscall 58）に対応。
GUI / メディア / 入力
PNG デコーダ & 画像ビューア: RFC 1950/1951 DEFLATE + PNG（RGBA/RGB/Palette/Gray/Adam7/PNG 入り ICO）を `image.c` に実装（`-mno-mmx -mgeneral-regs-only` で安全実行）。
ファイラー強化: フォルダ移動（`\[DIR]` / `\[..]`）・ホイールスクロール・拡張子別起動（PNG/BMP/ICO → 画像ビューア、`.exe/.elf` → 実行、`.wav` → 再生、テキスト → メモ帳）。
UI 高画質化 & 入力高速化: バイリニア補間、48×48 アンチエイリアスアイコン（`systemsource/image/`）、既定グラデーション壁紙、スクロール可能スタートメニュー＋アプリ一覧ウィンドウ、バッチキー入力。
xHCI USB Tablet: `-device qemu-xhci -device usb-tablet` でホストと 1:1 座標同期。
AAC 音声統合: `aac\_dec.c` により MP4（`mp4a`）再生時のオーディオ出力を統合。
ネットワーク / ストレージ / ブート / メモリ
TLS CA 検証 & HTTP/1.1: `ca\_bundle.h`（ISRG Root X1 / DigiCert G2 / GTS Root R1）と CMOS 時刻検証を既定有効化、平文 HTTP/1.1・自動 DHCP・リダイレクト追跡・`curl` / `dhcp` / `dns` を追加。
VFS `.bss` 128 MiB 削減: 静的プールを廃止し PMM 動的ページ割り当てへ移行。
FAT16 `fsck \[fix]` & NVMe インストーラ: FAT1/FAT2 ミラーとクラスタチェーンの検査・修復、`BOOTX64.EFI` 実書き込み＋GUI インストーラを追加。
NVIDIA PAT WC 加速: `IA32\_PAT` Write-Combining + 64bit `rep movsq` バースト転送を実装。
UEFI ブート堅牢化: `ExitBootServices` 8 回リトライと ACPI RSDP 取得を実装。
