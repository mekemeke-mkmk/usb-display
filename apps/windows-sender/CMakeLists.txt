add_executable(usb-display-sender
  src/main.cpp
)

target_link_libraries(usb-display-sender PRIVATE usb_display_common)
target_include_directories(usb-display-sender PRIVATE include)
