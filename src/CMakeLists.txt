project(staticAnalyzer)

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
                --yylineno
            ${FlexDir}/tokens.l
    )
ENDIF()
SET_SOURCE_FILES_PROPERTIES(${FlexOutput} GENERATED)

# bison -d -o parser.cpp parser.y
FIND_PACKAGE(BISON REQUIRED)
SET(BisonDir ${CMAKE_CURRENT_SOURCE_DIR}/parser)
SET(BisonOutput ${BisonDir}/parser.cpp )
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${BisonOutput}
            COMMAND ${BISON_EXECUTABLE}
                -d -o ${BisonOutput}
            ${BisonDir}/parser.y
    )
ENDIF()
SET_SOURCE_FILES_PROPERTIES(${BisonOutput} ${BisonDir}/parser.hpp GENERATED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${cryptominisat5_SOURCE_DIR}/src)

set(SOURCE_FILES analyzer/satAnalyzer.cpp analyzer/node.cpp analyzer/variableName.cpp core.cpp ${FlexOutput} ${BisonOutput})

set(Z3_FOUND 0)

if (Z3_FOUND)
    include_directories(${Z3_SOURCE_DIR}/src/api)
    set(LIBS ${LIBS} libz3)
    set(SOURCE_FILES ${SOURCE_FILES} analyzer/smtAnalyzer.cpp )
endif()

add_library(core ${SOURCE_FILES})
add_executable(staticAnalyzer main.cpp)

set(LIBS core libcryptominisat5)

target_link_libraries(staticAnalyzer ${LIBS})

