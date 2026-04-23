# usb-display

USB での有線 LAN 接続を使って、タブレットやノート PC を軽量な拡張ディスプレイとして扱うためのプロジェクトです。

このリポジトリは、まず「とにかく映る」ことを目標にした MVP から始めます。最初は USB 仮想 LAN 上の TCP 通信でフレームを送り、受信側で表示できる JPEG を保存する構成を前提にします。

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

## 画像送受信の最短デモ（MVP）

同一マシン上でも動作確認できます（実運用は USB 仮想 LAN 上の sender/receiver を想定）。

```sh
cmake -S . -B build
cmake --build build

# terminal 1 (receiver)
USB_DISPLAY_OUT_DIR=/tmp/usb-display-frames USB_DISPLAY_MAX_FRAMES=120 \
  ./build/apps/windows-receiver/usb-display-receiver

# terminal 2 (sender)
USB_DISPLAY_MAX_FRAMES=120 ./build/apps/windows-sender/usb-display-sender
```

任意の JPEG を送る場合:

```sh
USB_DISPLAY_INPUT_JPEG=/path/to/input.jpg USB_DISPLAY_MAX_FRAMES=120 \
  ./build/apps/windows-sender/usb-display-sender
```

連番 JPEG を映像として送る場合（`0001.jpg, 0002.jpg...` など）:

```sh
USB_DISPLAY_INPUT_DIR=/path/to/jpeg-sequence USB_DISPLAY_MAX_FRAMES=300 USB_DISPLAY_FPS=30 \
  ./build/apps/windows-sender/usb-display-sender
```

受信したフレームは `USB_DISPLAY_OUT_DIR` 配下に `frame-000001.jpg` 形式と `latest.jpg` として保存されます。`latest.jpg` を画像ビューアで開き直すと、送信状況を目視できます。

### USB 仮想 LAN で 2 台接続する例

1. 送信 PC と受信 PC を USB ケーブルで接続し、OS 側で USB tethering / RNDIS / ECM などの仮想 LAN を有効化する。
2. 受信 PC で `usb-display-receiver` を起動（`USB_DISPLAY_PORT` は必要に応じ変更）。
3. 受信 PC 側の仮想 LAN IP を確認（例: `192.168.137.2`）。
4. 送信 PC で `USB_DISPLAY_HOST=192.168.137.2` を指定して `usb-display-sender` を起動。

## 次フェーズ: 拡張ディスプレイとして認識させる

現行 MVP は「JPEG フレームの送受信」までです。Windows の拡張ディスプレイ認識には IDD (Indirect Display Driver) を次段階で統合します。

- IDD ドライバの最小実装（仮想モニタ列挙）
- 受信側レンダラを IDD の提示サーフェスへ接続
- ホスト側キャプチャを Desktop Duplication API へ差し替え
- 遅延計測を基にフレームスキップ・品質制御を追加

## 現在の優先順位

1. 固定領域キャプチャを作る
2. JPEG 圧縮 + TCP 送信を安定化
3. 受信側デコードと描画を作る
4. 遅延と帯域を測る
5. IDD 統合で OS 可視な拡張ディスプレイ化

## 編集バージョン運用

- 現在の編集バージョンは [EDIT_VERSION](/Users/naginagi/Documents/Codex/2026-04-22-git-hub/repo/EDIT_VERSION) で管理します。
- 各編集バッチごとに `versions/2026-04-22-git-hub-vX.Y.Z/` フォルダを作成します。
