add_library(usb_display_common STATIC
  frame_protocol.cpp
)
target_include_directories(usb_display_common PUBLIC include)
