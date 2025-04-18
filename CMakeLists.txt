cmake_minimum_required(VERSION 3.16)

project(describe LANGUAGES C CXX)
add_library(describe INTERFACE)
add_library(describe::describe ALIAS describe)

target_include_directories(describe INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(DESCRIBE_WITH_TESTS "Build tests for DESCRIBE()" OFF)

if (DESCRIBE_WITH_TESTS)
    file(GLOB TEST_SRC test/*.cpp)
    include(CTest)
    enable_testing()
    add_executable(describe_test ${TEST_SRC})
    add_test(NAME describe_test COMMAND $<TARGET_FILE:describe_test>)
    target_link_libraries(describe_test PRIVATE describe)
endif()
