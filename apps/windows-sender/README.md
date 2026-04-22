# windows-sender

Planned Windows-first sender implementation.

## Responsibilities

- Capture desktop frames
- Apply frame pacing
- Encode JPEG payloads
- Stream protocol frames to receiver
- Report capture and encode telemetry

## First milestone

Capture a fixed rectangle and emit a saved JPEG frame to verify the capture path independently from networking.
