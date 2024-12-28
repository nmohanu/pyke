#include <cassert>
#include <cstdint>

#include "gamestate.hpp"
#include "make_move.hpp"
#include "maskset.hpp"
#include "move.hpp"
#include "piece_moves.hpp"
#include "position.hpp"
#include "stack.hpp"
#include "util.hpp"

#ifndef PYKE_H
#define PYKE_H

namespace pyke {

template <bool white, int dtg, bool print_move>
uint64_t count_moves(Position& pos);

// Make ep move and ocunt. Offset is whether ep comes from left or right.
template <bool white, int offset, int dtg, bool print_move>
static inline uint64_t make_en_passant(Position& pos, Square ksq) {
	sq_pair epsq = pos.gamestate.get_ep_squares<white, offset>();

	ep_move<white>(epsq.first, epsq.second, pos);
	uint64_t loc_ret = !pos.is_attacked<white>(ksq) ? count_moves<!white, dtg - 1, false>(pos) : 0;
	unmake_ep_move<white>(epsq.first, epsq.second, pos);

	if (loc_ret && print_move) print_movecnt(epsq.first, epsq.second, loc_ret);
	return loc_ret;
};

// Count nodes following from ep moves.
template <bool white, int dtg, bool print_move>
static inline uint64_t generate_ep_moves(Position& pos, Square ksq) {
	uint8_t ep = pos.gamestate.get_en_passant();
	return (ep & 0x80 ? make_en_passant<white, -1, dtg, print_move>(pos, ksq) : 0)
		+ (ep & 0x40 ? make_en_passant<white, 1, dtg, print_move>(pos, ksq) : 0);
}

/*
// Create the castling move for given player and direction.
template <bool white, bool kingside, int dtg, bool print_move>
static inline uint64_t generate_castle_move(Position& pos, Square king_square) {
	Board& b = pos.board;

	bool has_right = white ? pos.gamestate.can_castle_king<white>() : pos.gamestate.can_castle_queen<white>();
	Square to = white ? (kingside ? 62 : 58) : (kingside ? 6 : 2);
	Square middle_square = white ? (kingside ? 61 : 59) : (kingside ? 5 : 3);

	bool middle_attacked = pos.is_attacked<white>(middle_square);
	bool to_attacked = pos.is_attacked<white>(to);

	bool to_occ = b.square_occ(to);
	bool middle_occ = b.square_occ(middle_square);

	// Check if the castling move is legal.
	if (!has_right || middle_attacked || to_attacked || to_occ || middle_occ) return 0;
	if constexpr (dtg <= 1 && !print_move)
		return 1;
	else {
		const uint8_t code = white ? (kingside ? 0 : 1) : (kingside ? 2 : 3);
		castle_move<white, code>(pos);
		int ret = 1 + count_moves<!white, dtg - 1, false>(pos);
		unmake_castle_move<white, code>(pos);
		return ret;
	}
}
*/

// Create king moves.
template <bool white, int dtg, bool print_move>
static inline uint64_t generate_king_moves(BitBoard cmt, Square king_square, Position& pos) {
	cmt &= piece_move::get_king_move(king_square);
	uint64_t ret = 0;
	while (cmt) {
		Square to = pop(cmt);
		uint64_t loc_ret = 0;
		if (pos.board.square_occ(to)) {
			Piece captured = pos.board.get_piece<white>(to);
			capture_move_wrapper<white, KING>(king_square, to, pos, captured);
			if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false>(pos);
			unmake_capture_wrapper<white, KING>(king_square, to, pos, captured);
		} else {
			plain_move<white, KING>(king_square, to, pos);
			if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false>(pos);
			unmake_plain_move<white, KING>(king_square, to, pos);
		}
		if (print_move && loc_ret) print_movecnt(king_square, to, loc_ret);
		ret += loc_ret;
	}
	return ret;
}

// Pawn double pushes.
template <bool white, int dtg, bool print_move>
static inline uint64_t generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
	if constexpr (dtg <= 1 && !print_move) {
		return popcnt(piece_move::get_pawn_double<white>(source, pos.board.occ_board) & cmt);
	} else {
		uint64_t ret = 0;
		while (source) {
			Square from = pop(source);
			BitBoard to_board = cmt & piece_move::get_pawn_double<white>(square_to_mask(from), pos.board.occ_board);
			while (to_board) {
				Square to = pop(to_board);
				pawn_double<white>(from, to, pos);
				uint64_t loc_ret = count_moves<!white, dtg - 1, false>(pos);
				unmake_pawn_double<white>(from, to, pos);
				if constexpr (print_move) print_movecnt(from, to, loc_ret);
				ret += loc_ret;
			}
		}
		return ret;
	}
}

