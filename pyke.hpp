#include <cassert>
#include <cerrno>
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

template <bool white, int dtg, bool print_move, CastlingRights cr, bool ep>
uint64_t count_moves(Position& pos);

/*
 *	ROOK
 */

// Count rook moves.
template <bool white, Piece p, bool capture, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_rook_moves(BitBoard cmt, Square from, Position& pos) {
	if (!cmt) return 0;
	uint64_t ret = 0;
	bool rm_ks = from == rook_start_squares[!white];
	bool rm_qs = from == rook_start_squares[!white + 1];

	while (cmt) {
		uint64_t loc_ret = 0;
		Square to = pop(cmt);
		BitBoard move = square_to_mask(from) | square_to_mask(to);
		if constexpr (capture) {
			// Capture moves.
			const Piece captured = pos.board.get_piece<!white>(to);
			BitBoard to_mask = square_to_mask(to);

			capture_move_wrapper<white, p>(pos.board, captured, move, to_mask);
			loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<white, true>(cr)>(pos)
				: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<white, false>(cr)>(pos)
							 : count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_capture_wrapper<white, p>(pos.board, captured, move, to_mask);
		} else {
			plain_move<white, p>(pos.board, move);
			loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<white, true>(cr)>(pos)
				: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<white, false>(cr)>(pos)
							 : count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_plain_move<white, p>(pos.board, move);
		}
		if constexpr (print_move) print_movecnt(from, to, loc_ret);
		ret += loc_ret;
	}
	return ret;
}

/*
 *	GENERAL
 */

template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t count_captures(BitBoard cmt, Square from, Position& pos) {
	if (!cmt) return 0;
	if constexpr (p == ROOK) {
		return generate_rook_moves<white, ROOK, true, dtg, print_move, cr>(cmt, from, pos);
	} else {
		uint64_t ret = 0;
		while (cmt) {
			uint64_t loc_ret = 0;
			BitBoard to = popextr(cmt);
			// Capture moves.
			const Piece captured = pos.board.get_piece<!white>(lbit(to));
			BitBoard move = square_to_mask(from) | to;

			capture_move_wrapper<white, p>(pos.board, captured, move, to);
			if (captured == ROOK) {
				constexpr int rook_sq_index = 2 * white;
				const bool rm_ks = to == rook_start_squares[rook_sq_index];
				const bool rm_qs = to == rook_start_squares[rook_sq_index + 1];
				loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<!white, true>(cr)>(pos)
					: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<!white, false>(cr)>(pos)
								 : count_moves<!white, dtg - 1, false, cr>(pos);
			} else {
				loc_ret += count_moves<!white, dtg - 1, false, cr>(pos);
			}
			unmake_capture_wrapper<white, p>(pos.board, captured, move, to);

			if constexpr (print_move) print_movecnt(from, lbit(to), loc_ret);
			ret += loc_ret;
		}
		return ret;
	}
}

template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t count_plain(BitBoard cmt, Square from, Position& pos) {
	if (!cmt) return 0;
	if constexpr (p == ROOK) {
		return generate_rook_moves<white, ROOK, false, dtg, print_move, cr>(cmt, from, pos);
	} else {
		uint64_t ret = 0;
		while (cmt) {
			uint64_t loc_ret = 0;
			BitBoard to = popextr(cmt);
			BitBoard move = square_to_mask(from) | to;

			plain_move<white, p>(pos.board, move);
			loc_ret += count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_plain_move<white, p>(pos.board, move);

			if constexpr (print_move) print_movecnt(from, lbit(to), loc_ret);
			ret += loc_ret;
		}
		return ret;
	}
}

