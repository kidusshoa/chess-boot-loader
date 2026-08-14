#include "move_validator.h"

#include "chess_board.h"

#include <algorithm>
#include <cstdlib>

namespace {

bool in_bounds(int file, int rank) {
    return file >= 0 && file <= 7 && rank >= 0 && rank <= 7;
}

int sign(int value) {
    if (value > 0) {
        return 1;
    }
    if (value < 0) {
        return -1;
    }
    return 0;
}

bool is_enemy(const Piece& piece, Color color) {
    return !piece.is_empty() && piece.color != color;
}

bool path_is_clear(const ChessBoard& board, int from_file, int from_rank, int to_file, int to_rank) {
    const int file_step = sign(to_file - from_file);
    const int rank_step = sign(to_rank - from_rank);

    int file = from_file + file_step;
    int rank = from_rank + rank_step;

    while (file != to_file || rank != to_rank) {
        if (!board.at(file, rank).is_empty()) {
            return false;
        }
        file += file_step;
        rank += rank_step;
    }

    return true;
}

bool validate_pawn_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank,
    Color color
) {
    const int file_delta = to_file - from_file;
    const int rank_delta = to_rank - from_rank;
    const Piece& destination = board.at(to_file, to_rank);

    if (color == Color::White) {
        if (file_delta == 0 && rank_delta == 1 && destination.is_empty()) {
            return true;
        }
        if (file_delta == 0 && rank_delta == 2 && from_rank == 1 && destination.is_empty() &&
            board.at(from_file, from_rank + 1).is_empty()) {
            return true;
        }
        if (std::abs(file_delta) == 1 && rank_delta == 1 && is_enemy(destination, color)) {
            return true;
        }
        return false;
    }

    if (file_delta == 0 && rank_delta == -1 && destination.is_empty()) {
        return true;
    }
    if (file_delta == 0 && rank_delta == -2 && from_rank == 6 && destination.is_empty() &&
        board.at(from_file, from_rank - 1).is_empty()) {
        return true;
    }
    if (std::abs(file_delta) == 1 && rank_delta == -1 && is_enemy(destination, color)) {
        return true;
    }

    return false;
}

bool validate_knight_move(int from_file, int from_rank, int to_file, int to_rank) {
    const int file_delta = std::abs(to_file - from_file);
    const int rank_delta = std::abs(to_rank - from_rank);
    return (file_delta == 2 && rank_delta == 1) || (file_delta == 1 && rank_delta == 2);
}

bool validate_king_move(int from_file, int from_rank, int to_file, int to_rank) {
    const int file_delta = std::abs(to_file - from_file);
    const int rank_delta = std::abs(to_rank - from_rank);
    return file_delta <= 1 && rank_delta <= 1;
}

bool validate_sliding_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank,
    bool diagonal,
    bool straight
) {
    const int file_delta = to_file - from_file;
    const int rank_delta = to_rank - from_rank;

    const bool is_diagonal = file_delta != 0 && rank_delta != 0 && std::abs(file_delta) == std::abs(rank_delta);
    const bool is_straight = (file_delta == 0) != (rank_delta == 0);

    if (diagonal && is_diagonal) {
        return path_is_clear(board, from_file, from_rank, to_file, to_rank);
    }
    if (straight && is_straight) {
        return path_is_clear(board, from_file, from_rank, to_file, to_rank);
    }

    return false;
}

}  // namespace

bool is_legal_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
) {
    if (!in_bounds(from_file, from_rank) || !in_bounds(to_file, to_rank)) {
        return false;
    }

    if (from_file == to_file && from_rank == to_rank) {
        return false;
    }

    const Piece& moving_piece = board.at(from_file, from_rank);
    const Piece& destination = board.at(to_file, to_rank);

    if (moving_piece.is_empty()) {
        return false;
    }

    if (!destination.is_empty() && destination.color == moving_piece.color) {
        return false;
    }

    switch (moving_piece.type) {
        case PieceType::Pawn:
            return validate_pawn_move(board, from_file, from_rank, to_file, to_rank, moving_piece.color);
        case PieceType::Knight:
            return validate_knight_move(from_file, from_rank, to_file, to_rank);
        case PieceType::Bishop:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, true, false);
        case PieceType::Rook:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, false, true);
        case PieceType::Queen:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, true, true);
        case PieceType::King:
            return validate_king_move(from_file, from_rank, to_file, to_rank);
        case PieceType::None:
        default:
            return false;
    }
}

std::vector<std::pair<int, int>> legal_moves_from(
    const ChessBoard& board,
    int from_file,
    int from_rank
) {
    std::vector<std::pair<int, int>> moves;

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            if (is_legal_move(board, from_file, from_rank, file, rank)) {
                moves.emplace_back(file, rank);
            }
        }
    }

    return moves;
}
