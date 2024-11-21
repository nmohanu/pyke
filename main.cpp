#include <iostream>

#include "perft.hpp"
#include "pyke.hpp"

using namespace pyke;

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";
	Position pos;
	pos.print_position();
	perft(pos, 1);
	pos.print_position();
	return 0;
}
