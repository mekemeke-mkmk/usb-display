#include <cstdint>
#include <iostream>
#include <vector>

#include "usb_display/frame_stream.hpp"

namespace {

bool expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "[fail] " << message << "\n";
    return false;
  }
  return true;
}

usb_display::FramePacket make_packet(std::uint64_t frame_id, std::uint8_t seed) {
  usb_display::FramePacket packet{};
  packet.header.frame_id = frame_id;
  packet.header.timestamp_us = frame_id * 1000;
  packet.header.width = 1280;
  packet.header.height = 720;
  packet.header.codec = usb_display::Codec::kJpeg;
  packet.payload = {seed, static_cast<std::uint8_t>(seed + 1), static_cast<std::uint8_t>(seed + 2)};
  packet.header.payload_len = static_cast<std::uint32_t>(packet.payload.size());
  return packet;
}

bool test_chunked_input_produces_packets() {
  usb_display::FramePacket first = make_packet(1, 10);
  usb_display::FramePacket second = make_packet(2, 20);

  const auto first_bytes = usb_display::serialize_packet(first);
  const auto second_bytes = usb_display::serialize_packet(second);

  std::vector<std::uint8_t> stream;
  stream.insert(stream.end(), first_bytes.begin(), first_bytes.end());
  stream.insert(stream.end(), second_bytes.begin(), second_bytes.end());

  usb_display::FrameStreamParser parser{};
  const std::size_t split_at = 7;
  auto r1 = parser.push_bytes(std::span<const std::uint8_t>(stream.data(), split_at));
  if (!expect(r1.ok, r1.error)) {
    return false;
  }
  auto r2 = parser.push_bytes(std::span<const std::uint8_t>(
      stream.data() + split_at, stream.size() - split_at));
  if (!expect(r2.ok, r2.error)) {
    return false;
  }

  usb_display::FramePacket out1{};
  usb_display::FramePacket out2{};
  bool ok = true;
  ok &= expect(parser.pop_packet(&out1), "missing first parsed packet");
  ok &= expect(parser.pop_packet(&out2), "missing second parsed packet");
  ok &= expect(!parser.pop_packet(&out2), "unexpected extra packet");
  ok &= expect(out1.header.frame_id == 1, "first frame id mismatch");
  ok &= expect(out2.header.frame_id == 2, "second frame id mismatch");
  return ok;
}

bool test_garbage_prefix_is_skipped() {
  usb_display::FramePacket packet = make_packet(99, 30);
  const auto packet_bytes = usb_display::serialize_packet(packet);

  std::vector<std::uint8_t> stream = {0x00, 0x11, 0x22, 0x33, 0x44};
  stream.insert(stream.end(), packet_bytes.begin(), packet_bytes.end());

  usb_display::FrameStreamParser parser{};
  const auto parse = parser.push_bytes(stream);
  if (!expect(parse.ok, parse.error)) {
    return false;
  }

  usb_display::FramePacket out{};
  bool ok = true;
  ok &= expect(parser.pop_packet(&out), "packet not parsed after garbage prefix");
  ok &= expect(out.header.frame_id == packet.header.frame_id, "frame id mismatch after garbage");
  return ok;
}

}  // namespace

int main() {
  bool ok = true;
  ok &= test_chunked_input_produces_packets();
  ok &= test_garbage_prefix_is_skipped();

  if (!ok) {
    return 1;
  }
  std::cout << "[ok] frame stream smoke tests passed\n";
  return 0;
}
