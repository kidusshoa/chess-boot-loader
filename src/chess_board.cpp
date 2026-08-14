#include "chess_board.h"

#include "asset_loader.h"
#include "board_renderer.h"
#include "piece_renderer.h"

ChessBoard::ChessBoard() {
    reset_to_starting_position();
}

void ChessBoard::reset_to_starting_position() {
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            squares_[rank][file] = {PieceType::None, Color::White};
        }
    }

    const PieceType back_rank[] = {
        PieceType::Rook,
        PieceType::Knight,
        PieceType::Bishop,
        PieceType::Queen,
        PieceType::King,
        PieceType::Bishop,
        PieceType::Knight,
        PieceType::Rook,
    };

    for (int file = 0; file < 8; ++file) {
        squares_[0][file] = {back_rank[file], Color::White};
        squares_[1][file] = {PieceType::Pawn, Color::White};
        squares_[6][file] = {PieceType::Pawn, Color::Black};
        squares_[7][file] = {back_rank[file], Color::Black};
    }
}

const Piece& ChessBoard::at(int file, int rank) const {
    return squares_[rank][file];
}

Piece& ChessBoard::at(int file, int rank) {
    return squares_[rank][file];
}

void ChessBoard::move_piece(int from_file, int from_rank, int to_file, int to_rank) {
    Piece moving_piece = squares_[from_rank][from_file];
    squares_[from_rank][from_file] = {PieceType::None, Color::White};
    squares_[to_rank][to_file] = moving_piece;

    if (moving_piece.type == PieceType::Pawn) {
        if ((moving_piece.color == Color::White && to_rank == 7) ||
            (moving_piece.color == Color::Black && to_rank == 0)) {
            squares_[to_rank][to_file].type = PieceType::Queen;
        }
    }
}

void ChessBoard::draw(
    SDL_Renderer* renderer,
    const AssetLoader& assets,
    const BoardRenderer& board_renderer
) const {
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const Piece& piece = squares_[rank][file];
            if (!piece.is_empty()) {
                PieceRenderer::draw(renderer, assets, board_renderer, piece, file, rank);
            }
        }
    }
}
