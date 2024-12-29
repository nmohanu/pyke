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
	MaskList() {
		for (int i = 0; i < N; i++) {
			// stack[i] = new MaskSet;
		}
	}
	~MaskList() {
		for (int i = 0; i < N; i++) {
			// if (stack[i]) delete stack[i];
		}
	}

	void point_next() { curr++; }
	void point_prev() { curr--; }
	MaskSet* pop() { return &stack[--curr]; }
	MaskSet* top() { return &stack[curr]; }

	int curr = 0;
};

struct Position {
	Position() {}
	Position(Position& other) {
		gamestate.set_data(other.gamestate.get_data());
		white_turn = other.white_turn;
		board = other.board.copy();
	}

	Board board;
	GameState gamestate;
	Stack<uint32_t> history;
	MaskList<1024> mask_list;
	bool white_turn = true;

	bool is_equal(Position& other);

	inline void moved() {
		white_turn = !white_turn;
		history.push(gamestate.get_data());
		mask_list.point_next();
		gamestate.reset_en_passant();
	}
	inline void unmoved() {
		white_turn = !white_turn;
		gamestate.set_data(history.pop());
		mask_list.point_prev();
	}

	inline BitBoard orth_mask() { return mask_list.top()->pinmask_orth; }

	inline BitBoard diag_mask() { return mask_list.top()->pinmask_dg; }

	inline MaskSet* get_mask() { return mask_list.top(); }

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
