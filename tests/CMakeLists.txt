################################
# GTest
################################
add_subdirectory(libs/googletest-release-1.8.0)
enable_testing()
include_directories(
        ${gmock_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}
        ${gtest_SOURCE_DIR}/include
        ${gtest_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR})

        ################################
# Unit Tests
################################
# Add test cpp file
add_executable( runUnitTests simpleGoogleTest.cpp)
# Link test executable against gtest & gtest_main
target_link_libraries(runUnitTests gtest gtest_main core libcryptominisat5)
#add_test( runUnitTests runUnitTests )