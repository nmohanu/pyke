#include <cassert>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iostream>

#include "pyke.hpp"

template <int depth>
int perft_internal(Position& pos) {
	clock_t start = clock();
	uint64_t nodes;

	if (pos.white_turn)
		nodes = pyke::count_moves<true, depth>(pos);
	else
		nodes = pyke::count_moves<false, depth>(pos);

	clock_t end = clock();
	double time_cost = double(end - start) / CLOCKS_PER_SEC;
	std::cout << "PERFT results: \nNodes evaluated: " << nodes << "\nTime cost: " << time_cost << '\n';
	std::cout << std::round((nodes / 1000000)) / time_cost << " Million nodes per second" << '\n';
	std::cout << "================================================================================ \n";
	return nodes;
}

int perft(Position& pos, int depth) {
	switch (depth) {
	case 0:
		return 1;
	case 1:
		return perft_internal<1>(pos);
	case 2:
		return perft_internal<2>(pos);
	case 3:
		return perft_internal<3>(pos);
	case 4:
		return perft_internal<4>(pos);
	case 5:
		return perft_internal<5>(pos);
	case 6:
		return perft_internal<6>(pos);
	case 7:
		return perft_internal<7>(pos);
	case 8:
		return perft_internal<8>(pos);
	case 9:
		return perft_internal<9>(pos);
	case 10:
		return perft_internal<10>(pos);
	default:
		std::cout << "Depth above 10 not supported." << std::endl;
		return 0;
	}
}
