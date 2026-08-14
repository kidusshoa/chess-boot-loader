#pragma once

class ChessBoard;

#include <utility>
#include <vector>

bool is_legal_move(
    const ChessBoard& board,
    int from_file,
    int from_rank,
    int to_file,
    int to_rank
);

std::vector<std::pair<int, int>> legal_moves_from(
    const ChessBoard& board,
    int from_file,
    int from_rank
);
