#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "sender_app.hpp"
#include "usb_display/frame_protocol.hpp"
#include "usb_display/telemetry.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace usb_display::sender {

namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle kInvalidSocket = -1;
#endif

void close_socket(SocketHandle socket_fd) {
#ifdef _WIN32
  closesocket(socket_fd);
#else
  close(socket_fd);
#endif
}

bool send_all(SocketHandle socket_fd, const std::uint8_t* data, std::size_t len) {
  std::size_t sent = 0;
  while (sent < len) {
#ifdef _WIN32
    const int n = send(socket_fd, reinterpret_cast<const char*>(data + sent),
                       static_cast<int>(len - sent), 0);
#else
    const ssize_t n = send(socket_fd, data + sent, len - sent, 0);
#endif
    if (n <= 0) {
      return false;
    }
    sent += static_cast<std::size_t>(n);
  }
  return true;
}

#ifdef _WIN32
class WsaContext {
 public:
  WsaContext() {
    WSADATA wsa{};
    ok_ = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
  }
  ~WsaContext() {
    if (ok_) {
      WSACleanup();
    }
  }
  bool ok() const { return ok_; }

 private:
  bool ok_ = false;
};
#endif

std::optional<int> parse_int_env(const char* name) {
  const char* raw = std::getenv(name);
  if (raw == nullptr || *raw == '\0') {
    return std::nullopt;
  }
  char* end = nullptr;
  const long value = std::strtol(raw, &end, 10);
  if (end == raw || *end != '\0') {
    return std::nullopt;
  }
  return static_cast<int>(value);
}

std::optional<std::string> parse_string_env(const char* name) {
  const char* raw = std::getenv(name);
  if (raw == nullptr || *raw == '\0') {
    return std::nullopt;
  }
  return std::string(raw);
}

bool has_jpeg_extension(const std::filesystem::path& path) {
  std::string ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return ext == ".jpg" || ext == ".jpeg";
}

bool is_likely_jpeg(const std::vector<std::uint8_t>& bytes) {
  return bytes.size() >= 4 && bytes[0] == 0xFF && bytes[1] == 0xD8 &&
         bytes[bytes.size() - 2] == 0xFF && bytes[bytes.size() - 1] == 0xD9;
}

SocketHandle connect_tcp(const std::string& host, std::uint16_t port) {
  SocketHandle fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == kInvalidSocket) {
    return kInvalidSocket;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
    close_socket(fd);
    return kInvalidSocket;
  }

  if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    close_socket(fd);
    return kInvalidSocket;
  }
  return fd;
}

std::vector<std::uint8_t> load_file(const std::filesystem::path& path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) {
    return {};
  }
  return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(ifs)),
                                   std::istreambuf_iterator<char>());
}

std::vector<std::filesystem::path> list_jpeg_files(const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> files;
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec)) {
    return files;
  }

  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (ec) {
      return {};
    }
    if (!entry.is_regular_file()) {
      continue;
    }
    if (has_jpeg_extension(entry.path())) {
      files.push_back(entry.path());
    }
  }
  std::sort(files.begin(), files.end());
  return files;
}

std::vector<std::uint8_t> default_jpeg_payload() {
  return {
      0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
      0x01, 0x01, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
      0x00, 0x10, 0x0B, 0x0C, 0x0E, 0x0C, 0x0A, 0x10, 0x0E, 0x0D, 0x0E, 0x12,
      0x11, 0x10, 0x13, 0x18, 0x28, 0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23,
      0x25, 0x1D, 0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33, 0x38, 0x37, 0x40,
      0x48, 0x5C, 0x4E, 0x40, 0x44, 0x57, 0x45, 0x37, 0x38, 0x50, 0x6D, 0x51,
      0x57, 0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D, 0x71, 0x79, 0x70, 0x64,
      0x78, 0x5C, 0x65, 0x67, 0x63, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x01,
      0x00, 0x01, 0x03, 0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01,
      0xFF, 0xC4, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xFF, 0xC4, 0x00,
      0x14, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01,
      0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0xD2, 0xCF, 0x20, 0xFF,
      0xD9,
  };
}

}  // namespace

SenderApp::SenderApp(RuntimeConfig config) : config_(std::move(config)) {}

