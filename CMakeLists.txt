cmake_minimum_required(VERSION 3.20)
project(usb_display LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src/common)
add_subdirectory(apps/windows-sender)
add_subdirectory(apps/windows-receiver)
