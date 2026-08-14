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
    src/asset_loader.cpp
    src/board_renderer.cpp
    src/chess_types.cpp
    src/chess_board.cpp
    src/piece_renderer.cpp
    src/game_controller.cpp
    src/game_ui.cpp
    src/move_validator.cpp
)

target_compile_definitions(chess-boot-loader PRIVATE
    CHESS_BOOT_LOADER_VERSION="${PROJECT_VERSION}"
)

target_include_directories(chess-boot-loader PRIVATE include)
target_link_libraries(chess-boot-loader PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
)

add_custom_command(
    TARGET chess-boot-loader
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:chess-boot-loader>/assets
    COMMENT "Copying assets next to chess-boot-loader binary"
)
