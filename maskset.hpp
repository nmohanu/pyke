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
	BitBoard king_cmt;

	uint8_t checkers = 0;

	inline uint8_t get_check_cnt() { return checkers; }

	inline void reset() {
		pinmask_dg = 0;
		pinmask_orth = 0;
		check_mask = 0;
		checkers = 0;
	}
};

// Create all the needed masks for the current position.
template <bool white>
void create_masks(Board& b, Square king_square, MaskSet& ret) {
	ret.reset();
	ret.can_move_to = ~b.get_player_occ<white>();

	BitBoard opp_board = b.get_player_occ<!white>();
	BitBoard own_board = b.get_player_occ<white>();
	BitBoard eq = b.get_piece_board<!white, QUEEN>();
	BitBoard eb = b.get_piece_board<!white, BISHOP>();
	BitBoard er = b.get_piece_board<!white, ROOK>();
	BitBoard diag_pinners = get_bishop_move(king_square, opp_board) & (eb | eq);
	BitBoard orth_pinners = get_rook_move(king_square, opp_board) & (er | eq);
	BitBoard k_checkers = get_knight_move(king_square) & b.get_piece_board<!white, KNIGHT>();
	BitBoard p_checkers = get_pawn_diags<white>(square_to_mask(king_square)) & b.get_piece_board<!white, PAWN>();
	BitBoard king_cnmt = 0;
	ret.king_cmt = get_king_move(king_square) & ret.can_move_to;

	if (k_checkers) {
		ret.check_mask |= k_checkers;
		ret.checkers++;
	} else if (p_checkers) {
		ret.check_mask |= p_checkers;
		ret.checkers++;
	}

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
}

#endif
