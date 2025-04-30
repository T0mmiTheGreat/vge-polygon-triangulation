# File: CMakeLists.txt
# Project: Polygon triangulation demo
# Author: Tomas Ludrovan
# Date: 2025-04-29
# License: MIT license

cmake_minimum_required(VERSION 3.10)

project(polygon
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    main.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

include_directories(.. ../imgui/)

# Imgui files
file (GLOB IMGUI_SOURCES
	../imgui/*.cpp ../imgui/*.h
	../imgui/backends/imgui_impl_sdl2.h ../imgui/backends/imgui_impl_sdl2.cpp
	../imgui/backends/imgui_impl_sdlrenderer2.h ../imgui/backends/imgui_impl_sdlrenderer2.cpp
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(SDL2PP_WITH_TTF OFF)
set(SDL2PP_WITH_IMAGE OFF)
set(SDL2PP_WITH_MIXER OFF)
add_subdirectory(../libSDL2pp libSDL2pp)

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} SDL2pp::SDL2pp)
