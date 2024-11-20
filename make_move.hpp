
#include <cstdint>

#include "pyke.hpp"

using namespace Pyke;

#ifndef MAKE_MOVE_H
#define MAKE_MOVE_H

// Add piece to board.
template <bool white, Piece p>
static void add_to_board(Board& b, Square s) {
	BitBoard mask = square_to_mask(s);
	*b.get_board_pointer<white, p>() |= mask;
	b.occ_board |= mask;
	if constexpr (white)
		b.w_board |= mask;
	else
		b.b_board |= mask;
}

// Remove piece from board.
template <bool white, Piece p>
static void remove_from_board(Board& b, Square s) {
	BitBoard mask = ~square_to_mask(s);
	*b.get_board_pointer<white, p>() &= mask;
	b.occ_board &= mask;
	if constexpr (white)
		b.w_board &= mask;
	else
		b.b_board &= mask;
}

// Move a piece.
template <bool white, Piece p>
static void move_piece(Square from, Square to, Board& b) {
	remove_from_board<white, p>(b, from);
	add_to_board<white, p>(b, to);
}

// Undo a piece move.
template <bool white, Piece p>
static void unmake_move_piece(Square from, Square to, Board& b) {
	remove_from_board<white>(b, to, p);
	add_to_board<white>(b, from, p);
}

// Do a plain, non capturing move.
template <bool white, Piece p>
static void plain_move(Move& m, Board& b) {
	move_piece<white, p>(m.get_from(), m.get_to(), b);
}

// Undo a non capturing move.
template <bool white, Piece p>
static void unmake_plain_move(Move& m, Board& b) {
	unmake_move_piece<white, p>(m.get_from(), m.get_to(), b);
}

// Castle and update castling rights.
template <bool white, uint8_t code>
static void castle_move(Board& b, GameState& gamestate) {
	move_piece<white>(king_start_squares[code], king_end_squares[code], KING, b);
	move_piece<white>(rook_start_squares[code], rook_end_squares[code], ROOK, b);
	gamestate.rm_cr<white>();
}

// Undo castling move.
template <bool white, uint8_t code>
static void unmake_castle_move(Board& b) {
	unmake_move_piece<white>(king_start_squares[code], king_end_squares[code], KING, b);
	unmake_move_piece<white>(rook_start_squares[code], rook_end_squares[code], ROOK, b);
}

// Do capture move.
template <bool white, Piece p, Piece captured>
static void capture_move(Move& move, Board& board) {
	move.set_content(captured);
	remove_from_board<!white, p>(board, move.get_to());
	move_piece<white>(move.get_from(), move.get_to(), move.get_piece(), board);
}

// Undo capture move.
template <bool white, Piece p, Piece captured>
static void unmake_capture_move(Move& m, Board& b) {
	unmake_move_piece<white, p>(m.get_from(), m.get_to(), b);
	add_to_board<!white, captured>(b, m.get_to());
}

// Do ep move.
template <bool white>
static void ep_move(Move& m, Board& b) {
	Square captured_sq = white ? (m.get_to() - 8) : (m.get_to() + 8);
	move_piece<white, PAWN>(m.get_from(), m.get_to(), b);
	remove_from_board<!white, PAWN>(b, captured_sq);
}

// Undo ep move.
template <bool white>
static void unmake_ep_move(Move& m, Board& b) {
	Square captured_sq = white ? (m.get_to() - 8) : (m.get_to() + 8);
	unmake_move_piece<white, PAWN>(m.get_from(), m.get_to(), PAWN, b);
	add_to_board<white, PAWN>(b, captured_sq);
}

// Do promo move.
template <bool white, Piece p, uint16_t data>
static void promo_move(Move& m, Board& b) {
	// Check if promotion was also a capture.
	constexpr Piece captured = data & 0b111;
	if constexpr (captured) {
		Piece captured = data & 0b111;
		m.set_content(captured);
		remove_from_board<white, captured>(b, m.get_to());
	}
	remove_from_board<white, PAWN>(b, m.get_from());
	add_to_board<white, p>(b, m.get_to());
}

