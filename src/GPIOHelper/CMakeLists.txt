message(STATUS "working on GPIOHelper dir")

FILE (GLOB GPIOHelper_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(GPIOHelper_SRCS ${GPIOHelper_SRCS} PARENT_SCOPE)