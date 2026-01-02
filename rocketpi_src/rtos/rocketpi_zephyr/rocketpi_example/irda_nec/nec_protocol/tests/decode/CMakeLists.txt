set(BIN_NAME decode_offline)

set(SRCS
    main.cpp
    raw_bits.cpp
    tv_lg_codes.cpp
)

add_executable(${BIN_NAME} ${SRCS})
add_dependencies(${BIN_NAME} nec_prot cpputest)

target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${BIN_NAME} nec_prot CppUTest CppUTestExt)

add_test(${BIN_NAME} ${BIN_NAME})
