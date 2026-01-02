set(BIN_NAME encode)

set(SRCS
    main.cpp
    encode.cpp
)

add_executable(${BIN_NAME} ${SRCS})
add_dependencies(${BIN_NAME} nec_prot cpputest)

target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${BIN_NAME} nec_prot CppUTest CppUTestExt)

add_test(${BIN_NAME} ${BIN_NAME})
