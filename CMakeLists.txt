cmake_minimum_required(VERSION 3.6)
project(staticAnalyzer)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_CXX_STANDARD 11)

# lex -o tokens.cpp tokens.l
# TODO: somehow does not work... FIX
# Create custom command for flex/lex (note the outputs)
FIND_PACKAGE(FLEX REQUIRED)
SET(FlexOutput ${CMAKE_SOURCE_DIR}/tokens.cpp)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${FlexOutput}
            COMMAND ${FLEX_EXECUTABLE}
                -o ${FlexOutput}
                ${CMAKE_SOURCE_DIR}/tokens.l
    )
ENDIF()

SET_SOURCE_FILES_PROPERTIES(${FlexOutput} GENERATED)

# bison -d -o parser.cpp parser.y
FIND_PACKAGE(BISON REQUIRED)
SET(BisonOutput ${CMAKE_SOURCE_DIR}/parser.cpp)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${BisonOutput}
            COMMAND ${BISON_EXECUTABLE}
                -o ${BisonOutput}
            ${CMAKE_SOURCE_DIR}/parser.y
    )
ENDIF()

SET_SOURCE_FILES_PROPERTIES(${BisonOutput} GENERATED)

set(SOURCE_FILES main.cpp ${FlexOutput} ${BisonOutput})
add_executable(staticAnalyzer ${SOURCE_FILES})