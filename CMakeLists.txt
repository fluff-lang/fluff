cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

project(fluff LANGUAGES C VERSION 0.1.0)

file(GLOB_RECURSE SOURCES src/*.c)
include_directories(include/)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PUBLIC -gdwarf-4 -Wall -O0 -DFLUFF_DEBUG)