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

Color opposite_color(Color color) {
    return color == Color::White ? Color::Black : Color::White;
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
    return file_delta <= 1 && rank_delta <= 1 && (file_delta + rank_delta > 0);
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

bool attacks_square(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int target_file,
    int target_rank
) {
    const Piece& piece = board.at(from_file, from_rank);
    if (piece.is_empty()) {
        return false;
    }

    const int file_delta = target_file - from_file;
    const int rank_delta = target_rank - from_rank;

    switch (piece.type) {
        case PieceType::Pawn:
            if (piece.color == Color::White) {
                return std::abs(file_delta) == 1 && rank_delta == 1;
            }
            return std::abs(file_delta) == 1 && rank_delta == -1;
        case PieceType::Knight:
            return validate_knight_move(from_file, from_rank, target_file, target_rank);
        case PieceType::Bishop:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, true, false);
        case PieceType::Rook:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, false, true);
        case PieceType::Queen:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, true, true);
        case PieceType::King:
            return validate_king_move(from_file, from_rank, target_file, target_rank);
        case PieceType::None:
        default:
            return false;
    }
}

}  // namespace

bool is_pseudo_legal_move(
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

bool find_king(const ChessBoard& board, Color color, int& file, int& rank) {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const Piece& piece = board.at(f, r);
            if (piece.type == PieceType::King && piece.color == color) {
                file = f;
                rank = r;
                return true;
            }
        }
    }
    return false;
}

bool is_square_attacked(const ChessBoard& board, int file, int rank, Color by_color) {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const Piece& piece = board.at(f, r);
            if (!piece.is_empty() && piece.color == by_color) {
                if (attacks_square(board, f, r, file, rank)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool is_in_check(const ChessBoard& board, Color color) {
    int king_file = 0;
    int king_rank = 0;
    if (!find_king(board, color, king_file, king_rank)) {
        return false;
    }
    return is_square_attacked(board, king_file, king_rank, opposite_color(color));
}

bool is_legal_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
) {
    if (!is_pseudo_legal_move(board, from_file, from_rank, to_file, to_rank)) {
        return false;
    }

    const Color moving_color = board.at(from_file, from_rank).color;
    ChessBoard simulated = board;
    simulated.move_piece(from_file, from_rank, to_file, to_rank);
    return !is_in_check(simulated, moving_color);
}

bool has_any_legal_move(const ChessBoard& board, Color color) {
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const Piece& piece = board.at(file, rank);
            if (!piece.is_empty() && piece.color == color) {
                if (!legal_moves_from(board, file, rank).empty()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool is_checkmate(const ChessBoard& board, Color color) {
    return is_in_check(board, color) && !has_any_legal_move(board, color);
}

bool is_stalemate(const ChessBoard& board, Color color) {
    return !is_in_check(board, color) && !has_any_legal_move(board, color);
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
