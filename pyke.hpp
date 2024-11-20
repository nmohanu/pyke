#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>

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

template <bool white, int depth_to_go>
int count_moves(Position& pos);

template <bool white, Piece piece>
static inline BitBoard make_reach_board(Square square, BitBoard occ) {
	switch (piece) {
	case PAWN:
		return piece_move::get_pawn_move<white, PawnMoveType::NON_DOUBLE>(square);
	case KING:
		return piece_move::get_king_move(square);
	case ROOK:
		return piece_move::get_rook_move(square, occ);
	case BISHOP:
		return piece_move::get_bishop_move(square, occ);
	case KNIGHT:
		return piece_move::get_knight_move(square);
	case QUEEN:
		return piece_move::get_queen_move(square, occ);
	default:
		throw std::invalid_argument("Illegal piece given.");
	}
}

// Create pin masks.
template <bool white, bool diagonal>
static inline BitBoard make_pin_mask(BitBoard& pinners, BitBoard& king_mask, Position& pos) {
	BitBoard pinmask = 0b0;
	BitBoard queen_board = *get_piece_board<!white>(pos.board, QUEEN);
	BitBoard slider_board = *get_piece_board<!white>(pos.board, diagonal ? BISHOP : ROOK);
	while (queen_board) {
		pinmask |= get_queen_move(pop(queen_board), pos.board);
	}
	while (slider_board) {
		pinmask |=
			diagonal ? get_bishop_move(pop(slider_board), pos.board) : get_rook_move(pop(slider_board), pos.board);
	}
	return pinmask & king_mask;
}

// Returns whether a square is under attack.
template <bool white>
static inline bool is_attacked(Square square, Position& pos) {
	return (get_pawn_move<white, PawnMoveType::ATTACKS>(square) & *get_piece_board<!white>(pos.board, PAWN))
		|| (get_knight_move(square) & *get_piece_board<!white>(pos.board, KNIGHT))
		|| (get_rook_move(square, pos.board)
				& (*get_piece_board<!white>(pos.board, ROOK) | *get_piece_board<!white>(pos.board, QUEEN))
			|| (get_bishop_move(square, pos.board)
				& (*get_piece_board<!white>(pos.board, BISHOP) | *get_piece_board<white>(pos.board, QUEEN)))
			|| (get_king_move(square) & *get_piece_board<!white>(pos.board, KING)));
}

// Create king moves.
template <bool white>
static inline void generate_king_moves(MaskSet& maskset, Square king_square, Position& pos) {
	BitBoard cmt = maskset.can_move_to & piece_move::get_king_move(king_square);
	Move move(KING);
	move.set_from(king_square);
	while (cmt) {
		Square to = pop(cmt);
		if (is_attacked<white>(to, pos)) continue;
		bool king_attacked = is_attacked<white>(king_square, pos);
		move.set_to(to);
		pos.movelist.push(move);
	}
}

// Create the castling move for given player and direction.
template <bool white, bool kingside>
static inline void generate_castle_move(Position& pos, Square king_square) {
	bool has_right = white ? pos.gamestate.can_castle_king<white>() : pos.gamestate.can_castle_queen<white>();
	Square to = white ? (kingside ? 62 : 58) : (kingside ? 6 : 2);
	Square middle_square = white ? (kingside ? 61 : 59) : (kingside ? 5 : 3);

	bool king_attacked = is_attacked<white>(king_square, pos);
	bool middle_attacked = is_attacked<white>(middle_square, pos);
	bool to_attacked = is_attacked<white>(to, pos);

	bool to_occ = Pyke::square_occ(to, pos.board);
	bool middle_occ = Pyke::square_occ(middle_square, pos.board);

	// Check if the castling move is legal.
	if (!has_right || king_attacked || middle_attacked || to_attacked || to_occ || middle_occ) return;

	// Push castling move.
	Move move(KING, MOVE_CASTLE, king_square, to);
	uint8_t content = white ? (kingside ? 0 : 1) : (kingside ? 2 : 3);
	move.set_content(content);
	pos.movelist.push(move);
}

