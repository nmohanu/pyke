#include <immintrin.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "defaults.hpp"
#include "util.hpp"

#ifndef LOOKUPTABLES_H
#define LOOKUPTABLES_H

// Lookup tables for piece moves.
// Bishop and rook tables are created below, these are made with a bit of help from "Chess Programming"'s youtube
// videos.'
constexpr uint64_t KING_MOVE_SQUARES[] = {
	4665729213955833856ULL,
	11592265440851656704ULL,
	5796132720425828352ULL,
	2898066360212914176ULL,
	1449033180106457088ULL,
	724516590053228544ULL,
	362258295026614272ULL,
	144959613005987840ULL,
	13853283560024178688ULL,
	16186183351374184448ULL,
	8093091675687092224ULL,
	4046545837843546112ULL,
	2023272918921773056ULL,
	1011636459460886528ULL,
	505818229730443264ULL,
	216739030602088448ULL,
	54114388906344448ULL,
	63227278716305408ULL,
	31613639358152704ULL,
	15806819679076352ULL,
	7903409839538176ULL,
	3951704919769088ULL,
	1975852459884544ULL,
	846636838289408ULL,
	211384331665408ULL,
	246981557485568ULL,
	123490778742784ULL,
	61745389371392ULL,
	30872694685696ULL,
	15436347342848ULL,
	7718173671424ULL,
	3307175149568ULL,
	825720045568ULL,
	964771708928ULL,
	482385854464ULL,
	241192927232ULL,
	120596463616ULL,
	60298231808ULL,
	30149115904ULL,
	12918652928ULL,
	3225468928ULL,
	3768639488ULL,
	1884319744ULL,
	942159872ULL,
	471079936ULL,
	235539968ULL,
	117769984ULL,
	50463488ULL,
	12599488ULL,
	14721248ULL,
	7360624ULL,
	3680312ULL,
	1840156ULL,
	920078ULL,
	460039ULL,
	197123ULL,
	49216ULL,
	57504ULL,
	28752ULL,
	14376ULL,
	7188ULL,
	3594ULL,
	1797ULL,
	770ULL
};

constexpr uint64_t KNIGHT_MOVE_SQUARES[] = {
	9077567998918656ULL,
	4679521487814656ULL,
	38368557762871296ULL,
	19184278881435648ULL,
	9592139440717824ULL,
	4796069720358912ULL,
	2257297371824128ULL,
	1128098930098176ULL,
	2305878468463689728ULL,
	1152939783987658752ULL,
	9799982666336960512ULL,
	4899991333168480256ULL,
	2449995666584240128ULL,
	1224997833292120064ULL,
	576469569871282176ULL,
	288234782788157440ULL,
	4620693356194824192ULL,
	11533718717099671552ULL,
	5802888705324613632ULL,
	2901444352662306816ULL,
	1450722176331153408ULL,
	725361088165576704ULL,
	362539804446949376ULL,
	145241105196122112ULL,
	18049583422636032ULL,
	45053588738670592ULL,
	22667534005174272ULL,
	11333767002587136ULL,
	5666883501293568ULL,
	2833441750646784ULL,
	1416171111120896ULL,
	567348067172352ULL,
	70506185244672ULL,
	175990581010432ULL,
	88545054707712ULL,
	44272527353856ULL,
	22136263676928ULL,
	11068131838464ULL,
	5531918402816ULL,
	2216203387392ULL,
	275414786112ULL,
	687463207072ULL,
	345879119952ULL,
	172939559976ULL,
	86469779988ULL,
	43234889994ULL,
	21609056261ULL,
	8657044482ULL,
	1075839008ULL,
	2685403152ULL,
	1351090312ULL,
	675545156ULL,
	337772578ULL,
	168886289ULL,
	84410376ULL,
	33816580ULL,
	4202496ULL,
	10489856ULL,
	5277696ULL,
	2638848ULL,
	1319424ULL,
	659712ULL,
	329728ULL,
	132096ULL
};