// Splits the reachable squares into non-captures and captures and calls the appropriate function.
template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr, MoveType mt = p>
static inline uint64_t generate_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	uint64_t ret = 0;
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(cmt & make_reach_board<white, p>(from, pos.board));
		} else {
			BitBoard piece_moves_to = cmt & make_reach_board<white, mt>(from, pos.board);
			BitBoard captures = piece_moves_to & pos.board.occ_board;
			BitBoard non_captures = piece_moves_to & ~captures;

			// Non-captures + captured.
			ret += count_plain<white, p, dtg, print_move, cr>(non_captures, from, pos);
			ret += count_captures<white, p, dtg, print_move, cr>(captures, from, pos);
		}
	}
	return ret;
}

/*
 *	KING
 */

// Create the castling move for given player and direction.
template <bool white, bool kingside, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_castle_move(Position& pos) {
	Board& b = pos.board;
	constexpr Square to = white ? (kingside ? 62 : 58) : (kingside ? 6 : 2);
	constexpr Square middle_square = white ? (kingside ? 61 : 59) : (kingside ? 5 : 3);
	constexpr Square ksq = white ? 60 : 4;

	if (b.square_occ(to) | b.square_occ(middle_square)) {
		return 0;
	} else if (!kingside && b.square_occ(queenside_middle_squares[white])) {
		return 0;
	} else if (pos.is_attacked<white>(middle_square) || pos.is_attacked<white>(to)) {
		return 0;
	} else if constexpr (dtg <= 1 && !print_move) {
		return 1;
	} else {
		constexpr uint8_t code = white ? (kingside ? 0 : 1) : (kingside ? 2 : 3);
		castle_move<white, code>(pos.board);
		uint64_t ret = count_moves<!white, dtg - 1, false, white ? rm_cr_w(cr) : rm_cr_b(cr)>(pos);
		unmake_castle_move<white, code>(pos.board);
		if constexpr (print_move) print_movecnt(ksq, to, ret);
		return ret;
	}
}

// Create king moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_king_moves(BitBoard cmt, Position& pos) {
	BitBoard ksq_mask = pos.board.get_piece_board<white, KING>();
	cmt &= get_king_move(lbit(ksq_mask));
	BitBoard captures = cmt & pos.board.get_player_occ<!white>();
	BitBoard non_captures = cmt & ~captures;
	uint64_t ret = 0;
	while (non_captures) {
		BitBoard to = popextr(non_captures);
		uint64_t loc_ret = 0;
		BitBoard move = ksq_mask | to;

		plain_move<white, KING>(pos.board, move);
		if (!pos.is_attacked<white>(lbit(to))) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
		unmake_plain_move<white, KING>(pos.board, move);

		if constexpr (print_move) print_movecnt(lbit(ksq_mask), lbit(to), loc_ret);
		ret += loc_ret;
	}

	while (captures) {
		BitBoard to = popextr(captures);
		uint64_t loc_ret = 0;
		Piece captured = pos.board.get_piece<!white>(lbit(to));
		BitBoard move = ksq_mask | to;

		capture_move_wrapper<white, KING>(pos.board, captured, move, to);
		if (!pos.is_attacked<white>(lbit(to))) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
		unmake_capture_wrapper<white, KING>(pos.board, captured, move, to);

		if constexpr (print_move) print_movecnt(lbit(ksq_mask), lbit(to), loc_ret);
		ret += loc_ret;
	}

	return ret;
}

/*
 *	PAWNS
 */

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

// Make ep move and ocunt. Offset is whether ep comes from left or right.
template <bool white, int offset, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t make_en_passant(Position& pos, uint8_t ep, MaskSet& msk) {
	uint64_t loc_ret;
	sq_pair epsq = get_ep_squares<white, offset>(ep);
	BitBoard move = square_to_mask(epsq.first) | square_to_mask(epsq.second);
	BitBoard capture_sq = square_to_mask(white ? epsq.second + 8 : epsq.second - 8);
	if (capture_sq & msk.pinmask_dg) return 0;

	ep_move<white>(pos.board, move, capture_sq);

	if (square_to_mask(epsq.first) & msk.unpinned)
		loc_ret = count_moves<!white, dtg - 1, false, cr>(pos);
	else
		loc_ret = !pos.is_attacked<white>(lbit(pos.board.get_piece_board<white, KING>()))
			? count_moves<!white, dtg - 1, false, cr>(pos)
			: 0;

	unmake_ep_move<white>(pos.board, move, capture_sq);

	if constexpr (print_move) print_movecnt(epsq.first, epsq.second, loc_ret);
	return loc_ret;
};

