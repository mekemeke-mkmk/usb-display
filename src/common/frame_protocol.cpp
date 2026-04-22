#include "usb_display/frame_protocol.hpp"

#include <cstddef>
#include <utility>

namespace usb_display {

namespace {

void append_u16(std::vector<std::uint8_t>* out, std::uint16_t value) {
  out->push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
  out->push_back(static_cast<std::uint8_t>(value & 0xFF));
}

void append_u32(std::vector<std::uint8_t>* out, std::uint32_t value) {
  out->push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
  out->push_back(static_cast<std::uint8_t>(value & 0xFF));
}

void append_u64(std::vector<std::uint8_t>* out, std::uint64_t value) {
  out->push_back(static_cast<std::uint8_t>((value >> 56) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 48) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 40) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 32) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
  out->push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
  out->push_back(static_cast<std::uint8_t>(value & 0xFF));
}

std::uint16_t read_u16(std::span<const std::uint8_t> bytes, std::size_t offset) {
  return (static_cast<std::uint16_t>(bytes[offset]) << 8) |
         static_cast<std::uint16_t>(bytes[offset + 1]);
}

std::uint32_t read_u32(std::span<const std::uint8_t> bytes, std::size_t offset) {
  return (static_cast<std::uint32_t>(bytes[offset]) << 24) |
         (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
         (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
         static_cast<std::uint32_t>(bytes[offset + 3]);
}

std::uint64_t read_u64(std::span<const std::uint8_t> bytes, std::size_t offset) {
  return (static_cast<std::uint64_t>(bytes[offset]) << 56) |
         (static_cast<std::uint64_t>(bytes[offset + 1]) << 48) |
         (static_cast<std::uint64_t>(bytes[offset + 2]) << 40) |
         (static_cast<std::uint64_t>(bytes[offset + 3]) << 32) |
         (static_cast<std::uint64_t>(bytes[offset + 4]) << 24) |
         (static_cast<std::uint64_t>(bytes[offset + 5]) << 16) |
         (static_cast<std::uint64_t>(bytes[offset + 6]) << 8) |
         static_cast<std::uint64_t>(bytes[offset + 7]);
}

}  // namespace

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

std::vector<std::uint8_t> serialize_packet(const FramePacket& packet) {
  FramePacket normalized = packet;
  normalized.header.payload_len = static_cast<std::uint32_t>(normalized.payload.size());

  const ParseResult validation = validate_packet(normalized);
  if (!validation.ok) {
    return {};
  }

  std::vector<std::uint8_t> bytes;
  bytes.reserve(kFixedHeaderBytes + normalized.payload.size());
  bytes.insert(bytes.end(), kMagic.begin(), kMagic.end());
  append_u16(&bytes, kProtocolVersion);
  append_u16(&bytes, kFixedHeaderBytes);
  append_u64(&bytes, normalized.header.frame_id);
  append_u64(&bytes, normalized.header.timestamp_us);
  append_u16(&bytes, normalized.header.width);
  append_u16(&bytes, normalized.header.height);
  append_u16(&bytes, static_cast<std::uint16_t>(normalized.header.codec));
  append_u16(&bytes, normalized.header.flags);
  append_u32(&bytes, normalized.header.payload_len);
  bytes.insert(bytes.end(), normalized.payload.begin(), normalized.payload.end());
  return bytes;
}

ParseResult parse_packet(std::span<const std::uint8_t> bytes, FramePacket* out_packet) {
  if (out_packet == nullptr) {
    return {.ok = false, .error = "output packet pointer is null"};
  }
  if (bytes.size() < kFixedHeaderBytes) {
    return {.ok = false, .error = "input smaller than fixed header"};
  }
  if (bytes[0] != static_cast<std::uint8_t>(kMagic[0]) ||
      bytes[1] != static_cast<std::uint8_t>(kMagic[1]) ||
      bytes[2] != static_cast<std::uint8_t>(kMagic[2]) ||
      bytes[3] != static_cast<std::uint8_t>(kMagic[3])) {
    return {.ok = false, .error = "invalid magic"};
  }

  const std::uint16_t version = read_u16(bytes, 4);
  if (version != kProtocolVersion) {
    return {.ok = false, .error = "unsupported protocol version"};
  }

  const std::uint16_t header_len = read_u16(bytes, 6);
  if (header_len != kFixedHeaderBytes) {
    return {.ok = false, .error = "unsupported header length"};
  }

  FramePacket parsed{};
  parsed.header.frame_id = read_u64(bytes, 8);
  parsed.header.timestamp_us = read_u64(bytes, 16);
  parsed.header.width = read_u16(bytes, 24);
  parsed.header.height = read_u16(bytes, 26);
  parsed.header.codec = static_cast<Codec>(read_u16(bytes, 28));
  parsed.header.flags = read_u16(bytes, 30);
  parsed.header.payload_len = read_u32(bytes, 32);

  const std::size_t expected_size = static_cast<std::size_t>(kFixedHeaderBytes) +
                                    static_cast<std::size_t>(parsed.header.payload_len);
  if (bytes.size() != expected_size) {
    return {.ok = false, .error = "frame size and payload length mismatch"};
  }

  parsed.payload.assign(bytes.begin() + kFixedHeaderBytes, bytes.end());
  const ParseResult validation = validate_packet(parsed);
  if (!validation.ok) {
    return validation;
  }

  *out_packet = std::move(parsed);
  return {.ok = true, .error = ""};
}

}  // namespace usb_display