// rook magic numbers
constexpr uint64_t rook_magic_numbers[64] = {
	0x1004081002402ULL,	   0x2006104900a0804ULL,  0x12001008414402ULL,	 0x20030a0244872ULL,	0x4024081001000421ULL,
	0x80c0084100102001ULL, 0x2100190040002085ULL, 0x280001040802101ULL,	 0x44440041009200ULL,	0x2935610830022400ULL,
	0x20c020080040080ULL,  0x1001004080100ULL,	  0x2060820c0120200ULL,	 0x4008142004410100ULL, 0x40802000401080ULL,
	0x101002200408200ULL,  0x2048440040820001ULL, 0x20020004010100ULL,	 0x202008004008002ULL,	0x2002080100110004ULL,
	0x100021010009ULL,	   0x30200100110040ULL,	  0x1000422010034000ULL, 0x480400080088020ULL,	0x200046502000484ULL,
	0x124080204001001ULL,  0x2080040080800200ULL, 0x14800401802800ULL,	 0x188020010100100ULL,	0x4200011004500ULL,
	0x440003000200801ULL,  0x4040008040800020ULL, 0x80004600042881ULL,	 0x80088400100102ULL,	0x20080800400ULL,
	0x2000a00200410ULL,	   0x20100080080080ULL,	  0x21200680100081ULL,	 0x2040002120081000ULL, 0x100400080208000ULL,
	0x20008806104ULL,	   0x4010040010010802ULL, 0x8094004002004100ULL, 0x481828014002800ULL,	0x140848010000802ULL,
	0x12108a0010204200ULL, 0x100808020004000ULL,  0x80044006422000ULL,	 0x801000060821100ULL,	0x2200800100020080ULL,
	0x2802200800400ULL,	   0x208808088000400ULL,  0x120800800801000ULL,	 0x100802000801000ULL,	0x2024401000200040ULL,
	0x800098204000ULL,	   0x2080088004402900ULL, 0x8480008002000100ULL, 0x3001c0002010008ULL,	0x200020010080420ULL,
	0x100081001000420ULL,  0x2801880a0017001ULL,  0x140002000100040ULL,	 0x8a80104000800020ULL
};

// bishop magic numbers
constexpr uint64_t bishop_magic_numbers[64] = {
	0x4010011029020020ULL, 0x8918844842082200ULL, 0x6000020202d0240ULL,	 0x28000010020204ULL,	0x8040002811040900ULL,
	0x500201010098b028ULL, 0x4a02012000ULL,		  0xa010109502200ULL,	 0x4010801011c04ULL,	0x5a84841004010310ULL,
	0x20906061210001ULL,   0x802241102020002ULL,  0x300000261044000aULL, 0x4000020e01040044ULL, 0x180806108200800ULL,
	0x500861011240000ULL,  0x8012020600211212ULL, 0x1a2208080504f080ULL, 0x201011000808101ULL,	0x90080104000041ULL,
	0x1840060a44020800ULL, 0x110400a6080400ULL,	  0x400408a884001800ULL, 0x209188240001000ULL,	0x42008c0340209202ULL,
	0x623000a080011400ULL, 0x2010004880111000ULL, 0x80020a0200100808ULL, 0x8646020080080080ULL, 0x2000402200300c08ULL,
	0x8422100208500202ULL, 0x8004200962a00220ULL, 0x1c004001012080ULL,	 0xe4004081011002ULL,	0x800040400a011002ULL,
	0x4001001021004000ULL, 0x20080002081110ULL,	  0x152048408022401ULL,	 0x2010100a02021202ULL, 0x220200865090201ULL,
	0x8002020480840102ULL, 0xe900410884800ULL,	  0x810102c808880400ULL, 0x85040820080400ULL,	0x4002801a02003ULL,
	0x810018200204102ULL,  0x404022024108200ULL,  0x400210c3880100ULL,	 0x801010402020200ULL,	0x401484104104005ULL,
	0x4011002100800ULL,	   0x40308200402ULL,	  0x1020a0a020400ULL,	 0x24d0080801082102ULL, 0x21001420088ULL,
	0x4050404440404ULL,	   0x3c0808410220200ULL,  0x1080820820060210ULL, 0x2112080446200010ULL, 0x581104180800210ULL,
	0x108060845042010ULL,  0x10190041080202ULL,	  0x2004208a004208ULL,	 0x40040844404084ULL
};
// Create lookup tables for bisshops and rooks.

