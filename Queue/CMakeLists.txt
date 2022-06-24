cmake_minimum_required(VERSION 3.22)

project(Queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Queue)
target_sources(Queue PRIVATE "main.cpp")
target_include_directories(Queue PRIVATE "${PROJECT_SOURCE_DIR}")
add_subdirectory("Implementation")

add_library(range_v3 INTERFACE IMPORTED)
set_target_properties(range_v3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES D:/Study/Projects/vcpkg/installed/x86-windows/include)

target_link_libraries(Queue PUBLIC range_v3)

install(TARGETS Queue)
