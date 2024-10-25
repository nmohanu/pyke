#include <cstdint>

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
	(this->*do_move_functions[move_type])(move);

	// Toggle player sign.
	player_sign = !player_sign;
}

// Function for plain move type.
void Position::plain_move(Move move) { move_piece(get_move_from(move), get_move_to(move), get_move_piece(move)); }

// Function for capturing moves.
void Position::capture_move(Move move) {
	Piece captured_piece = get_move_content(move);
	// Move the moving piece.
	move_piece(get_move_from(move), get_move_to(move), get_move_piece(move));

	// Update board for captured piece.
	remove_from_board(captured_piece, get_move_to(move));
}

// Function for promotion moves.
void Position::promotion_move(Move move) {
	Piece piece = get_move_piece(move);
	Piece new_piece = get_move_content(move);

	// Update boards.
	remove_from_board(piece, get_move_from(move));
	add_to_board(new_piece, get_move_to(move));
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
	move_piece(king_start_squares[castling_code], king_end_squares[castling_code], get_move_piece(move));
	move_piece(rook_start_squares[castling_code], rook_end_squares[castling_code], sign_piece(W_ROOK));

	// Update castling rights.
	remove_castling_right(player_sign, 0b11);
}

void Position::en_passant_move(Move move) {}

void Position::pawn_2frwrd_move(Move move) {
	move_piece(get_move_from(move), get_move_to(move), get_move_piece(move));

	// pawn moved two forward, update en passant status.
}

// King move.
void Position::king_move(Move move) {
	move_piece(get_move_from(move), get_move_to(move), get_move_piece(move));
	remove_castling_right(player_sign, 0b11);
}

void Position::king_take_move(Move move) {
	capture_move(move);
	remove_castling_right(player_sign, 0b11);
}

// Execute rook move that has a change we need to update castling rights.
void Position::rook_move(Move move) {
	// Get start square.
	uint8_t start_square = get_move_from(move);

	// Move rook.
	move_piece(get_move_from(move), get_move_to(move), get_move_piece(move));

	// 0b01 if kingside. 0b10 if queenside.
	uint8_t mask = start_square == 63 || start_square == 7;
	mask += 2 * (start_square == 56 || start_square == 0);
	// Update castling rights.
	remove_castling_right(player_sign, mask);
}
