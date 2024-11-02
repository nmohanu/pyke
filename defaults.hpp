// Default boards.
#include <cstdint>

#ifndef UTIL_H

// 8 bits startsquare, 8 bits end square.
typedef uint32_t Move;
typedef uint8_t Square;
typedef uint8_t Piece;
typedef uint64_t BitBoard;
typedef uint8_t Flag;
typedef uint8_t Rank;
typedef uint8_t File;
typedef uint16_t GameState;

// Default bitboards.
constexpr uint64_t INIT_ROOK_SQUARES = 0b1000000100000000000000000000000000000000000000000000000010000001ULL;
constexpr uint64_t INIT_KNIGHT_SQUARES = 0b0100001000000000000000000000000000000000000000000000000001000010ULL;
constexpr uint64_t INIT_BISHOP_SQUARES = 0b0010010000000000000000000000000000000000000000000000000000100100ULL;
constexpr uint64_t INIT_QUEEN_SQUARES = 0b0001000000000000000000000000000000000000000000000000000000010000ULL;
constexpr uint64_t INIT_KING_SQUARES = 0b0000100000000000000000000000000000000000000000000000000000001000ULL;
constexpr uint64_t INIT_PAWN_SQUARES = 0b0000000011111111000000000000000000000000000000001111111100000000ULL;
constexpr uint64_t INIT_BLACK_PIECES = 0b1111111111111111000000000000000000000000000000000000000000000000ULL;
constexpr uint64_t INIT_TOTAL_SQUARES = 0b1111111111111111000000000000000000000000000000001111111111111111ULL;
constexpr uint64_t INIT_WHITE_PIECES = 0b0000000000000000000000000000000000000000000000001111111111111111ULL;

// Boards.
#define PAWN_BOARD		  bit_boards[0]
#define KING_BOARD		  bit_boards[1]
#define ROOK_BOARD		  bit_boards[2]
#define BISHOP_BOARD	  bit_boards[3]
#define KNIGHT_BOARD	  bit_boards[4]
#define QUEEN_BOARD		  bit_boards[5]
#define TOTAL_BOARD		  bit_boards[6]
#define BLACK_PIECE_BOARD bit_boards[7]

// Signed pieces.

#define PAWN   0
#define KING   1
#define ROOK   2
#define BISHOP 3
#define KNIGHT 4
#define QUEEN  5
#define EMPTY  6

// Instruction decoding.
#define get_move_piece(m)	((m >> 13) & 0b111)
#define get_move_type(m)	(m & 0b111)
#define get_move_content(m) ((m >> 8) & 0b1111)
#define get_move_from(m)	((m >> 24) & 0b1111'1111)
#define get_move_to(m)		((m >> 16) & 0b1111'1111)

#define set_move_piece(m, p)   (m = ((m & ~(0b111 << 13)) | ((p) << 13)))
#define set_move_from(m, s)	   (m = ((m & ~(0xFF << 24)) | ((s) << 24)))
#define set_move_to(m, s)	   (m = ((m & ~(0xFF << 16)) | ((s) << 16)))
#define set_move_content(m, c) (m = ((m & ~(0b111 << 8)) | ((c) << 8)))
#define set_move_type(m, t)	   (m = ((m & ~0b111) | (t)))

// Util

// Convert square to shift amount.
#define square_to_shamt(s) (63 - s)

// Convert square to mask.
#define square_to_mask(s) (1ULL << square_to_shamt(s))

// Set en passant status.
#define set_en_passant(n) (pos_flag = (pos_flag & 0b1111'1111'0000'0000) | n)
#define get_en_passant()  (pos_flag & 0b1111'1111)

// Set castling flag.
#define set_castling(n) (pos_flag = (pos_flag & ((0b0000'0000 | n) << 8)))
#define get_castling()	pos_flag >> 8;

// Get color sign of a piece.
#define get_piece_sign(p) (p > 5)

// Get board of piece p.
#define get_board(p) (bit_boards[p])

// Types: 0 = plain move, 1 = castle, 2 = capture, 3 = EP, 4 = promotion,
// 5 = pawn two forward, 6 = king move, 7 = king take, 8 = rook move.
#define MOVE_PLAIN		 0
#define MOVE_CASTLE		 1
#define MOVE_CAPTURE	 2
#define MOVE_EP			 3
#define MOVE_PROMO		 4
#define MOVE_PAWN_DOUBLE 5

#endif	// !UTIL.H