// Count nodes following from ep moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_ep_moves(Position& pos, uint8_t ep, MaskSet& msk) {
	return (ep & 0x80 ? make_en_passant<white, -1, dtg, print_move, cr>(pos, ep, msk) : 0)
		+ (ep & 0x40 ? make_en_passant<white, 1, dtg, print_move, cr>(pos, ep, msk) : 0);
}

// Pawn double pushes.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
	if constexpr (dtg <= 1 && !print_move) {
		return popcnt(get_pawn_double<white>(source, pos.board.occ_board) & cmt);
	} else {
		uint64_t ret = 0;
		while (source) {
			BitBoard from = popextr(source);
			BitBoard to_board = cmt & get_pawn_double<white>(from, pos.board.occ_board);
			while (to_board) {
				BitBoard to = popextr(to_board);
				BitBoard move = from | to;
				bool ep = pawn_double<white>(pos.board, move, to, pos.ep_flag);
				uint64_t loc_ret = ep ? count_moves<!white, dtg - 1, false, cr, true>(pos)
									  : count_moves<!white, dtg - 1, false, cr, false>(pos);
				unmake_pawn_double<white>(pos.board, move);
				if constexpr (print_move) print_movecnt(lbit(from), lbit(to), loc_ret);
				ret += loc_ret;
			}
		}
		return ret;
	}
}

// Concrete generator for pawn moves. Creates double pushes, pushes and captures, in bulk if possible.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_pawn_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	BitBoard occ = pos.board.occ_board;
	BitBoard cmt_free = cmt & ~occ;
	BitBoard cmt_captures = cmt & occ;
	BitBoard pawns_on_start = pieces & (white ? pawn_start_w : pawn_start_b);
	uint64_t ret = generate_pawn_double<white, dtg, print_move, cr>(cmt, pos, pawns_on_start);

	// Generate moves in bulk.
	if constexpr (dtg <= 1 && !print_move) {
		ret += popcnt(get_pawn_forward<white>(pieces) & cmt_free);
		ret += popcnt(get_pawn_left<white>(can_capture_left(pieces)) & cmt_captures);
		ret += popcnt(get_pawn_right<white>(can_capture_right(pieces)) & cmt_captures);
	} else {
		while (pieces) {
			Square from = pop(pieces);
			BitBoard captures = get_pawn_move<white, PawnMoveType::ATTACKS>(from, occ) & cmt_captures;
			BitBoard non_captures = get_pawn_move<white, PawnMoveType::FORWARD>(from, occ) & cmt_free;
			ret += count_plain<white, PAWN, dtg, print_move, cr>(non_captures, from, pos);
			ret += count_captures<white, PAWN, dtg, print_move, cr>(captures, from, pos);
		}
	}
	return ret;
}

// Wrapper.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_pawn(Position& pos, MaskSet& msk) {
	BitBoard can_move_from = pos.board.get_piece_board<white, PAWN>();
	BitBoard pawns_on_promo = can_move_from & (white ? promotion_from_w : promotion_from_b);
	can_move_from &= ~pawns_on_promo;
	BitBoard pinned = can_move_from & ~msk.unpinned;
	BitBoard unpinned = can_move_from & msk.unpinned;

	// Unpinned + pinned.
	return generate_pawn_moves<white, dtg, print_move, cr>(msk.can_move_to, unpinned, pos)
		+ generate_pawn_moves<white, dtg, print_move, cr>(msk.can_move_to & ~msk.unpinned, pinned, pos);
}