template <bool white>
static MaskSet create_masks(Position& pos, Square king_square) {
	Board& board = pos.board;
	MaskSet ret;
	// King reach.
	ret.king_dg = get_bishop_move(king_square, board);
	ret.king_orth = get_rook_move(king_square, board);
	ret.king_kn = get_knight_move(king_square);
	ret.king_pw = get_pawn_move<white>(king_square);

	// Generally, a player can move only to empty or enemy squares.
	ret.can_move_to = ~get_player_board<white>(board);

	// Get all potential pinner pieces.
	ret.orth_pinners =
		rook_mask_table[king_square] & (*get_piece_board<!white>(board, ROOK) | *get_piece_board<!white>(board, QUEEN));
	ret.dg_pinners = bishop_mask_table[king_square]
		& (*get_piece_board<!white>(board, BISHOP) | *get_piece_board<!white>(board, QUEEN));

	// Get only the pinners closest to the king.
	ret.orth_pinners &= rook_attacks[ret.orth_pinners][king_square];
	ret.dg_pinners &= bishop_attacks[ret.dg_pinners][king_square];

	// Make pinmask.
	ret.pinmask_dg = make_pin_mask<white, true>(ret.dg_pinners, ret.king_dg, pos);
	ret.pinmask_orth = make_pin_mask<white, false>(ret.orth_pinners, ret.king_orth, pos);

	// Check there are pieces directly attacking the king.
	ret.orth_checkers = ret.king_orth & ret.orth_pinners;
	ret.dg_checkers = ret.king_dg & ret.dg_pinners;
	ret.kn_checkers = ret.king_kn & *get_piece_board<!white>(board, KNIGHT);
	ret.pw_checkers = ret.king_pw & *get_piece_board<!white>(board, PAWN);

	return ret;
}

template <bool white, int depth_to_go>
static inline int generate_ep_moves(Position& pos, Square king_sq) {
	int ret = 0;
	uint8_t ep = pos.gamestate.get_en_passant();
	// In most cases, no ep is possible.
	if (!ep) return 0;
	auto make_en_passant = [&](int8_t from_offset) {
		// Rank to move to.
		File to = ep & 0b00001111;
		// Whether en passant comes from the left.
		bool from_the_left = ep & 0b10000000;
		// From which file the pawn comes.
		File from = to + from_offset;
		// The start and end rank.
		Rank start_rank = white ? 4 : 3;
		Rank end_rank = white ? 5 : 2;
		// Start and end square.
		Square start_square = from + start_rank * 8;
		Square end_square = to + end_rank * 8;

		ep_move<white>(start_square, end_square, pos);
		if (!is_attacked<white>(king_sq, pos)) {
			if constexpr (depth_to_go == 1)
				ret += 1;
			else
				ret += 1 + count_moves<white, depth_to_go - 1>(pos);
		}
		unmake_ep_move<white>(start_square, end_square, pos);
		return ret;
	};

	if (ep & 0b1000'0000) ret += make_en_passant(-1);
	if (ep & 0b0100'0000) ret += make_en_passant(1);

	return ret;
}

// Pawn double pushes.
template <bool white, int depth_to_go>
static inline int generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
	ret == 0;
	while (source) {
		Square from = pop(source);
		while (cmt) {
			Square to = pop(cmt);
			if constexpr (depth_to_go == 1)
				ret += 1;
			else {
				pawn_double<white>(from, to, pos);
				ret += 1 + count_moves<white, depth_to_go - 1>(pos);
				unmake_pawn_double<white>(from, to, pos);
			}
		}
	}
	return ret;
}

// Generate all possible promotion moves.
template <bool white, int depth_to_go>
static inline int generate_promotions(Position& pos, BitBoard cmt, BitBoard source) {
	auto generate_promo_pieces = [&](auto... pieces, Piece captured, Square from, Square to) {
		promo_move<white, pieces, captured>(from, to, pos);
		int count = 1 + count_moves<white, depth_to_go - 1>(pos);
		unmake_promo_move<white, p, captured>(from, to, pos);
		return count;
	};
	int ret = 0;
	while (source) {
		Square from = pop(source);
		while (cmt) {
			Square to = pop(cmt);
			if constexpr (depth_to_go == 1) {
				return 4;
			} else {
				const Piece at_to = pos.board.get_piece<white, true>(to);
				ret += generate_promo_pieces(ROOK, KNIGHT, BISHOP, QUEEN, at_to, from, to);
			}
		}
	}
}

// Push moves from a given position to the target squares.
template <bool white, Piece p, bool capture, int depth_to_go>
static inline int generate_move_or_capture(BitBoard cmt, Square from, Position& pos, Move move, Piece piece) {
	int ret = 0;
	while (cmt) {
		Square to = pop(cmt);
		if constexpr (depth_to_go == 1)
			ret += 1;
		else if constexpr (capture) {
			// Capture moves.
			const Piece captured = pos.board.get_piece<white, true>(to);
			capture_move_wrapper<white, p>(from, to, pos, captured);
			ret += 1 + count_moves<white, depth_to_go - 1>(pos);
			unmake_capture_wrapper<white, p>(from, to, pos, captured);
		} else {
			// Plain, non special moves.
			plain_move<white, p>(from, to, pos.board);
			ret += 1 + count_moves<white, depth_to_go - 1>(pos);
			unmake_plain_move<white, p>(from, to, pos.board);
		}
	}
	return ret;
}

// Create moves given a from and to board..
template <bool white, Piece p, int depth_to_go>
static inline int generate_moves(BitBoard cmt, BitBoard& pieces, Position& pos, Piece piece) {
	int ret = 0;
	// For pawns, handle special cases seperately.
	if (constexpr p == PAWN) {
		BitBoard pawns_on_start = pieces & (white ? pawn_start_w : pawn_start_b);
		BitBoard pawns_on_promo = pieces & (white ? promotion_from_w : promotion_from_b);
		ret += generate_pawn_double<white, depth_to_go>(cmt, pos, pawns_on_start);
		ret += generate_promotions<white, depth_to_go>(pos, cmt, source);
		pieces &= ~pawns_on_promo;
	}
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		BitBoard piece_moves_to = cmt & make_reach_board<white, p>(from, pos.board);
		BitBoard captures = piece_moves_to & (white ? pos.board.b_board : pos.board.w_board);
		BitBoard non_captures &= ~captures;

		// Non-captures.
		ret += generate_move_or_capture<white, false, depth_to_go>(can_move_to, from, pos, move, piece);
		// Captures.
		ret += generate_move_or_capture<white, true, depth_to_go>(captures, from, pos, move, piece);
	}
	return ret;
}

