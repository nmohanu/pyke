#include <iostream>

#include "perft.hpp"
#include "pyke.hpp"

using namespace Pyke;

int main(int argc, char* argv[]) {
	std::cout << "Pyke chess move generator by Nathanael Mohanu \n";
	Position pos;
	print_position(pos);
	perft(4, pos);
	return 0;
}
