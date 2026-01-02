# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(radar_sample)

target_sources(app PRIVATE
    src/main.c
    mg58f18_radar/driver_mg58f18_radar.c
    mg58f18_radar/driver_mg58f18_radar_interface.c
    mg58f18_radar/driver_mg58f18_radar_test.c
)

target_include_directories(app PRIVATE mg58f18_radar)
