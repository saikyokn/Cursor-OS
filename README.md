# Saikyokn OS (x64 Native / UEFI Boot)
Developed with ❤️ by [Saikyokn]

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Arch](https://img.shields.io/badge/Arch-x86__64-orange.svg)
![Platform](https://img.shields.io/badge/Platform-UEFI-green.svg)

**Saikyokn OS** は、現代的なコンピューティング環境を目指してゼロから開発されている、x86-64アーキテクチャ専用の自作オペレーティングシステムです。

## 🚀 プロジェクトの概要

多くの自作OSプロジェクトがレガシーなBIOS形式を採用する中、本プロジェクトでは**UEFI (Unified Extensible Firmware Interface)** をベースとしたモダンなブートプロセスを採用しています。16bit/32bitの制約に縛られず、起動直後から64bitロングモードの広大なメモリ空間と高度なCPU機能を活用することを設計思想としています。

## 🛠 技術仕様 (Technical Specifications)

- **Architecture:** x86_64 (Intel 64 / AMD64)
- **Bootloader:** UEFI Application (GOPによる高解像度出力対応)
- **CPU Mode:** 64-bit Long Mode (paging enabled)
- **Memory Management:** 4-Level Paging / Physical Memory Manager
- **Graphics:** UEFI Graphics Output Protocol (GOP) Framebuffer
- **Language:** Assembly (nasm), C / C++

## ✨ 主な特徴

- **UEFI Native Boot:** BIOS互換機能(CSM)に頼らず、純粋なUEFI環境で動作。
- **Modern Architecture:** 汎用レジスタの拡張(R8-R15)や、効率的な引数渡しを活用したx64ネイティブ実装。
- **Original Kernel:** マイクロカーネル的なアプローチを目指した独自設計のカーネル。
- **GUI Foundation:** フレームバッファを直接制御し、独自の描画エンジンによるデスクトップ環境を構築。

## 📂 ディレクトリ構成（例）

```text
.
├── bootloader/    # UEFIブートローダーのソースコード
├── kernel/        # x64カーネル本体
├── common/        # 共通ライブラリ・ヘッダー
├── scripts/       # ビルド・エミュレータ起動用スクリプト
└── limine.conf    # (ブートローダーにLimine等を使用している場合)
