#!/bin/sh

set -eu

if ! command -v clang++ >/dev/null 2>&1; then
  echo "[fail] clang++ is required for smoke test"
  exit 1
fi

out_bin="/tmp/usb-display-protocol-smoke"
stream_bin="/tmp/usb-display-stream-smoke"

clang++ \
  -std=c++20 \
  -Wall \
  -Wextra \
  -Werror \
  -I src/common/include \
  src/common/frame_protocol.cpp \
  tests/protocol_smoke.cpp \
  -o "$out_bin"

"$out_bin"

clang++ \
  -std=c++20 \
  -Wall \
  -Wextra \
  -Werror \
  -I src/common/include \
  src/common/frame_protocol.cpp \
  src/common/frame_stream.cpp \
  tests/frame_stream_smoke.cpp \
  -o "$stream_bin"

"$stream_bin"
