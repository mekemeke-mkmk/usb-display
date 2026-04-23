# windows-sender

Windows-first sender implementation for MVP bring-up over USB virtual LAN (TCP).

## Responsibilities

- Capture desktop frames (planned)
- Apply frame pacing
- Encode JPEG payloads
- Stream protocol frames to receiver
- Report capture and encode telemetry

## Current bootstrap behavior

`usb-display-sender` opens a TCP connection and sends protocol frames containing JPEG payload bytes.

- `USB_DISPLAY_HOST` and `USB_DISPLAY_PORT` override default destination (`127.0.0.1:45888`).
- `USB_DISPLAY_INPUT_JPEG` can point to a single local `.jpg` file used as payload.
- `USB_DISPLAY_INPUT_DIR` can point to a directory of `.jpg` / `.jpeg` files; sender streams files in sorted order as a simple video sequence.
- If neither input setting is provided, sender uses an embedded 1x1 JPEG payload.
- `USB_DISPLAY_MAX_FRAMES` controls how many frames are sent (default: `120`).
- `USB_DISPLAY_FPS` controls pacing (default: runtime config `30`, clamped to `1..120`).

This is enough to validate end-to-end image transport before wiring desktop capture.
