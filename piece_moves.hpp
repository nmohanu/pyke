#include <cstdint>

#include "board.hpp"
#include "lookup_tables.hpp"

#ifndef PIECE_MOVES_H
#define PIECE_MOVES_H

// King move.
static inline uint64_t get_king_move(uint8_t square) { return KING_MOVE_SQUARES[square]; }
// Knight move logic.
static inline uint64_t get_knight_move(uint8_t square) { return KNIGHT_MOVE_SQUARES[square]; }

// Bishop moving logic.
static inline uint64_t get_bishop_move(uint8_t square, const BitBoard& occ) {
	uint64_t mask = bishop_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= bishop_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return bishop_attacks[square][occupancy];
}

// Rook move logic.
static inline uint64_t get_rook_move(uint8_t square, const BitBoard& occ) {
	uint64_t mask = rook_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= rook_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return rook_attacks[square][occupancy];
}

// Queen move logic.
static inline uint64_t get_queen_move(uint8_t square, const BitBoard& occ) {
	return get_bishop_move(square, occ) | get_rook_move(square, occ);
}

// Only the attack squares. Used to check if king is checked by pawn.
template <bool white>
static inline uint64_t get_pawn_attacks(uint64_t board, uint8_t square) {
	if (white) return ((board << 9) | (board << 7)) & (0xFFULL << (64 - (square - square % 8)));
	return (board >> 9) | (board >> 7) & (0xFFULL << (48 - (square - square % 8)));
}

// Black pawn.
static inline uint64_t get_pawn_move_black(uint8_t square) {
	uint64_t move_board = 0b0;
	move_board |= ((1ULL << ((63 - square) + 8)));
	// Attacks.
	move_board |= ((1ULL << (63 - square + 9) | 1ULL << (63 - square + 7))) & (0xFFULL << (48 - (square - square % 8)));
	return move_board;
}

// White pawn.
static inline uint64_t get_pawn_move_white(uint8_t square) {
	uint64_t move_board = 0b0;
	move_board |= 1ULL << ((63 - square) - 8);
	// Attacks.
	move_board |= (1ULL << (63 - square - 9) | 1ULL << (63 - square - 7)) & (0xFFULL << (64 - (square - square % 8)));
	;
	return move_board;
}

// Pawn moving.
template <bool white>
static inline uint64_t get_pawn_move(uint8_t square) {
	if (white)
		return get_pawn_move_white(square);
	else
		return get_pawn_move_black(square);
}

// Pawn move including double push.
template <bool white>
static inline uint64_t get_pawn_double(uint8_t square) {
	uint64_t ret;
	if (white) {
		ret = 1ULL << ((63 - square) - 8);
		ret &= ((ret >> 8) & (0xFFULL << 32));
	} else {
		ret = ((1ULL << ((63 - square) + 8)));
		ret &= (((ret << 8) & (0xFFULL << 24)));
	}
	return ret;
}

#endif	// !DEBUG
