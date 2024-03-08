project(describe LANGUAGES C CXX)
add_library(describe INTERFACE)
add_library(describe::describe ALIAS describe)
target_include_directories(describe INTERFACE include)

option(DESCRIBE_WITH_TESTS "Build tests for DESCRIBE()" OFF)
if (DESCRIBE_WITH_TESTS)
    add_subdirectory(test)
endif()