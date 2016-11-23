cmake_minimum_required(VERSION 3.6)

project(staticAnalyzerRoot)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#support for llvm tidy
set(CMAKE_ENABLE_COMPILE_COMMANDS ON)

set(CMAKE_MACOSX_RPATH 1)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build.")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build.")
ELSE()
    message("Some other build type.")
ENDIF()

add_subdirectory(provers/cryptominisat-5.0.1)
add_subdirectory(tests/libs/googletest-release-1.8.0)
add_subdirectory(tests)
add_subdirectory(src)