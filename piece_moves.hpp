#include <cstdint>

#include "lookup_tables.hpp"

inline constexpr uint64_t get_king_move(uint8_t square) {
	// Get squares from memory.
	return KING_MOVE_SQUARES[square];
}

// Bishop moving logic.
inline constexpr uint64_t get_bishop_move(uint8_t square, uint64_t occ) {
	uint64_t mask = bishop_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= bishop_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));

	return bishop_attacks[square][occupancy];
}

// Knight move logic.
inline constexpr uint64_t get_knight_move(uint8_t square) {
	// Get squares from memory.
	uint64_t move_board = KNIGHT_MOVE_SQUARES[square];
	return move_board;
}

// Rook move logic.
inline constexpr uint64_t get_rook_move(uint8_t square, uint64_t occ) {
	uint64_t mask = rook_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= rook_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return rook_attacks[square][occupancy];
}

// Queen move logic.
inline constexpr uint64_t get_queen_move(uint8_t square, uint64_t occ) {
	return get_bishop_move(square, occ) | get_rook_move(square, occ);
}

// Only the attack squares.
template <bool white>
inline constexpr uint64_t get_pawn_attacks(uint64_t board, uint8_t square) {
	if (white) return ((board << 9) | (board << 7)) & (0xFFULL << (64 - (square - square % 8)));
	return (board >> 9) | (board >> 7) & (0xFFULL << (48 - (square - square % 8)));
}

// Black pawn.
inline constexpr uint64_t get_pawn_move_black(uint8_t square) {
	uint64_t move_board = 0b0;

	// is white piece.
	move_board |= ((1ULL << ((63 - square) + 8)));
	// Check if pawn can move 2 squares. Only if in initial position and target square is empty.
	move_board |= (((move_board << 8) & (0xFFULL << 24)));
	// Attacking squares.
	move_board |= ((1ULL << (63 - square + 9) | 1ULL << (63 - square + 7))) & (0xFFULL << (48 - (square - square % 8)));
	return move_board;
}

// White pawn.
inline constexpr uint64_t get_pawn_move_white(uint8_t square) {
	uint64_t move_board = 0b0;

	move_board |= 1ULL << ((63 - square) - 8);
	// Check if pawn can move 2 squares. Only if in initial position and target square is empty.
	move_board |= ((move_board >> 8) & (0xFFULL << 32));
	// Attacking squares.
	move_board |= (1ULL << (63 - square - 9) | 1ULL << (63 - square - 7)) & (0xFFULL << (64 - (square - square % 8)));
	;
	return move_board;
}

// Pawn moving.
template <bool white>
inline constexpr uint64_t get_pawn_move(uint8_t square, bool player_sign) {
	if (white)
		return get_pawn_move_white(square);
	else
		return get_pawn_move_black(square);
}
