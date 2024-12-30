#include <ios>
#include <iostream>
#include <string>
#include <vector>

#include "perft.hpp"
#include "pyke.hpp"

using namespace pyke;

const std::vector<std::string> moves = {};
const int PERFT_TARGET = 7;

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";
	Position pos;
	for (auto& m : moves) {
		move_from_string(m, pos);
	}
	pos.board.print_board();
	perft(pos, PERFT_TARGET - moves.size());
	pos.board.print_board();

	return 0;
}
