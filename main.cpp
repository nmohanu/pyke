#include <iostream>

#include "position.hpp"

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";

	Position position;
	//  position.do_move(position.str_to_move("d2d3"));
	//  position.do_move(position.str_to_move("a7a5"));
	//  position.do_move(position.str_to_move("d1d2"));
	position.print_to_terminal();
	position.perft(5);
	position.print_to_terminal();
	return 0;
}
