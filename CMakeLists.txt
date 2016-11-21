cmake_minimum_required(VERSION 3.6)

project(staticAnalyzerRoot)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MACOSX_RPATH 1)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build.")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build.")
ELSE()
    message("Some other build type.")
ENDIF()

add_subdirectory(tests)
add_subdirectory(provers/cryptominisat-5.0.1)
add_subdirectory(src)