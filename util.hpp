
#include <cstdint>
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

inline bool get_bit_64(uint64_t num, uint8_t pos) { return (num & (1ULL << (63 - pos))) != 0; }

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

inline uint8_t notation_to_square(std::string notation) {
	int column = notation[0] - 'a';
	int row = 7 - (notation[1] - '1');
	return ((row * 8) + column);
}

inline uint8_t pop(uint64_t& b) {
	Square index = uint8_t(__builtin_clzll(b));
	b ^= (1ULL << (63 - index));
	return index;
}

static std::string move_to_string(Move move) {
	std::string start_notation = make_chess_notation(move.get_from());
	std::string destination_notation = make_chess_notation(move.get_to());
	std::string promotion_letter = "";
	if (move.get_type() == MOVE_PROMO) {
		switch (move.get_content()) {
		case 1:
			promotion_letter = "q";
			break;
		case 2:
			promotion_letter = "r";
			break;
		case 3:
			promotion_letter = "b";
			break;
		case 4:
			promotion_letter = "n";
			break;
		default:
			break;
		}
	}
	return start_notation + destination_notation + promotion_letter;
}

constexpr static inline uint8_t square_to_shamt(Square s) { return 63 - s; }

constexpr static inline uint64_t square_to_mask(Square s) { return 1ULL << square_to_shamt(s); }

#endif
