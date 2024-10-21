#include <cstdint>

#include "defaults.hpp"

#ifndef POSITION_H
#define POSITION_H

class Position {
public:
	Position();
	~Position();

	// Print board to terminal.
	void print();

private:
	// Array with the bitboards, these define the current position.
	uint64_t bit_boards[14] = {
		KING_SQUARES & ~BLACK_PIECES,	 // W_KING
		QUEEN_SQUARES & ~BLACK_PIECES,	 // W_QUEEN
		ROOK_SQUARES & ~BLACK_PIECES,	 // W_ROOK
		BISHOP_SQUARES & ~BLACK_PIECES,	 // W_BISHOP
		KNIGHT_SQUARES & ~BLACK_PIECES,	 // W_KNIGHT
		PAWN_SQUARES & ~BLACK_PIECES,	 // W_PAWN
		KING_SQUARES& BLACK_PIECES,		 // B_KING
		QUEEN_SQUARES& BLACK_PIECES,	 // B_QUEEN
		ROOK_SQUARES& BLACK_PIECES,		 // B_ROOK
		BISHOP_SQUARES& BLACK_PIECES,	 // B_BISHOP
		KNIGHT_SQUARES& BLACK_PIECES,	 // B_KNIGHT
		PAWN_SQUARES& BLACK_PIECES,		 // B_PAWN
		TOTAL_SQUARES,					 // All pieces
		BLACK_PIECES					 // Black pieces.
	};

	// Keep track of player at turn.
	bool white_to_turn = true;

	// From left to right:
	// white kingside, white queenside, black kingside, black queenside.
	uint8_t casling_rights = 0b0000'1111;

	// left most bit indicates whether an passant comes from left or right file.
	// Second bit is the color sign of the pawn that can be captured.
	// the right most bits indicate the file on which an passant is captured.
	uint8_t en_passant = 0b00000000;

	uint64_t get_pawn_move(uint8_t square, bool is_black);
	uint64_t get_rook_move(uint8_t square, bool is_black);
	uint64_t get_bishop_move(uint8_t square, bool is_black);
	uint64_t get_queen_move(uint8_t square, bool is_black);
	uint64_t get_king_move(uint8_t square, bool is_black);
	uint64_t get_knight_move(uint8_t square, bool is_black);

	uint64_t (Position::* move_functions[6])(uint8_t, bool) = {
		&Position::get_pawn_move,
		&Position::get_rook_move,
		&Position::get_bishop_move,
		&Position::get_queen_move,
		&Position::get_king_move,
		&Position::get_knight_move
	};
};

#endif	// !POSITION_H
