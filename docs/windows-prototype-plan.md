# Windows Prototype Plan

## Sender sequence

1. Capture a fixed desktop region with Desktop Duplication
2. Convert or map the frame into an encoder-friendly buffer
3. JPEG-encode with a bounded quality setting
4. Wrap the payload in the frame protocol
5. Send over a persistent TCP connection

## Receiver sequence

1. Accept a single TCP client
2. Parse frame envelopes from the stream
3. Decode JPEG bytes into a renderable bitmap
4. Present immediately and drop stale frames if behind

## Engineering rules

- Do not queue unlimited frames
- Prefer dropping old frames to growing latency
- Keep the wire protocol versioned from day one
- Record timing before optimization starts
