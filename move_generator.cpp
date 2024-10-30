#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "position.hpp"

// Create attack board for specific piece.
Board Position::make_reach_board(Square square, Piece piece_type) {
	Board move_board = 0b0;
	switch (piece_type) {
	case 0:
		move_board = get_pawn_move(square, !player_sign);
		break;
	case 1:
		move_board = get_king_move(square);
		break;
	case 2:
		move_board = get_rook_move(square);
		break;
	case 3:
		move_board = get_bishop_move(square);
		break;
	case 4:
		move_board = get_knight_move(square);
		break;
	case 5:
		move_board = get_queen_move(square);
		break;
	}

	return move_board;
}

// Look around king position to see if king can be captured.
bool Position::move_legal(Move move, Board enemy_board) {
	do_move(move);
	enemy_board &= ~(square_to_mask(get_move_to(move)));
	Square square = get_king_pos(!player_sign);
	bool is_check = is_attacked(square, enemy_board);
	undo_move(move);

	return !is_check;
}

// Returns whether a square is under attack.
bool Position::is_attacked(Square square, Board enemy_board) {
	return (get_pawn_move(square, !player_sign) & bit_boards[PAWN] & enemy_board)
		|| (get_knight_move(square) & bit_boards[KNIGHT] & enemy_board)
		|| (get_rook_move(square) & (bit_boards[ROOK] | bit_boards[QUEEN]) & enemy_board)
		|| (get_bishop_move(square) & (bit_boards[BISHOP] | bit_boards[QUEEN]) & enemy_board)
		|| (get_king_move(square) & bit_boards[KING] & enemy_board);
}

// Push possible moves the the move stack.
MoveList<Move> Position::generate_move_list() {
	Move* last = possible_moves.top();
	// Get all squares that current player has a piece on.
	Board player_piece_board = player_sign ? BLACK_PIECE_BOARD : ((~BLACK_PIECE_BOARD) & TOTAL_BOARD);
	Board enemy_board = TOTAL_BOARD & (~player_piece_board);

	gen_type_plain_and_capture(player_piece_board, enemy_board);
	gen_type_castle(player_piece_board, enemy_board);
	gen_type_ep(player_piece_board, enemy_board);
	gen_type_pawn(player_piece_board, enemy_board);

	return possible_moves.from(last);
}

