cmake_minimum_required(VERSION 3.16)
project(chess-boot-loader VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET GLOBAL sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET GLOBAL SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET GLOBAL SDL2_ttf)

add_executable(chess-boot-loader
    src/main.cpp
    src/boot_splash.cpp
)

target_include_directories(chess-boot-loader PRIVATE include)
target_link_libraries(chess-boot-loader PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
)
