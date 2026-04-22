#include "usb_display/frame_protocol.hpp"

namespace usb_display {

ParseResult validate_packet(const FramePacket& packet) {
  if (packet.header.payload_len != packet.payload.size()) {
    return {.ok = false, .error = "payload length mismatch"};
  }
  if (packet.header.width == 0 || packet.header.height == 0) {
    return {.ok = false, .error = "invalid frame dimensions"};
  }
  if (packet.header.codec != Codec::kJpeg) {
    return {.ok = false, .error = "unsupported codec"};
  }
  return {.ok = true, .error = ""};
}

}  // namespace usb_display
