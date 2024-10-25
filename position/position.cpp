#include "position.hpp"

#include <cstdint>

// Position constructor.
Position::Position() {}
// Position destructor.
Position::~Position() {}

// Create attack board for specific piece.
uint64_t Position::make_reach_board(Square square, bool is_black, Piece piece_type) {
	return (this->*move_functions[piece_type - 6 * is_black])(square, is_black);
}

// Get the piece on given square.
uint8_t Position::get_piece(Square square) const {
	uint64_t bit_mask = 1ULL << (63 - square);
	uint8_t piece = 0;
	for (int i = 0; i < 12; i++) piece += !(bit_boards[i] & bit_mask) && (i == piece);
	return piece;
}
