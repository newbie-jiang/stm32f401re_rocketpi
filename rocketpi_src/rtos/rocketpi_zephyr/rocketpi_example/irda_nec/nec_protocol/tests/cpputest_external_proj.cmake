include(cpputest_common.cmake)

include(ExternalProject)
ExternalProject_Add(cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
    INSTALL_DIR ${EXTERNAL_INSTALL_LOCATION}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
