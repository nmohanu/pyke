
#include <immintrin.h>

#include <cstdint>
#include <iostream>
#include <string>

#include "move.hpp"

#ifndef UTIL_H
#define UTIL_H

inline std::string make_chess_notation(int index) {
	// Convert column index (0-7) to letter (a-h)
	char column = 'a' + (index % 8);
	// Convert row index (0-7) to number (1-8)
	char row = '8' - (index / 8);
	return std::string(1, column) + std::string(1, row);
}

inline uint8_t notation_to_square(std::string notation) {
	int column = notation[0] - 'a';
	int row = 7 - (notation[1] - '1');
	return ((row * 8) + column);
}

// Returns the position of the least significant bit.
inline uint8_t lbit(uint64_t n) { return __builtin_clzll(n); }

inline constexpr uint8_t square_to_shamt(Square s) { return 63 - s; }

inline constexpr uint64_t square_to_mask(Square s) { return 1ULL << (63 - s); }

inline bool get_bit_64(uint64_t num, uint8_t s) { return (num & (square_to_mask(s))); }

inline uint8_t pop(uint64_t& b) {
	Square index = lbit(b);
	b ^= square_to_mask(index);
	return index;
}

inline BitBoard popextr(uint64_t& b) {
	BitBoard index = 1ULL << __builtin_ctzll(b);
	b ^= index;
	return index;
}

static inline void print_movecnt(Square start_square, Square end_square, uint64_t cnt) {
	if (cnt)
		std::cout << make_chess_notation(start_square) << make_chess_notation(end_square) << ": " << std::to_string(cnt)
				  << '\n';
}

inline void print_bitboard(uint64_t bitboard) {
	printf("\n");
	// loop over board ranks.
	for (int rank = 0; rank < 8; rank++) {
		// loop over board files.
		for (int file = 0; file < 8; file++) {
			// convert file & rank into square index.
			int square = rank * 8 + file;

			// print ranks.
			if (!file) printf("  %d ", 8 - rank);

			// print bit state (either 1 or 0).
			printf(" %d", get_bit_64(bitboard, square) ? 1 : 0);
		}
		// print new line every rank.
		printf("\n");
	}
}

inline BitBoard pext(BitBoard bits, BitBoard mask) { return _pext_u64(bits, mask); }

#endif
