#include <cassert>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iostream>

#include "make_move.hpp"
#include "pyke.hpp"

using namespace Pyke;

const bool DEBUG = true;

template <bool root, bool white>
static uint64_t perft_rec(int depth, Position& pos) {
	uint64_t node_count = 0, nodes_found = 0;
	const bool leaf = (depth == 2);
	for (auto& m : generate_movelist<white>(pos)) {
		if (root && depth <= 1) {
			nodes_found = 1;
			node_count++;
		} else {
			Position copy = Position(pos);
			print_position(pos);
			make_move<white>(m, pos);
			nodes_found = leaf ? generate_movelist<white>(pos).size() : perft_rec<false, !white>(depth - 1, pos);
			unmake_move<white>(m, pos);
			assert(pos.is_equal(copy));
		}
		node_count += nodes_found;
		if (root) std::cout << move_to_string(m) << ": " << nodes_found << '\n';
	}
	return node_count;
}

static void perft(int depth, Position& pos) {
	clock_t start = clock();
	uint64_t nodes;
	if (pos.white_turn)
		nodes = perft_rec<true, true>(depth, pos);
	else
		nodes = perft_rec<true, false>(depth, pos);

	clock_t end = clock();
	double time_cost = double(end - start) / CLOCKS_PER_SEC;
	std::cout << "PERFT results: \nNodes evaluated: " << nodes << "\nTime cost: " << time_cost << '\n';
	std::cout << std::round((nodes / 1000000)) / time_cost << " Million nodes per second" << '\n';
	std::cout << "================================================================================ \n";
}
