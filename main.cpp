#include <iostream>
#include <string>
#include <vector>

#include "perft.hpp"
#include "pyke.hpp"

using namespace pyke;

const std::vector<std::string> moves = {};
const int PERFT_TARGET = 6;

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";
	Position pos;
	for (auto& m : moves) {
		move_from_string(m, pos);
	}
	pos.print_position();
	perft(pos, PERFT_TARGET - moves.size());
	pos.print_position();
	print_bitboard(pos.board.w_pawn);
	print_bitboard(pos.board.b_pawn);
	return 0;
}
