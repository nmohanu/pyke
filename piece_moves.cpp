#include "piece_moves.hpp"

// King move.
static inline uint64_t get_king_move(Square square) { return KING_MOVE_SQUARES[square]; }
// Knight move logic.
static inline uint64_t get_knight_move(Square square) { return KNIGHT_MOVE_SQUARES[square]; }

// Bishop moving logic.
static inline uint64_t get_bishop_move(Square square, const BitBoard& occ) {
	uint64_t mask = bishop_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= bishop_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return bishop_attacks[square][occupancy];
}

// Rook move logic.
static inline uint64_t get_rook_move(Square square, const BitBoard& occ) {
	uint64_t mask = rook_mask_table[square];
	uint64_t occupancy = occ & mask;
	occupancy *= rook_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return rook_attacks[square][occupancy];
}

// Queen move logic.
static inline uint64_t get_queen_move(Square square, const BitBoard& occ) {
	return get_bishop_move(square, occ) | get_rook_move(square, occ);
}

// Only the attack squares. Used to check if king is checked by pawn.
template <bool white>
static inline uint64_t get_pawn_attacks(const BitBoard& piece_board, Square square) {
	if constexpr (white)
		return ((piece_board << 9) | (piece_board << 7));
	else
		return (piece_board >> 9) | (piece_board >> 7);
}

// Pawn moving.
template <bool white>
static inline uint64_t get_pawn_non_double(Square square) {
	uint64_t move_board = 0b0;
	if constexpr (white) {
		move_board |= ((1ULL << ((63 - square) + 8)));
		// Attacks.
		move_board |=
			((1ULL << (63 - square + 9) | 1ULL << (63 - square + 7))) & (0xFFULL << (48 - (square - square % 8)));

	} else {
		move_board |= 1ULL << ((63 - square) - 8);
		// Attacks.
		move_board |=
			(1ULL << (63 - square - 9) | 1ULL << (63 - square - 7)) & (0xFFULL << (64 - (square - square % 8)));
	}
	return move_board;
}

// Pawn move including double push.
template <bool white>
static inline uint64_t get_pawn_double(Square square) {
	uint64_t ret;
	if constexpr (white) {
		ret = 1ULL << ((63 - square) - 8);
		ret &= ((ret >> 8) & (0xFFULL << 32));
	} else {
		ret = ((1ULL << ((63 - square) + 8)));
		ret &= (((ret << 8) & (0xFFULL << 24)));
	}
	return ret;
}
