
#include <cassert>
#include <cstdint>
#include <ios>
#include <iostream>
#include <string>

#include "defaults.hpp"
#include "position.hpp"

#ifndef MAKE_MOVE_H
#define MAKE_MOVE_H

// Add piece to board.
template <bool white, Piece p>
static void add_to_board(Board& b, BitBoard mask) {
	*b.get_board_pointer<white, p>() |= mask;
	b.occ_board |= mask;
	if constexpr (white)
		b.w_board |= mask;
	else
		b.b_board |= mask;
}

// Remove piece from board.
template <bool white, Piece p>
static void remove_from_board(Board& b, BitBoard mask) {
	*b.get_board_pointer<white, p>() ^= mask;
	b.occ_board ^= mask;
	if constexpr (white)
		b.w_board ^= mask;
	else
		b.b_board ^= mask;
}

// Move a piece.
template <bool white, Piece p>
static void move_piece(Board& b, BitBoard move) {
	*b.get_board_pointer<white, p>() ^= move;
	b.occ_board ^= move;
	if constexpr (white)
		b.w_board ^= move;
	else
		b.b_board ^= move;
}

// Undo a piece move.
template <bool white, Piece p>
static void unmake_move_piece(Board& b, BitBoard move) {
	move_piece<white, p>(b, move);
}

// Do a plain, non capturing move.
template <bool white, Piece p>
static void plain_move(Board& b, BitBoard move) {
	move_piece<white, p>(b, move);
}

// Undo a non capturing move.
template <bool white, Piece p>
static void unmake_plain_move(Board& b, BitBoard move) {
	move_piece<white, p>(b, move);
}

// Castle and update castling rights.
template <bool white, uint8_t code>
static void castle_move(Board& b) {
	move_piece<white, KING>(b, square_to_mask(king_start_squares[code]) | square_to_mask(king_end_squares[code]));
	move_piece<white, ROOK>(b, square_to_mask(rook_start_squares[code]) | square_to_mask(rook_end_squares[code]));
}

// Undo castling move.
template <bool white, uint8_t code>
static void unmake_castle_move(Board& b) {
	unmake_move_piece<white, KING>(
		b, square_to_mask(king_start_squares[code]) | square_to_mask(king_end_squares[code])
	);
	unmake_move_piece<white, ROOK>(
		b, square_to_mask(rook_start_squares[code]) | square_to_mask(rook_end_squares[code])
	);
}

// Do capture move.
template <bool white, Piece p, Piece captured>
static void capture_move(Board& b, BitBoard move, BitBoard to) {
	remove_from_board<!white, captured>(b, to);
	move_piece<white, p>(b, move);
}

// Undo capture move.
template <bool white, Piece p, Piece captured>
static void unmake_capture_move(Board& b, BitBoard move, BitBoard to) {
	unmake_move_piece<white, p>(b, move);
	add_to_board<!white, captured>(b, to);
}

// Do ep move.
template <bool white>
static void ep_move(Board& b, BitBoard move, BitBoard capture_sq) {
	move_piece<white, PAWN>(b, move);
	remove_from_board<!white, PAWN>(b, capture_sq);
}

// Undo ep move.
template <bool white>
static void unmake_ep_move(Board& b, BitBoard move, BitBoard capture_sq) {
	unmake_move_piece<white, PAWN>(b, move);
	add_to_board<!white, PAWN>(b, capture_sq);
}

// Do promo move.
template <bool white, Piece p, Piece captured>
static void promo_move(Square from, Square to, Position& pos) {
	// Check if promotion was also a capture.
	if constexpr (captured != EMPTY) remove_from_board<white, captured>(pos.board, to);
	remove_from_board<white, PAWN>(pos.board, from);
	add_to_board<white, p>(pos.board, to);
}

// Undo promo move.
template <bool white, Piece p, Piece captured>
static void unmake_promo_move(Square from, Square to, Position& pos) {
	add_to_board<white, PAWN>(pos.board, from, PAWN);
	remove_from_board<white, p>(pos.board, to);
	// Check if promotion was also a capture.
	if constexpr (captured != EMPTY) add_to_board<white, captured>(pos.board, to);
}

