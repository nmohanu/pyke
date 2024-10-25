#include "game.hpp"

#include <iostream>

Game::Game() { position = nullptr; }

Game::~Game() {
	if (!position) return;
	delete position;
	position = nullptr;
}

void Game::play() {
	bool should_quit = false;
	while (!should_quit) {
		position->print();
		std::cout << "move <movestring>		Do move, e.g. a2a4 \n"
				  << "perft <depth>			Do perft test up to given depth. \n"
				  << "pyke get				Get best move. \n"
				  << "pyke do				Let engine do best move. \n"
				  << "quit					Exit program. \n";

		// Get user input.
		std::string command;
		std::cin >> command;

		if (command.substr(0, 5) == "perft") {
		}
	}
}
