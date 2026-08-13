#include "chess_types.h"

const char* piece_asset_file(PieceType type) {
    switch (type) {
        case PieceType::King:
            return "c.png";
        case PieceType::Queen:
            return "java.png";
        case PieceType::Bishop:
            return "python.png";
        case PieceType::Knight:
            return "javascript.png";
        case PieceType::Rook:
            return "rust.png";
        case PieceType::Pawn:
            return "go.png";
        case PieceType::None:
        default:
            return nullptr;
    }
}

const char* piece_fallback_label(PieceType type) {
    switch (type) {
        case PieceType::King:
            return "C";
        case PieceType::Queen:
            return "Java";
        case PieceType::Bishop:
            return "Py";
        case PieceType::Knight:
            return "JS";
        case PieceType::Rook:
            return "Rust";
        case PieceType::Pawn:
            return "Go";
        case PieceType::None:
        default:
            return "?";
    }
}
