#include <immintrin.h>

#include <cstdint>

#include "board.hpp"
#include "lookup_tables.hpp"

#ifndef PIECE_MOVES_H
#define PIECE_MOVES_H

// King move.
static inline BitBoard get_king_move(const Square square) { return KING_MOVE_SQUARES[square]; }
// Knight move logic.
static inline BitBoard get_knight_move(const Square square) { return KNIGHT_MOVE_SQUARES[square]; }

// Bishop moving logic.
static inline BitBoard get_bishop_move(const Square square, const BitBoard occ) {
	return bishop_pext_atk[bishop_pext_offset[square] + static_cast<uint32_t>(pext(occ, bishop_mask_table[square]))];
}

// Rook move logic.
static inline BitBoard get_rook_move(const Square square, const BitBoard occ) {
	return rook_pext_atk[rook_pext_offset[square] + static_cast<uint32_t>(pext(occ, rook_mask_table[square]))];
}

// Queen move logic.
static inline BitBoard get_queen_move(const Square square, const BitBoard occ) {
	return get_bishop_move(square, occ) | get_rook_move(square, occ);
}

template <bool white>
static inline BitBoard get_pawn_left(const BitBoard piece_board) {
	if constexpr (white)
		return (piece_board << 9);
	else
		return (piece_board >> 7);
}

template <bool white>
static inline BitBoard get_pawn_right(const BitBoard piece_board) {
	if constexpr (white)
		return (piece_board << 7);
	else
		return (piece_board >> 9);
}

// Get pawn diagonals, without trimming any form of illegal moves or edge cases.
template <bool white>
static inline BitBoard get_pawn_diags(const BitBoard piece_board) {
	if constexpr (white)
		return (piece_board << 9) | (piece_board << 7);
	else
		return (piece_board >> 9) | (piece_board >> 7);
}

// Only the attack squares. Used to check if king is checked by pawn.
template <bool white>
static inline BitBoard get_pawn_attacks(const BitBoard piece_board) {
	Square s = lbit(piece_board);
	if constexpr (white)
		return (get_pawn_diags<white>(piece_board)) & pawn_edge_masks[0][s];
	else
		return (get_pawn_diags<white>(piece_board)) & pawn_edge_masks[1][s];
}

// Get pawn forward.
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

// Calls the requested pawn move type function.
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

constexpr static std::array<std::array<uint64_t, 64>, 64> create_betweens() {
	std::array<std::array<uint64_t, 64>, 64> ret{};
	for (int s1 = 0; s1 < 64; s1++) {
		for (int s2 = 0; s2 < 64; s2++) {
			if (s1 == s2) continue;
			BitBoard rs1 = get_rook_move(s1, square_to_mask(s2));
			BitBoard rs2 = get_rook_move(s2, square_to_mask(s1));
			BitBoard ds1 = get_bishop_move(s1, square_to_mask(s2));
			BitBoard ds2 = get_bishop_move(s2, square_to_mask(s1));

			if (rs1 & square_to_mask(s2)) {
				BitBoard orth = rs1 & rs2;
				ret[s1][s2] |= orth | square_to_mask(s2);
			} else if (ds1 & square_to_mask(s2)) {
				BitBoard diag = ds1 & ds2;
				ret[s1][s2] |= diag | square_to_mask(s2);
			} else {
				continue;
			}
		}
	}
	return ret;
}

// Returns the reach of a given piece. For pawns, it returns the reach without double push.
template <bool white, Piece piece>
static inline BitBoard make_reach_board(Square square, Board& b) {
	switch (piece) {
	case PAWN:
		return get_pawn_move<white, PawnMoveType::NON_DOUBLE>(square, occb);
	case KING:
		return get_king_move(square);
	case ROOK:
	case QUEEN_ORTH:
		return get_rook_move(square, occb);
	case BISHOP:
	case QUEEN_DIAG:
		return get_bishop_move(square, occb);
	case KNIGHT:
		return get_knight_move(square);
	case QUEEN:
		return get_queen_move(square, occb);
	}
}

const static std::array<std::array<uint64_t, 64>, 64> between_squares = create_betweens();

#endif
