#include <cstdint>

#include "defaults.hpp"
#include "stack.hpp"

#ifndef POSITION_H
#define POSITION_H

class Position {
public:
	Position();
	~Position();

	// Print board to terminal.
	void print();

private:
	// Make a move board for given piece at given square.
	uint64_t make_reach_board(Square s, bool is_black, uint8_t piece_type);

	// Do a move on the board.
	void do_move(Move move);

	// Specific move functions based on move type.
	void plain_move(Move move);
	void castle_move(Move move);
	void capture_move(Move move);
	void en_passant_move(Move move);
	void promotion_move(Move move);

	void (Position::* do_move_functions[5])(Move) = {
		&Position::plain_move,
		&Position::castle_move,
		&Position::capture_move,
		&Position::en_passant_move,
		&Position::promotion_move
	};

	void move_piece(Square from, Square to, Piece piece);

	// Returns the piece on given square.
	uint8_t get_piece(Square s) const;

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

	// Keep track of sign of player at turn. (black = true).
	bool player_sign;

	// Move functions.
	uint64_t get_pawn_move(Square s, bool is_black);
	uint64_t get_rook_move(Square s, bool is_black);
	uint64_t get_bishop_move(Square s, bool is_black);
	uint64_t get_queen_move(Square s, bool is_black);
	uint64_t get_king_move(Square s, bool is_black);
	uint64_t get_knight_move(Square s, bool is_black);

	// Color specific pawn move functions.
	uint64_t get_pawn_move_black(Square s, bool is_black);
	uint64_t get_pawn_move_white(Square s, bool is_black);

	// Fast indexing of the needed move board generator.
	uint64_t (Position::* move_functions[8])(uint8_t, bool) = {
		&Position::get_pawn_move,
		&Position::get_rook_move,
		&Position::get_bishop_move,
		&Position::get_queen_move,
		&Position::get_king_move,
		&Position::get_knight_move,
		&Position::get_pawn_move_white,
		&Position::get_pawn_move_black
	};

	// Stacks.
	// History of position flags.
	// [11:8]: castling rights.
	// [6:0]: EP status.
	Stack<uint16_t> flag_history;

	// List of moves possible in current position.
	// Types: 0 = plain move, 1 = castle, 2 = capture, 3 = EP, 4 = promotion.
	//	move	sign&piece    castle/captured/promote    type
	// [31:16]    [15:12]            [11:8]              [2:0]
	Stack<uint32_t> possible_moves;

	// Default position flag.
	uint32_t pos_flag = 0b1111 << 8;
};

#endif	// !POSITION_H
