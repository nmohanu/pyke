#include <cstdint>

#include "board.hpp"
#include "gamestate.hpp"
#include "maskset.hpp"
#include "move.hpp"
#include "stack.hpp"

#ifndef POSITION_H
#define POSITION_H

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
	Stack<Move> movelist;
	Stack<MaskSet*> masks;
	bool white_turn = true;
	MaskSet* msk;

	bool is_equal(Position& other);

	inline void moved() {
		white_turn = !white_turn;
		history.push(gamestate.get_data());
		masks.push(msk);
		gamestate.reset_en_passant();
	}
	inline void unmoved() {
		white_turn = !white_turn;
		gamestate.set_data(history.pop());
		msk = masks.pop();
	}

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

	void print_position();
};

#endif
