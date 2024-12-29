#include <ios>
#include <iostream>
#include <string>
#include <vector>

#include "perft.hpp"
#include "pyke.hpp"

using namespace pyke;

const std::vector<std::string> moves = {};
const int PERFT_TARGET = 8;

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";
	Position pos;
	for (auto& m : moves) {
		move_from_string(m, pos);
	}
	// pawn_double<false>(notation_to_square("b7"), notation_to_square("b5"), pos);
	// ep_move<true>(notation_to_square("a5"), notation_to_square("b6"), pos);
	// std::cout << unsigned(pos.gamestate.get_data()) << '\n';
	pos.board.print_board();
	perft(pos, PERFT_TARGET - moves.size());
	pos.board.print_board();

	return 0;
}
