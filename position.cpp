#include "position.hpp"

#include <iostream>

bool Position::is_equal(Position& other) {
	return (white_turn == other.white_turn) && (gamestate.get_data() == other.gamestate.get_data())
		&& board.is_equal(other.board);
}
