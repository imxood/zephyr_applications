
cmake_minimum_required(VERSION 3.13.1)

# set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/custom.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(f103c8_app)

target_sources(
  app
  PRIVATE
  src/main.c
  # src/usb.c
)
