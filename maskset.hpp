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
	BitBoard pinmask_dg;
	BitBoard pinmask_orth;
	BitBoard check_mask;

	uint8_t checkers;

	inline uint8_t get_check_cnt() { return checkers; }

	// Create all the needed masks for the current position.
	template <bool white>
	void create_masks(Board& b, Square king_square) {
		checkers = 0;
		can_move_to = ~b.get_player_occ<white>();

		BitBoard c_mask = 0, p_diag_mask = 0, p_orth_mask = 0;

		BitBoard k_checkers = piece_move::get_knight_move(king_square) & b.get_piece_board<!white, KNIGHT>();
		BitBoard p_checkers =
			piece_move::get_pawn_diags<white>(square_to_mask(king_square)) & b.get_piece_board<!white, PAWN>();

		if (k_checkers) {
			c_mask |= k_checkers;
			checkers++;
		}
		if (p_checkers) {
			c_mask |= p_checkers;
			checkers++;
		}

		BitBoard opp_board = b.get_player_occ<!white>();
		BitBoard eq = b.get_piece_board<!white, QUEEN>();
		BitBoard eb = b.get_piece_board<!white, BISHOP>();
		BitBoard er = b.get_piece_board<!white, ROOK>();

		BitBoard diag_pinners = piece_move::get_bishop_move(king_square, opp_board) & (eb | eq);
		BitBoard orth_pinners = piece_move::get_rook_move(king_square, opp_board) & (er | eq);

		auto process_pinners = [&](BitBoard& pinboard, BitBoard& goal_mask) {
			while (pinboard) {
				Square src = pop(pinboard);
				BitBoard between = between_squares[king_square][src];

				switch (__builtin_popcountll(between & b.get_player_occ<white>())) {
				case 0:
					c_mask |= between | square_to_mask(src);
					checkers++;
					break;
				case 1:
					goal_mask |= between | square_to_mask(src);
					break;
				}
			}
		};

		process_pinners(diag_pinners, p_diag_mask);
		process_pinners(orth_pinners, p_orth_mask);

		pinmask_dg = p_diag_mask;
		pinmask_orth = p_orth_mask;
		check_mask = c_mask;
	}
};

#endif
