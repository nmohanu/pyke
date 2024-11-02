#include "defaults.hpp"
#include "piece_moves.hpp"

struct Board {
	BitBoard w_pawn = INIT_PAWN_SQUARES & INIT_WHITE_PIECES;
	BitBoard w_king = INIT_KING_SQUARES & INIT_WHITE_PIECES;
	BitBoard w_rook = INIT_ROOK_SQUARES & INIT_WHITE_PIECES;
	BitBoard w_bishop = INIT_BISHOP_SQUARES & INIT_WHITE_PIECES;
	BitBoard w_knight = INIT_KNIGHT_SQUARES & INIT_WHITE_PIECES;
	BitBoard w_queen = INIT_QUEEN_SQUARES & INIT_WHITE_PIECES;

	BitBoard b_pawn = INIT_PAWN_SQUARES & INIT_BLACK_PIECES;
	BitBoard b_king = INIT_KING_SQUARES & INIT_BLACK_PIECES;
	BitBoard b_rook = INIT_ROOK_SQUARES & INIT_BLACK_PIECES;
	BitBoard b_bishop = INIT_BISHOP_SQUARES & INIT_BLACK_PIECES;
	BitBoard b_knight = INIT_KNIGHT_SQUARES & INIT_BLACK_PIECES;
	BitBoard b_queen = INIT_QUEEN_SQUARES & INIT_BLACK_PIECES;

	BitBoard w_board = INIT_WHITE_PIECES;
	BitBoard b_board = INIT_BLACK_PIECES;

	BitBoard occ_board = INIT_TOTAL_SQUARES;

	BitBoard w_king_dg = get_bishop_move(60, occ_board);
	BitBoard w_king_hz = get_rook_move(60, occ_board);

	BitBoard b_king_dg = get_bishop_move(4, occ_board);
	BitBoard b_king_hz = get_rook_move(4, occ_board);
};
