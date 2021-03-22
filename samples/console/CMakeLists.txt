project("PiGpIoConsole" VERSION "${RELEASE_VERSION}" LANGUAGES CXX)

MESSAGE(STATUS "Try Build ${PROJECT_NAME} in Version ${RELEASE_VERSION}")

configure_file("${MAINFOLDER}/samples/console/config.hpp.in" "${MAINFOLDER}//samples/console/config.hpp")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

## Use all the *.cpp files we found under this folder for the project Find better way for coomen cpp files
FILE(GLOB SRCS "*.cpp" "*/*.cpp")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

add_executable(${project_BIN} ${SRCS} ${easylogging_SRCS} ${utils_SRCS} ${GPIOHelper_SRCS} ${exception_SRCS})

set(MAINFONTSOURCE "${MAINFOLDER}/samples/console/loggerConsole.conf")
add_custom_command(TARGET ${project_BIN} POST_BUILD # Adds a post-build event to MyTest
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
                       "${MAINFONTSOURCE}"      # <--this is in-file
                       $<TARGET_FILE_DIR:${project_BIN}>)