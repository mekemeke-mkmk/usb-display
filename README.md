# usb-display

USB での有線 LAN 接続を使って、タブレットやノート PC を軽量な拡張ディスプレイとして扱うためのプロジェクトです。

このリポジトリは、まず「とにかく映る」ことを目標にした MVP から始めます。最初は USB 仮想 LAN 上の TCP 通信でフレームを送り、受信側で描画する構成を前提にします。

## コンセプト

- USB 2.0 クラスの帯域でも動作可能
- 有線前提で低遅延を重視
- 既存の GPU 映像出力やドックに依存しない
- 将来的には OS から仮想ディスプレイとして認識させる
- Windows 先行、将来はマルチ OS 対応

## MVP の方針

- 解像度: `1280x720`
- fps 上限: `30`
- 圧縮: `JPEG`
- 通信: `TCP over USB virtual LAN`
- 対象: 送信側で画面の一部をキャプチャし、受信側で表示する

## ディレクトリ構成

- `docs/` 要件、アーキテクチャ、ロードマップ
- `proto/` フレームプロトコル定義
- `src/common/` 送受信で共有する型・設定・テレメトリ
- `apps/windows-sender/` Windows 送信側の実装方針
- `apps/windows-receiver/` Windows 受信側の実装方針
- `scripts/` 環境確認スクリプト
- `tools/` 依存の少ない補助ツール
- `versions/` 編集バージョンをフォルダ名で管理

## すぐ使うコマンド

```sh
./scripts/check-env.sh
node ./tools/print-plan.mjs
node ./tools/validate-docs.mjs
./scripts/test-smoke.sh
```

`cmake` がある環境では次も実行できます。

```sh
cmake -S . -B build
cmake --build build
```

## 現在の優先順位

1. Windows 向け MVP の実装言語を決める
2. 固定領域キャプチャを作る
3. JPEG 圧縮 + TCP 送信を作る
4. 受信側デコードと描画を作る
5. 遅延と帯域を測る

## 編集バージョン運用

- 現在の編集バージョンは [EDIT_VERSION](/Users/naginagi/Documents/Codex/2026-04-22-git-hub/repo/EDIT_VERSION) で管理します。
- 各編集バッチごとに `versions/2026-04-22-git-hub-vX.Y.Z/` フォルダを作成します。