// For a given piece, generate all possible normal/capture moves.
template <bool white, Piece p, int depth_to_go>
static inline int generate_any(Board& b, MaskSet& maskset) {
	int ret = 0;
	BitBoard can_move_from = b.get_piece_board<white, p>();
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = can_move_from & maskset.king_dg & maskset.pinmask_dg;
	BitBoard pinned_orth = can_move_from & maskset.king_orth & maskset.pinmask_orth;
	BitBoard unpinned = can_move_from & ~(pinned_dg | pinned_orth);

	// Unpinned.
	ret += generate_moves<white, p>(maskset.can_move_to, unpinned, b);
	// Pinned diagonally.
	ret += generate_moves<white, p>(maskset.can_move_to & maskset.pinmask_dg, pinned_dg, b);
	// Pinned orthogonally.
	ret += generate_moves<white, p>(maskset.can_move_to & maskset.pinmask_orth, pinned_orth, b);

	return ret;
}

// Create move list for given position.
template <bool white, int depth_to_go>
int count_moves(Position& pos) {
	if constexpr (depth_to_go == 0) return 1;
	int ret = 0;
	// Make king mask.
	Square king_square = __builtin_clzll(pos.board.get_piece_board<white, KING>());
	// Create all needed masks.
	MaskSet maskset = create_masks<white>(pos, king_square);
	// Lambda function for piece generation.
	auto generate_for_pieces = [&](auto... pieces) {
		(generate_any<white, pieces, depth_to_go>(pos.board, maskset), ...);
	};
	// Amount of checkers.
	uint8_t checker_cnt = maskset.get_check_cnt();
	// Conditionals only taken when king is in check. If double check, only king can move. Else, limit the
	// target squares to the checkmask and skip castling moves..
	if (checker_cnt >= 2)
		goto king;
	else if (checker_cnt) {
		maskset.can_move_to &= maskset.get_check_mask();
		goto no_castle;
	}
	// Castling moves.
	ret += generate_castle_move<white, true>(pos, king_square);
	ret += generate_castle_move<white, false>(pos, king_square);
no_castle:
	// Generate moves.
	ret += generate_for_pieces(ROOK, BISHOP, KNIGHT, QUEEN, PAWN);
	ret += generate_ep_moves<white, depth_to_go>(pos, king_square);
	// King and queen side castle.
king:
	ret += generate_king_moves<white>(maskset, king_square, pos);
	return ret;
}
};	// namespace pyke

#endif
