cmake_minimum_required(VERSION 3.21)
project(lmj)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Werror -Wextra -Wpedantic -Wall -march=native")
set(CMAKE_EXE_LINKER_FLAGS "-static")

#include_directories(src src/containers src/io src/math src/utils)
add_executable(lmj src/tests.cpp)
