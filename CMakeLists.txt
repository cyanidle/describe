project(describe LANGUAGES C CXX)
add_library(describe INTERFACE)
add_library(describe::describe ALIAS describe)
target_include_directories(describe INTERFACE .)