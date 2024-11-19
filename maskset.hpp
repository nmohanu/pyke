#include "defaults.hpp"

#ifndef MASKSET_H
#define MASKSET_H

struct MaskSet {
	// King mask.
	BitBoard king_dg;
	BitBoard king_orth;
	BitBoard king_kn;
	BitBoard king_pw;

	// squares that are empty or enemy;
	BitBoard can_move_to;

	// Pinner pieces
	BitBoard orth_pinners;
	BitBoard dg_pinners;

	// Pinmask.
	BitBoard pinmask_dg;
	BitBoard pinmask_orth;

	// Checkers.
	BitBoard orth_checkers;
	BitBoard dg_checkers;
	BitBoard kn_checkers;
	BitBoard pw_checkers;

	inline uint8_t get_check_cnt() {
		return __builtin_popcount(orth_checkers) + __builtin_popcount(dg_checkers) + __builtin_popcount(kn_checkers)
			+ __builtin_popcount(pw_checkers);
	}

	inline BitBoard get_check_mask() { return orth_checkers | dg_checkers | kn_checkers | pw_checkers; }
};

#endif