// Generate all possible promotion moves.
template <bool white, int dtg, bool print_move>
static inline uint64_t generate_promotions(Position& pos, BitBoard cmt, BitBoard source) {
	return 0;
	/*all         Board::is_equal(Board const&
	const auto generate_promo_pieces = [&](const Piece piece, const Piece captured, Square from, Square to) {
		promo_move<white, piece, captured>(from, to, pos);
		int count = 1 + count_moves<!white, dtg - 1>(pos);
		unmake_promo_move<white, piece, captured>(from, to, pos);
		return count;
	};
	int ret = 0;
	while (source) {
		Square from = pop(source);
		while (cmt) {
			Square to = pop(cmt);
			if constexpr (dtg <= 1) {
				return 4;
			} else {
				const Piece at_to = pos.board.get_piece<white>(to);
				ret += generate_promo_pieces(ROOK, at_to, from, to);
				ret += generate_promo_pieces(KNIGHT, at_to, from, to);
				ret += generate_promo_pieces(BISHOP, at_to, from, to);
				ret += generate_promo_pieces(QUEEN, at_to, from, to);
			}
		}
	}
	return ret;
*/
}

// Push moves from a given position to the target squares.
template <bool white, Piece p, bool capture, int dtg, bool print_move>
static inline uint64_t generate_move_or_capture(BitBoard cmt, Square from, Position& pos) {
	if (!cmt) return 0;
	uint64_t ret = 0;
	while (cmt) {
		uint64_t loc_ret = 0;
		Square to = pop(cmt);
		if constexpr (capture) {
			// Capture moves.
			const Piece captured = pos.board.get_piece<!white>(to);
			capture_move_wrapper<white, p>(from, to, pos, captured);
			loc_ret += count_moves<!white, dtg - 1, false>(pos);
			unmake_capture_wrapper<white, p>(from, to, pos, captured);
		} else {
			plain_move<white, p>(from, to, pos);
			loc_ret += count_moves<!white, dtg - 1, false>(pos);
			unmake_plain_move<white, p>(from, to, pos);
		}
		if constexpr (print_move) print_movecnt(from, to, loc_ret);
		ret += loc_ret;
	}
	return ret;
}

template <bool white, int dtg, bool print_move>
static inline uint64_t generate_pawn_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	BitBoard occ = pos.board.occ_board;
	BitBoard cmt_free = cmt & ~occ;
	BitBoard cmt_captures = cmt & occ;
	BitBoard pawns_on_start = pieces & (white ? pawn_start_w : pawn_start_b);
	uint64_t ret = generate_pawn_double<white, dtg, print_move>(cmt, pos, pawns_on_start);

	// Generate moves in bulk.
	if constexpr (dtg <= 1 && !print_move) ret += popcnt(piece_move::get_pawn_forward<white>(pieces) & cmt_free);

	while (pieces) {
		Square from = pop(pieces);
		BitBoard captures = piece_move::get_pawn_move<white, PawnMoveType::ATTACKS>(from, occ) & cmt_captures;
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(captures);
		} else {
			BitBoard non_captures = piece_move::get_pawn_move<white, PawnMoveType::FORWARD>(from, occ) & cmt_free;
			ret += generate_move_or_capture<white, PAWN, false, dtg, print_move>(non_captures, from, pos);
			ret += generate_move_or_capture<white, PAWN, true, dtg, print_move>(captures, from, pos);
		}
	}
	return ret;
}