// Do pawn double forward move.
template <bool white>
static bool pawn_double(Board& b, BitBoard move, BitBoard to, uint8_t& ep_flag) {
	move_piece<white, PAWN>(b, move);

	// pawn moved two forward, update en passant status.
	bool left_is_opp_pawn = b.get_piece_board<!white, PAWN>() & (to << 1);
	bool right_is_opp_pawn = b.get_piece_board<!white, PAWN>() & (to >> 1);
	File file = lbit(to) % 8;

	// Check if en passant is possible.
	bool possible_left = left_is_opp_pawn && !(file == 0);
	bool possible_right = right_is_opp_pawn && !(file == 7);

	// Update flag.
	ep_flag = 0;
	if (possible_left) set_en_passant(true, file, ep_flag);
	if (possible_right) set_en_passant(false, file, ep_flag);

	return possible_left || possible_right;
}

// Undo double pawn move.
template <bool white>
static void unmake_pawn_double(Board& b, BitBoard move) {
	unmake_move_piece<white, PAWN>(b, move);
}

template <bool white, Piece p>
static void capture_move_wrapper(Board& b, Piece captured, BitBoard move, BitBoard capture_sq) {
	switch (captured) {
	case PAWN:
		capture_move<white, p, PAWN>(b, move, capture_sq);
		break;
	case KNIGHT:
		capture_move<white, p, KNIGHT>(b, move, capture_sq);
		break;
	case BISHOP:
		capture_move<white, p, BISHOP>(b, move, capture_sq);
		break;
	case ROOK:
		capture_move<white, p, ROOK>(b, move, capture_sq);
		break;
	case QUEEN:
		capture_move<white, p, QUEEN>(b, move, capture_sq);
		break;
	}
}

template <bool white, Piece p>
static void unmake_capture_wrapper(Board& b, Piece captured, BitBoard move, BitBoard capture_sq) {
	switch (captured) {
	case PAWN:
		unmake_capture_move<white, p, PAWN>(b, move, capture_sq);
		break;
	case KNIGHT:
		unmake_capture_move<white, p, KNIGHT>(b, move, capture_sq);
		break;
	case BISHOP:
		unmake_capture_move<white, p, BISHOP>(b, move, capture_sq);
		break;
	case ROOK:
		unmake_capture_move<white, p, ROOK>(b, move, capture_sq);
		break;
	case QUEEN:
		unmake_capture_move<white, p, QUEEN>(b, move, capture_sq);
		break;
	}
}

// Add piece to board.
static void add_to_board(Board& b, Square s, Piece p, bool white) {
	BitBoard mask = square_to_mask(s);
	*b.get_board_pointer(white, p) |= mask;
	b.occ_board |= mask;
	if (white)
		b.w_board |= mask;
	else
		b.b_board |= mask;
}

// Remove piece from board.
static void remove_from_board(Board& b, Square s, Piece p, bool white) {
	BitBoard mask = ~square_to_mask(s);
	*b.get_board_pointer(white, p) &= mask;
	b.occ_board &= mask;
	if (white)
		b.w_board &= mask;
	else
		b.b_board &= mask;
}

// Move a piece.
static void move_piece(Square from, Square to, Board& b, bool white, Piece p) {
	remove_from_board(b, from, p, white);
	add_to_board(b, to, p, white);
}

static void move_from_string(std::string m, Position& p) {
	Square from = notation_to_square(m.substr(0, 2));
	Square to = notation_to_square(m.substr(2, 2));
	std::cout << "move " << m << " from " << unsigned(from) << " to " << unsigned(to) << '\n';
	bool white = p.white_turn;
	Piece captured_b = p.board.get_piece<true>(to);
	Piece captured_w = p.board.get_piece<false>(to);
	std::cout << "ON MOVE: " << white << '\n';
	Piece move_w = p.board.get_piece<true>(from);

	if (captured_w != EMPTY) {
		remove_from_board(p.board, to, captured_w, true);
	} else if (captured_b != EMPTY) {
		remove_from_board(p.board, to, captured_b, false);
	}

	move_piece(from, to, p.board, white, !white ? p.board.get_piece<false>(from) : move_w);

	p.white_turn = !p.white_turn;
}

#endif
