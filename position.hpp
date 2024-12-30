#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "board.hpp"
#include "gamestate.hpp"
#include "maskset.hpp"
#include "move.hpp"
#include "stack.hpp"

#ifndef POSITION_H
#define POSITION_H

template <std::size_t N>
struct MaskList {
	MaskSet stack[N];
	void point_next() { curr++; }
	void point_prev() { curr--; }
	MaskSet* top() { return &stack[curr]; }
	int curr = 0;
};

struct Position {
	Position() {}

	Board board;
	uint8_t ep_flag;
	MaskList<1024> mask_list;
	bool white_turn = true;

	inline void moved() { mask_list.point_next(); }
	inline void unmoved() { mask_list.point_prev(); }

	inline BitBoard orth_mask() { return mask_list.top()->pinmask_orth; }
	inline BitBoard diag_mask() { return mask_list.top()->pinmask_dg; }
	inline MaskSet* get_mask() { return mask_list.top(); }
	inline BitBoard get_cmt() { return mask_list.top()->can_move_to; }

	// Returns whether a square is under attack.
	template <bool white>
	inline bool is_attacked(Square square) {
		using namespace piece_move;
		return (get_pawn_move<white, PawnMoveType::ATTACKS>(square, board.occ_board)
				& board.get_piece_board<!white, PAWN>())
			|| (get_knight_move(square) & board.get_piece_board<!white, KNIGHT>())
			|| (get_rook_move(square, board.occ_board)
				& (board.get_piece_board<!white, ROOK>() | board.get_piece_board<!white, QUEEN>()))
			|| (get_bishop_move(square, board.occ_board)
				& (board.get_piece_board<!white, BISHOP>() | board.get_piece_board<!white, QUEEN>()))
			|| (get_king_move(square) & board.get_piece_board<!white, KING>());
	}
};

#endif
