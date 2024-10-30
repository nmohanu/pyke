#include <cassert>
#include <cstdint>
#include <iostream>

#include "position.hpp"

// Do a move.
void Position::do_move(Move move) {
	// Decide what to do.
	uint8_t move_type = get_move_type(move);

	// Push current flag to the stack.
	flag_history.push(pos_flag);

	// Reset en passant status.
	set_en_passant(0);

	// Call move function based on move type.
	switch (move_type) {
	case 0:
		plain_move(move);
		break;
	case 1:
		castle_move(move);
		break;
	case 2:
		capture_move(move);
		break;
	case 3:
		en_passant_move(move);
		break;
	case 4:
		promotion_move(move);
		break;
	case 5:
		pawn_2frwrd_move(move);
		break;
	}

	// Update castle rights if needed.
	switch (get_move_piece(move)) {
	case KING:
		remove_castling_right(player_sign, 0b11);
		break;
	case ROOK:
		// 0b01 if kingside. 0b10 if queenside.
		Square start_square = get_move_from(move);
		uint8_t mask = start_square == 63 || start_square == 7;
		mask += 2 * (start_square == 56 || start_square == 0);
		// Update castling rights.
		remove_castling_right(player_sign, mask);
		break;
	}

	// Toggle player sign.
	player_sign = !player_sign;
}

// Function for plain move type.
void Position::plain_move(Move move) {
	move_piece(get_move_from(move), get_move_to(move), get_move_piece(move), player_sign);
}

// Function for capturing moves.
void Position::capture_move(Move move) {
	Square from = get_move_from(move);
	Piece captured_piece = get_move_content(move);

	if (captured_piece != EMPTY) remove_from_board(captured_piece, get_move_to(move), !player_sign);

	// Move the moving piece.
	move_piece(from, get_move_to(move), get_move_piece(move), player_sign);
}

// Function for promotion moves.
void Position::promotion_move(Move move) {
	Piece piece = PAWN;
	Piece new_piece = get_move_content(move);
	Square to = get_move_to(move);
	Piece captured = get_piece(to);

	// Update boards.
	remove_from_board(piece, get_move_from(move), player_sign);
	add_to_board(new_piece, to, player_sign);

	if (captured == EMPTY) return;

	remove_from_board(captured, to, !player_sign);
}

// Execute castling move.
void Position::castle_move(Move move) {
	// Pre-defined start and end positions.
	uint8_t king_start_squares[4] = {60, 60, 4, 4};
	uint8_t rook_start_squares[4] = {63, 56, 7, 0};
	uint8_t king_end_squares[4] = {62, 58, 6, 2};
	uint8_t rook_end_squares[4] = {61, 59, 5, 3};

	// Get castling type.
	uint8_t castling_code = get_move_content(move);

	// Do move.
	move_piece(king_start_squares[castling_code], king_end_squares[castling_code], KING, player_sign);
	move_piece(rook_start_squares[castling_code], rook_end_squares[castling_code], ROOK, player_sign);

	// Update castling rights.
	remove_castling_right(player_sign, 0b11);
}

// Do ep move.
void Position::en_passant_move(Move move) {
	Square from = get_move_from(move);
	Square to = get_move_to(move);
	Square captured_pawn_square = ((to + 8) - 16 * player_sign);

	move_piece(from, to, PAWN, player_sign);
	remove_from_board(PAWN, captured_pawn_square, !player_sign);
}

// Move pawn 2 squares forward and update en passant status. "Left"" and "right" are as seen
// from white's perspective.
void Position::pawn_2frwrd_move(Move move) {
	Square to = get_move_to(move);
	move_piece(get_move_from(move), to, PAWN, player_sign);

	// pawn moved two forward, update en passant status.
	Square square_left = to - 1;
	Square square_right = to + 1;
	Piece piece_left = get_piece(square_left);
	Piece piece_right = get_piece(square_right);
	Piece moving_piece = PAWN;
	bool left_is_pawn = piece_left == PAWN;
	bool right_is_pawn = piece_right == PAWN;
	bool left_piece_sign = square_to_mask(square_left) & BLACK_PIECE_BOARD;
	bool right_piece_sign = square_to_mask(square_right) & BLACK_PIECE_BOARD;
	bool left_is_opp_pawn = left_is_pawn && (left_piece_sign != player_sign);
	bool right_is_opp_pawn = right_is_pawn && (right_piece_sign != player_sign);

	// Edge of board cases.
	bool not_edge_left = to % 8 != 0;
	bool not_edge_right = to % 8 != 7;

	// Check if en passant is possible.
	bool possible_left = left_is_opp_pawn && not_edge_left;
	bool possible_right = right_is_opp_pawn && not_edge_right;

	uint8_t new_ep_flag;
	// Update flag.
	if (possible_left || possible_right) new_ep_flag = to % 8;
	if (possible_left) new_ep_flag |= 0b1000'0000;
	if (possible_right) new_ep_flag |= 0b0100'0000;

	set_en_passant(new_ep_flag);
}
