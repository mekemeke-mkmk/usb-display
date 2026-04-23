#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "receiver_app.hpp"
#include "usb_display/frame_stream.hpp"
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

namespace usb_display::receiver {

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

SocketHandle make_server_socket(std::uint16_t port) {
  SocketHandle fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == kInvalidSocket) {
    return kInvalidSocket;
  }

  int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    close_socket(fd);
    return kInvalidSocket;
  }
  if (listen(fd, 1) != 0) {
    close_socket(fd);
    return kInvalidSocket;
  }
  return fd;
}

bool write_file(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes) {
  std::ofstream ofs(path, std::ios::binary);
  if (!ofs) {
    return false;
  }
  ofs.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return ofs.good();
}

bool is_likely_jpeg(const std::vector<std::uint8_t>& bytes) {
  return bytes.size() >= 4 && bytes[0] == 0xFF && bytes[1] == 0xD8 &&
         bytes[bytes.size() - 2] == 0xFF && bytes[bytes.size() - 1] == 0xD9;
}

std::filesystem::path frame_path(const std::filesystem::path& out_dir, std::uint64_t frame_id) {
  std::ostringstream name;
  name << "frame-" << std::setfill('0') << std::setw(6) << frame_id << ".jpg";
  return out_dir / name.str();
}

}  // namespace

ReceiverApp::ReceiverApp(RuntimeConfig config) : config_(std::move(config)) {}

int ReceiverApp::run() {
#ifdef _WIN32
  WsaContext wsa;
  if (!wsa.ok()) {
    std::cerr << "[receiver] WSAStartup failed\n";
    return 1;
  }
#endif

  const std::filesystem::path out_dir = parse_string_env("USB_DISPLAY_OUT_DIR").value_or(
#ifdef _WIN32
      "./usb-display-frames"
#else
      "/tmp/usb-display-frames"
#endif
  );

  const int max_frames = std::max(1, parse_int_env("USB_DISPLAY_MAX_FRAMES").value_or(120));
  const std::uint16_t port = static_cast<std::uint16_t>(
      std::clamp(parse_int_env("USB_DISPLAY_PORT").value_or(static_cast<int>(config_.port)), 1,
                 65535));

  std::error_code ec;
  std::filesystem::create_directories(out_dir, ec);
  if (ec) {
    std::cerr << "[receiver] failed to create output dir: " << out_dir << "\n";
    return 1;
  }

  SocketHandle listen_fd = make_server_socket(port);
  if (listen_fd == kInvalidSocket) {
    std::cerr << "[receiver] listen failed on :" << port << "\n";
    return 1;
  }

  std::cout << "[receiver] listening on :" << port << "\n";
  SocketHandle client_fd = accept(listen_fd, nullptr, nullptr);
  close_socket(listen_fd);
  if (client_fd == kInvalidSocket) {
    std::cerr << "[receiver] accept failed\n";
    return 1;
  }

  std::cout << "[receiver] connected. writing JPEG frames to " << out_dir << "\n";

  FrameStreamParser parser{};
  TelemetrySnapshot telemetry{};
  std::array<std::uint8_t, 8192> chunk{};

  while (telemetry.frames_received < static_cast<std::uint64_t>(max_frames)) {
#ifdef _WIN32
    const int n = recv(client_fd, reinterpret_cast<char*>(chunk.data()), static_cast<int>(chunk.size()), 0);
#else
    const ssize_t n = recv(client_fd, chunk.data(), chunk.size(), 0);
#endif
    if (n == 0) {
      break;
    }
    if (n < 0) {
      std::cerr << "[receiver] recv failed\n";
      close_socket(client_fd);
      return 1;
    }

    telemetry.bytes_received += static_cast<std::uint64_t>(n);
    const ParseResult parse =
        parser.push_bytes(std::span<const std::uint8_t>(chunk.data(), static_cast<std::size_t>(n)));
    if (!parse.ok) {
      std::cerr << "[receiver] parse error: " << parse.error << "\n";
      close_socket(client_fd);
      return 1;
    }

    for (;;) {
      FramePacket packet{};
      if (!parser.pop_packet(&packet)) {
        break;
      }

      if (!is_likely_jpeg(packet.payload)) {
        std::cerr << "[receiver] dropping non-JPEG payload for frame " << packet.header.frame_id << "\n";
        continue;
      }

      ++telemetry.frames_received;
      ++telemetry.frames_rendered;

      const auto per_frame_path = frame_path(out_dir, packet.header.frame_id);
      if (!write_file(per_frame_path, packet.payload)) {
        std::cerr << "[receiver] failed to write frame file: " << per_frame_path << "\n";
        close_socket(client_fd);
        return 1;
      }

      const auto latest_path = out_dir / "latest.jpg";
      if (!write_file(latest_path, packet.payload)) {
        std::cerr << "[receiver] failed to write latest.jpg\n";
        close_socket(client_fd);
        return 1;
      }

      std::cout << "[receiver] frame " << packet.header.frame_id << " saved: " << per_frame_path
                << "\n";

      if (telemetry.frames_received >= static_cast<std::uint64_t>(max_frames)) {
        break;
      }
    }
  }

  close_socket(client_fd);
  std::cout << "[receiver] done frames_received=" << telemetry.frames_received
            << " bytes_received=" << telemetry.bytes_received << " latest=" << (out_dir / "latest.jpg")
            << "\n";
  return 0;
}

}  // namespace usb_display::receiver

int main() {
  usb_display::RuntimeConfig config{};
  usb_display::receiver::ReceiverApp app(config);
  return app.run();
}
