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

// Make ep move and ocunt. Offset is whether ep comes from left or right.
template <bool white, int offset, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t make_en_passant(Position& pos, Square ksq) {
	sq_pair epsq = pos.gamestate.get_ep_squares<white, offset>();

	ep_move<white>(epsq.first, epsq.second, pos);
	uint64_t loc_ret = !pos.is_attacked<white>(ksq) ? count_moves<!white, dtg - 1, false, cr>(pos) : 0;
	unmake_ep_move<white>(epsq.first, epsq.second, pos);

	if (loc_ret && print_move) print_movecnt(epsq.first, epsq.second, loc_ret);
	return loc_ret;
};

// Count nodes following from ep moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_ep_moves(Position& pos, Square ksq) {
	uint8_t ep = pos.gamestate.get_en_passant();
	return (ep & 0x80 ? make_en_passant<white, -1, dtg, print_move, cr>(pos, ksq) : 0)
		+ (ep & 0x40 ? make_en_passant<white, 1, dtg, print_move, cr>(pos, ksq) : 0);
}

// Create the castling move for given player and direction.
template <bool white, bool kingside, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_castle_move(Position& pos, Square king_square) {
	Board& b = pos.board;
	constexpr Square to = white ? (kingside ? 62 : 58) : (kingside ? 6 : 2);
	constexpr Square middle_square = white ? (kingside ? 61 : 59) : (kingside ? 5 : 3);

	if (!has_cr_right<white, kingside, cr>() | b.square_occ(to) | b.square_occ(middle_square)) {
		return 0;
	} else if (!kingside && b.square_occ(queenside_middle_squares[white])) {
		return 0;
	} else if (pos.is_attacked<white>(middle_square) || pos.is_attacked<white>(to)) {
		return 0;
	} else if constexpr (dtg <= 1 && !print_move) {
		return 1;
	} else {
		constexpr uint8_t code = white ? (kingside ? 0 : 1) : (kingside ? 2 : 3);
		castle_move<white, code>(pos);
		uint64_t ret = count_moves<!white, dtg - 1, false, white ? rm_cr_w(cr) : rm_cr_b(cr)>(pos);
		unmake_castle_move<white, code>(pos);
		if (ret && print_move) print_movecnt(king_square, to, ret);
		return ret;
	}
}

// Create king moves.
template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_king_moves(BitBoard cmt, Square king_square, Position& pos) {
	cmt &= piece_move::get_king_move(king_square);
	uint64_t ret = 0;
	while (cmt) {
		Square to = pop(cmt);
		uint64_t loc_ret = 0;
		if (pos.board.square_occ(to)) {
			Piece captured = pos.board.get_piece<!white>(to);
			capture_move_wrapper<white, KING>(king_square, to, pos, captured);
			if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
			unmake_capture_wrapper<white, KING>(king_square, to, pos, captured);
		} else {
			plain_move<white, KING>(king_square, to, pos);
			if (!pos.is_attacked<white>(to)) loc_ret += count_moves<!white, dtg - 1, false, rm_cr<white>(cr)>(pos);
			unmake_plain_move<white, KING>(king_square, to, pos);
		}
		if (print_move && loc_ret) print_movecnt(king_square, to, loc_ret);
		ret += loc_ret;
	}
	return ret;
}

// Pawn double pushes.
template <bool white, int dtg, bool print_move, CastlingRights cr>
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
				bool ep = pawn_double<white>(from, to, pos);
				uint64_t loc_ret = ep ? count_moves<!white, dtg - 1, false, cr, true>(pos)
									  : count_moves<!white, dtg - 1, false, cr, false>(pos);
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
		if constexpr (capture) {
			// Capture moves.
			const Piece captured = pos.board.get_piece<!white>(to);
			capture_move_wrapper<white, p>(from, to, pos, captured);
			loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<white, true>(cr)>(pos)
				: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<white, false>(cr)>(pos)
							 : count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_capture_wrapper<white, p>(from, to, pos, captured);
		} else {
			plain_move<white, p>(from, to, pos);
			loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<white, true>(cr)>(pos)
				: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<white, false>(cr)>(pos)
							 : count_moves<!white, dtg - 1, false, cr>(pos);
			unmake_plain_move<white, p>(from, to, pos);
		}
		if constexpr (print_move) print_movecnt(from, to, loc_ret);
		ret += loc_ret;
	}
	return ret;
}

