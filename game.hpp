#include "position/position.hpp"

class Game {
public:
	Game();
	~Game();
	// Play game.
	void play();

private:
	Position* position;
};
