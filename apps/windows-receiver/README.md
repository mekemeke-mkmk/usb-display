# windows-receiver

Planned Windows-first receiver implementation.

## Responsibilities

- Accept protocol frames over TCP
- Decode JPEG payloads
- Render the newest available frame
- Expose latency and drop counters

## First milestone

Load a local JPEG repeatedly in a render loop to validate the presentation path before networking is introduced.
