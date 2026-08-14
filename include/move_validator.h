#pragma once

class ChessBoard;

#include "chess_types.h"

#include <utility>
#include <vector>

bool is_pseudo_legal_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
);

bool is_legal_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
);

bool find_king(const ChessBoard& board, Color color, int& file, int& rank);
bool is_square_attacked(const ChessBoard& board, int file, int rank, Color by_color);
bool is_in_check(const ChessBoard& board, Color color);
bool has_any_legal_move(const ChessBoard& board, Color color);
bool is_checkmate(const ChessBoard& board, Color color);
bool is_stalemate(const ChessBoard& board, Color color);

std::vector<std::pair<int, int>> legal_moves_from(
    const ChessBoard& board,
    int from_file,
    int from_rank
);
