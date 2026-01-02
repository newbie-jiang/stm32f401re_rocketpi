cmake_minimum_required(VERSION 3.15)

project(nec_protocol)

option(BUILD_TESTS "Build tests" OFF)

add_subdirectory(lib)

if(BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif(BUILD_TESTS)
