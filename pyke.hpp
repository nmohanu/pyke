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
NodeCount count_moves(Position& pos);

/*
 *	GENERAL
 */

// Count plain, non capture moves.
template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr, bool take>
inline NodeCount process_move(BitBoard cmt, BitBoard from, Position& pos) {
	NodeCount acc = 0;
	while (cmt) {
		const BitBoard to = popextr(cmt), move = from | to;
		Piece captured;
		// Do move.
		if constexpr (take) {
			captured = pos.board.get_piece<!white>(to);
			capture_move_wrapper<white, p>(pos.board, captured, move, to);
		} else plain_move<white, p>(pos.board, move);
		// Count.
		const NodeCount loc_ret = count_moves<!white, dtg - 1, false, cr>(pos);
		// Undo move.
		if constexpr (take) unmake_capture_wrapper<white, p>(pos.board, captured, move, to);
		else unmake_plain_move<white, p>(pos.board, move);
		// Output move and count if needed.
		if constexpr (print_move) print_movecnt(lbit(from), lbit(to), loc_ret);
		acc += loc_ret;
	}
	return acc;
}

// Splits the reachable squares into non-captures and captures and calls the appropriate function.
template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr, MoveType mt = p>
inline NodeCount generate_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	NodeCount ret = 0;
	// For all instances of given piece.
	while (pieces) {
		const BitBoard from = popextr(pieces);
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(cmt & make_reach_board<white, p>(lbit(from), pos.board));
		} else {
			const BitBoard piece_cmt = cmt & make_reach_board<white, mt>(lbit(from), pos.board);
			const BitBoard captures = piece_cmt & pos.board.occ_board, non_captures = piece_cmt & ~captures;

			// Non-captures + captured.
			ret += process_move<white, p, dtg, print_move, cr, false>(non_captures, from, pos);
			ret += process_move<white, p, dtg, print_move, cr, true>(captures, from, pos);
		}
	}
	return ret;
}

/*
 *	KING
 */

// Create the castling move for given player and direction.
template <bool white, bool kingside, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_castle_move(Position& pos) {
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
		NodeCount ret = count_moves<!white, dtg - 1, false, white ? rm_cr_w(cr) : rm_cr_b(cr)>(pos);
		unmake_castle_move<white, code>(pos.board);
		if constexpr (print_move) print_movecnt(ksq, to, ret);
		return ret;
	}
}

