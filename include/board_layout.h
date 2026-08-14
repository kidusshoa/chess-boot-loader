#pragma once

// Measured from assets/chess-board-banner-vector.jpg (1920x1920).
constexpr int BOARD_TEXTURE_SIZE = 1920;
constexpr int BOARD_GRID_ORIGIN = 48;
constexpr int BOARD_GRID_SIZE = 1824;

constexpr float BOARD_GRID_ORIGIN_NORM =
    static_cast<float>(BOARD_GRID_ORIGIN) / static_cast<float>(BOARD_TEXTURE_SIZE);
constexpr float BOARD_GRID_SIZE_NORM =
    static_cast<float>(BOARD_GRID_SIZE) / static_cast<float>(BOARD_TEXTURE_SIZE);
