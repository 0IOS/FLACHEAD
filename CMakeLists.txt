cmake_minimum_required(VERSION 3.20)

project(FLACHEAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)

add_executable(FLACHEAD
    src/main.cpp
    src/app/Application.cpp
    src/system/Window.cpp
    src/core/Renderer.cpp
)
target_link_libraries(FLACHEAD PRIVATE SDL3::SDL3)