MESSAGE(STATUS "working on src dir")

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(GPIOHelper)

set(GPIOHelper_SRCS ${GPIOHelper_SRCS} PARENT_SCOPE)
set(easylogging_SRCS ${easylogging_SRCS} PARENT_SCOPE)
set(utils_SRCS ${utils_SRCS} PARENT_SCOPE)
set(exception_SRCS ${exception_SRCS} PARENT_SCOPE)
set(GPIOHelper_SRCS ${GPIOHelper_SRCS} PARENT_SCOPE)