// Create moves given a from and to board..
template <bool white, Piece p, int dtg, bool print_move>
static inline uint64_t generate_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	uint64_t ret = 0;
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(cmt & make_reach_board<white, p>(from, pos.board));
		} else {
			BitBoard captures;
			BitBoard non_captures;
			BitBoard piece_moves_to = cmt & make_reach_board<white, p>(from, pos.board);
			captures = piece_moves_to & (white ? pos.board.b_board : pos.board.w_board);
			non_captures = piece_moves_to & ~captures;

			// Non-captures.
			ret += generate_move_or_capture<white, p, false, dtg, print_move>(non_captures, from, pos);
			// Captures.
			ret += generate_move_or_capture<white, p, true, dtg, print_move>(captures, from, pos);
		}
	}
	return ret;
}

// For a given piece, generate all possible normal/capture moves.
template <bool white, Piece p, int dtg, bool print_move>
static inline uint64_t generate_any(Position& pos) {
	Board& b = pos.board;
	uint64_t ret = 0;
	BitBoard can_move_from = b.get_piece_board<white, p>();

	if constexpr (p == PAWN) {
		BitBoard pawns_on_promo = can_move_from & (white ? promotion_from_w : promotion_from_b);
		can_move_from &= ~pawns_on_promo;
	}
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = can_move_from & pos.msk->pinmask_dg;
	BitBoard pinned_orth = can_move_from & pos.msk->pinmask_orth;
	BitBoard unpinned = can_move_from & ~(pinned_dg | pinned_orth);

	if constexpr (p == PAWN) {
		ret += generate_pawn_moves<white, dtg, print_move>(pos.msk->can_move_to, unpinned, pos);
		ret += generate_pawn_moves<white, dtg, print_move>(pos.msk->can_move_to & pos.msk->pinmask_dg, pinned_dg, pos);
		ret +=
			generate_pawn_moves<white, dtg, print_move>(pos.msk->can_move_to & pos.msk->pinmask_orth, pinned_orth, pos);
	} else {
		// Unpinned.
		ret += generate_moves<white, p, dtg, print_move>(pos.msk->can_move_to, unpinned, pos);
		// Pinned diagonally.
		ret += generate_moves<white, p, dtg, print_move>(pos.msk->can_move_to & pos.msk->pinmask_dg, pinned_dg, pos);
		// Pinned orthogonally.
		ret +=
			generate_moves<white, p, dtg, print_move>(pos.msk->can_move_to & pos.msk->pinmask_orth, pinned_orth, pos);
	}
	return ret;
}

// Create move list for given position.
template <bool white, int dtg, bool print_move>
uint64_t count_moves(Position& pos) {
	if constexpr (dtg < 1)
		return 1;
	else {
		// Make masks.
		Square king_square = __builtin_clzll(pos.board.get_piece_board<white, KING>());
		pos.msk = new MaskSet;
		pos.msk->create_masks<white>(pos.board, king_square);

		uint64_t ret = generate_king_moves<white, dtg, print_move>(pos.msk->can_move_to, king_square, pos);
		// If double check, only king can move. Else, limit the target squares to the checkmask and skip castling.
		switch (pos.msk->checkers) {
		case 0:
			// ret += generate_castle_move<white, true, dtg, print_move>(pos, king_square);
			// ret += generate_castle_move<white, false, dtg, print_move>occ_board(pos, king_square);
			break;
		case 1:
			pos.msk->can_move_to &= pos.msk->check_mask;
			break;
		default:
			return ret;
		}
		// Generate moves.
		ret += generate_any<white, ROOK, dtg, print_move>(pos);
		ret += generate_any<white, BISHOP, dtg, print_move>(pos);
		ret += generate_any<white, QUEEN, dtg, print_move>(pos);
		ret += generate_any<white, KNIGHT, dtg, print_move>(pos);
		ret += generate_any<white, PAWN, dtg, print_move>(pos);
		ret += generate_ep_moves<white, dtg, print_move>(pos, king_square);
		return ret;
	}
}

};	// namespace pyke

#endif
