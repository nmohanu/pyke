#include <cassert>

#include "position.hpp"

void Position::undo_move(Move move) {
	// Decide what to undo.
	uint8_t move_type = get_move_type(move);
	assert(get_move_piece(move) != EMPTY);

	// Toggle player sign.
	player_sign = !player_sign;

	// Call move function based on move type.
	switch (move_type) {
	case 0:
		undo_plain_move(move);
		break;
	case 1:
		undo_castle_move(move);
		break;
	case 2:
		undo_capture_move(move);
		break;
	case 3:
		undo_en_passant_move(move);
		break;
	case 4:
		undo_promotion_move(move);
		break;
	case 5:
		undo_plain_move(move);
		break;
	}

	// Restore flags.
	pos_flag = flag_history.pop();
}

void Position::undo_plain_move(Move move) {
	move_piece(get_move_to(move), get_move_from(move), get_move_piece(move), player_sign);
}

void Position::undo_castle_move(Move move) {
	// Pre-defined start and end positions.
	uint8_t king_start_squares[4] = {60, 60, 4, 4};
	uint8_t rook_start_squares[4] = {63, 56, 7, 0};
	uint8_t king_end_squares[4] = {62, 58, 6, 2};
	uint8_t rook_end_squares[4] = {61, 59, 5, 3};

	// Get castling type.
	uint8_t castling_code = get_move_content(move);

	// Do move.
	move_piece(king_end_squares[castling_code], king_start_squares[castling_code], KING, player_sign);
	move_piece(rook_end_squares[castling_code], rook_start_squares[castling_code], ROOK, player_sign);
}

void Position::undo_capture_move(Move move) {
	Square from = get_move_from(move);
	Square to = get_move_to(move);
	Piece captured_piece = get_move_content(move);
	// Move the moving piece.
	move_piece(to, from, get_move_piece(move), player_sign);

	// Update board for captured piece.
	add_to_board(captured_piece, to, !player_sign);
}

void Position::undo_en_passant_move(Move move) {
	Square from = get_move_from(move);
	Square to = get_move_to(move);
	Square captured_pawn_square = ((to + 8) - 16 * player_sign);
	Piece moving_piece = PAWN;
	Piece captured_piece = PAWN;

	move_piece(to, from, moving_piece, player_sign);
	add_to_board(captured_piece, captured_pawn_square, !player_sign);
}

void Position::undo_promotion_move(Move move) {
	Piece piece = PAWN;
	Piece new_piece = get_move_content(move);
	Square to = get_move_to(move);
	Piece captured = get_piece(to);

	// Update boards.
	remove_from_board(new_piece, to, player_sign);
	add_to_board(piece, get_move_from(move), player_sign);

	if (captured == EMPTY) return;

	add_to_board(captured, to, !player_sign);
}
