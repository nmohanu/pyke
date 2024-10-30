#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

#include "stack.hpp"
#include "util.hpp"

#ifndef POSITION_H
#define POSITION_H

class Position {
public:
	Position();
	~Position();

	// Print board to terminal.
	void print();

	// Generate possible moves and add them to the stack.
	MoveList<Move> generate_move_list();

	// PERFT test.
	void perft(int depth);

	// Print position.
	void print_to_terminal();

	// Do a move on the board.
	void do_move(Move move);
	void undo_move(Move move);

	Move str_to_move(std::string m);

	// private:
	template <bool root>
	uint64_t perft_rec(int depth);

	// Return move in string format.
	std::string move_to_string(Move move);

	// Different functions for different move types.
	void gen_type_plain_and_capture(Board player_piece_board, Board enemy_board);
	void gen_type_castle(Board player_piece_board, Board enemy_board);
	void gen_type_ep(Board player_piece_board, Board enemy_board);
	void gen_type_pawn(Board player_piece_board, Board enemy_board);

	// Make a move board for given piece at given square.
	Board make_reach_board(Square s, uint8_t piece_type);

	// Checks if a move is legal.
	bool move_legal(Move move, Board enemy_board);

	// Check if a square is under attack.
	bool is_attacked(Square square, Board enemy_board);

	// Specific move functions based on move type.
	void plain_move(Move move);
	void castle_move(Move move);
	void capture_move(Move move);
	void en_passant_move(Move move);
	void promotion_move(Move move);
	void king_move(Move move);
	void rook_move(Move move);
	void pawn_2frwrd_move(Move move);

	// Undo moves.
	void undo_plain_move(Move move);
	void undo_castle_move(Move move);
	void undo_capture_move(Move move);
	void undo_en_passant_move(Move move);
	void undo_promotion_move(Move move);

	// Returns the piece on given square.
	uint8_t get_piece(Square s) const;

	// Array with the bitboards, these define the current position.
	Board bit_boards[8] = {
		PAWN_SQUARES,
		KING_SQUARES,
		ROOK_SQUARES,
		BISHOP_SQUARES,
		KNIGHT_SQUARES,
		QUEEN_SQUARES,
		TOTAL_SQUARES,
		BLACK_PIECES
	};

	// Keep track of sign of player at turselect linen. (black = true).
	bool player_sign = false;

	// Move functions.
	Board get_pawn_move(Square s, Flag player_sign);
	Board get_rook_move(Square s);
	Board get_bishop_move(Square s);
	Board get_queen_move(Square s);
	Board get_king_move(Square s);
	Board get_knight_move(Square s);

	// Color specific pawn move functions.
	Board get_pawn_move_black(Square s, bool is_black);
	Board get_pawn_move_white(Square s, bool is_black);

	// Returns the position of the king of player at move.
	Square get_king_pos(bool sign);

	// Stacks.
	// History of position flags.
	// [11:8]: castling rights.
	// [6:0]: EP status.
	Stack<uint16_t> flag_history;

	// List of moves possible in current position.
	// Types: 0 = plain move, 1 = castle, 2 = capture, 3 = EP, 4 = promotion,
	// 5 = pawn two forward.
	//	move	   piece		castle/captured/promote			 type
	// [31:16]    [15:13]            [11:8]						[4:0]
	Stack<Move> possible_moves;

	// Default position flag.
	uint16_t pos_flag = 0b1111'0000'0000;

	// Inline functions.

	// Add piece to bitboards.
	inline void add_to_board(Piece piece, int square, bool piece_sign) {
		Board m = square_to_mask(square);
		get_board(piece) |= m;
		TOTAL_BOARD |= m;
		if (piece_sign) BLACK_PIECE_BOARD |= m;
	}

	// Remove a piece from bitboards.
	inline void remove_from_board(Piece piece, int square, bool piece_sign) {
		Board m_not = ~square_to_mask(square);
		get_board(piece) &= m_not;
		TOTAL_BOARD &= m_not;

		// Remove from black piece board if piece is black.
		if (piece_sign) BLACK_PIECE_BOARD &= m_not;
	}

	// Move piece from a to b.
	inline void move_piece(Square from, Square to, Piece piece, bool piece_sign) {
		// Update piece board.
		remove_from_board(piece, from, piece_sign);
		add_to_board(piece, to, piece_sign);
	}

	// Take away castling rights. takes 2 bits as argument.
	// e.g. mask = 0b01 takes away queenside castling rights.
	// 0b11 takes both queenside and kingside castling rights.
	inline void remove_castling_right(bool color_sign, uint8_t mask) {
		// Isolate castling rights field.
		uint8_t castling_rights = pos_flag >> 8;

		// Toggle off the bits.
		mask = ~mask;
		mask <<= color_sign * 2;
		castling_rights &= mask;

		// Update position flags.
		set_castling(castling_rights);
	}

	inline void push_if_legal(Move move, Board enemy_board) {
		if (move_legal(move, enemy_board)) possible_moves.push(move);
	}

	int captures = 0;
	int ep_moves = 0;
	int castles = 0;
	int promotions = 0;
	int white_moves = 0;
	int black_moves = 0;
};

#endif	// !POSITION_H