// Create king moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_king_moves(BitBoard cmt, Position& pos) {
	BitBoard ksq_mask = pos.board.get_piece_board<white, KING>();
	cmt &= get_king_move(lbit(ksq_mask));
	BitBoard captures = cmt & pos.board.get_player_occ<!white>();
	BitBoard non_captures = cmt & ~captures;
	NodeCount ret = 0;
	while (non_captures) {
		BitBoard to = popextr(non_captures);
		NodeCount loc_ret = 0;
		BitBoard move = ksq_mask | to;

		plain_move<white, KING>(pos.board, move);
		if (!pos.is_attacked<white>(lbit(to))) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
		unmake_plain_move<white, KING>(pos.board, move);

		if constexpr (print_move) print_movecnt(lbit(ksq_mask), lbit(to), loc_ret);
		ret += loc_ret;
	}

	while (captures) {
		BitBoard to = popextr(captures);
		NodeCount loc_ret = 0;
		Piece captured = pos.board.get_piece<!white>(to);
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
inline NodeCount generate_promotions(Position& pos, BitBoard cmt, BitBoard source) {
	// TODO...
	return 0;
}

// Make ep move and ocunt. Offset is whether ep comes from left or right.
template <bool white, int offset, int dtg, bool print_move, CastlingRights cr>
inline NodeCount make_en_passant(Position& pos, uint8_t ep, MaskSet& msk) {
	NodeCount loc_ret;
	sq_pair epsq = get_ep_squares<white, offset>(ep);
	BitBoard move = square_to_mask(epsq.first) | square_to_mask(epsq.second);
	BitBoard capture_sq = square_to_mask(white ? epsq.second + 8 : epsq.second - 8);
	if (capture_sq & msk.pin_dg) return 0;

	ep_move<white>(pos.board, move, capture_sq);

	if (square_to_mask(epsq.first) & msk.npin) loc_ret = count_moves<!white, dtg - 1, false, cr>(pos);
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
inline NodeCount generate_ep_moves(Position& pos, uint8_t ep, MaskSet& msk) {
	return (ep & 0x80 ? make_en_passant<white, -1, dtg, print_move, cr>(pos, ep, msk) : 0)
		+ (ep & 0x40 ? make_en_passant<white, 1, dtg, print_move, cr>(pos, ep, msk) : 0);
}

// Pawn double pushes.
template <bool white, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
	if constexpr (dtg <= 1 && !print_move) {
		return popcnt(get_pawn_double<white>(source, pos.board.occ_board) & cmt);
	} else {
		NodeCount ret = 0;
		while (source) {
			BitBoard from = popextr(source);
			BitBoard to_board = cmt & get_pawn_double<white>(from, pos.board.occ_board);
			while (to_board) {
				BitBoard to = popextr(to_board);
				BitBoard move = from | to;
				bool ep = pawn_double<white>(pos.board, move, to, pos.ep_flag);
				NodeCount loc_ret = ep ? count_moves<!white, dtg - 1, false, cr, true>(pos)
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
inline NodeCount generate_pawn_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	// Isolate free and occupied squares.
	BitBoard occ = pos.board.occ_board, cmt_free = cmt & ~occ, cmt_captures = cmt & occ;
	BitBoard pawns_on_start = pieces & start_rank(white);
	NodeCount acc = generate_pawn_double<white, dtg, print_move, cr>(cmt, pos, pawns_on_start);

	// Generate moves in bulk.
	if constexpr (dtg <= 1 && !print_move) {
		acc += popcnt(get_pawn_forward<white>(pieces) & cmt_free);
		acc += popcnt(get_pawn_left<white>(ccl(pieces)) & cmt_captures);
		acc += popcnt(get_pawn_right<white>(ccr(pieces)) & cmt_captures);
	} else {
		while (pieces) {
			BitBoard from = popextr(pieces);
			BitBoard captures = get_pawn_move<white, PawnMoveType::ATTACKS>(from, occ) & cmt_captures;
			BitBoard non_captures = get_pawn_move<white, PawnMoveType::FORWARD>(from, occ) & cmt_free;
			acc += process_move<white, PAWN, dtg, print_move, cr, false>(non_captures, from, pos);
			acc += process_move<white, PAWN, dtg, print_move, cr, true>(captures, from, pos);
		}
	}
	return acc;
}

// Wrapper. Split pinned and unpinned pawns for non-promo pawns.
template <bool white, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_pawn(Position& pos, MaskSet& msk) {
	// Extract pawns on promo rank, they are handled seperately.
	BitBoard cmf = pos.piecebrd<white, PAWN>(), on_promo = cmf & promo_rank(white), not_promo = cmf & ~on_promo;
	return generate_pawn_moves<white, dtg, print_move, cr>(msk.cmt, not_promo & msk.npin, pos)
		+ generate_pawn_moves<white, dtg, print_move, cr>(msk.cmt & ~msk.npin, not_promo & ~msk.npin, pos);
}

/*
 *	SLIDERS
 */

// Slider moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_sliders(Position& pos, MaskSet& msk) {
	// Sources.
	BitBoard b = pos.piecebrd<white, BISHOP>(), r = pos.piecebrd<white, ROOK>(), q = pos.piecebrd<white, QUEEN>();
	// Unpinned.
	BitBoard unp_b = b & msk.npin, unp_r = r & msk.npin, unp_q = q & msk.npin;
	// Pinned can move to.
	BitBoard pin_cmt_diag = msk.cmt & msk.pin_dg, pin_cmt_orth = msk.cmt & msk.pin_orth;
	// Pinmasks exclusive direction.
	BitBoard dg_only = msk.pin_dg & ~msk.pin_orth, orth_only = msk.pin_orth & ~msk.pin_dg;
	// Isolate pinned pieces.
	BitBoard pin_b = b & dg_only, pin_q_diag = q & dg_only, pin_r = r & orth_only, pin_q_orth = q & orth_only;

	// Pinned + unpinned.
	return generate_moves<white, BISHOP, dtg, print_move, cr>(pin_cmt_diag, pin_b, pos)
		+ generate_moves<white, BISHOP, dtg, print_move, cr>(msk.cmt, unp_b, pos)
		+ generate_moves<white, QUEEN_DIAG, dtg, print_move, cr>(pin_cmt_diag, pin_q_diag, pos)
		+ generate_moves<white, QUEEN, dtg, print_move, cr>(msk.cmt, unp_q, pos)
		+ generate_moves<white, QUEEN_ORTH, dtg, print_move, cr>(pin_cmt_orth, pin_q_orth, pos)
		+ generate_moves<white, ROOK, dtg, print_move, cr>(pin_cmt_orth, pin_r, pos)
		+ generate_moves<white, ROOK, dtg, print_move, cr>(msk.cmt, unp_r, pos);
}

/*
 *	KNIGHTS
 */

// Count knight moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
inline NodeCount generate_knight(Position& pos, MaskSet& msk) {
	return generate_moves<white, KNIGHT, dtg, print_move, cr>(msk.cmt, pos.piecebrd<white, KNIGHT>() & msk.npin, pos);
}

/*
 *	MAIN
 */

// Main counting function.
template <bool white, int dtg, bool print_move, CastlingRights cr, bool ep = false>
NodeCount count_moves(Position& pos) {
	if constexpr (dtg < 1) return 1;
	else {
		uint8_t ep_flag = ep ? pos.ep_flag : 0;

		// Make masks.
		MaskSet& msk = create_masks<white>(pos.board, pos.masks.go_next());

		// King moves can always be generated.
		NodeCount ret = generate_king_moves<white, dtg, print_move, cr>(msk.cmt, pos);

		// If double check, only king can move. Else, limit the target squares to the checkmask and skip castling.
		switch (msk.checkers) {
		case 0:
			ret += generate_castle_move<white, true, dtg, print_move, cr>(pos);
			ret += generate_castle_move<white, false, dtg, print_move, cr>(pos);
			break;
		case 1:
			msk.cmt &= msk.check_mask;
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
