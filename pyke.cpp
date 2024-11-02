#include "pyke.hpp"

#include <cstdint>

using namespace Pyke;

template <bool white>
constexpr static inline BitBoard get_player_board(Board& board) {
	return white ? board.w_board : board.b_board;
}

template <bool white>
constexpr static inline BitBoard get_king_hz(Board& board) {
	return white ? board.w_king_hz : board.b_king_hz;
}

template <bool white>
constexpr static inline BitBoard get_king_dg(Board& board) {
	return white ? board.w_king_dg : board.b_king_dg;
}

template <bool white, Piece piece>
constexpr static inline BitBoard get_piece_board(Board& board) {
	switch (piece) {
	case PAWN:
		return white ? board.w_pawn : board.b_pawn;
	case KING:
		return white ? board.w_king : board.w_pawn;
	case ROOK:
		return white ? board.w_rook : board.b_rook;
	case BISHOP:
		return white ? board.w_bishop : board.b_bishop;
	case KNIGHT:
		return white ? board.w_knight : board.b_knight;
	case QUEEN:
		return white ? board.w_queen : board.b_queen;
	}
}

template <bool white, Piece piece>
BitBoard make_reach_board(Square square, Board& board) {
	BitBoard move_board = 0b0;
	switch (piece) {
	case 0:
		move_board = get_pawn_move<white>(square);
		break;
	case 1:
		move_board = get_king_move(square);
		break;
	case 2:
		move_board = get_rook_move(square, board.occ_board);
		break;
	case 3:
		move_board = get_bishop_move(square, board.occ_board);
		break;
	case 4:
		move_board = get_knight_move(square);
		break;
	case 5:
		move_board = get_queen_move(square, board.occ_board);
		break;
	}

	return move_board;
}

// Create moves given a from and to board..
template <bool white, Piece piece>
constexpr static inline void generate_moves(BitBoard cmt, BitBoard& pieces, Stack<Move>& movelist, Board& board) {
	Move move = 0b0;
	set_move_piece(move, piece);
	// For all instances of given piece.
	while (pieces) {
		Square from = pop(pieces);
		set_move_from(move, from);
		BitBoard can_move_to = cmt & make_reach_board<white, piece>(from, board);
		while (can_move_to) {
			Square to = pop(can_move_to);
			set_move_to(move, to);
			movelist.push(move);
		}
	}
}
template <bool white, bool diagonal>
constexpr static inline BitBoard make_pin_mask(Board& board, BitBoard& pinners, BitBoard& king_mask) {
	BitBoard pinmask = 0b0;
	BitBoard queen_board = get_piece_board<!white, QUEEN>(board);
	BitBoard slider_board = get_piece_board<!white, diagonal ? BISHOP : ROOK>(board);
	while (queen_board) {
		pinmask |= get_queen_move(pop(queen_board), board.occ_board);
	}
	while (slider_board) {
		pinmask |= diagonal ? get_bishop_move(pop(slider_board), board.occ_board)
							: get_rook_move(pop(slider_board), board.occ_board);
	}
	return pinmask & king_mask;
}

template <bool white>
MoveList<Move> MoveGenerator::generate_movelist(Board& board, GameState state) {
	Move* last = movelist.top();

	// Generally, a player can move only to empty or enemy squares.
	BitBoard can_move_to = ~get_player_board<white>(board);

	// Make king mask.
	Square king_square = __builtin_clzll(get_piece_board<white, KING>());

	// King reach.
	BitBoard king_dg = get_bishop_move(king_square, board.occ_board);
	BitBoard king_orth = get_rook_move(king_square, board.occ_board);
	BitBoard king_kn = get_knight_move(king_square);
	BitBoard king_pw = get_pawn_move<white>(king_square, board.occ_board);

	// Get all potential pinner pieces.
	BitBoard orth_pinners =
		rook_mask_table[king_square] & (get_piece_board<!white, ROOK>(board) | get_piece_board<!white, QUEEN>(board));
	BitBoard dg_pinners = bishop_mask_table[king_square]
		& (get_piece_board<!white, BISHOP>(board) | get_piece_board<!white, QUEEN>(board));

	// Get only the pinners closest to the king.
	orth_pinners &= rook_attacks[orth_pinners][king_square];
	dg_pinners &= bishop_attacks[dg_pinners][king_square];

	// Make pinmask.
	BitBoard pinmask_dg = make_pin_mask<white, true>(board, dg_pinners, king_dg);
	BitBoard pinmask_orth = make_pin_mask<white, false>(board, orth_pinners, king_orth);

	// Check there are pieces directly attacking the king.
	BitBoard orth_checkers = king_orth & orth_pinners;
	BitBoard dg_checkers = king_dg & dg_pinners;
	BitBoard kn_checkers = king_kn & get_piece_board<!white, KNIGHT>(board);
	BitBoard pw_checkers = king_pw & get_piece_board<!white, PAWN>(board);

	// Amount of checkers.
	uint8_t checker_cnt = __builtin_popcount(orth_checkers) + __builtin_popcount(king_dg)
		+ __builtin_popcount(kn_checkers) + __builtin_popcount(pw_checkers);

	// Conditionals only taken when king is in check.
	if (checker_cnt >= 2) {
		// Double check, only king moves are possible and only to unseen squares.
	} else if (checker_cnt) {
		// King is under check, player can only make a blocking move or capture checker.
		BitBoard check_mask = orth_checkers | dg_checkers | kn_checkers | pw_checkers;
		can_move_to &= check_mask;
	}

	// Make non-pawn and non-king moves.
	for (Piece p = ROOK; p <= QUEEN; p++) {
		// Split pinned pieces from non-pinned pieces.
		BitBoard piece_board = get_piece_board<white, p>(board);
		BitBoard pinned_dg = piece_board & king_dg & pinmask_dg;
		BitBoard pinned_orth = piece_board & king_orth & pinmask_orth;
		piece_board &= ~(king_dg | king_orth);

		// Unpinned.
		generate_moves<white, p>(can_move_to, piece_board, movelist, board);
		// Pinned diagonally.
		generate_moves<white, p>(can_move_to & pinmask_dg, pinned_dg, movelist, board);
		// Pinned orthogonally.
		generate_moves<white, p>(can_move_to & pinmask_orth, pinned_orth, movelist, board);
	}

	// Return a sublist containing all newly generated moves.
	return movelist.from(last);
}