constexpr uint64_t make_bishop_mask(uint8_t square) {
	square = 63 - square;
	// result attacks bitboard.
	uint64_t attacks = 0ULL;

	// init ranks & files.
	int r = 0;
	int f = 0;

	// init target rank & files.
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant bishop occupancy bits.
	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

	// return attack map.
	return attacks;
}

constexpr uint64_t make_rook_mask(uint8_t square) {
	square = 63 - square;
	// result attacks bitboard.
	uint64_t attacks = 0ULL;

	// init ranks & files.
	int r = 0;
	int f = 0;

	// init target rank & files.
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant rook occupancy bits.
	for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
	for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
	for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
	for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

	// return attack map.
	return attacks;
}

// Bishop moving logic.
constexpr uint64_t bishop_attack_on_fly(uint8_t square, uint64_t occupation) {
	uint64_t move_board = 0b0;

	int square_copy = square;
	uint64_t bit_mask = 1ULL << (63 - square);
	// Continue until another piece is found. Repeat for each direction.
	// Up left.
	while (square_copy >= 0 && square_copy % 8 != 0) {
		bit_mask <<= 9;
		square_copy -= 9;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);
	// Move up-right direction.
	while (square_copy >= 0 && square_copy % 8 != 7) {
		bit_mask <<= 7;
		square_copy -= 7;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);

	// Move down-right direction.
	while (square_copy < 64 && square_copy % 8 != 7) {
		bit_mask >>= 9;
		square_copy += 9;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);

	// Move down-left direction.
	while (square_copy < 64 && square_copy % 8 != 0) {
		bit_mask >>= 7;
		square_copy += 7;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	return move_board;
}

// Rook move logic.
constexpr uint64_t rook_attack_on_fly(uint8_t square, uint64_t occupation) {
	uint64_t move_board = 0b0;
	int square_copy = square;
	uint64_t bit_mask = 1ULL << (63 - square);
	// Continue until another piece is found. Repeat for each direction.
	// left.
	while (square_copy >= 0 && square_copy % 8 != 0) {
		bit_mask <<= 1;
		square_copy--;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);

	// Move up.
	while (square_copy >= 0) {
		bit_mask <<= 8;
		square_copy -= 8;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);

	// Move right.
	while (square_copy < 64 && square_copy % 8 != 7) {
		bit_mask >>= 1;
		square_copy++;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	square_copy = square;
	bit_mask = 1ULL << (63 - square);

	// Move down.
	while (square_copy < 64) {
		bit_mask >>= 8;
		square_copy += 8;
		move_board |= bit_mask;
		if (occupation & bit_mask) break;  // Stop if a piece is found.
	}

	return move_board;
}

// Make lookup tables for the masks.
constexpr std::array<uint64_t, 64> make_bishop_masks() {
	std::array<uint64_t, 64> values{};
	for (int square = 0; square < 64; square++) {
		// Make masks for all squares.
		values[square] = make_bishop_mask(square);
	}
	return values;
}

constexpr std::array<uint64_t, 64> make_rook_masks() {
	std::array<uint64_t, 64> values{};
	for (int square = 0; square < 64; square++) {
		// Make masks for all squares.
		values[square] = make_rook_mask(square);
	}
	return values;
}

// Mask lookup table instances.
constexpr std::array<uint64_t, 64> bishop_mask_table = make_bishop_masks();
constexpr std::array<uint64_t, 64> rook_mask_table = make_rook_masks();

// Max possible permutations for a mask.
constexpr int max_permutations_rooks = 4096;
constexpr int max_permutations_bishops = 512;

// Create all mask permutations for blocker squares for rooks.
constexpr std::array<uint64_t, max_permutations_rooks> create_all_rook_perms(uint64_t movement_mask) {
	std::array<uint64_t, max_permutations_rooks> blocker_boards{};
	std::array<int, 64> move_indices{};
	int move_count = 0;

	// Loop over all squares and check if piece has range here.
	for (int square = 0; square < 64; ++square) {
		// Check if square intersects with movement mask.
		if ((movement_mask >> square) & 1) {
			move_indices[move_count++] = square;
		}
	}

	// Calculate the number of possible permutations.
	int perm_amount = 1 << move_count;

	// Create all possible blocker boards.
	for (int perm_index = 0; perm_index < perm_amount; ++perm_index) {
		uint64_t blocker_board = 0;
		for (int bit_index = 0; bit_index < move_count; ++bit_index) {
			int bit = (perm_index >> bit_index) & 1;
			blocker_board |= static_cast<uint64_t>(bit) << move_indices[bit_index];
		}
		blocker_boards[perm_index] = blocker_board;
	}

	return blocker_boards;
}

// Create all mask permutations for blocker squares for bishops.
constexpr std::array<uint64_t, max_permutations_bishops> create_all_bishop_perms(uint64_t movement_mask) {
	std::array<uint64_t, max_permutations_bishops> blocker_boards{};
	std::array<int, 64> move_indices{};
	int move_count = 0;

	// Loop over all squares and check if piece has range here.
	for (int square = 0; square < 64; ++square) {
		// Check if square intersects with movement mask.
		if ((movement_mask >> square) & 1) {
			move_indices[move_count++] = square;
		}
	}

	// Calculate the number of possible permutations.
	int perm_amount = 1 << move_count;

	// Create all possible blocker boards.
	for (int perm_index = 0; perm_index < perm_amount; ++perm_index) {
		uint64_t blocker_board = 0;
		for (int bit_index = 0; bit_index < move_count; ++bit_index) {
			int bit = (perm_index >> bit_index) & 1;
			blocker_board |= static_cast<uint64_t>(bit) << move_indices[bit_index];
		}
		blocker_boards[perm_index] = blocker_board;
	}

	return blocker_boards;
}

// Make lookup table for rooks.
constexpr std::array<std::array<uint64_t, 4096>, 64> create_rook_attacks() {
	std::array<std::array<uint64_t, 4096>, 64> values{};
	for (int square = 0; square < 64; square++) {
		uint64_t rook_mask = rook_mask_table[square];
		int rook_relevant_bits = __builtin_popcountll(rook_mask);
		auto rook_blocker_boards = create_all_rook_perms(rook_mask);
		for (int board = 0; board < 4096; ++board) {
			uint64_t blocker_board = rook_blocker_boards[board];
			int index = (blocker_board * rook_magic_numbers[square]) >> (64 - rook_relevant_bits);
			values[square][index] = rook_attack_on_fly(square, blocker_board);
		}
	}
	return values;
};

// Make lookup table for bishops.
constexpr std::array<std::array<uint64_t, 512>, 64> create_bishop_attacks() {
	std::array<std::array<uint64_t, 512>, 64> values{};
	for (int square = 0; square < 64; square++) {
		uint64_t bishop_mask = bishop_mask_table[square];
		int bishop_relevant_bits = __builtin_popcountll(bishop_mask);
		auto bishop_blocker_boards = create_all_bishop_perms(bishop_mask);
		for (int board = 0; board < 512; ++board) {
			uint64_t blocker_board = bishop_blocker_boards[board];
			int index = (blocker_board * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits);
			values[square][index] = bishop_attack_on_fly(square, blocker_board);
		}
	}
	return values;
};

// Table instances.
static const std::array<std::array<uint64_t, 512>, 64> bishop_attacks = create_bishop_attacks();
static const std::array<std::array<uint64_t, 4096>, 64> rook_attacks = create_rook_attacks();

constexpr std::array<std::array<uint64_t, 64>, 2> make_pawn_edge_masks() {
	std::array<std::array<uint64_t, 64>, 2> ret = {};

	for (int s = 0; s < 64; s++) {
		ret[0][s] = (0xFF00ULL << (56 - (s & ~7)));
		ret[1][s] = (0xFFULL << (48 - (s & ~7)));
	}

	return ret;
}

static const std::array<std::array<uint64_t, 64>, 2> pawn_edge_masks = make_pawn_edge_masks();

// PEXT

static constexpr uint32_t rook_pext_size = 102400;
static constexpr uint32_t bishop_pext_size = 5248;

// Rook offsets.
static std::array<BitBoard, 64> init_rook_pext_offset() {
	std::array<BitBoard, 64> ret;
	uint32_t offset = 0;
	for (Square s = 0; s < 64; s++) {
		uint32_t perm_amount = 1 << popcnt(rook_mask_table[s]);
		ret[s] = offset;
		offset += perm_amount;
	}

	return ret;
}

static const std::array<BitBoard, 64> rook_pext_offset = init_rook_pext_offset();

// Bishop offsets.
static std::array<BitBoard, 64> init_bishop_pext_offset() {
	std::array<BitBoard, 64> ret;
	uint32_t offset = 0;
	for (Square s = 0; s < 64; s++) {
		uint32_t perm_amount = 1 << popcnt(bishop_mask_table[s]);
		ret[s] = offset;
		offset += perm_amount;
	}

	return ret;
}

static const std::array<BitBoard, 64> bishop_pext_offset = init_bishop_pext_offset();

// Bishop attacks.
static std::array<BitBoard, bishop_pext_size> init_bishop_pext_atk() {
	std::array<BitBoard, bishop_pext_size> ret{};
	uint32_t offset = 0;
	for (Square s = 0; s < 64; ++s) {
		uint32_t mask_bits = popcnt(bishop_mask_table[s]);
		uint32_t perm_amount = 1 << mask_bits;

		for (uint32_t perm = 0; perm < perm_amount; ++perm) {
			BitBoard blocker = _pdep_u64(static_cast<uint64_t>(perm), bishop_mask_table[s]);
			ret[offset + perm] = bishop_attack_on_fly(s, blocker);
		}
		offset += perm_amount;
	}

	return ret;
}
static const std::array<BitBoard, bishop_pext_size> bishop_pext_atk = init_bishop_pext_atk();

// Rook attacks.
static std::array<BitBoard, rook_pext_size> init_rook_pext_atk() {
	std::array<BitBoard, rook_pext_size> ret{};
	uint32_t offset = 0;
	for (Square s = 0; s < 64; ++s) {
		uint32_t mask_bits = popcnt(rook_mask_table[s]);
		uint32_t perm_amount = 1 << mask_bits;

		for (uint32_t perm = 0; perm < perm_amount; ++perm) {
			BitBoard blocker = _pdep_u64(static_cast<uint64_t>(perm), rook_mask_table[s]);
			ret[offset + perm] = rook_attack_on_fly(s, blocker);
		}
		offset += perm_amount;
	}

	return ret;
}
static const std::array<BitBoard, rook_pext_size> rook_pext_atk = init_rook_pext_atk();

// En pessant squares.
static constexpr inline std::pair<Square, Square> ep_sqs_wl[8] =
	{{0, 0}, {24, 17}, {25, 18}, {26, 19}, {27, 20}, {28, 21}, {29, 22}, {30, 23}};

static constexpr inline std::pair<Square, Square> ep_sqs_wr[8] =
	{{25, 16}, {26, 17}, {27, 18}, {28, 19}, {29, 20}, {30, 21}, {31, 22}, {0, 0}};

static constexpr inline std::pair<Square, Square> ep_sqs_bl[8] =
	{{0, 0}, {32, 41}, {33, 42}, {34, 43}, {35, 44}, {36, 45}, {37, 46}, {38, 47}};

static constexpr inline std::pair<Square, Square> ep_sqs_br[8] =
	{{33, 40}, {34, 41}, {35, 42}, {36, 43}, {37, 44}, {38, 45}, {39, 46}, {0, 0}};

#endif
