#include "usb_display/frame_stream.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace usb_display {

namespace {

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

std::size_t find_magic_offset(std::span<const std::uint8_t> bytes) {
  if (bytes.size() < kMagic.size()) {
    return bytes.size();
  }
  for (std::size_t i = 0; i + kMagic.size() <= bytes.size(); ++i) {
    if (bytes[i] == static_cast<std::uint8_t>(kMagic[0]) &&
        bytes[i + 1] == static_cast<std::uint8_t>(kMagic[1]) &&
        bytes[i + 2] == static_cast<std::uint8_t>(kMagic[2]) &&
        bytes[i + 3] == static_cast<std::uint8_t>(kMagic[3])) {
      return i;
    }
  }
  return bytes.size();
}

}  // namespace

FrameStreamParser::FrameStreamParser(std::size_t max_frame_bytes)
    : max_frame_bytes_(max_frame_bytes) {}

ParseResult FrameStreamParser::push_bytes(std::span<const std::uint8_t> bytes) {
  if (bytes.empty()) {
    return {.ok = true, .error = ""};
  }

  buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());

  for (;;) {
    if (buffer_.size() < kMagic.size()) {
      return {.ok = true, .error = ""};
    }

    const std::size_t magic_offset = find_magic_offset(buffer_);
    if (magic_offset == buffer_.size()) {
      const std::size_t keep = std::min<std::size_t>(kMagic.size() - 1, buffer_.size());
      buffer_.erase(buffer_.begin(), buffer_.end() - static_cast<std::ptrdiff_t>(keep));
      return {.ok = true, .error = ""};
    }

    if (magic_offset > 0) {
      buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(magic_offset));
      if (buffer_.size() < kFixedHeaderBytes) {
        return {.ok = true, .error = ""};
      }
    }

    if (buffer_.size() < kFixedHeaderBytes) {
      return {.ok = true, .error = ""};
    }

    const std::uint16_t version = read_u16(buffer_, 4);
    if (version != kProtocolVersion) {
      buffer_.erase(buffer_.begin());
      continue;
    }

    const std::uint16_t header_len = read_u16(buffer_, 6);
    if (header_len != kFixedHeaderBytes) {
      buffer_.erase(buffer_.begin());
      continue;
    }

    const std::uint32_t payload_len = read_u32(buffer_, 32);
    const std::size_t frame_bytes =
        static_cast<std::size_t>(header_len) + static_cast<std::size_t>(payload_len);

    if (frame_bytes > max_frame_bytes_) {
      return {.ok = false, .error = "frame exceeds parser max_frame_bytes"};
    }
    if (buffer_.size() < frame_bytes) {
      return {.ok = true, .error = ""};
    }

    FramePacket packet{};
    const ParseResult parse =
        parse_packet(std::span<const std::uint8_t>(buffer_.data(), frame_bytes), &packet);
    if (!parse.ok) {
      return parse;
    }

    parsed_packets_.push_back(std::move(packet));
    buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(frame_bytes));
  }
}

bool FrameStreamParser::pop_packet(FramePacket* out_packet) {
  if (out_packet == nullptr || parsed_packets_.empty()) {
    return false;
  }
  *out_packet = std::move(parsed_packets_.front());
  parsed_packets_.pop_front();
  return true;
}

}  // namespace usb_display
