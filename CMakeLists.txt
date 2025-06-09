cmake_minimum_required(VERSION 3.16)
project(leakdetector)

set(CMAKE_C_STANDARD 23)

add_library(leakdetector STATIC src/leakdetector.c)
target_include_directories(leakdetector PUBLIC include)
target_compile_options(leakdetector PRIVATE -std=c23 -Wall -Wextra -Wpedantic)
