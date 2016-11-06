
set(CMAKE_CXX_STANDARD 11)

# lex -o tokens.cpp tokens.l
# Create custom command for flex/lex (note the outputs)
FIND_PACKAGE(FLEX REQUIRED)
SET(FlexDir ${CMAKE_CURRENT_SOURCE_DIR}/parser)
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
SET(BisonDir ${CMAKE_CURRENT_SOURCE_DIR}/parser)
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

include_directories(${CMAKE_SOURCE_DIR}/cryptominisat-5.0.1/src)

set(SOURCE_FILES analyzer/analyzer.cpp core.cpp ${FlexOutput} ${BisonOutput})
add_library(core ${SOURCE_FILES})
add_executable(staticAnalyzer main.cpp)
target_link_libraries(staticAnalyzer core libcryptominisat5)