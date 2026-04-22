# Frame Protocol v1

## Transport

- TCP stream
- One sender to one receiver
- Big-endian integer fields

## Envelope

```text
magic        4 bytes   "USBD"
version      2 bytes   0x0001
header_len   2 bytes   bytes in header including extensions
frame_id     8 bytes   monotonically increasing
timestamp_us 8 bytes   sender monotonic clock in microseconds
width        2 bytes   encoded frame width
height       2 bytes   encoded frame height
codec        2 bytes   1 = JPEG
flags        2 bytes   bitfield for key-frame or dirty-region hints
payload_len  4 bytes   encoded payload size in bytes
payload      N bytes   codec payload
```

## Receiver behavior

- Reject unknown magic
- Reject unsupported version
- Reject payloads above a configured safety cap
- Continue parsing only on clean frame boundaries
