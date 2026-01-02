cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME rocketpi_led_sample)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

set(THREADX_ROOT ${CMAKE_SOURCE_DIR}/threadx-6.4.3.202503_rel)

# Integrate ThreadX core and Cortex-M4 GNU port sources directly into the
# application target so the RTOS builds as part of the firmware image.
add_subdirectory(${THREADX_ROOT}/common threadx_common)
add_subdirectory(${THREADX_ROOT}/ports/cortex_m4/gnu threadx_port)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/ThreadX/app_threadx.c
    ${CMAKE_SOURCE_DIR}/Core/ThreadX/tx_low_level.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${THREADX_ROOT}/common/inc
    ${THREADX_ROOT}/ports/cortex_m4/gnu/inc
    ${CMAKE_SOURCE_DIR}/Core/ThreadX
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    TX_INCLUDE_USER_DEFINE_FILE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Create binary and Intel HEX images from the ELF
if(CMAKE_OBJCOPY)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
                $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
                ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex
                $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
                ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
        COMMENT "Generating BIN and HEX images"
    )
else()
    message(WARNING "CMAKE_OBJCOPY not defined; BIN/HEX images will not be generated.")
endif()
