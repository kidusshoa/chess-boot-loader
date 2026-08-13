#pragma once

enum class PieceType {
    None,
    King,
    Queen,
    Bishop,
    Knight,
    Rook,
    Pawn,
};

enum class Color {
    White,
    Black,
};

struct Piece {
    PieceType type = PieceType::None;
    Color color = Color::White;

    bool is_empty() const {
        return type == PieceType::None;
    }
};

const char* piece_asset_file(PieceType type);
const char* piece_fallback_label(PieceType type);
