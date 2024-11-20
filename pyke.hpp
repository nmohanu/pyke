#include <cassert>
#include <iostream>
#include <stdexcept>

#include "gamestate.hpp"
#include "maskset.hpp"
#include "move.hpp"
#include "piece_moves.hpp"
#include "position.hpp"
#include "stack.hpp"
#include "util.hpp"

#ifndef PYKE_H
#define PYKE_H
namespace Pyke {

static void print_position(Position& pos);

template <bool white, Piece piece>
static inline BitBoard make_reach_board(Square square, BitBoard occ) {
	switch (piece) {
	case 0:
		return get_pawn_move<white>(square);
	case 1:
		return get_king_move(square);
	case 2:
		return get_rook_move(square, occ);
	case 3:
		return get_bishop_move(square, occ);
	case 4:
		return get_knight_move(square);
	case 5:
		return get_queen_move(square, occ);
	default:
		throw std::invalid_argument("Illegal piece given.");
	}
}

// Push moves from a given position to the target squares.
template <bool white, bool capture>
static inline void generate_move_or_capture(BitBoard cmt, Square from, Position& pos, Move move, Piece piece) {
	while (cmt) {
		Square to = pop(cmt);
		move.set_to(to);
		if (capture) move.set_type(MOVE_CAPTURE);
		pos.movelist.push(move);
	}
}

// Create moves given a from and to board..
template <bool white, bool is_pawn_double>
static inline void generate_moves(BitBoard cmt, BitBoard& pieces, Position& pos, Piece piece) {
	Move move(piece);
	if (is_pawn_double) move.set_type(MOVE_PAWN_DOUBLE);
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		move.set_from(from);
		BitBoard can_move_to = cmt & make_reach_board<white>(from, pos.board, piece);
		BitBoard captures = can_move_to & (white ? pos.board.b_board : pos.board.w_board);
		can_move_to &= ~captures;

		// Non-captures.
		generate_move_or_capture<white, false>(can_move_to, from, pos, move, piece);
		// Captures.
		generate_move_or_capture<white, true>(captures, from, pos, move, piece);
	}
}

