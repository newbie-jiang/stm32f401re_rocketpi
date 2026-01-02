set(LIB_SRCS
    nec_prot.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(nec_prot ${LIB_SRCS})
