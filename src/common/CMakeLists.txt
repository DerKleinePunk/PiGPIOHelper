MESSAGE(STATUS "working on common dir")

ADD_SUBDIRECTORY(easylogging)
ADD_SUBDIRECTORY(exception)
ADD_SUBDIRECTORY(utils)

set(easylogging_SRCS ${easylogging_SRCS} PARENT_SCOPE)
set(exception_SRCS ${exception_SRCS} PARENT_SCOPE)
set(utils_SRCS ${utils_SRCS} PARENT_SCOPE)