template <bool white, Piece p, bool capture, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_move_or_capture(BitBoard cmt, Square from, Position& pos) {
	if (!cmt) return 0;

	if constexpr (p == ROOK) {
		return generate_rook_moves<white, ROOK, capture, dtg, print_move, cr>(cmt, from, pos);
	} else {
		uint64_t ret = 0;
		while (cmt) {
			uint64_t loc_ret = 0;
			Square to = pop(cmt);
			if constexpr (capture) {
				// Capture moves.
				const Piece captured = pos.board.get_piece<!white>(to);
				bool rm_ks = to == rook_start_squares[2 * white];
				bool rm_qs = to == rook_start_squares[2 * white + 1];
				capture_move_wrapper<white, p>(from, to, pos, captured);
				loc_ret += rm_ks ? count_moves<!white, dtg - 1, false, rm_cr<!white, true>(cr)>(pos)
					: rm_qs		 ? count_moves<!white, dtg - 1, false, rm_cr<!white, false>(cr)>(pos)
								 : count_moves<!white, dtg - 1, false, cr>(pos);
				unmake_capture_wrapper<white, p>(from, to, pos, captured);
			} else {
				plain_move<white, p>(from, to, pos);
				loc_ret += count_moves<!white, dtg - 1, false, cr>(pos);
				unmake_plain_move<white, p>(from, to, pos);
			}
			if constexpr (print_move) print_movecnt(from, to, loc_ret);
			ret += loc_ret;
		}
		return ret;
	}
}

template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_pawn_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	BitBoard occ = pos.board.occ_board;
	BitBoard cmt_free = cmt & ~occ;
	BitBoard cmt_captures = cmt & occ;
	BitBoard pawns_on_start = pieces & (white ? pawn_start_w : pawn_start_b);
	uint64_t ret = generate_pawn_double<white, dtg, print_move, cr>(cmt, pos, pawns_on_start);

	// Generate moves in bulk.
	if constexpr (dtg <= 1 && !print_move) ret += popcnt(piece_move::get_pawn_forward<white>(pieces) & cmt_free);

	while (pieces) {
		Square from = pop(pieces);
		BitBoard captures = piece_move::get_pawn_move<white, PawnMoveType::ATTACKS>(from, occ) & cmt_captures;
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(captures);
		} else {
			BitBoard non_captures = piece_move::get_pawn_move<white, PawnMoveType::FORWARD>(from, occ) & cmt_free;
			ret += generate_move_or_capture<white, PAWN, false, dtg, print_move, cr>(non_captures, from, pos);
			ret += generate_move_or_capture<white, PAWN, true, dtg, print_move, cr>(captures, from, pos);
		}
	}
	return ret;
}

template <bool white, Piece p, int dtg, bool print_move, CastlingRights cr, MoveType mt = p>
static inline uint64_t generate_moves(BitBoard cmt, BitBoard pieces, Position& pos) {
	if (!(cmt && pieces)) return 0;
	uint64_t ret = 0, captures, non_captures, piece_moves_to;
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		if constexpr (dtg <= 1 && !print_move) {
			ret += popcnt(cmt & make_reach_board<white, p>(from, pos.board));
		} else {
			piece_moves_to = cmt & make_reach_board<white, mt>(from, pos.board);
			captures = piece_moves_to & (white ? pos.board.b_board : pos.board.w_board);
			non_captures = piece_moves_to & ~captures;

			// Non-captures + captured.
			ret += generate_move_or_capture<white, p, false, dtg, print_move, cr>(non_captures, from, pos);
			ret += generate_move_or_capture<white, p, true, dtg, print_move, cr>(captures, from, pos);
		}
	}
	return ret;
}

