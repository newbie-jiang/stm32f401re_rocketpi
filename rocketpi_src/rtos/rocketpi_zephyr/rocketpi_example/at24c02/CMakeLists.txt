# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(at24c02_sample)

target_sources(app PRIVATE
  src/main.c
  at24xx/driver_at24cxx.c
  at24xx/driver_at24cxx_basic.c
  at24xx/driver_at24cxx_read_test.c
  at24xx/driver_at24cxx_interface.c
  soft_i2c/soft_i2c.c
)

target_include_directories(app PRIVATE
  at24xx
  soft_i2c
)
