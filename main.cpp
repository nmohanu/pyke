#include <iostream>

#include "position.hpp"

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";

	Position position;
	// position.do_move(position.str_to_move("d2d3"));
	// position.do_move(position.str_to_move("d7d5"));
	// position.do_move(position.str_to_move("e1d2"));
	// position.do_move(position.str_to_move("d5d4"));
	position.print_to_terminal();
	position.perft(5);
	position.print_to_terminal();
	return 0;
}
