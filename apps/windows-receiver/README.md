# windows-receiver

Windows-first receiver implementation for MVP bring-up over USB virtual LAN (TCP).

## Responsibilities

- Accept protocol frames over TCP
- Decode JPEG payloads (planned)
- Render the newest available frame (planned)
- Expose latency and drop counters

## Current bootstrap behavior

`usb-display-receiver` listens for TCP connections and parses framed packets using `FrameStreamParser`.

- `USB_DISPLAY_PORT` overrides default listen port (`45888`).
- `USB_DISPLAY_OUT_DIR` selects where received JPEG payloads are written.
  - Windows default: `./usb-display-frames`
  - Non-Windows default: `/tmp/usb-display-frames`
- `USB_DISPLAY_MAX_FRAMES` sets how many frames to receive before exit (default: `120`).
- Each received frame is saved as `frame-000001.jpg` style files and `latest.jpg` is updated.
- Non-JPEG payloads are ignored as invalid safety guard.

This validates transport + framing + payload persistence before decode/render and IDD integration.
