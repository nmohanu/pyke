#include <cstdint>

#include "board.hpp"
#include "defaults.hpp"
#include "piece_moves.hpp"

#ifndef MASKSET_H
#define MASKSET_H

struct MaskSet {
	// squares that are empty or enemy;
	BitBoard can_move_to;

	// Pinmask.
	BitBoard pinmask_dg = 0;
	BitBoard pinmask_orth = 0;
	BitBoard check_mask = 0;
	BitBoard unpinned = 0;

	uint8_t checkers = 0;

	inline uint8_t get_check_cnt() { return checkers; }
};

// Create all the needed masks for the current position.
template <bool white>
MaskSet create_masks(Board& b, Square king_square) {
	MaskSet ret;
	ret.can_move_to = ~b.get_player_occ<white>();

	BitBoard k_checkers = piece_move::get_knight_move(king_square) & b.get_piece_board<!white, KNIGHT>();
	BitBoard p_checkers =
		piece_move::get_pawn_diags<white>(square_to_mask(king_square)) & b.get_piece_board<!white, PAWN>();

	if (k_checkers) {
		ret.check_mask |= k_checkers;
		ret.checkers++;
	} else if (p_checkers) {
		ret.check_mask |= p_checkers;
		ret.checkers++;
	}

	BitBoard opp_board = b.get_player_occ<!white>();
	BitBoard own_board = b.get_player_occ<white>();
	BitBoard eq = b.get_piece_board<!white, QUEEN>();
	BitBoard eb = b.get_piece_board<!white, BISHOP>();
	BitBoard er = b.get_piece_board<!white, ROOK>();
	BitBoard diag_pinners = piece_move::get_bishop_move(king_square, opp_board) & (eb | eq);
	BitBoard orth_pinners = piece_move::get_rook_move(king_square, opp_board) & (er | eq);

	auto process_pinners = [&](BitBoard& pinboard, BitBoard& goal_mask) {
		while (pinboard) {
			Square src = pop(pinboard);
			BitBoard between = between_squares[king_square][src];

			switch (popcnt(between & own_board)) {
			case 0:
				ret.check_mask |= between;
				ret.checkers++;
				break;
			case 1:
				goal_mask |= between;
				break;
			}
		}
	};

	process_pinners(diag_pinners, ret.pinmask_dg);
	process_pinners(orth_pinners, ret.pinmask_orth);
	ret.unpinned = ~(ret.pinmask_dg | ret.pinmask_orth);
	return ret;
}

#endif
