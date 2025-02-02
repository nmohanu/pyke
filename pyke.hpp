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
			Square to = pop(cmt);
			// Capture moves.
			const Piece captured = pos.board.get_piece<!white>(to);
			BitBoard move = square_to_mask(from) | square_to_mask(to);
			BitBoard to_mask = square_to_mask(to);

			capture_move_wrapper<white, p>(pos.board, captured, move, to_mask);
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
			unmake_capture_wrapper<white, p>(pos.board, captured, move, to_mask);

			if constexpr (print_move) print_movecnt(from, to, loc_ret);
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
			Square to = pop(cmt);
			BitBoard move = square_to_mask(from) | square_to_mask(to);

			plain_move<white, p>(pos.board, move);
			loc_ret += count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_plain_move<white, p>(pos.board, move);

			if constexpr (print_move) print_movecnt(from, to, loc_ret);
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
		pos.set_ksq<white>(to);
		uint64_t ret = count_moves<!white, dtg - 1, false, white ? rm_cr_w(cr) : rm_cr_b(cr)>(pos);
		unmake_castle_move<white, code>(pos.board);
		pos.set_ksq<white>(ksq);
		if constexpr (print_move) print_movecnt(ksq, to, ret);
		return ret;
	}
}

// Create king moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_king_moves(BitBoard cmt, Position& pos) {
	Square ksq = pos.get_ksq<white>();
	cmt &= get_king_move(ksq);
	BitBoard captures = cmt & pos.board.get_player_occ<!white>();
	BitBoard non_captures = cmt & ~captures;
	uint64_t ret = 0;
	while (non_captures) {
		Square to = pop(non_captures);
		uint64_t loc_ret = 0;
		BitBoard move = square_to_mask(ksq) | square_to_mask(to);

		plain_move<white, KING>(pos.board, move);
		pos.set_ksq<white>(to);
		if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
		unmake_plain_move<white, KING>(pos.board, move);

		if constexpr (print_move) print_movecnt(ksq, to, loc_ret);
		ret += loc_ret;
	}

	while (captures) {
		Square to = pop(captures);
		uint64_t loc_ret = 0;
		Piece captured = pos.board.get_piece<!white>(to);
		BitBoard move = square_to_mask(ksq) | square_to_mask(to);
		BitBoard to_mask = square_to_mask(to);

		capture_move_wrapper<white, KING>(pos.board, captured, move, to_mask);
		pos.set_ksq<white>(to);
		if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
		unmake_capture_wrapper<white, KING>(pos.board, captured, move, to_mask);

		if constexpr (print_move) print_movecnt(ksq, to, loc_ret);
		ret += loc_ret;
	}

	pos.set_ksq<white>(ksq);

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
	if (capture_sq & msk.pin_dg) return 0;

	ep_move<white>(pos.board, move, capture_sq);

	if (square_to_mask(epsq.first) & msk.nopin)
		loc_ret = count_moves<!white, dtg - 1, false, cr>(pos);
	else
		loc_ret = !pos.is_attacked<white>(pos.get_ksq<white>()) ? count_moves<!white, dtg - 1, false, cr>(pos) : 0;

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
static inline NodeCount generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
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
static inline NodeCount generate_pawn_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
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
			acc += count_plain<white, PAWN, dtg, print_move, cr>(non_captures, lbit(from), pos);
			acc += count_captures<white, PAWN, dtg, print_move, cr>(captures, lbit(from), pos);
		}
	}
	return acc;
}

// Wrapper.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline NodeCount generate_pawn(Position& pos, MaskSet& msk) {
	// Extract pawns on promo rank, they are handled seperately.
	BitBoard cmf = pos.piece_brd<white, PAWN>(), on_promo = cmf & promo_rank(white), not_promo = cmf & ~on_promo;
	// Count pinned + unpinned.
	return generate_pawn_moves<white, dtg, print_move, cr>(msk.cmt, not_promo & msk.nopin, pos)
		+ generate_pawn_moves<white, dtg, print_move, cr>(msk.cmt & ~msk.nopin, not_promo & ~msk.nopin, pos);
}

/*
 *	SLIDERS
 */

// Slider moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline NodeCount generate_sliders(Position& pos, MaskSet& msk) {
	// Sources.
	BitBoard b = pos.piece_brd<white, BISHOP>(), r = pos.piece_brd<white, ROOK>(), q = pos.piece_brd<white, QUEEN>();
	// Unpinned.
	BitBoard unp_b = b & msk.nopin, unp_r = r & msk.nopin, unp_q = q & msk.nopin;
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
static inline NodeCount generate_knight(Position& pos, MaskSet& msk) {
	return generate_moves<white, KNIGHT, dtg, print_move, cr>(msk.cmt, pos.piece_brd<white, KNIGHT>() & msk.nopin, pos);
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
		MaskSet& msk = create_masks<white>(pos.board, pos.get_ksq<white>(), pos.masks.go_next());

		// King moves can always be generated.
		uint64_t ret = generate_king_moves<white, dtg, print_move, cr>(msk.cmt, pos);

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
