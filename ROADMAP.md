CursorOS Series 3 / 4 — Roadmap
R4V1（出荷済み — Ring 3 分離 / SMP / モダンドライバ / SDK）
[x] ソースツリー 12 サブシステム再構成 & `Makefile.r3v63` (`OPT=-O2` / `SMOKE_TEST=1`)
[x] Ring 3 ELF64 ダイナミックローダ + 最小 C SDK（`sdk/crt0.S`, `csr.h`, `user.ld`）
[x] ユーザ仮想空間を `PML4[1]`（`0x8000000000`）へ分離、W^X・`PAGE_NX`・`spinlock` 導入
[x] detached / 孤児ゾンビ回収、GUI ループ周期回収 (`sched_poll_gui`)
[x] SMP マルチコア起動（MADT + `0x8000` トランポリン）& 並列 Q16.16 AI 推論
---
R4V2（現在 — Ring 3 プリエンプト / 実用ネット・GUI / 堅牢化）
> **ステータス**: 実験的リリース（実機長期運用は未検証。詳細は [KNOWN_ISSUES.md](KNOWN_ISSUES.md)）
[x] セキュリティ & 安定性: Ring 3 強制タイマープリエンプト、BearSSL CA 証明書検証（ISRG/DigiCert/GTS）、`sys_exec` detached/独立スロット化、`usermode_ring3_return_stub` による callee-saved レジスタ復元
[x] UEFI ブート堅牢化: `boot.c` の `ExitBootServices` 最大 8 回リトライ、`ConfigurationTable` からの ACPI `h.rsdp` 取得
[x] メモリ最適化: VFS 128 MiB 静的プール廃止（PMM 動的ページ割り当てへ移行）
[x] データ保護・復旧: FAT16 整合性チェック & 修復コマンド（`fsck [fix]`）、NVMe ESP インストーラ（`BOOTX64.EFI` 実書き込み & GUI）
[x] Ring 3 & アプリ拡張: PE32+（`.exe`）ローダ + `HELLO.EXE`、マウスイベント（syscall 58）、電卓・ファイルビューア・ペイント・テキストエディタ、`×` ボタン強制回収
[x] GUI & メディア: PNG/ICO 内包 PNG デコーダ、画像ビューア（画面クリップ＋自動解放）、フォルダ移動対応ファイラー、スタートメニュースクロール＆アプリ一覧、48×48 アイコン & グラデーション壁紙、xHCI USB Tablet、AAC 音声統合（`aac_dec.c`）、NVIDIA PAT Write-Combining 加速
[x] CI / 自動検証: `.github/workflows/build.yml`（SDK/アイコン同期チェック + `-O0/-O2` × `NO_SMP=0/1` ビルド + QEMU スモークテスト + `CRASH_R3.ELF` `#PF` 復旧テスト）
---
R4V3（次）
[ ] FAAD2 / フル AAC-LC 復号統合: 外部 FAAD2 または完全 IMDCT パイプライン統合による高音質 MP4 / YouTube 音声再生
[ ] exFAT ジャーナル & 破損復旧: `fsck` の exFAT / NVMe セクタ修復対応
[ ] 実機検証: ThinkPad E495 での `SMOKE_TEST=1` 完全通過および GPU/オーディオ連続稼働確認
