#include "board.hpp"
#include "gamestate.hpp"
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
	Stack<GameState> history;
	Stack<Move> movelist;
	bool white_turn = true;

	bool is_equal(Position& other);
	void moved();
	void unmoved();

	void print_position();
};

#endif
