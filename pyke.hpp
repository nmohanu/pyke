#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

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

template <bool white, int depth_to_go, bool print_move>
int count_moves(Position& pos);

// Returns whether a square is under attack.
template <bool white>
static inline bool is_attacked(Square square, Board& b) {
	using namespace piece_move;
	BitBoard& c = b.occ_board;
	return (get_pawn_move<white, PawnMoveType::ATTACKS>(square, b) & b.get_piece_board<!white, PAWN>())
		|| (get_knight_move(square) & b.get_piece_board<!white, KNIGHT>())
		|| (get_rook_move(square, c) & (b.get_piece_board<!white, ROOK>() | b.get_piece_board<!white, QUEEN>()))
		|| (get_bishop_move(square, c) & (b.get_piece_board<!white, BISHOP>() | b.get_piece_board<white, QUEEN>()))
		|| (get_king_move(square) & b.get_piece_board<!white, KING>());
}

// Returns the reach of a given piece. For pawns, it returns the reach without double push.
template <bool white, Piece piece>
static inline BitBoard make_reach_board(Square square, Board& b) {
	BitBoard& occ = b.occ_board;
	switch (piece) {
	case PAWN:
		return piece_move::get_pawn_move<white, PawnMoveType::NON_DOUBLE>(square, b);
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
	BitBoard queen_board = pos.board.get_piece_board<!white, QUEEN>();
	BitBoard slider_board = pos.board.get_piece_board<!white, diagonal ? BISHOP : ROOK>();
	while (queen_board) pinmask |= piece_move::get_queen_move(pop(queen_board), pos.board.occ_board);
	while (slider_board)
		pinmask |= diagonal ? piece_move::get_bishop_move(pop(slider_board), pos.board.occ_board)
							: piece_move::get_rook_move(pop(slider_board), pos.board.occ_board);

	return pinmask & king_mask;
}

// Create all the needed masks for the current position.
template <bool white>
static MaskSet create_masks(Position& pos, Square king_square) {
	Board& b = pos.board;
	BitBoard& c = b.occ_board;
	MaskSet ret;
	// King reach.
	ret.king_dg = piece_move::get_bishop_move(king_square, c);
	ret.king_orth = piece_move::get_rook_move(king_square, c);
	ret.king_kn = piece_move::get_knight_move(king_square);
	ret.king_pw = piece_move::get_pawn_move<white, PawnMoveType::ATTACKS>(king_square, b);

	// A player can move only to empty or enemy squares.
	ret.can_move_to = ~b.get_player_occ<white>();

	// Get all potential pinner pieces.
	ret.orth_pinners =
		rook_mask_table[king_square] & (b.get_piece_board<!white, ROOK>() | b.get_piece_board<!white, QUEEN>());
	ret.dg_pinners =
		bishop_mask_table[king_square] & (b.get_piece_board<!white, BISHOP>() | b.get_piece_board<!white, QUEEN>());

	// Get only the pinners closest to the king.
	ret.orth_pinners &= rook_attacks[ret.orth_pinners][king_square];
	ret.dg_pinners &= bishop_attacks[ret.dg_pinners][king_square];

	// Make pinmask.
	ret.pinmask_dg = make_pin_mask<white, true>(ret.dg_pinners, ret.king_dg, pos);
	ret.pinmask_orth = make_pin_mask<white, false>(ret.orth_pinners, ret.king_orth, pos);

	// Check there are pieces directly attacking the king.
	ret.orth_checkers = ret.king_orth & ret.orth_pinners;
	ret.dg_checkers = ret.king_dg & ret.dg_pinners;
	ret.kn_checkers = ret.king_kn & b.get_piece_board<!white, KNIGHT>();
	ret.pw_checkers = ret.king_pw & b.get_piece_board<!white, PAWN>();

	return ret;
}

template <bool white, int depth_to_go, bool print_move>
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
		if (!is_attacked<white>(king_sq, pos.board)) {
			if constexpr (depth_to_go <= 1)
				ret += 1;
			else
				ret += 1 + count_moves<white, depth_to_go - 1, false>(pos);
		}
		unmake_ep_move<white>(start_square, end_square, pos);
		return ret;
	};

	if (ep & 0b1000'0000) ret += make_en_passant(-1);
	if (ep & 0b0100'0000) ret += make_en_passant(1);

	return ret;
}

// Create the castling move for given player and direction.
template <bool white, bool kingside, int depth_to_go, bool print_move>
static inline int generate_castle_move(Position& pos, Square king_square) {
	Board& b = pos.board;

	bool has_right = white ? pos.gamestate.can_castle_king<white>() : pos.gamestate.can_castle_queen<white>();
	Square to = white ? (kingside ? 62 : 58) : (kingside ? 6 : 2);
	Square middle_square = white ? (kingside ? 61 : 59) : (kingside ? 5 : 3);

	bool middle_attacked = is_attacked<white>(middle_square, b);
	bool to_attacked = is_attacked<white>(to, b);

	bool to_occ = b.square_occ(to);
	bool middle_occ = b.square_occ(middle_square);

	// Check if the castling move is legal.
	if (!has_right || middle_attacked || to_attacked || to_occ || middle_occ) return 0;
	if constexpr (depth_to_go <= 1)
		return 1;
	else {
		const uint8_t code = white ? (kingside ? 0 : 1) : (kingside ? 2 : 3);
		castle_move<white, code>(pos);
		int ret = 1 + count_moves<!white, depth_to_go - 1, false>(pos);
		unmake_castle_move<white, code>(pos);
		return ret;
	}
}

// Create king moves.
template <bool white, int depth_to_go, bool print_move>
static inline int generate_king_moves(BitBoard cmt, Square king_square, Position& pos) {
	cmt &= piece_move::get_king_move(king_square);
	int ret = 0;
	while (cmt) {
		Square to = pop(cmt);
		if (is_attacked<white>(to, pos.board))
			continue;
		else if constexpr (depth_to_go <= 1)
			ret += 1;
		else {
			if (pos.board.square_occ(to)) {
				Piece captured = pos.board.get_piece<white>(to);
				capture_move_wrapper<white, KING>(king_square, to, pos, captured);
				ret += 1 + count_moves<!white, depth_to_go - 1, false>(pos);
				unmake_capture_wrapper<white, KING>(king_square, to, pos, captured);
			} else {
				plain_move<white, KING>(king_square, to, pos);
				ret += 1 + count_moves<!white, depth_to_go - 1, false>(pos);
				unmake_plain_move<white, KING>(king_square, to, pos);
			}
		}
	}
	return ret;
}

// Pawn double pushes.
template <bool white, int depth_to_go, bool print_move>
static inline int generate_pawn_double(BitBoard cmt, Position& pos, BitBoard source) {
	int ret = 0;
	while (source) {
		Square from = pop(source);
		BitBoard to_board = cmt & piece_move::get_pawn_double<white>(square_to_mask(from));
		while (to_board) {
			Square to = pop(to_board);
			int n = 0;
			if constexpr (print_move) std::cout << make_chess_notation(from) << make_chess_notation(to) << ": ";
			if constexpr (depth_to_go <= 1)
				n += 1;
			else {
				pawn_double<white>(from, to, pos);
				n += count_moves<!white, depth_to_go - 1, false>(pos);
				unmake_pawn_double<white>(from, to, pos);
			}
			if constexpr (print_move) std::cout << std::to_string(n) << '\n';
			ret += n;
		}
	}
	return ret;
}

// Generate all possible promotion moves.
template <bool white, int depth_to_go, bool print_move>
static inline int generate_promotions(Position& pos, BitBoard cmt, BitBoard source) {
	return 0;
	/*
	const auto generate_promo_pieces = [&](const Piece piece, const Piece captured, Square from, Square to) {
		promo_move<white, piece, captured>(from, to, pos);
		int count = 1 + count_moves<!white, depth_to_go - 1>(pos);
		unmake_promo_move<white, piece, captured>(from, to, pos);
		return count;
	};
	int ret = 0;
	while (source) {
		Square from = pop(source);
		while (cmt) {
			Square to = pop(cmt);
			if constexpr (depth_to_go <= 1) {
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
template <bool white, Piece p, bool capture, int depth_to_go, bool print_move>
static inline int generate_move_or_capture(BitBoard cmt, Square from, Position& pos) {
	int ret = 0;
	while (cmt) {
		int n = 0;
		Square to = pop(cmt);
		if constexpr (print_move) std::cout << make_chess_notation(from) << make_chess_notation(to) << ": ";
		if constexpr (depth_to_go <= 1)
			n += 1;
		else if constexpr (capture) {
			// Capture moves.
			const Piece captured = pos.board.get_piece<white>(to);
			capture_move_wrapper<white, p>(from, to, pos, captured);
			n += count_moves<!white, depth_to_go - 1, false>(pos);
			unmake_capture_wrapper<white, p>(from, to, pos, captured);
		} else {
			// Plain, non special moves.
			plain_move<white, p>(from, to, pos);
			n += count_moves<!white, depth_to_go - 1, false>(pos);
			unmake_plain_move<white, p>(from, to, pos);
		}
		if constexpr (print_move) std::cout << std::to_string(n) << '\n';
		ret += n;
	}
	return ret;
}

// Create moves given a from and to board..
template <bool white, Piece p, int depth_to_go, bool print_move>
static inline int generate_moves(BitBoard cmt, BitBoard& pieces, Position& pos) {
	int ret = 0;
	// For pawns, handle special cases seperately.
	if constexpr (p == PAWN) {
		BitBoard pawns_on_start = pieces & (white ? pawn_start_w : pawn_start_b);
		BitBoard pawns_on_promo = pieces & (white ? promotion_from_w : promotion_from_b);
		ret += generate_pawn_double<white, depth_to_go, print_move>(cmt, pos, pawns_on_start);
		ret += generate_promotions<white, depth_to_go, print_move>(pos, cmt, pawns_on_promo);
		pieces &= ~pawns_on_promo;
	}
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		BitBoard piece_moves_to = cmt & make_reach_board<white, p>(from, pos.board);
		BitBoard captures = piece_moves_to & (white ? pos.board.b_board : pos.board.w_board);
		BitBoard non_captures = piece_moves_to & ~captures;

		// Non-captures.
		ret += generate_move_or_capture<white, p, false, depth_to_go, print_move>(non_captures, from, pos);
		// Captures.
		ret += generate_move_or_capture<white, p, true, depth_to_go, print_move>(captures, from, pos);
	}
	return ret;
}

// For a given piece, generate all possible normal/capture moves.
template <bool white, Piece p, int depth_to_go, bool print_move>
static inline int generate_any(Position& pos, MaskSet& maskset) {
	Board& b = pos.board;
	int ret = 0;
	BitBoard can_move_from = b.get_piece_board<white, p>();
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = can_move_from & maskset.king_dg & maskset.pinmask_dg;
	BitBoard pinned_orth = can_move_from & maskset.king_orth & maskset.pinmask_orth;
	BitBoard unpinned = can_move_from & ~(pinned_dg | pinned_orth);

	// Unpinned.
	ret += generate_moves<white, p, depth_to_go, print_move>(maskset.can_move_to, unpinned, pos);
	// Pinned diagonally.
	ret += generate_moves<white, p, depth_to_go, print_move>(maskset.can_move_to & maskset.pinmask_dg, pinned_dg, pos);
	// Pinned orthogonally.
	ret +=
		generate_moves<white, p, depth_to_go, print_move>(maskset.can_move_to & maskset.pinmask_orth, pinned_orth, pos);

	return ret;
}

// Create move list for given position.
template <bool white, int depth_to_go, bool print_move>
int count_moves(Position& pos) {
	if constexpr (depth_to_go <= 0) return 0;
	int ret = 0;
	// Make king mask.
	Square king_square = __builtin_clzll(pos.board.get_piece_board<white, KING>());
	// Create all needed masks.
	MaskSet maskset = create_masks<white>(pos, king_square);
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
	// ret += generate_castle_move<white, true, depth_to_go, print_move>(pos, king_square);
	// ret += generate_castle_move<white, false, depth_to_go, print_move>(pos, king_square);
no_castle:
	// Generate moves.
	ret += generate_any<white, ROOK, depth_to_go, print_move>(pos, maskset);
	ret += generate_any<white, BISHOP, depth_to_go, print_move>(pos, maskset);
	ret += generate_any<white, KNIGHT, depth_to_go, print_move>(pos, maskset);
	ret += generate_any<white, QUEEN, depth_to_go, print_move>(pos, maskset);
	ret += generate_any<white, PAWN, depth_to_go, print_move>(pos, maskset);
	// ret += generate_ep_moves<white, depth_to_go, print_move>(pos, king_square);
	// King and queen side castle.
king:
	ret += generate_king_moves<white, depth_to_go, print_move>(maskset.can_move_to, king_square, pos);
	return ret;
}

};	// namespace pyke

#endif
