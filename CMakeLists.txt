cmake_minimum_required(VERSION 3.16)
project(chess-boot-loader VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET GLOBAL sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET GLOBAL SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET GLOBAL SDL2_ttf)
pkg_check_modules(RSVG REQUIRED IMPORTED_TARGET GLOBAL librsvg-2.0)

set(PIECE_ICON_NAMES c java python javascript rust go)
find_program(RSVG_CONVERT rsvg-convert)

set(GENERATED_PIECE_PNGS "")
if(RSVG_CONVERT)
    foreach(icon_name ${PIECE_ICON_NAMES})
        set(icon_svg ${CMAKE_SOURCE_DIR}/assets/pieces/${icon_name}.svg)
        set(icon_png ${CMAKE_BINARY_DIR}/assets/pieces/${icon_name}.png)
        add_custom_command(
            OUTPUT ${icon_png}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets/pieces
            COMMAND ${RSVG_CONVERT} -w 256 -h 256 ${icon_svg} -o ${icon_png}
            DEPENDS ${icon_svg}
            COMMENT "Rasterizing ${icon_name}.svg to PNG"
        )
        list(APPEND GENERATED_PIECE_PNGS ${icon_png})
    endforeach()
endif()

add_executable(chess-boot-loader
    src/main.cpp
    src/boot_splash.cpp
    src/asset_loader.cpp
    src/svg_loader.cpp
    src/board_renderer.cpp
    src/chess_types.cpp
    src/chess_board.cpp
    src/piece_renderer.cpp
    src/game_controller.cpp
    src/game_ui.cpp
    src/move_validator.cpp
)

if(GENERATED_PIECE_PNGS)
    add_custom_target(generate_piece_pngs DEPENDS ${GENERATED_PIECE_PNGS})
    add_dependencies(chess-boot-loader generate_piece_pngs)
endif()

target_compile_definitions(chess-boot-loader PRIVATE
    CHESS_BOOT_LOADER_VERSION="${PROJECT_VERSION}"
)

target_include_directories(chess-boot-loader PRIVATE include)
target_link_libraries(chess-boot-loader PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
    PkgConfig::RSVG
)

add_custom_command(
    TARGET chess-boot-loader
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:chess-boot-loader>/assets
    COMMENT "Copying assets next to chess-boot-loader binary"
)

if(GENERATED_PIECE_PNGS)
    add_custom_command(
        TARGET chess-boot-loader
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_BINARY_DIR}/assets/pieces
            $<TARGET_FILE_DIR:chess-boot-loader>/assets/pieces
        COMMENT "Copying rasterized piece PNGs next to binary"
    )
endif()
