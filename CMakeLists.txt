cmake_minimum_required(VERSION 3.21)
project(lmj)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static -Werror -Wextra -Wall -Wpedantic -march=native")

#include_directories(src src/lmj_containers src/lmj_io src/lmj_math src/lmj_utils)
add_executable(lmj src/tests.cpp)
