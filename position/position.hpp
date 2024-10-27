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
	void pawn_2frwrd_move(Move move);
	void king_move(Move move);
	void king_take_move(Move move);
	void rook_move(Move move);

	void (Position::* do_move_functions[9])(Move) = {
		&Position::plain_move,
		&Position::castle_move,
		&Position::capture_move,
		&Position::en_passant_move,
		&Position::promotion_move,
		&Position::pawn_2frwrd_move,
		&Position::king_move,
		&Position::king_take_move,
		&Position::rook_move,
	};

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

	// Stacks.
	// History of position flags.
	// [11:8]: castling rights.
	// [6:0]: EP status.
	Stack<uint16_t> flag_history;

	// List of moves possible in current position.
	// Types: 0 = plain move, 1 = castle, 2 = capture, 3 = EP, 4 = promotion,
	// 5 = pawn two forward, 6 = king move, 7 = king take, 8 = rook move.
	//	move	sign&piece    castle/captured/promote    type
	// [31:16]    [15:12]            [11:8]              [4:0]
	Stack<uint32_t> possible_moves;

	// Default position flag.
	uint32_t pos_flag = 0b1111 << 8;

	// Inline functions.

	// Add piece to bitboards.
	inline void add_to_board(uint8_t piece, int square) {
		bool piece_sign = get_piece_sign(piece);
		uint64_t m = square_to_mask(square);
		get_board(piece) |= m;
		TOTAL_BOARD |= m;
		BLACK_PIECE_BOARD |= (m & piece_sign);
	}

	// Remove a piece from bitboards.
	inline void remove_from_board(uint8_t piece, int square) {
		bool piece_sign = get_piece_sign(piece);
		uint64_t m_not = ~square_to_mask(square);
		get_board(piece) &= m_not;
		TOTAL_BOARD &= m_not;

		// Add to black piece board if piece is black.
		BLACK_PIECE_BOARD &= m_not | ((1ULL & piece_sign) << square_to_shamt(square));
	}

	// Move piece from a to b.
	inline void move_piece(Square from, Square to, Piece piece) {
		// Update piece board.
		remove_from_board(piece, from);
		add_to_board(piece, to);
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
};

#endif	// !POSITION_H
