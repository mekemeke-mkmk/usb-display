# Architecture

## System overview

```text
[Sender PC]
  capture -> encode -> packetize -> tcp stream over usb virtual lan
                                               |
                                               v
[Receiver PC]
  tcp receive -> decode -> render
```

## Phase split

### Phase 1: visible prototype

- Capture a fixed desktop region or single window
- JPEG-encode frames
- Stream frames over TCP
- Decode and render on the receiver

### Phase 2: performance

- Dirty-region detection
- Frame coalescing and skip logic
- Quality adaptation under bandwidth pressure

### Phase 3: resilience

- Reconnect handshake
- Sequence gap handling
- Backpressure management

### Phase 4: virtual display

- Windows indirect display driver path
- OS-visible display surface integration
- Touch and pointer return path

## Recommended boundaries

### Sender

- `capture`: platform-specific screen capture
- `encoder`: JPEG now, low-latency codec later
- `scheduler`: frame pacing and dirty-region decisions
- `transport`: TCP framing and reconnect behavior

### Receiver

- `transport`: stream parser and reconnection
- `decoder`: JPEG decode
- `renderer`: present frames with minimal copy count
- `stats`: latency, jitter, dropped-frame counters
