cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(f767ig_app)

target_sources(
  app
  PRIVATE
  src/main.c
  src/usb.c
  # src/test_hid.c
)