int SenderApp::run() {
#ifdef _WIN32
  WsaContext wsa;
  if (!wsa.ok()) {
    std::cerr << "[sender] WSAStartup failed\n";
    return 1;
  }
#endif

  const int max_frames = std::max(1, parse_int_env("USB_DISPLAY_MAX_FRAMES").value_or(120));
  const std::string host = parse_string_env("USB_DISPLAY_HOST").value_or(config_.host);
  const std::uint16_t port = static_cast<std::uint16_t>(
      std::clamp(parse_int_env("USB_DISPLAY_PORT").value_or(static_cast<int>(config_.port)), 1,
                 65535));
  const std::uint16_t fps_cap = static_cast<std::uint16_t>(
      std::clamp(parse_int_env("USB_DISPLAY_FPS").value_or(static_cast<int>(config_.fps_cap)), 1,
                 120));

  std::vector<std::filesystem::path> jpeg_sequence;
  if (const auto input_dir = parse_string_env("USB_DISPLAY_INPUT_DIR")) {
    jpeg_sequence = list_jpeg_files(*input_dir);
    if (jpeg_sequence.empty()) {
      std::cerr << "[sender] no JPEG files found in USB_DISPLAY_INPUT_DIR=" << *input_dir << "\n";
      return 1;
    }
    std::cout << "[sender] using JPEG sequence from " << *input_dir << " (" << jpeg_sequence.size()
              << " files)\n";
  }

  const auto input_path = parse_string_env("USB_DISPLAY_INPUT_JPEG");
  std::vector<std::uint8_t> static_payload =
      (input_path.has_value()) ? load_file(*input_path) : default_jpeg_payload();

  if (jpeg_sequence.empty() && static_payload.empty()) {
    std::cerr << "[sender] failed to load JPEG payload\n";
    return 1;
  }
  if (jpeg_sequence.empty() && !is_likely_jpeg(static_payload)) {
    std::cerr << "[sender] payload is not a JPEG file\n";
    return 1;
  }

  std::cout << "[sender] connect to " << host << ":" << port << "\n";
  SocketHandle socket_fd = connect_tcp(host, port);
  if (socket_fd == kInvalidSocket) {
    std::cerr << "[sender] connect failed\n";
    return 1;
  }

  std::cout << "[sender] streaming " << max_frames << " frame(s) at " << fps_cap << " fps\n";

  TelemetrySnapshot telemetry{};
  const auto frame_interval = std::chrono::milliseconds(1000 / fps_cap);

  for (int i = 0; i < max_frames; ++i) {
    std::vector<std::uint8_t> payload;
    if (jpeg_sequence.empty()) {
      payload = static_payload;
    } else {
      payload = load_file(jpeg_sequence[static_cast<std::size_t>(i) % jpeg_sequence.size()]);
      if (!is_likely_jpeg(payload)) {
        std::cerr << "[sender] invalid JPEG in sequence: "
                  << jpeg_sequence[static_cast<std::size_t>(i) % jpeg_sequence.size()] << "\n";
        close_socket(socket_fd);
        return 1;
      }
    }

    FramePacket packet{};
    packet.header.frame_id = static_cast<std::uint64_t>(i + 1);
    packet.header.timestamp_us = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    packet.header.width = config_.width;
    packet.header.height = config_.height;
    packet.header.codec = Codec::kJpeg;
    packet.payload = std::move(payload);

    const auto bytes = serialize_packet(packet);
    if (bytes.empty()) {
      std::cerr << "[sender] serialize failed at frame " << (i + 1) << "\n";
      close_socket(socket_fd);
      return 1;
    }

    if (!send_all(socket_fd, bytes.data(), bytes.size())) {
      std::cerr << "[sender] send failed at frame " << (i + 1) << "\n";
      close_socket(socket_fd);
      return 1;
    }

    ++telemetry.frames_captured;
    ++telemetry.frames_sent;
    telemetry.bytes_sent += bytes.size();

    std::cout << "[sender] sent frame " << packet.header.frame_id << " (" << packet.payload.size()
              << " payload bytes)\n";

    std::this_thread::sleep_for(frame_interval);
  }

  std::cout << "[sender] done frames_sent=" << telemetry.frames_sent
            << " bytes_sent=" << telemetry.bytes_sent << "\n";
  close_socket(socket_fd);
  return 0;
}

}  // namespace usb_display::sender

int main() {
  usb_display::RuntimeConfig config{};
  usb_display::sender::SenderApp app(config);
  return app.run();
}
