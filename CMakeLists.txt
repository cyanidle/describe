cmake_minimum_required(VERSION 3.16)

project(describe LANGUAGES C CXX)
add_library(describe INTERFACE)
add_library(describe::describe ALIAS describe)

target_include_directories(describe INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(DESCRIBE_TESTS "Build tests for DESCRIBE()" OFF)
option(DESCRIBE_EXAMPLES "Build examples for DESCRIBE()" OFF)

if (DESCRIBE_TESTS)
    file(GLOB TEST_SRC test/*.cpp)
    include(CTest)
    enable_testing()
    add_executable(describe_test ${TEST_SRC})
    add_test(NAME describe_test COMMAND $<TARGET_FILE:describe_test>)
    target_link_libraries(describe_test PRIVATE describe)
endif()

if (DESCRIBE_EXAMPLES)
    find_package(Boost COMPONENTS json)
    if (BOOST_FOUND)
        add_executable(describe_boost_example ./examples/boost.cpp)
        target_link_libraries(describe_boost_example PRIVATE Boost::json describe)
    endif()
    find_package(nlohmann_json)
    if (nlohmann_json_FOUND)
        add_executable(describe_nlohmann_example ./examples/nlohmann.cpp)
        target_link_libraries(describe_nlohmann_example PRIVATE nlohmann_json::nlohmann_json describe)
    endif()
endif()
