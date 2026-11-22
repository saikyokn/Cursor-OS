# CursorOS Series 4 — R4V1 Release Notes

**Version:** R4V1  
**Target:** QEMU / OVMF および ThinkPad E495 等の x86-64 UEFI 環境  
**Status:** 実験的リリース（非実用・開発検証向け）

---

## 概要

R4V1 は、R3V63 をベースにソースツリーを 12 サブシステムへ再構成し、**Ring 3 ELF64 実行環境と最小 C SDK**、**SMP マルチコア並列 AI 推論**、**Intel HDA / AHCI SATA / VirtIO-Net**、**GUI ターミナル／Web ブラウザ／漢字変換 IME**、**アドレス空間分離（`PML4[1]`）および W^X / NX / Spinlock 強化** を統合した開発マイルストーンです。

- 実機手順: [E495.md](E495.md)
- QEMU 手順: [QEMU.md](QEMU.md)
- 既知の問題・セキュリティ詳細: [KNOWN_ISSUES.md](KNOWN_ISSUES.md)
- 次版計画: [ROADMAP.md](ROADMAP.md)

---

## ⚠️ セキュリティおよび実用可否に関する重要なお知らせ

**CursorOS R4V1 はまだ実用段階ではありません。** 以下の理由により、本番業務・日常利用・信頼できないコードや Web ページの実行には使用しないでください。

1. **既知のセキュリティ脆弱性・隔離の限界**:
   - **Ring 3 の強制プリエンプト未実装**: 悪意ある、あるいは無限ループに陥った Ring 3 アプリが CPU を占有し続ける可能性があります。
   - **Web ブラウザ／TLS の検証緩和**: BearSSL 接続は証明書チェーン／ホスト名の厳格な信頼検証を行わない経路が残っており、ブラウザにもプロセス単位の完全なサンドボックスはありません。
   - **一部レガシー経路の残存**: `sys_exec`（システムコール 61）や `usermode_load_bin` はスロット 1 固定経路を残しています（ターミナルの `exec` は `sched_spawn_user_detached` へ移行済み）。
   - **カーネル境界・ブート経路の未完部分**: VFS/FAT 書き込みの権限分離が未成熟であるほか、`boot.c` の `ExitBootServices` 失敗時リトライや RSDP 引き渡しが未完のため、実機ファームウェア環境によっては起動が不安定になる場合があります。

---

## 💾 データ保護とデータ復旧について

実験用 OS の性質上、ストレージ操作やカーネルパニックによってファイルシステムが破損するリスクがあります。

### 事前のデータ保全（推奨）
- **QEMU 利用時**: 通常の試行では `run-snap`（`-snapshot` 相当）を使用するか、`fs.img` / `nvme.img` のコピーを別途退避してください。
- **実機テスト時**: **必ず Windows 等が入っていない検証用空ディスク／テスト用パーティション**を使用し、事前にフルバックアップを取得してください。`install wipe YES` は GPT 全体を書き換える破壊的操作です。
- **exFAT 保護**: NVMe 上の exFAT（`EXFAT/`）はゲスト内で**読み取り専用（RO）**としてマウントされ、誤書き込みを防ぐ設計になっています。

### トラブル時・起動不能時のデータ復旧手順
1. **UEFI 実機で起動できなくなった場合**:
   - PC のファームウェアブートメニュー（F12 等）から元の OS（Windows Boot Manager 等）を選択して起動できます。
   - ESP（EFI System Partition）上の `\EFI\CURSOROS\` フォルダを削除すれば CursorOS のブートエントリを安全に除去できます（他の OS の `\EFI\Microsoft\` 等は触らないでください）。
2. **FAT16 イメージ／パーティションのデータ復旧**:
   - CursorOS による FAT 書き込み中にハングやパニックが起きた場合は、別の正常な OS（Linux / Windows）からイメージやドライブをマウントし、`fsck.vfat`（Linux）や `chkdsk`（Windows）で整合性を修復したうえで重要ファイルを救出してください。
   - 修復前にイメージファイル（`fs.img`）やパーティションの生ダンプを取ってから作業することを強く推奨します。
3. **カーネルパニック発生時の調査情報**:
   - 画面または `debugcon` ログに出力される `CR2`、`RIP`、直前の操作内容、および `SMOKE_TEST=1` での再現可否を記録してください。

---

## R4V1 新機能ハイライト

### 1. Ring 3 ELF64 分離実行と C SDK (`sdk/`)
- ユーザ仮想アドレス空間を `PML4[1]`（`0x8000000000` 以降）へ完全移設し、カーネル恒等マップ（`PML4[0]`）との衝突を構造的に解消。
- `PT_LOAD` 範囲検査（`[USER_CODE_BASE, USER_HEAP_BASE)`）、重なり検査、`PF_W | PF_X` 拒否、`PAGE_NX`（ビット 63）保持、`usermode_ptr_ok` の厳格化、`munmap` の境界チェック＋物理ページ解放を実装。
- `sdk/crt0.S`、`sdk/csr.h`、`sdk/user.ld`、`sdk/Makefile` を整備し、サンプル 4 本（`HELLO_SDK.ELF`, `SYSMON_R3.ELF`, `AI_R3.ELF`, `SNAKE_R3.ELF`）を VFS へ埋め込み。
- タスク作成時に `is_user` / `detached` / 世代カウンタをロック内で設定し、ターミナル `exec` と GUI ループ（`sched_poll_gui()`）による自動回収に対応。

### 2. メモリ・ロック・SMP マルチコア
- `spinlock.h`（`spin_lock_irqsave`）を導入し、PMM およびスケジューラへ適用。
- PMM を最大 64 GiB（64 ビットビットマップ高速スキャン）へ拡張。
- ACPI MADT 解析と `0x8000` リアル→プロテクト→ロングモードトランポリンによる AP 起動、および `smp_parallel_for` による Q16.16 AI 推論の並列化を実装。

### 3. ドライバ・ネットワーク
- **Intel HDA** オーディオドライバ（AC'97 自動フォールバック付き）を追加。
- **AHCI SATA DMA** ドライバを追加。
- **VirtIO-Net** ドライバを追加し、DHCP/DNS リトライおよび 8 エントリ DNS キャッシュを実装。
- BearSSL 接続で葉証明書の公開鍵を `br_x509_decoder` 経由で正しく取得するよう修正。

### 4. GUI・日本語 IME・開発ツール
- **GUI ターミナル**（`help`, `ls`, `cat`, `exec <elf>`, `browse`, `cpu`, `disk`, `mem` 等）と **GUI Web ブラウザ**を追加。
- **辞書ベース漢字変換 IME**（Space 変換・候補バー・`MPLUS.TTF` 自動初期化・豆腐フォールバック）を追加。
- ウィンドウのドラッグ移動／多辺リサイズ／Z 順／最小化・最大化、高解像度アイコンキャッシュ描画、ダーティ矩形・カーソル退避による描画高速化を実装。
- `bin/Makefile.r3v63` で `OPT=-O2` ビルド、および `bin/smoke_test.sh`（`SMOKE_TEST=1` セルフテスト含む）に対応。

---

## ビルド方法

```powershell
cd bin
mingw32-make -f Makefile.r3v63 csr
mingw32-make -f Makefile.r3v63 CC=C:/Users/saikyokn/Downloads/w64devkit/bin/gcc.exe OPT=-O2
```

セルフテスト有効版:
```powershell
mingw32-make -f Makefile.r3v63 CC=C:/Users/saikyokn/Downloads/w64devkit/bin/gcc.exe OPT=-O2 SMOKE_TEST=1
```
