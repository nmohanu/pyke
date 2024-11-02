#include <iostream>

#include "position.hpp"

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";

	Position position;
	position.print_to_terminal();
	position.perft(6);
	position.print_to_terminal();
	return 0;
}