// Make EP moves.
void Position::gen_type_ep(Board player_piece_board, Board enemy_board) {
	uint8_t ep_flag = get_en_passant();

	// In most cases, no EP will be possible.
	if (!ep_flag) return;
	Move move = 0b0;

	auto make_en_passant = [&](int8_t from_offset) {
		set_move_type(move, MOVE_EP);

		// Rank to move to.
		File to = ep_flag & 0b00001111;

		// Whether en passant comes from the left.
		bool from_the_left = ep_flag & 0b10000000;

		// From which file the pawn comes.
		File from = to + from_offset;

		// The start and end rank.
		Rank start_rank = player_sign ? 4 : 3;
		Rank end_rank = player_sign ? 5 : 2;

		// Start and end square.
		Square start_square = from + start_rank * 8;
		Square end_square = to + end_rank * 8;

		// Set move squares.
		set_move_from(move, start_square);
		set_move_to(move, end_square);

		push_if_legal(move, enemy_board);
	};

	if (ep_flag & 0b1000'0000) make_en_passant(-1);
	if (ep_flag & 0b0100'0000) make_en_passant(1);
}

// Generate castling moves.
void Position::gen_type_castle(Board player_piece_board, Board enemy_board) {
	uint8_t castling_flag = get_castling();

	bool castle_right_queen = castling_flag & (0b1 << (!player_sign * 2));
	bool castle_right_king = castling_flag & (0b10 << (!player_sign * 2));

	// Check if player has castle rights.
	if (!castle_right_king && !castle_right_queen) return;

	// Get king square.
	Square king_square = __builtin_clzll(get_board(KING) & player_piece_board);

	// Check if squares are free.
	bool kingside_free = get_piece(king_square + 1) == EMPTY && get_piece(king_square + 2) == EMPTY;
	bool queenside_free = get_piece(king_square - 1) == EMPTY && get_piece(king_square - 2) == EMPTY;

	// Check if squares are not attacked.
	bool kingside_check_free = !is_attacked(king_square + 1, enemy_board) && !is_attacked(king_square + 2, enemy_board);
	bool queenside_check_free =
		!is_attacked(king_square - 1, enemy_board) && !is_attacked(king_square - 2, enemy_board);

	bool can_castle_kingside = castle_right_king && kingside_free && kingside_check_free;
	bool can_castle_queenside = castle_right_queen && queenside_free && queenside_check_free;

	Move move = 0b0;
	set_move_from(move, king_square);
	set_move_type(move, MOVE_CASTLE);

	if (can_castle_kingside) {
		set_move_to(move, king_square + 2);
		// Castle code is 0 for white, 2 for black.
		set_move_content(move, 2 * player_sign);
		push_if_legal(move, enemy_board);
	}
	if (can_castle_queenside) {
		set_move_to(move, king_square - 2);
		// 0 for white, 3 for black.
		set_move_content(move, 3 * player_sign);
		push_if_legal(move, enemy_board);
	}
}

// We need to treat different pawn move types differently.
// Promotion moves and normal moves should be done seperately, to prevent non-promo moves to the last rank.
void Position::gen_type_pawn(Board player_piece_board, Board enemy_board) {
	Move move = 0b0;
	set_move_piece(move, PAWN);

	Board pawn_board = get_board(PAWN) & player_piece_board;

	while (__builtin_popcountll(pawn_board)) {
		Square square = __builtin_clzll(pawn_board);
		set_move_from(move, square);

		// Piece's reach.
		Board move_squares = make_reach_board(square, PAWN);

		// Only empty squares.
		Board moveable_squares = move_squares & ~TOTAL_BOARD;

		// Capture squares.
		Board capture_squares = move_squares & enemy_board;

		// Promotion move end squares.
		Board promotion_squares = move_squares & 0b1111'1111ULL | (move_squares & (0b1111'1111ULL << 56));

		// Prune the last and first rank, these are promotion moves.
		moveable_squares &= ~(0b1111'1111ULL);
		moveable_squares &= ~(0b1111'1111ULL << 56);

		// Generate plain moves.
		while (__builtin_popcountll(moveable_squares)) {
			// Square that the piece moves to.
			Square moves_to = __builtin_clzll(moveable_squares);
			set_move_to(move, moves_to);

			// Check if pawn moves two squares.
			bool moves_two_squares = std::abs(moves_to - square) == 16;
			set_move_type(move, (moves_two_squares ? MOVE_PAWN_DOUBLE : MOVE_PLAIN));

			push_if_legal(move, enemy_board);

			moveable_squares &= ~(square_to_mask(moves_to));
		}

		// Generate capture moves.
		set_move_type(move, MOVE_CAPTURE);
		while (__builtin_popcountll(capture_squares)) {
			// Square that the piece moves to.
			Square moves_to = __builtin_clzll(capture_squares);
			set_move_to(move, moves_to);
			set_move_content(move, get_piece(moves_to));
			push_if_legal(move, enemy_board);

			capture_squares &= ~(square_to_mask(moves_to));
		}

		// Promotions.
		set_move_type(move, MOVE_PROMO);
		while (__builtin_popcountll(promotion_squares)) {
			Square moves_to = __builtin_clzll(promotion_squares);
			set_move_to(move, moves_to);

			// Possible pieces to promote to.
			for (Piece promote_to = 1; promote_to < 5; promote_to++) {
				// Square that the piece moves to.
				set_move_content(move, promote_to);
				push_if_legal(move, enemy_board);
			}

			promotion_squares &= ~(square_to_mask(moves_to));
		}

		pawn_board &= ~(square_to_mask(square));
	}
}

// Non special case moves: knight, bisshop, queen.
// Split available squares into empty and opponent occupied, and push the moves.
void Position::gen_type_plain_and_capture(Board player_piece_board, uint64_t enemy_board) {
	Move move;
	for (Piece piece = KING; piece <= QUEEN; piece++) {
		// Pieces that we make moves for.
		Board piece_board = get_board(piece) & player_piece_board;
		move = 0b0;
		set_move_piece(move, piece);
		assert(piece != 0);

		// While there are instances of the piece left on the board...
		while (__builtin_popcountll(piece_board)) {
			Square square = __builtin_clzll(piece_board);
			set_move_from(move, square);

			// Squares that the piece can move to.
			Board move_squares = make_reach_board(square, piece);

			// Only empty squares.
			Board empty_squares = move_squares & ~TOTAL_BOARD;

			// Only opponent squares.
			Board capture_squares = move_squares & enemy_board;

			// Generate plain moves.
			set_move_type(move, 0b000);
			while (__builtin_popcountll(empty_squares)) {
				// Square that the piece moves to.
				Square moves_to = __builtin_clzll(empty_squares);
				set_move_to(move, moves_to);
				push_if_legal(move, enemy_board);

				empty_squares &= ~(square_to_mask(moves_to));
			}

			// Generate capture moves.
			set_move_type(move, 0b010);

			while (__builtin_popcountll(capture_squares)) {
				// Square that the piece moves to.
				Square moves_to = __builtin_clzll(capture_squares);
				set_move_to(move, moves_to);
				set_move_content(move, get_piece(moves_to));
				push_if_legal(move, enemy_board);

				capture_squares &= ~(square_to_mask(moves_to));
			}
			piece_board &= ~(square_to_mask(square));
		}
	}
}
