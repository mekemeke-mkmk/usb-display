# MVP Specification

## Goal

USB 仮想 LAN を経由して、キャプチャ、圧縮、送信、受信、描画までを一通り成立させること。

## In scope

- Fixed resolution: `1280x720`
- Frame rate cap: `30 fps`
- Wired connection via USB virtual LAN
- TCP transport
- JPEG compression
- Single display stream

## Out of scope

- Virtual display driver
- Audio
- Touch input
- Multi-monitor
- Cross-platform support

## Acceptance criteria

- Receiver displays frames from the sender
- Sender can recover from temporary receiver restart
- Frame pacing remains stable without unbounded memory growth
- Basic telemetry reports fps, average frame size, and dropped frames
