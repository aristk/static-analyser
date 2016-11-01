cmake_minimum_required(VERSION 3.6)
project(staticAnalyzer)

#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)

# lex -o tokens.cpp tokens.l
# Create custom command for flex/lex (note the outputs)
FIND_PACKAGE(FLEX REQUIRED)
SET(FlexDir ${CMAKE_SOURCE_DIR}/parser)
SET(FlexOutput ${FlexDir}/tokens.cpp)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${FlexOutput}
            COMMAND ${FLEX_EXECUTABLE}
                -o ${FlexOutput}
            ${FlexDir}/tokens.l
    )
ENDIF()
SET_SOURCE_FILES_PROPERTIES(${FlexOutput} GENERATED)

# bison -d -o parser.cpp parser.y
FIND_PACKAGE(BISON REQUIRED)
SET(BisonDir ${CMAKE_SOURCE_DIR}/parser)
SET(BisonOutput ${BisonDir}/parser.cpp)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${BisonOutput}
            COMMAND ${BISON_EXECUTABLE}
                -d -o ${BisonOutput}
            ${BisonDir}/parser.y
    )
ENDIF()
SET_SOURCE_FILES_PROPERTIES(${BisonOutput} GENERATED)

include_directories(${BisonDir})

set(SOURCE_FILES analyzer/analyzer.cpp main.cpp ${FlexOutput} ${BisonOutput})
add_executable(staticAnalyzer ${SOURCE_FILES})