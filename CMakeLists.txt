cmake_minimum_required(VERSION 3.6)

#set(CMAKE_VERBOSE_MAKEFILE ON)

project(staticAnalyzer)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(cryptominisat-5.0.1)