// Undo promo move.
template <bool white, Piece promo, bool captured>
static void unmake_promo_move(Move& m, Board& b) {
	add_to_board<white, PAWN>(b, m.get_from(), PAWN);
	remove_from_board<white, promo>(b, m.get_to());

	// Check if promotion was also a capture.
	if constexpr (captured != EMPTY) {
		add_to_board<white, captured>(b, m.get_to(), m.get_content());
	}
}

// Do pawn double forward move.
template <bool white>
static void pawn_double(Move& m, Board& b, GameState& gamestate) {
	move_piece<white, PAWN>(m.get_from(), m.get_to(), b);

	// pawn moved two forward, update en passant status.
	bool left_is_pawn = b.get_piece<white>(m.get_to() - 1) == PAWN;
	bool right_is_pawn = b.get_piece<white>(m.get_to() + 1) == PAWN;
	bool left_piece_sign = square_to_mask(m.get_to() - 1) & b.w_board;
	bool right_piece_sign = square_to_mask(m.get_to() + 1) & b.w_board;
	bool left_is_opp_pawn = left_is_pawn && (left_piece_sign != white);
	bool right_is_opp_pawn = right_is_pawn && (right_piece_sign != white);

	// Edge of board cases.
	bool not_edge_left = m.get_to() % 8 != 0;
	bool not_edge_right = m.get_to() % 8 != 7;

	// Check if en passant is possible.
	bool possible_left = left_is_opp_pawn && not_edge_left;
	bool possible_right = right_is_opp_pawn && not_edge_right;

	// Update flag.
	uint8_t file = m.get_to() % 8;
	if (possible_left) gamestate.set_en_passant(possible_left, file);
	if (possible_right) gamestate.set_en_passant(possible_right, file);
}

// Undo double pawn move.
template <bool white>
static void unmake_pawn_double(Move& m, Board& b) {
	unmake_move_piece<white, PAWN>(m.get_from(), m.get_to(), b);
}

template <bool white, MoveType type, Piece p, uint8_t data>
void make_move(Move& m, Position& pos) {
	pos.history.push(pos.gamestate);

	constexpr uint8_t cap = data & 0b111;
	switch (type) {
	case MoveType::PLAIN:
		if constexpr (cap)
			capture_move<white, p, data>(m, pos.board);
		else
			plain_move<white, p>(m, pos.board);
		break;
	case MoveType::CASTLE:
		castle_move<white, cap>(m, pos.board, pos.gamestate);
		break;
	case MoveType::PAWN_MOVE:
		switch (cap) {
		case 0:
			ep_move<white>(m, pos.board);
			break;
		case 1:
			pawn_double<white>(m, pos.board, pos.gamestate);
			break;
		case 2:
			promo_move<white, p>(m, pos.board);
			break;
		}
		break;
	case MoveType::KING_MOVE:
		if constexpr (data & 0b111)
			capture_move<white, KING, data>(m, pos.board);
		else
			plain_move<white, KING>(m, pos.board);
		// Remove castling rights.
		pos.gamestate.rm_cr<white>();
	case MoveType::ROOK_MOVE:
		if constexpr (data & 0b111)
			capture_move<white, ROOK, data>(m, pos.board);
		else
			plain_move<white, ROOK>(m, pos.board);
		// Remove castling rights.
	}

	pos.white_turn = !pos.white_turn;
}

template <bool white>
void unmake_move(Move& move, Position& pos) {
	pos.white_turn = !pos.white_turn;

	switch (move.get_type()) {
	case 0:
		unmake_plain_move<white>(move, pos.board);
		break;
	case 1:
		unmake_castle_move<white>(move, pos.board);
		break;
	case 2:
		unmake_capture_move<white>(move, pos.board);
		break;
	case 3:
		unmake_ep_move<white>(move, pos.board);
		break;
	case 4:
		unmake_pawn_double<white>(move, pos.board, pos.gamestate);
		break;
	case 5:
		unmake_promo_move<white, false>(move, pos.board);
		break;
	case 6:
		unmake_promo_move<white, true>(move, pos.board);
		break;
	}

	pos.gamestate = pos.history.pop();
}

#endif
