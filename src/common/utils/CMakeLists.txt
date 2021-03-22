message(STATUS "working on utils dir")

FILE (GLOB utils_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(utils_SRCS ${utils_SRCS} PARENT_SCOPE)