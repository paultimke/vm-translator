cmake_minimum_required(VERSION 3.13)

project(vm-translator VERSION 0.0.1
                      LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

add_subdirectory(src)

message("Build type is ${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Test")
    enable_testing()
    add_subdirectory(test)
endif()

