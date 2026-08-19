#include "chess.h"

static int abs_int(int value) {
    return value < 0 ? -value : value;
}

static int sign_int(int value) {
    if (value > 0) {
        return 1;
    }
    if (value < 0) {
        return -1;
    }
    return 0;
}

static bool in_bounds(int file, int rank) {
    return file >= 0 && file <= 7 && rank >= 0 && rank <= 7;
}

static color_t opposite_color(color_t color) {
    return color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

static bool piece_is_empty(const piece_t* piece) {
    return piece->type == PIECE_NONE;
}

static bool is_enemy_piece(const piece_t* piece, color_t color) {
    return !piece_is_empty(piece) && piece->color != color;
}

const char* chess_piece_label(piece_type_t type) {
    switch (type) {
        case PIECE_KING:
            return "C";
        case PIECE_QUEEN:
            return "Java";
        case PIECE_BISHOP:
            return "Py";
        case PIECE_KNIGHT:
            return "JS";
        case PIECE_ROOK:
            return "Rust";
        case PIECE_PAWN:
            return "Go";
        case PIECE_NONE:
        default:
            return "?";
    }
}

void chess_board_reset(chess_board_t* board) {
    const piece_type_t back_rank[] = {
        PIECE_ROOK,
        PIECE_KNIGHT,
        PIECE_BISHOP,
        PIECE_QUEEN,
        PIECE_KING,
        PIECE_BISHOP,
        PIECE_KNIGHT,
        PIECE_ROOK,
    };

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            board->squares[rank][file].type = PIECE_NONE;
            board->squares[rank][file].color = COLOR_WHITE;
        }
    }

    for (int file = 0; file < 8; ++file) {
        board->squares[0][file] = (piece_t){back_rank[file], COLOR_WHITE};
        board->squares[1][file] = (piece_t){PIECE_PAWN, COLOR_WHITE};
        board->squares[6][file] = (piece_t){PIECE_PAWN, COLOR_BLACK};
        board->squares[7][file] = (piece_t){back_rank[file], COLOR_BLACK};
    }
}

const piece_t* chess_at(const chess_board_t* board, int file, int rank) {
    return &board->squares[rank][file];
}

piece_t* chess_at_mut(chess_board_t* board, int file, int rank) {
    return &board->squares[rank][file];
}

void chess_move_piece(chess_board_t* board, int from_file, int from_rank, int to_file, int to_rank) {
    const piece_t moving = board->squares[from_rank][from_file];
    board->squares[from_rank][from_file] = (piece_t){PIECE_NONE, COLOR_WHITE};
    board->squares[to_rank][to_file] = moving;

    if (moving.type == PIECE_PAWN) {
        if ((moving.color == COLOR_WHITE && to_rank == 7) || (moving.color == COLOR_BLACK && to_rank == 0)) {
            board->squares[to_rank][to_file].type = PIECE_QUEEN;
        }
    }
}

static bool path_is_clear(const chess_board_t* board, int from_file, int from_rank, int to_file, int to_rank) {
    const int file_step = sign_int(to_file - from_file);
    const int rank_step = sign_int(to_rank - from_rank);

    int file = from_file + file_step;
    int rank = from_rank + rank_step;

    while (file != to_file || rank != to_rank) {
        if (!piece_is_empty(chess_at(board, file, rank))) {
            return false;
        }
        file += file_step;
        rank += rank_step;
    }

    return true;
}

static bool validate_pawn_move(
    const chess_board_t* board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank,
    color_t color
) {
    const int file_delta = to_file - from_file;
    const int rank_delta = to_rank - from_rank;
    const piece_t* destination = chess_at(board, to_file, to_rank);

    if (color == COLOR_WHITE) {
        if (file_delta == 0 && rank_delta == 1 && piece_is_empty(destination)) {
            return true;
        }
        if (file_delta == 0 && rank_delta == 2 && from_rank == 1 && piece_is_empty(destination) &&
            piece_is_empty(chess_at(board, from_file, from_rank + 1))) {
            return true;
        }
        if (abs_int(file_delta) == 1 && rank_delta == 1 && is_enemy_piece(destination, color)) {
            return true;
        }
        return false;
    }

    if (file_delta == 0 && rank_delta == -1 && piece_is_empty(destination)) {
        return true;
    }
    if (file_delta == 0 && rank_delta == -2 && from_rank == 6 && piece_is_empty(destination) &&
        piece_is_empty(chess_at(board, from_file, from_rank - 1))) {
        return true;
    }
    if (abs_int(file_delta) == 1 && rank_delta == -1 && is_enemy_piece(destination, color)) {
        return true;
    }

    return false;
}

static bool validate_knight_move(int from_file, int from_rank, int to_file, int to_rank) {
    const int file_delta = abs_int(to_file - from_file);
    const int rank_delta = abs_int(to_rank - from_rank);
    return (file_delta == 2 && rank_delta == 1) || (file_delta == 1 && rank_delta == 2);
}

static bool validate_king_move(int from_file, int from_rank, int to_file, int to_rank) {
    const int file_delta = abs_int(to_file - from_file);
    const int rank_delta = abs_int(to_rank - from_rank);
    return file_delta <= 1 && rank_delta <= 1 && (file_delta + rank_delta > 0);
}

