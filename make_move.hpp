
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
static void move_piece(Square from, Square to, Board& board) {
	remove_from_board<white, p>(board, from);
	add_to_board<white, p>(board, to);
}

// Undo a piece move.
template <bool white, Piece p>
static void unmake_move_piece(Square from, Square to, Board& board) {
	remove_from_board<white>(board, to, p);
	add_to_board<white>(board, from, p);
}

// Do a plain, non capturing move.
template <bool white, Piece p>
static void plain_move(Move& move, Board& board) {
	move_piece<white, p>(move.get_from(), move.get_to(), board);
}

// Undo a non capturing move.
template <bool white, Piece p>
static void unmake_plain_move(Move& move, Board& board) {
	unmake_move_piece<white, p>(move.get_from(), move.get_to(), board);
}

// Castle and update castling rights.
template <bool white, uint8_t code>
static void castle_move(Move& move, Board& board, GameState& gamestate) {
	move_piece<white>(king_start_squares[code], king_end_squares[code], KING, board);
	move_piece<white>(rook_start_squares[code], rook_end_squares[code], ROOK, board);
	gamestate.rm_cr<white>();
}

// Undo castling move.
template <bool white, uint8_t code>
static void unmake_castle_move(Move& move, Board& board) {
	unmake_move_piece<white>(king_start_squares[code], king_end_squares[code], KING, board);
	unmake_move_piece<white>(rook_start_squares[code], rook_end_squares[code], ROOK, board);
}

template <bool white>
static void capture_move(Move& move, Board& board) {
	Piece captured = get_piece<white>(move.get_to(), board);
	move.set_content(captured);
	remove_from_board<!white>(board, move.get_to(), captured);
	move_piece<white>(move.get_from(), move.get_to(), move.get_piece(), board);
}

template <bool white>
static void unmake_capture_move(Move& move, Board& board) {
	unmake_move_piece<white>(move.get_from(), move.get_to(), move.get_piece(), board);
	add_to_board<!white>(board, move.get_to(), move.get_content());
}

template <bool white>
static void ep_move(Move& move, Board& board) {
	Square captured_sq = white ? (move.get_to() - 8) : (move.get_to() + 8);
	move_piece<white>(move.get_from(), move.get_to(), PAWN, board);
	remove_from_board<white>(board, captured_sq, PAWN);
}

template <bool white>
static void unmake_ep_move(Move& move, Board& board) {
	Square captured_sq = white ? (move.get_to() - 8) : (move.get_to() + 8);
	unmake_move_piece<white>(move.get_from(), move.get_to(), PAWN, board);
	add_to_board<white>(board, captured_sq, PAWN);
}

template <bool white>
static void promo_move(Move& move, Board& board) {
	Piece new_piece = move.get_content();
	// Check if promotion was also a capture.
	if (board.square_occ(move.get_to())) {
		move.set_content(get_piece<!white>(move.get_to(), board));
		remove_from_board<white>(board, move.get_to(), move.get_content());
	}

	remove_from_board<white>(board, move.get_from(), PAWN);
	add_to_board<white>(board, move.get_to(), move.get_content());
}

template <bool white, bool capture>
static void unmake_promo_move(Move& move, Board& board) {
	add_to_board<white>(board, move.get_from(), PAWN);
	Piece promo_piece = get_piece<white>(move.get_to(), board);
	remove_from_board<white>(board, move.get_to(), promo_piece);

	// Check if promotion was also a capture.
	if constexpr (capture) {
		add_to_board<white>(board, move.get_to(), move.get_content());
		move.set_content(promo_piece);
	}
}

template <bool white>
static void pawn_double(const Move& move, Board& board, GameState& gamestate) {
	move_piece<white>(move.get_from(), move.get_to(), PAWN, board);

	// pawn moved two forward, update en passant status.
	Square square_left = move.get_to() - 1;
	Square square_right = move.get_to() + 1;
	Piece piece_left = get_piece<white>(square_left, board);
	Piece piece_right = get_piece<white>(square_right, board);
	Piece moving_piece = PAWN;
	bool left_is_pawn = piece_left == PAWN;
	bool right_is_pawn = piece_right == PAWN;
	bool left_piece_sign = square_to_mask(square_left) & board.w_board;
	bool right_piece_sign = square_to_mask(square_right) & board.w_board;
	bool left_is_opp_pawn = left_is_pawn && (left_piece_sign != white);
	bool right_is_opp_pawn = right_is_pawn && (right_piece_sign != white);

	// Edge of board cases.
	bool not_edge_left = move.get_to() % 8 != 0;
	bool not_edge_right = move.get_to() % 8 != 7;

	// Check if en passant is possible.
	bool possible_left = left_is_opp_pawn && not_edge_left;
	bool possible_right = right_is_opp_pawn && not_edge_right;

	// Update flag.
	uint8_t file = move.get_to() % 8;

	if (possible_left) gamestate.set_en_passant(possible_left, file);
	if (possible_right) gamestate.set_en_passant(possible_right, file);
}

template <bool white>
static void unmake_pawn_double(const Move& move, Board& board, GameState& gamestate) {
	unmake_move_piece<white>(move.get_from(), move.get_to(), move.get_piece(), board);
}

template <bool white>
void make_move(Move& move, Position& pos) {
	pos.history.push(pos.gamestate);

	switch (move.get_type()) {
	case 0:
		plain_move<white>(move, pos.board);
		break;
	case 1:
		castle_move<white>(move, pos.board, pos.gamestate);
		break;
	case 2:
		capture_move<white>(move, pos.board);
		break;
	case 3:
		ep_move<white>(move, pos.board);
		break;
	case 4:
		promo_move<white>(move, pos.board);
		break;
	case 5:
		pawn_double<white>(move, pos.board, pos.gamestate);
		break;
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
