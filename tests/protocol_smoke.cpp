#include <cstdint>
#include <iostream>
#include <vector>

#include "usb_display/frame_protocol.hpp"

namespace {

bool expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "[fail] " << message << "\n";
    return false;
  }
  return true;
}

bool test_roundtrip() {
  usb_display::FramePacket packet{};
  packet.header.frame_id = 42;
  packet.header.timestamp_us = 1234567;
  packet.header.width = 1280;
  packet.header.height = 720;
  packet.header.codec = usb_display::Codec::kJpeg;
  packet.header.flags = 0;
  packet.payload = {0x01, 0xAB, 0xFF, 0x10};
  packet.header.payload_len = static_cast<std::uint32_t>(packet.payload.size());

  const auto bytes = usb_display::serialize_packet(packet);
  if (!expect(!bytes.empty(), "serialize_packet produced empty buffer")) {
    return false;
  }

  usb_display::FramePacket parsed{};
  const auto parse = usb_display::parse_packet(bytes, &parsed);
  if (!expect(parse.ok, parse.error)) {
    return false;
  }

  bool ok = true;
  ok &= expect(parsed.header.frame_id == packet.header.frame_id, "frame_id mismatch");
  ok &= expect(parsed.header.timestamp_us == packet.header.timestamp_us, "timestamp mismatch");
  ok &= expect(parsed.header.width == packet.header.width, "width mismatch");
  ok &= expect(parsed.header.height == packet.header.height, "height mismatch");
  ok &= expect(parsed.payload == packet.payload, "payload mismatch");
  return ok;
}

bool test_invalid_payload_len() {
  usb_display::FramePacket packet{};
  packet.payload = {0x01, 0x02};
  packet.header.payload_len = 999;

  const auto result = usb_display::validate_packet(packet);
  return expect(!result.ok, "validate_packet should fail on payload mismatch");
}

bool test_truncated_buffer() {
  usb_display::FramePacket packet{};
  packet.payload = {0x01, 0x02, 0x03, 0x04};
  packet.header.payload_len = static_cast<std::uint32_t>(packet.payload.size());

  auto bytes = usb_display::serialize_packet(packet);
  if (!expect(bytes.size() > 2, "test setup failed")) {
    return false;
  }
  bytes.pop_back();

  usb_display::FramePacket parsed{};
  const auto parse = usb_display::parse_packet(bytes, &parsed);
  return expect(!parse.ok, "parse_packet should fail on truncated frame");
}

}  // namespace

int main() {
  bool ok = true;
  ok &= test_roundtrip();
  ok &= test_invalid_payload_len();
  ok &= test_truncated_buffer();

  if (!ok) {
    return 1;
  }
  std::cout << "[ok] protocol smoke tests passed\n";
  return 0;
}
