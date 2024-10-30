#include <cstdint>

#include "lookup_tables.hpp"
#include "position.hpp"

uint64_t Position::get_king_move(Square square) {
	// Get squares from memory.
	return KING_MOVE_SQUARES[square];
}

// Bishop moving logic.
uint64_t Position::get_bishop_move(Square square) {
	uint64_t mask = bishop_mask_table[square];
	uint64_t occupancy = TOTAL_BOARD & mask;
	occupancy *= bishop_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));

	return bishop_attacks[square][occupancy];
}

// Knight move logic.
uint64_t Position::get_knight_move(Square square) {
	// Get squares from memory.
	uint64_t move_board = KNIGHT_MOVE_SQUARES[square];
	return move_board;
}

// Rook move logic.
uint64_t Position::get_rook_move(Square square) {
	uint64_t mask = rook_mask_table[square];
	uint64_t occupancy = TOTAL_BOARD & mask;
	occupancy *= rook_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return rook_attacks[square][occupancy];
}

// Queen move logic.
uint64_t Position::get_queen_move(Square square) { return get_bishop_move(square) | get_rook_move(square); }

// Pawn moving.
uint64_t Position::get_pawn_move(Square square, Flag player_sign) {
	if (player_sign) return get_pawn_move_black(square, player_sign);
	return get_pawn_move_white(square, player_sign);
}

// Black pawn.
uint64_t Position::get_pawn_move_black(Square square, bool is_black) {
	uint64_t move_board = 0b0;

	// is white piece.
	move_board |= ((1ULL << ((63 - square) + 8) & ~TOTAL_BOARD));
	// Check if pawn can move 2 squares. Only if in initial position and target square is empty.
	move_board |= (((move_board << 8) & (0xFFULL << 24)) & ~TOTAL_BOARD);
	// Attacking squares.
	move_board |=
		((1ULL << (63 - square + 9) | 1ULL << (63 - square + 7))
		 & (0xFFULL << (64 - (square - square % 8)) & BLACK_PIECE_BOARD));
	return move_board;
}

// White pawn.
uint64_t Position::get_pawn_move_white(Square square, bool is_black) {
	uint64_t move_board = 0b0;

	move_board |= 1ULL << ((63 - square) - 8) & ~TOTAL_BOARD;
	// Check if pawn can move 2 squares. Only if in initial position and target square is empty.
	move_board |= ((move_board >> 8) & (0xFFULL << 32)) & ~TOTAL_BOARD;
	// Attacking squares.
	move_board |= (1ULL << (63 - square - 9) | 1ULL << (63 - square - 7))
		& (0xFFULL << (64 - (square - square % 8) - 16) & ~(BLACK_PIECE_BOARD)&TOTAL_BOARD);
	return move_board;
}
