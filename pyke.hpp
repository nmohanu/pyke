#include "board.hpp"
#include "stack.hpp"
#include "util.hpp"

namespace Pyke {

struct MoveGenerator {
	Stack<Move> movelist;

	template <bool white>
	MoveList<Move> generate_movelist(Board& board, GameState gamestate);
};

};	// namespace Pyke