static bool validate_sliding_move(
    const chess_board_t* board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank,
    bool diagonal,
    bool straight
) {
    const int file_delta = to_file - from_file;
    const int rank_delta = to_rank - from_rank;

    const bool is_diagonal =
        file_delta != 0 && rank_delta != 0 && abs_int(file_delta) == abs_int(rank_delta);
    const bool is_straight = (file_delta == 0) != (rank_delta == 0);

    if (diagonal && is_diagonal) {
        return path_is_clear(board, from_file, from_rank, to_file, to_rank);
    }
    if (straight && is_straight) {
        return path_is_clear(board, from_file, from_rank, to_file, to_rank);
    }

    return false;
}

static bool attacks_square(
    const chess_board_t* board,
    int from_file,
    int from_rank,
    int target_file,
    int target_rank
) {
    const piece_t* piece = chess_at(board, from_file, from_rank);
    if (piece_is_empty(piece)) {
        return false;
    }

    const int file_delta = target_file - from_file;
    const int rank_delta = target_rank - from_rank;

    switch (piece->type) {
        case PIECE_PAWN:
            if (piece->color == COLOR_WHITE) {
                return abs_int(file_delta) == 1 && rank_delta == 1;
            }
            return abs_int(file_delta) == 1 && rank_delta == -1;
        case PIECE_KNIGHT:
            return validate_knight_move(from_file, from_rank, target_file, target_rank);
        case PIECE_BISHOP:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, true, false);
        case PIECE_ROOK:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, false, true);
        case PIECE_QUEEN:
            return validate_sliding_move(board, from_file, from_rank, target_file, target_rank, true, true);
        case PIECE_KING:
            return validate_king_move(from_file, from_rank, target_file, target_rank);
        case PIECE_NONE:
        default:
            return false;
    }
}

static bool is_pseudo_legal_move(
    const chess_board_t* board,
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

    const piece_t* moving_piece = chess_at(board, from_file, from_rank);
    const piece_t* destination = chess_at(board, to_file, to_rank);

    if (piece_is_empty(moving_piece)) {
        return false;
    }

    if (!piece_is_empty(destination) && destination->color == moving_piece->color) {
        return false;
    }

    switch (moving_piece->type) {
        case PIECE_PAWN:
            return validate_pawn_move(board, from_file, from_rank, to_file, to_rank, moving_piece->color);
        case PIECE_KNIGHT:
            return validate_knight_move(from_file, from_rank, to_file, to_rank);
        case PIECE_BISHOP:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, true, false);
        case PIECE_ROOK:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, false, true);
        case PIECE_QUEEN:
            return validate_sliding_move(board, from_file, from_rank, to_file, to_rank, true, true);
        case PIECE_KING:
            return validate_king_move(from_file, from_rank, to_file, to_rank);
        case PIECE_NONE:
        default:
            return false;
    }
}

static bool find_king(const chess_board_t* board, color_t color, int* file, int* rank) {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const piece_t* piece = chess_at(board, f, r);
            if (piece->type == PIECE_KING && piece->color == color) {
                *file = f;
                *rank = r;
                return true;
            }
        }
    }
    return false;
}

static bool is_square_attacked(const chess_board_t* board, int file, int rank, color_t by_color) {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const piece_t* piece = chess_at(board, f, r);
            if (!piece_is_empty(piece) && piece->color == by_color) {
                if (attacks_square(board, f, r, file, rank)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool chess_is_in_check(const chess_board_t* board, color_t color) {
    int king_file = 0;
    int king_rank = 0;
    if (!find_king(board, color, &king_file, &king_rank)) {
        return false;
    }
    return is_square_attacked(board, king_file, king_rank, opposite_color(color));
}

static void chess_copy_board(chess_board_t* dst, const chess_board_t* src) {
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            dst->squares[rank][file] = src->squares[rank][file];
        }
    }
}

bool chess_is_legal_move(
    const chess_board_t* board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
) {
    if (!is_pseudo_legal_move(board, from_file, from_rank, to_file, to_rank)) {
        return false;
    }

    const color_t moving_color = chess_at(board, from_file, from_rank)->color;
    chess_board_t simulated;
    chess_copy_board(&simulated, board);
    chess_move_piece(&simulated, from_file, from_rank, to_file, to_rank);
    return !chess_is_in_check(&simulated, moving_color);
}

int chess_legal_moves_from(
    const chess_board_t* board,
    int from_file,
    int from_rank,
    square_t* out,
    int max_out
) {
    int count = 0;

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            if (chess_is_legal_move(board, from_file, from_rank, file, rank)) {
                if (out && count < max_out) {
                    out[count].file = file;
                    out[count].rank = rank;
                }
                ++count;
            }
        }
    }

    return count;
}

static bool has_any_legal_move(const chess_board_t* board, color_t color) {
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const piece_t* piece = chess_at(board, file, rank);
            if (!piece_is_empty(piece) && piece->color == color) {
                if (chess_legal_moves_from(board, file, rank, 0, 0) > 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool chess_is_checkmate(const chess_board_t* board, color_t color) {
    return chess_is_in_check(board, color) && !has_any_legal_move(board, color);
}

bool chess_is_stalemate(const chess_board_t* board, color_t color) {
    return !chess_is_in_check(board, color) && !has_any_legal_move(board, color);
}
