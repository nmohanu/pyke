#include <cstdint>

#include "board.hpp"
#include "lookup_tables.hpp"

#ifndef PIECE_MOVES_H
#define PIECE_MOVES_H

namespace piece_move {

// King move.
inline BitBoard get_king_move(Square square) { return KING_MOVE_SQUARES[square]; }
// Knight move logic.
inline BitBoard get_knight_move(Square square) { return KNIGHT_MOVE_SQUARES[square]; }

// Bishop moving logic.
inline BitBoard get_bishop_move(Square square, const BitBoard& occ) {
	BitBoard mask = bishop_mask_table[square];
	BitBoard occupancy = occ & mask;
	occupancy *= bishop_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return bishop_attacks[square][occupancy];
}

// Rook move logic.
inline BitBoard get_rook_move(Square square, const BitBoard& occ) {
	BitBoard mask = rook_mask_table[square];
	BitBoard occupancy = occ & mask;
	occupancy *= rook_magic_numbers[63 - square];
	occupancy >>= (64 - __builtin_popcountll(mask));
	return rook_attacks[square][occupancy];
}

// Queen move logic.
inline BitBoard get_queen_move(Square square, const BitBoard& occ) {
	return piece_move::get_bishop_move(square, occ) | piece_move::get_rook_move(square, occ);
}
// Only the attack squares. Used to check if king is checked by pawn.
template <bool white>
static inline BitBoard get_pawn_attacks(const BitBoard& piece_board) {
	if constexpr (white)
		return ((piece_board << 9) | (piece_board << 7));
	else
		return (piece_board >> 9) | (piece_board >> 7);
}

template <bool white>
static inline BitBoard get_pawn_forward(const BitBoard& piece_board) {
	if constexpr (white)
		return piece_board << 8;
	else
		return piece_board >> 8;
}

// Pawn move including double push.
template <bool white>
static inline BitBoard get_pawn_double(const BitBoard& piece_board) {
	return get_pawn_forward<white>(get_pawn_forward<white>(piece_board));
}

template <bool white, PawnMoveType type>
inline BitBoard get_pawn_move(Square s) {
	BitBoard p_board = square_to_mask(s);
	switch (type) {
	case PawnMoveType::ATTACKS:
		return get_pawn_attacks<white>(p_board);
	case PawnMoveType::FORWARD:
		return get_pawn_forward<white>(p_board);
	case PawnMoveType::NON_DOUBLE:
		return get_pawn_attacks<white>(p_board) | get_pawn_forward<white>(p_board);
	case PawnMoveType::DOUBLE_FORWARD:
		return get_pawn_double<white>(p_board);
	case PawnMoveType::ALL:
		return get_pawn_move<white, PawnMoveType::NON_DOUBLE>(s)
			| get_pawn_move<white, PawnMoveType::DOUBLE_FORWARD>(s);
	}
}

}  // namespace piece_move

#endif	// !DEBUG
