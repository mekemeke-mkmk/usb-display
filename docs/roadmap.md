# Roadmap

## Phase 0: workspace bootstrap

- Repository structure
- VSCode settings
- Environment check script
- Protocol draft
- Architecture notes

## Phase 1: minimal prototype

- Sender capture region selection
- JPEG frame encoding
- TCP frame transport
- Receiver decode and on-screen display
- Telemetry logging

Exit condition:
The receiver shows live frames over a USB virtual LAN connection.

## Phase 2: bandwidth and latency

- Dirty-region detection
- Adaptive JPEG quality
- Frame skip under load
- Timing instrumentation

## Phase 3: operational stability

- Automatic reconnect
- Heartbeat and version handshake
- Error categorization
- Long-run soak tests

## Phase 4: virtual display integration

- Evaluate Windows IDD implementation path
- Separate transport/render core from OS display presentation
- Prototype indirect display driver integration
