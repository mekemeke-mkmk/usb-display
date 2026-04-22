add_executable(usb-display-receiver
  src/main.cpp
)

target_link_libraries(usb-display-receiver PRIVATE usb_display_common)
target_include_directories(usb-display-receiver PRIVATE include)
