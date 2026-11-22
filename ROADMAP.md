# CursorOS Series 3 / 4 — Roadmap

## R3V61（出荷済み）

WAV/AC97・`assoc`・真プリエンプト可視化・Start ICO。  
成果物: `dist/CursorOS-R3V61.zip`。

---

## R3V62（出荷済み — 実用デスクトップ基盤）

IME・解像度・GUI メッセージキュー・AI syscalls・exFAT・vnode P0。  
成果物: `dist/CursorOS-R3V62.zip`。

- [x] メモ帳 IME（ローマ字→ひらがな）+ UTF-8 描画 + タスクバー「あ/A」
- [x] WAV バックグラウンド再生（GUI 操作と両立、Esc で停止）
- [x] 解像度アップ（既定 1280×720、最大 1080p）+ virtio `set_mode` 本線化
- [x] GUI 入力をメッセージキュー化（Win32 PostMessage / DispatchMessage 型）
- [x] AI 系システムコール（`SYS_AI_*`）+ `AITEST.CSR`
- [x] GGUF 読み取り上限 ~3.5 GiB + ブートディスク ~4 GiB
- [x] exFAT 標準化（NVMe データボリューム + 大容量ストリーム、ESP は FAT16）
- [x] **ファイル/メモリ現代OS化 P0**: `vnode` + page cache + fdtable/llama 統合

---

## R3V63（出荷済み — mmap / Assistant / QEMU 描画）

vnode mmap・AI ツール拡充・virtio 描画ポリッシュ。  
成果物: `dist/CursorOS-R3V63.zip`。

- [x] vnode: file-backed mmap / demand paging（`vmm`・GGUF は連続 below-4G 不要）
- [x] AI Assistant ツール拡充（mem/net/gpu/sound/proc/time/disk/vfs + sound.beep）
- [x] QEMU 描画系ポリッシュ（present 連続失敗のみ disable、dirty≥50%→full、modeset 後フル再描画、damage 統一）

---

## R4V1（現在 — Ring 3 分離 / SMP / モダンドライバ / SDK）

> **ステータス**: 実験的リリース（実用未達・既知のセキュリティ制限あり。詳細は [KNOWN_ISSUES.md](KNOWN_ISSUES.md)）

- [x] ソースツリー 12 サブシステム再構成 & `Makefile.r3v63` (`OPT=-O2` / `SMOKE_TEST=1`)
- [x] Ring 3 ELF64 ダイナミックローダ + 最小 C SDK（`sdk/crt0.S`, `csr.h`, `user.ld`, サンプル 4 本）
- [x] ユーザ仮想空間を `PML4[1]`（`0x8000000000`）へ分離、W^X (`PF_W|PF_X` 拒否)・`PAGE_NX` 保持、`spinlock` (irqsave) 導入
- [x] detached / 孤児ゾンビ回収、ターミナル `exec` の `sched_spawn_user_detached` 化、GUI ループ周期回収
- [x] SMP マルチコア起動（MADT + `0x8000` トランポリン）& 並列 Q16.16 AI 推論
- [x] Intel HDA オーディオ、AHCI SATA DMA、VirtIO-Net + DHCP/DNS キャッシュ
- [x] GUI ターミナル、Web ブラウザ（URL バー）、辞書ベース漢字変換 IME、高解像度アイコン＆カーソル最適化
- [x] `smoke_test.sh` 厳格化 + カーネル内自己テスト経路（`R3V63_SMOKE_TEST`）

---

## R4V2（次）

- [ ] **セキュリティ & 安定性**: Ring 3 プロセス級の強制タイマープリエンプト、BearSSL CA 証明書検証の厳格化、`sys_exec`（syscall 61）の detached/独立スロット化
- [ ] **UEFI ブート堅牢化**: `boot.c` の `ExitBootServices` 失敗時 `GetMemoryMap` リトライ実装、`SystemTable->ConfigurationTable` からの `h.rsdp` 取得
- [ ] **メモリ最適化**: VFS プリロード縮小（ディレクトリ＋小ファイルのみ、128 MiB `.bss` 削減）
- [ ] **データ保護・復旧**: FAT16 ジャーナル／整合性チェック支援コマンドの追加
- [ ] **Audio**: AAC デコーダ統合による H.264 / YouTube 音声再生対応
- [ ] **CI / 実機検証**: QEMU + 実機 E495 での `SMOKE_TEST=1` 完全通過確認

## 意図的に載せないもの

- Chromium 級フルブラウザ／完全な YouTube SPA
- DRM / Widevine
- 本番向けサンドボックス保証（現段階では研究・学習・実験用途に限定）
