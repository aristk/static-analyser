cmake_minimum_required(VERSION 3.6)

project(staticAnalyzerRoot)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_MACOSX_RPATH 1)

# Custom cmake search
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules/")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(cryptominisat-5.0.1)
add_subdirectory(provers/z3-z3-4.5.0)