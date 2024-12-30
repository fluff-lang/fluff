cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

project(fluff LANGUAGES C VERSION 0.1.0)

file(GLOB_RECURSE SOURCES src/*.c)
include_directories(include/)

# TODO: maybe use sphinx for documentation?
option(BUILD_DOC "Build documentation" OFF)
find_package(Doxygen)
if(BUILD_DOC)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message("Doxygen is required for building documentation")
    endif()
endif(BUILD_DOC)

add_executable(${PROJECT_NAME} ${SOURCES})

# TODO: make threads not a requirement
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(${PROJECT_NAME} PUBLIC -gdwarf-4 -Wall -O0 -DFLUFF_DEBUG)