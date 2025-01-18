#include <cstdint>

#include "defaults.hpp"
#include "lookup_tables.hpp"

#ifndef GAMESTATE_H
#define GAMESTATE_H

constexpr CastlingRights make_cr_flag(bool bk, bool bq, bool wk, bool wq) {
	uint8_t ret = 0b0;
	ret |= wk ? (1ULL << 3) : 0;
	ret |= wq ? (1ULL << 2) : 0;
	ret |= bk ? (1ULL << 1) : 0;
	ret |= bq ? (1ULL) : 0;
	return ret;
}

constexpr inline CastlingRights rm_cr_w(CastlingRights flag) { return flag & 0b0011; }
constexpr inline CastlingRights rm_cr_b(CastlingRights flag) { return flag & 0b1100; }
constexpr inline CastlingRights rm_cr_wk(CastlingRights flag) { return flag & ~wk_mask; }
constexpr inline CastlingRights rm_cr_wq(CastlingRights flag) { return flag & ~wq_mask; }
constexpr inline CastlingRights rm_cr_bk(CastlingRights flag) { return flag & ~bk_mask; }
constexpr inline CastlingRights rm_cr_bq(CastlingRights flag) { return flag & ~bq_mask; }

constexpr inline bool get_cr_bq(CastlingRights flag) { return flag & bq_mask; }
constexpr inline bool get_cr_bk(CastlingRights flag) { return flag & bk_mask; }
constexpr inline bool get_cr_wq(CastlingRights flag) { return flag & wq_mask; }
constexpr inline bool get_cr_wk(CastlingRights flag) { return flag & wk_mask; }

template <bool white>
constexpr inline CastlingRights rm_cr(CastlingRights flag) {
	return white ? rm_cr_w(flag) : rm_cr_b(flag);
}

template <bool white, bool kingside>
constexpr inline CastlingRights rm_cr(CastlingRights flag) {
	return white ? kingside ? rm_cr_wk(flag) : rm_cr_wq(flag) : kingside ? rm_cr_bk(flag) : rm_cr_bq(flag);
}

template <bool white, bool kingside, CastlingRights cr>
constexpr inline bool has_cr_right() {
	return white ? kingside ? get_cr_wk(cr) : get_cr_wq(cr) : kingside ? get_cr_bk(cr) : get_cr_bq(cr);
}

inline void set_en_passant(bool left, uint8_t file, uint8_t& data) {
	if (left) {
		data |= 0b1000'0000;
	} else {
		data |= 0b0100'0000;
	}
	data |= file;
}

template <bool white, int offset>
inline std::pair<Square, Square> get_ep_squares(uint8_t ep) {
	if constexpr (white && (offset == -1)) {
		return ep_sqs_wl[ep & 0b00001111];
	} else if constexpr (white) {
		return ep_sqs_wr[ep & 0b00001111];
	} else if constexpr (!white && (offset == -1)) {
		return ep_sqs_bl[ep & 0b00001111];
	} else {
		return ep_sqs_br[ep & 0b00001111];
	}
}
#endif
