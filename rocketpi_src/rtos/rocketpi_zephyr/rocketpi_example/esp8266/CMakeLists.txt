cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp8266_sample)

if("$ENV{DEBUG}" STREQUAL "1")
    add_compile_definitions(DEBUG=1)
endif()

file(GLOB APP_SOURCES_C src/*.c)
set(APP_SOURCES "${APP_SOURCES_C}")
message("-- APP_SOURCES=${APP_SOURCES}")

target_include_directories(app PRIVATE src)
target_sources(app PRIVATE "${APP_SOURCES}")
target_link_libraries(app PRIVATE zephyr zephyr_interface)
