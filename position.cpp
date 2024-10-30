#include "position.hpp"

#include <sys/types.h>

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

// Position constructor.
Position::Position() {}
// Position destructor.
Position::~Position() {}

void Position::perft(int depth) {
	clock_t start = clock();
	uint64_t nodes = perft_rec<true>(depth);

	clock_t end = clock();
	double time_cost = double(end - start) / CLOCKS_PER_SEC;
	std::cout << "PERFT results: \nNodes evaluated: " << nodes << "\nTime cost: " << time_cost << '\n';
	std::cout << std::round((nodes / 1000000)) / time_cost << " Million nodes per second" << '\n';
	std::cout << "Captures: " << captures << ", castles: " << castles << ", ep: " << ep_moves
			  << ", promotions:" << promotions << '\n';
	std::cout << "All moves cleared? " << (possible_moves.top() == possible_moves.stack) << '\n';
	std::cout << "================================================================================ \n";
}

template <bool root>
uint64_t Position::perft_rec(int depth) {
	uint64_t node_count = 0, nodes_found = 0;
	const bool leaf = (depth == 2);

	for (const auto& m : generate_move_list()) {
		if (root && depth <= 1) {
			nodes_found = 1;
			node_count++;
		} else {
			do_move(m);
			nodes_found = leaf ? generate_move_list().size() : perft_rec<false>(depth - 1);
		}
		node_count += nodes_found;
		undo_move(m);
		if (root) std::cout << move_to_string(m) << ": " << nodes_found << '\n';
	}
	return node_count;
}

std::string Position::move_to_string(Move move) {
	std::string start_notation = make_chess_notation(get_move_from(move));
	std::string destination_notation = make_chess_notation(get_move_to(move));
	std::string promotion_letter = "";
	if (get_move_type(move) == MOVE_PROMO) {
		switch (get_move_content(move)) {
		case 1:
			promotion_letter = "q";
			break;
		case 2:
			promotion_letter = "r";
			break;
		case 3:
			promotion_letter = "b";
			break;
		case 4:
			promotion_letter = "n";
			break;
		default:
			break;
		}
	}
	return start_notation + destination_notation + promotion_letter;
}

Move Position::str_to_move(std::string m) {
	Square from = notation_to_square(m.substr(0, 2));
	Square to = notation_to_square(m.substr(2, 2));

	Move ret = 0;
	set_move_piece(ret, get_piece(from));
	set_move_from(ret, from);
	set_move_to(ret, to);

	if (std::abs(to - from) == 16 && get_move_piece(ret) == PAWN) set_move_type(ret, MOVE_PAWN_DOUBLE);

	Piece captured = get_piece(to);
	if (captured != EMPTY) {
		set_move_type(ret, 2);
		set_move_content(ret, captured);
	}

	return ret;
}

// Get the piece on given square.
uint8_t Position::get_piece(Square square) const {
	Board bit_mask = 1ULL << (63 - square);

	if (!(TOTAL_BOARD & bit_mask)) return EMPTY;

	uint8_t piece = 0;
	for (int i = 0; i < 6; i++) piece += !(bit_boards[i] & bit_mask) && (i == piece);
	return piece;
}

Square Position::get_king_pos(bool sign) {
	Board king = KING_BOARD;
	Board king_black = KING_BOARD & BLACK_PIECE_BOARD;
	Board king_white = king & ~king_black;
	return __builtin_clzll(sign ? king_black : king_white);
}

// Print board to terminal.
void Position::print_to_terminal() {
	std::cout << "   a b c d e f g h" << std::endl;
	std::cout << " +-----------------+" << std::endl;
	for (int row = 7; row >= 0; row--) {
		std::cout << row + 1 << "| ";
		for (int col = 0; col < 8; col++) {
			Square square = (7 - row) * 8 + col;
			uint8_t piece_type = get_piece((7 - row) * 8 + col);
			char piece;
			switch (piece_type) {
			case KING:
				piece = 'K';
				break;
			case QUEEN:
				piece = 'Q';
				break;
			case ROOK:
				piece = 'R';
				break;
			case BISHOP:
				piece = 'B';
				break;
			case KNIGHT:
				piece = 'N';
				break;
			case PAWN:
				piece = 'P';
				break;
			case EMPTY:
				piece = ' ';
				break;
			default:
				piece = '?';  // Unknown piece type
				break;
			}
			if (square_to_mask(square) & BLACK_PIECE_BOARD) piece = tolower(piece);
			std::cout << piece << ' ';
		}
		std::cout << "|" << row + 1 << std::endl;
	}
	std::cout << " +-----------------+" << std::endl;
	std::cout << "   a b c d e f g h" << std::endl;
}