/*
 *	SLIDERS
 */

template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_sliders(Position& pos, MaskSet& msk) {
	BitBoard bishops = pos.board.get_piece_board<white, BISHOP>();
	BitBoard rooks = pos.board.get_piece_board<white, ROOK>();
	BitBoard queens = pos.board.get_piece_board<white, QUEEN>();
	BitBoard unp_b = bishops & msk.unpinned;
	BitBoard unp_r = rooks & msk.unpinned;
	BitBoard unp_q = queens & msk.unpinned;
	BitBoard pin_cmt_diag = msk.can_move_to & msk.pinmask_dg;
	BitBoard pin_cmt_orth = msk.can_move_to & msk.pinmask_orth;
	BitBoard dg_not_orth = msk.pinmask_dg & ~msk.pinmask_orth;
	BitBoard orth_not_dg = msk.pinmask_orth & ~msk.pinmask_dg;
	BitBoard pin_b = bishops & dg_not_orth;
	BitBoard pin_q_diag = queens & dg_not_orth;
	BitBoard pin_r = rooks & orth_not_dg;
	BitBoard pin_q_orth = queens & orth_not_dg;

	// Pinned + unpinned.
	return generate_moves<white, BISHOP, dtg, print_move, cr>(pin_cmt_diag, pin_b, pos)
		+ generate_moves<white, BISHOP, dtg, print_move, cr>(msk.can_move_to, unp_b, pos)
		+ generate_moves<white, QUEEN_DIAG, dtg, print_move, cr>(pin_cmt_diag, pin_q_diag, pos)
		+ generate_moves<white, QUEEN, dtg, print_move, cr>(msk.can_move_to, unp_q, pos)
		+ generate_moves<white, QUEEN_ORTH, dtg, print_move, cr>(pin_cmt_orth, pin_q_orth, pos)
		+ generate_moves<white, ROOK, dtg, print_move, cr>(pin_cmt_orth, pin_r, pos)
		+ generate_moves<white, ROOK, dtg, print_move, cr>(msk.can_move_to, unp_r, pos);
}

/*
 *	KNIGHTS
 */

// Count knight moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_knight(Position& pos, MaskSet& msk) {
	BitBoard cmf = pos.board.get_piece_board<white, KNIGHT>() & msk.unpinned;
	return generate_moves<white, KNIGHT, dtg, print_move, cr>(msk.can_move_to, cmf, pos);
}

/*
 *	MAIN
 */

// Main counting function.
template <bool white, int dtg, bool print_move, CastlingRights cr, bool ep = false>
uint64_t count_moves(Position& pos) {
	if constexpr (dtg < 1)
		return 1;
	else {
		uint8_t ep_flag = ep ? pos.ep_flag : 0;

		// Make masks.
		MaskSet& msk = create_masks<white>(pos.board, pos.masks.go_next());

		// King moves can always be generated.
		uint64_t ret = generate_king_moves<white, dtg, print_move, cr>(msk.can_move_to, pos);

		// If double check, only king can move. Else, limit the target squares to the checkmask and skip castling.
		switch (msk.checkers) {
		case 0:
			ret += generate_castle_move<white, true, dtg, print_move, cr>(pos);
			ret += generate_castle_move<white, false, dtg, print_move, cr>(pos);
			break;
		case 1:
			msk.can_move_to &= msk.check_mask;
			break;
		default:
			return ret;
		}

		// Generate moves.
		ret += generate_sliders<white, dtg, print_move, cr>(pos, msk);
		ret += generate_pawn<white, dtg, print_move, cr>(pos, msk);
		ret += generate_knight<white, dtg, print_move, cr>(pos, msk);
		if constexpr (ep) ret += generate_ep_moves<white, dtg, print_move, cr>(pos, ep_flag, msk);
		pos.masks.point_prev();
		return ret;
	}
}

};	// namespace pyke

#endif
