cmake_minimum_required(VERSION 3.20)

project(FLACHEAD VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(FLACHEAD
    src/main.cpp
    src/app/Application.cpp
    src/system/Window.cpp
)

target_link_libraries(FLACHEAD PRIVATE SDL2::SDL2)