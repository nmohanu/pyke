#include <stdexcept>

#include "defaults.hpp"
#include "util.hpp"

#ifndef BOARD_H
#define BOARD_H

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

	bool is_equal(const Board& other) const {
		return (w_pawn == other.w_pawn) && (w_king == other.w_king) && (w_rook == other.w_rook)
			&& (w_bishop == other.w_bishop) && (w_knight == other.w_knight) && (w_queen == other.w_queen)
			&& (b_pawn == other.b_pawn) && (b_king == other.b_king) && (b_rook == other.b_rook)
			&& (b_bishop == other.b_bishop) && (b_knight == other.b_knight) && (b_queen == other.b_queen)
			&& (w_board == other.w_board) && (b_board == other.b_board) && (occ_board == other.occ_board);
	}

	Board copy() const {
		Board new_board;
		new_board.w_pawn = w_pawn;
		new_board.w_king = w_king;
		new_board.w_rook = w_rook;
		new_board.w_bishop = w_bishop;
		new_board.w_knight = w_knight;
		new_board.w_queen = w_queen;

		new_board.b_pawn = b_pawn;
		new_board.b_king = b_king;
		new_board.b_rook = b_rook;
		new_board.b_bishop = b_bishop;
		new_board.b_knight = b_knight;
		new_board.b_queen = b_queen;

		new_board.w_board = w_board;
		new_board.b_board = b_board;
		new_board.occ_board = occ_board;

		return new_board;
	}

	// Returns the player's occupacion board.
	template <bool white>
	inline BitBoard get_player_occ() {
		return white ? w_board : b_board;
	}

	template <bool white, Piece p>
	inline BitBoard* get_board_pointer() {
		switch (p) {
		case PAWN:
			return white ? &w_pawn : &b_pawn;
		case KING:
			return white ? &w_king : &b_king;
		case ROOK:
			return white ? &w_rook : &b_rook;
		case BISHOP:
			return white ? &w_bishop : &b_bishop;
		case KNIGHT:
			return white ? &w_knight : &b_knight;
		case QUEEN:
		case QUEEN_DIAG:
		case QUEEN_ORTH:
			return white ? &w_queen : &b_queen;
		}
	}

	inline BitBoard* get_board_pointer(bool white, Piece p) {
		switch (p) {
		case PAWN:
			return white ? &w_pawn : &b_pawn;
		case KING:
			return white ? &w_king : &b_king;
		case ROOK:
			return white ? &w_rook : &b_rook;
		case BISHOP:
			return white ? &w_bishop : &b_bishop;
		case KNIGHT:
			return white ? &w_knight : &b_knight;
		case QUEEN:
			return white ? &w_queen : &b_queen;
		}
		throw std::invalid_argument("Board does not exist.");
	}

	// Gets the board for a given color and piece.
	template <bool white, Piece p>
	inline BitBoard get_piece_board() {
		return *get_board_pointer<white, p>();
	}

	// Returns the piece on the square or EMPTY by defualt.
	template <bool white>
	inline Piece get_piece(Square square) {
		BitBoard mask = square_to_mask(square);
		if (get_piece_board<white, PAWN>() & mask) return PAWN;
		if (get_piece_board<white, KNIGHT>() & mask) return KNIGHT;
		if (get_piece_board<white, BISHOP>() & mask) return BISHOP;
		if (get_piece_board<white, ROOK>() & mask) return ROOK;
		if (get_piece_board<white, QUEEN>() & mask) return QUEEN;
		if (get_piece_board<white, KING>() & mask) return KING;
		return EMPTY;
	}

	// Returns whether a square is occupied or not.
	inline bool square_occ(Square square) { return get_bit_64(occ_board, square); }

	void print_board();
};

#endif
