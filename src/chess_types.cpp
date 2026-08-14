#include "chess_types.h"

const char* piece_asset_file(PieceType type) {
    switch (type) {
        case PieceType::King:
            return "c.svg";
        case PieceType::Queen:
            return "java.svg";
        case PieceType::Bishop:
            return "python.svg";
        case PieceType::Knight:
            return "javascript.svg";
        case PieceType::Rook:
            return "rust.svg";
        case PieceType::Pawn:
            return "go.svg";
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
