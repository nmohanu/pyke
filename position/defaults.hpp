// Default boards.
#include <cstdint>

// 8 bits startsquare, 8 bits end square.
typedef uint32_t Move;
typedef uint8_t Square;
typedef uint8_t Piece;

// Default bitboards.
constexpr uint64_t ROOK_SQUARES = 0b1000000100000000000000000000000000000000000000000000000010000001ULL;
constexpr uint64_t KNIGHT_SQUARES = 0b0100001000000000000000000000000000000000000000000000000001000010ULL;
constexpr uint64_t BISHOP_SQUARES = 0b0010010000000000000000000000000000000000000000000000000000100100ULL;
constexpr uint64_t QUEEN_SQUARES = 0b0001000000000000000000000000000000000000000000000000000000010000ULL;
constexpr uint64_t KING_SQUARES = 0b0000100000000000000000000000000000000000000000000000000000001000ULL;
constexpr uint64_t PAWN_SQUARES = 0b0000000011111111000000000000000000000000000000001111111100000000ULL;
constexpr uint64_t BLACK_PIECES = 0b1111111111111111000000000000000000000000000000000000000000000000ULL;
constexpr uint64_t TOTAL_SQUARES = 0b1111111111111111000000000000000000000000000000001111111111111111ULL;

// Boards.
#define W_KING_BOARD	  bit_boards[W_KING]
#define W_QUEEN_BOARD	  bit_boards[W_QUEEN]
#define W_ROOK_BOARD	  bit_boards[W_ROOK]
#define W_BISHOP_BOARD	  bit_boards[W_BISHOP]
#define W_KNIGHT_BOARD	  bit_boards[W_KNIGHT]
#define W_PAWN_BOARD	  bit_boards[W_PAWN]
#define B_KING_BOARD	  bit_boards[B_KING]
#define B_QUEEN_BOARD	  bit_boards[B_QUEEN]
#define B_ROOK_BOARD	  bit_boards[B_ROOK]
#define B_BISHOP_BOARD	  bit_boards[B_BISHOP]
#define B_KNIGHT_BOARD	  bit_boards[B_KNIGHT]
#define B_PAWN_BOARD	  bit_boards[B_PAWN]
#define TOTAL_BOARD		  bit_boards[TOTAL]
#define BLACK_PIECE_BOARD bit_boards[COLOR_BOARD]

#define W_KING		0
#define W_QUEEN		1
#define W_ROOK		2
#define W_BISHOP	3
#define W_KNIGHT	4
#define W_PAWN		5
#define B_KING		6
#define B_QUEEN		7
#define B_ROOK		8
#define B_BISHOP	9
#define B_KNIGHT	10
#define B_PAWN		11
#define TOTAL		12
#define COLOR_BOARD 13
#define EMPTY		14
#define INVALID		15

// Instruction decoding.
#define get_move_piece(m)	(m & (0b1111 << 12))
#define get_move_type(m)	(m & 0b111)
#define get_move_content(m) (m & (0b1111 << 8))
#define get_move_from(m)	(m & (0b1111'1111 << 24))
#define get_move_to(m)		(m & (0b1111'1111 << 16))

// Util

// Convert square to shift amount.
#define square_to_shamt(s) (63 - s)

// Convert square to mask.
#define square_to_mask(s) (1ULL << square_to_shamt(s))

// Set en passant status.
#define set_en_passant(n) ((pos_flag = pos_flag & 0b0000'0000) | n)

// Set castling status.
#define set_castling(n) (pos_flag = (pos_flag & (0b0000'0000 << 8)) | n)

// Get board of piece p.
#define get_board(p) (bit_boards[p])

// Add the color sign to an unsigned piece.
#define sign_piece(p)		 (p + 6 * player_sign)
#define unsign_piece(p)		 (p - 6 * player_sign)
#define invert_piece_sign(p) (unsign_piece(p) + 6 * !player_sign)

// Get color sign of a piece.
#define get_piece_sign(p) (p > 5)