template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_pawn(Position& pos) {
	BitBoard can_move_from = pos.board.get_piece_board<white, PAWN>();
	BitBoard pawns_on_promo = can_move_from & (white ? promotion_from_w : promotion_from_b);
	can_move_from &= ~pawns_on_promo;
	BitBoard pinned_dg = can_move_from & pos.get_mask()->pinmask_dg;
	BitBoard pinned_orth = can_move_from & pos.get_mask()->pinmask_orth;
	BitBoard unpinned = can_move_from & ~(pinned_dg | pinned_orth);

	// Unpinned + diagonally pinned + orthogonally pinned.
	return generate_pawn_moves<white, dtg, print_move, cr>(pos.get_mask()->can_move_to, unpinned, pos)
		+ generate_pawn_moves<white, dtg, print_move, cr>(pos.get_cmt() & pos.diag_mask(), pinned_dg, pos)
		+ generate_pawn_moves<white, dtg, print_move, cr>(pos.get_cmt() & pos.orth_mask(), pinned_orth, pos);
}

template <bool white, int dtg, bool print_move, CastlingRights cr>
static inline uint64_t generate_knight(Position& pos) {
	BitBoard cmf = pos.board.get_piece_board<white, KNIGHT>() & ~(pos.diag_mask() | pos.orth_mask());
	return generate_moves<white, KNIGHT, dtg, print_move, cr>(pos.get_mask()->can_move_to, cmf, pos);
}

template <bool white, int dtg, bool print_move, CastlingRights cr, Piece p>
static inline uint64_t generate_sliders(Position& pos) {
	BitBoard src = pos.board.get_piece_board<white, p>();
	BitBoard unpinned = src & ~pos.diag_mask() & ~pos.orth_mask();
	BitBoard pinned, pin_cmt;

	if constexpr (p == QUEEN_DIAG || p == BISHOP) {
		pinned = src & pos.diag_mask() & ~pos.orth_mask();
		pin_cmt = pos.get_mask()->can_move_to & pos.diag_mask();
	} else {
		pinned = src & pos.orth_mask() & ~pos.diag_mask();
		pin_cmt = pos.get_mask()->can_move_to & pos.orth_mask();
	}

	// Pinned + unpinned.
	return generate_moves<white, p, dtg, print_move, cr>(pin_cmt, pinned, pos)
		+ generate_moves<white, p, dtg, print_move, cr>(pos.get_mask()->can_move_to, unpinned, pos);
}

template <bool white, int dtg, bool print_move, CastlingRights cr, bool ep = false>
uint64_t count_moves(Position& pos) {
	if constexpr (dtg < 1)
		return 1;
	else {
		// Make masks.
		Square king_square = lbit(pos.board.get_piece_board<white, KING>());
		pos.get_mask()->create_masks<white>(pos.board, king_square);

		uint64_t ret = generate_king_moves<white, dtg, print_move, cr>(pos.get_mask()->can_move_to, king_square, pos);
		// If double check, only king can move. Else, limit the target squares to the checkmask and skip castling.
		switch (pos.get_mask()->checkers) {
		case 0:
			ret += generate_castle_move<white, true, dtg, print_move, cr>(pos, king_square);
			ret += generate_castle_move<white, false, dtg, print_move, cr>(pos, king_square);
			break;
		case 1:
			pos.get_mask()->can_move_to &= pos.get_mask()->check_mask;
			break;
		default:
			return ret;
		}
		// Generate moves.
		ret += generate_sliders<white, dtg, print_move, cr, BISHOP>(pos);
		ret += generate_sliders<white, dtg, print_move, cr, QUEEN_DIAG>(pos);
		ret += generate_sliders<white, dtg, print_move, cr, ROOK>(pos);
		ret += generate_sliders<white, dtg, print_move, cr, QUEEN_ORTH>(pos);
		ret += generate_pawn<white, dtg, print_move, cr>(pos);
		ret += generate_knight<white, dtg, print_move, cr>(pos);
		if constexpr (ep) ret += generate_ep_moves<white, dtg, print_move, cr>(pos, king_square);
		return ret;
	}
}

};	// namespace pyke

#endif
