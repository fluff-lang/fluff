# CMake initializers
cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Project
project(Fluff LANGUAGES C VERSION 0.1.0)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source
file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE INCLUDES include/*.h)
list(REMOVE_ITEM SOURCES src/main.c)
include_directories(include/)

# Packages
find_package(Threads REQUIRED)

# Library
add_library(libfluff SHARED ${SOURCES})
target_include_directories(libfluff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(libfluff PROPERTIES OUTPUT_NAME "fluff")

target_link_libraries(libfluff ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(libfluff m)

# CLI
add_executable(fluff src/main.c)

target_link_libraries(fluff PRIVATE libfluff)
target_compile_options(fluff PUBLIC -gdwarf-4 -Wall -O0 -DFLUFF_DEBUG)

# Install
install(TARGETS libfluff
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS fluff)

install(FILES ${INCLUDES}
    DESTINATION include
)

# Documentation
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