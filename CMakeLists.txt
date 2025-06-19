cmake_minimum_required(VERSION 3.16)
project(leakdetector)

set(CMAKE_C_STANDARD 23)

add_library(leakdetector STATIC src/leakdetector.c)
target_include_directories(leakdetector PUBLIC include)
target_compile_options(leakdetector PRIVATE -std=c23 -Wall -Wextra -Wpedantic)

target_compile_options(leakdetector INTERFACE "-include${CMAKE_CURRENT_SOURCE_DIR}/include/leakdetector.h")

# Support Tracy.
if(TRACY_ENABLE)
    target_compile_definitions(leakdetector PRIVATE "TRACY_ENABLE")
    target_link_libraries(leakdetector PRIVATE TracyClient)
endif()

# Enable leak detector.
if(LEAKDETECTOR)
    target_compile_definitions(leakdetector PRIVATE "LEAKDETECTOR")
endif()
