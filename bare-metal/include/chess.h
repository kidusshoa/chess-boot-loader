#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PIECE_NONE = 0,
    PIECE_KING,
    PIECE_QUEEN,
    PIECE_BISHOP,
    PIECE_KNIGHT,
    PIECE_ROOK,
    PIECE_PAWN,
} piece_type_t;

typedef enum {
    COLOR_WHITE = 0,
    COLOR_BLACK,
} color_t;

typedef struct {
    piece_type_t type;
    color_t color;
} piece_t;

typedef struct {
    int file;
    int rank;
} square_t;

typedef struct {
    piece_t squares[8][8];
} chess_board_t;

void chess_board_reset(chess_board_t* board);
const piece_t* chess_at(const chess_board_t* board, int file, int rank);
piece_t* chess_at_mut(chess_board_t* board, int file, int rank);
void chess_move_piece(chess_board_t* board, int from_file, int from_rank, int to_file, int to_rank);

bool chess_is_legal_move(const chess_board_t* board, int from_file, int from_rank, int to_file, int to_rank);
int chess_legal_moves_from(const chess_board_t* board, int from_file, int from_rank, square_t* out, int max_out);
bool chess_is_in_check(const chess_board_t* board, color_t color);
bool chess_is_checkmate(const chess_board_t* board, color_t color);
bool chess_is_stalemate(const chess_board_t* board, color_t color);
const char* chess_piece_label(piece_type_t type);