// Create pin masks.
template <bool white, bool diagonal>
constexpr static inline BitBoard make_pin_mask(BitBoard& pinners, BitBoard& king_mask, Position& pos) {
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
constexpr static inline bool is_attacked(Square square, Position& pos) {
	return (get_pawn_move<white>(square) & *get_piece_board<!white>(pos.board, PAWN))
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
	BitBoard cmt = maskset.can_move_to & get_king_move(king_square);
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
constexpr static MaskSet create_masks(Position& pos, Square king_square) {
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

// Generate all possible promotion moves.
template <bool white>
static inline void generate_promotions(Position& pos, BitBoard can_move_to) {
	BitBoard promoting_pawns = *get_piece_board<white>(pos.board, PAWN) & (0xFFULL << (white ? 48 : 8));
	// Usually, no promotional pawns are available.
	if (!promoting_pawns) return;
	Move move(MOVE_PROMO, PAWN);
	while (promoting_pawns) {
		move.set_from(pop(promoting_pawns));
		while (can_move_to) {
			move.set_to(pop(can_move_to));
			for (Piece promote_to = ROOK; promote_to <= QUEEN; promote_to++) {
				// Square that the piece moves to.
				move.set_content(promote_to);
				pos.movelist.push(move);
			}
		}
	}
}

template <bool white>
static inline void generate_ep_moves(Position& pos, MaskSet& maskset) {
	uint8_t ep = pos.gamestate.get_en_passant();
	// In most cases, no ep is possible.
	if (!ep) return;
	Move move(MOVE_EP, PAWN);
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
		Square captured_square = white ? (to - 8) : (to + 8);

		BitBoard taking_pawn = 1ULL << (63 - start_square);
		BitBoard end_state = 1ULL << (63 - end_square);
		BitBoard captured_pawn_board = 1ULL << (63 - captured_square);

		bool legal = true;
		// Check if taken pawn is pinned to player's king.
		if (captured_pawn_board & maskset.king_dg & maskset.pinmask_dg) legal = false;
		// Check if en passant moving pawn is pinned.
		if (taking_pawn & maskset.king_dg & maskset.pinmask_dg) {
			// If pinned diagonally, the ending square must be in same diagonal pin.
			if (!(end_square & maskset.king_dg & maskset.pinmask_dg)) legal = false;
		}
		// If pinned orthogonally, EP is never possible.
		if (taking_pawn & maskset.king_orth & maskset.pinmask_orth) legal = false;

		if (legal) {
			move.set_from(start_square);
			move.set_to(end_square);

			pos.movelist.push(move);
		}
	};

	if (ep & 0b1000'0000) make_en_passant(-1);
	if (ep & 0b0100'0000) make_en_passant(1);
}

// Create moves given a from and to board..
template <bool white>
static inline void generate_move_pawn_double(BitBoard cmt, BitBoard& pieces, Position& pos) {
	Move move(PAWN);
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		move.set_from(from);
		BitBoard can_move_to = cmt & get_pawn_double<white>(from);
		generate_move_or_capture<white, false>(can_move_to, from, pos, move, PAWN);
	}
}

template <bool white>
constexpr static inline void generate_pawn_normal(Position& pos, MaskSet& maskset, BitBoard source) {
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = source & maskset.king_dg & maskset.pinmask_dg;
	BitBoard pinned_orth = source & maskset.king_orth & maskset.pinmask_orth;
	source &= ~(pinned_dg | pinned_orth);

	// Unpinned.
	generate_moves<white, false>(maskset.can_move_to, source, pos, PAWN);
	// Pinned diagonally.
	generate_moves<white, false>(maskset.can_move_to & maskset.pinmask_dg, pinned_dg, pos, PAWN);
	// Pinned orthogonally.
	generate_moves<white, false>(maskset.can_move_to & maskset.pinmask_orth, pinned_orth, pos, PAWN);
}

template <bool white>
constexpr static inline void generate_pawn_double(Position& pos, MaskSet& maskset, BitBoard source) {
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = source & maskset.king_dg & maskset.pinmask_dg;
	BitBoard pinned_orth = source & maskset.king_orth & maskset.pinmask_orth;
	source &= ~(pinned_dg | pinned_orth);

	// Unpinned.
	generate_moves<white, true>(maskset.can_move_to, source, pos, PAWN);
	// Pinned orthogonally.
	generate_moves<white, true>(maskset.can_move_to & maskset.pinmask_orth, pinned_orth, pos, PAWN);
}

template <bool white>
constexpr static inline void generate_pawn_moves(Position& pos, MaskSet& maskset) {
	BitBoard pawns = *get_piece_board<white>(pos.board, PAWN);
	BitBoard pawns_on_start = pawns & (white ? pawn_start_w : pawn_start_b);
	BitBoard pawns_on_promo = pawns & (white ? promotion_from_w : promotion_from_b);
	// Captures and pushes non special cases.
	BitBoard normal_squares = maskset.can_move_to & ~pawns_on_promo;
	generate_pawn_normal<white>(pos, maskset, normal_squares);
	// Double forward moves.
	generate_pawn_double<white>(pos, maskset, pawns_on_start);
	// Promotion moves.
	generate_promotions<white>(pos, promotion_to_squares & maskset.can_move_to);
	generate_ep_moves<white>(pos, maskset);
}

// For a given piece, generate all possible normal/capture moves.
template <bool white, Piece p>
constexpr static inline void generate_any(Board& b, MaskSet& maskset) {
	if constexpr (p == PAWN) {
		generate_pawn_moves<white>(b, maskset);
		return;
	}

	BitBoard can_move_from = *b.get_piece_board<white, p>();
	// Split pinned pieces from non-pinned pieces.
	BitBoard pinned_dg = can_move_from & maskset.king_dg & maskset.pinmask_dg;
	BitBoard pinned_orth = can_move_from & maskset.king_orth & maskset.pinmask_orth;
	can_move_from &= ~(pinned_dg | pinned_orth);

	// Unpinned.
	generate_moves<white, false>(maskset.can_move_to, can_move_from, b, p);
	// Pinned diagonally.
	generate_moves<white, false>(maskset.can_move_to & maskset.pinmask_dg, pinned_dg, b, p);
	// Pinned orthogonally.
	generate_moves<white, false>(maskset.can_move_to & maskset.pinmask_orth, pinned_orth, b, p);
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
	auto generate_for_pieces = [&](auto... pieces) { (generate_any<white, pieces>(pos.board, maskset), ...); };
	// Amount of checkers.
	uint8_t checker_cnt = maskset.get_check_cnt();
	// Conditionals only taken when king is in check. If double check, only king can move. Else, limit the target
	// squares to the checkmask and skip castling moves..
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
	// King and queen side castle.
king:
	ret += generate_king_moves<white>(maskset, king_square, pos);
	return ret;
}

};	// namespace Pyke

#endif
