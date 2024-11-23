#include "board.hpp"
#include "lookup_tables.hpp"

#ifndef PIECE_MOVES_H
#define PIECE_MOVES_H

namespace piece_move {

// King move.
constexpr static inline BitBoard get_king_move(const Square square) { return KING_MOVE_SQUARES[square]; }
// Knight move logic.
constexpr static inline BitBoard get_knight_move(const Square square) { return KNIGHT_MOVE_SQUARES[square]; }

// Bishop moving logic.
static inline BitBoard get_bishop_move(const Square square, const BitBoard occ) {
	const BitBoard mask = bishop_mask_table[square];
	const BitBoard occupancy = ((occ & mask) * bishop_magic_numbers[square]) >> __builtin_popcountll(~mask);
	return bishop_attacks[square][occupancy];
}

// Rook move logic.
static inline BitBoard get_rook_move(const Square square, const BitBoard occ) {
	const BitBoard mask = rook_mask_table[square];
	const BitBoard occupancy = ((occ & mask) * rook_magic_numbers[square]) >> __builtin_popcountll(~mask);
	return rook_attacks[square][occupancy];
}

// Queen move logic.
static inline BitBoard get_queen_move(const Square square, const BitBoard occ) {
	return piece_move::get_bishop_move(square, occ) | piece_move::get_rook_move(square, occ);
}
// Only the attack squares. Used to check if king is checked by pawn.
template <bool white>
static inline BitBoard get_pawn_attacks(const BitBoard piece_board) {
	Square s = __builtin_clzll(piece_board);
	if constexpr (white) return ((piece_board << 9) | (piece_board << 7)) & (0xFFULL << (64 - (s - s % 8)));
	return ((piece_board >> 9) | (piece_board >> 7)) & (0xFFULL << (64 - (s - s % 8) - 16));
}

template <bool white>
static inline BitBoard get_pawn_forward(const BitBoard piece_board) {
	if constexpr (white)
		return piece_board << 8;
	else
		return piece_board >> 8;
}

// Pawn move including double push.
template <bool white>
static inline BitBoard get_pawn_double(const BitBoard piece_board, const BitBoard occ) {
	BitBoard single = get_pawn_forward<white>(piece_board) & ~occ;
	return get_pawn_forward<white>(single) & ~occ;
}

template <bool white, PawnMoveType type>
static inline BitBoard get_pawn_move(const Square s, BitBoard occ) {
	BitBoard p_board = square_to_mask(s);
	switch (type) {
	case PawnMoveType::ATTACKS:
		return get_pawn_attacks<white>(p_board);
	case PawnMoveType::FORWARD:
		return get_pawn_forward<white>(p_board);
	case PawnMoveType::NON_DOUBLE:
		return get_pawn_attacks<white>(p_board) | get_pawn_forward<white>(p_board);
	case PawnMoveType::DOUBLE_FORWARD:
		return get_pawn_double<white>(p_board, occ);
	case PawnMoveType::ALL:
		return get_pawn_move<white, PawnMoveType::NON_DOUBLE>(s, occ)
			| get_pawn_move<white, PawnMoveType::DOUBLE_FORWARD>(s, occ);
	}
}

}  // namespace piece_move

#endif
