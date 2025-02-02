#include "board.hpp"

void Board::print_board() {
	std::cout << "   a b c d e f g h" << std::endl;
	std::cout << " +-----------------+" << std::endl;
	for (int row = 7; row >= 0; row--) {
		std::cout << row + 1 << "| ";
		for (int col = 0; col < 8; col++) {
			Square square = (7 - row) * 8 + col;
			bool white = w_board & square_to_mask(square);
			uint8_t piece_type =
				white ? get_piece<true>(square_to_mask(square)) : get_piece<false>(square_to_mask(square));
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
				piece = '?';
				break;
			}
			if (square_to_mask(square) & b_board) piece = tolower(piece);
			std::cout << piece << ' ';
		}
		std::cout << "|" << row + 1 << std::endl;
	}
	std::cout << " +-----------------+" << std::endl;
	std::cout << "   a b c d e f g h" << std::endl;
}
