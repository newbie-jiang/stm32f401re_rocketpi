cmake_minimum_required(VERSION 3.13)

# Allow the application to override the FreeRTOS port and heap implementation.
set(FREERTOS_PORT "GCC/ARM_CM4F" CACHE STRING "FreeRTOS portable directory relative to portable/")
set_property(CACHE FREERTOS_PORT PROPERTY STRINGS "GCC/ARM_CM4F")

set(FREERTOS_HEAP_IMPL "4" CACHE STRING "FreeRTOS heap implementation number (1..5)")
set_property(CACHE FREERTOS_HEAP_IMPL PROPERTY STRINGS "1" "2" "3" "4" "5")

set(_freertos_kernel_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/timers.c
)

set(_freertos_port_dir ${CMAKE_CURRENT_SOURCE_DIR}/portable/${FREERTOS_PORT})
if(NOT EXISTS ${_freertos_port_dir})
    message(FATAL_ERROR "FreeRTOS port directory not found: ${_freertos_port_dir}")
endif()

set(_freertos_port_sources
    ${_freertos_port_dir}/port.c
)

set(_freertos_heap_source ${CMAKE_CURRENT_SOURCE_DIR}/portable/MemMang/heap_${FREERTOS_HEAP_IMPL}.c)
if(NOT EXISTS ${_freertos_heap_source})
    message(FATAL_ERROR "FreeRTOS heap implementation not found: ${_freertos_heap_source}")
endif()

add_library(freertos_kernel STATIC
    ${_freertos_kernel_sources}
    ${_freertos_port_sources}
    ${_freertos_heap_source}
)

target_include_directories(freertos_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${_freertos_port_dir}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(FreeRTOS::Kernel ALIAS freertos_kernel)
