#include "position.hpp"

#include <cstdint>

// Position constructor.
Position::Position() {}
// Position destructor.
Position::~Position() {}

// Create attack board for specific piece.
uint64_t Position::make_reach_board(Square square, bool is_black, Piece piece_type) {
	uint64_t move_board = 0b0;
	piece_type = unsign_piece(piece_type);
	switch (piece_type) {
	case 0:
		move_board = get_pawn_move(square, is_black);
		break;
	case 1:
		move_board = get_rook_move(square, is_black);
		break;
	case 2:
		move_board = get_bishop_move(square, is_black);
		break;
	case 3:
		move_board = get_queen_move(square, is_black);
		break;
	case 4:
		move_board = get_king_move(square, is_black);
		break;
	case 5:
		move_board = get_knight_move(square, is_black);
		break;
	}
	return move_board;
}

// Get the piece on given square.
uint8_t Position::get_piece(Square square) const {
	uint64_t bit_mask = 1ULL << (63 - square);
	uint8_t piece = 0;
	for (int i = 0; i < 12; i++) piece += !(bit_boards[i] & bit_mask) && (i == piece);
	return piece;
}
