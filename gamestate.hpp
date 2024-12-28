#include <cstdint>

#ifndef GAMESTATE_H
#define GAMESTATE_H

#define bq_mask (1ULL << 8)
#define bk_mask (1ULL << 9)
#define wq_mask (1ULL << 10)
#define wk_mask (1ULL << 11)

struct GameState {
public:
	inline void set_en_passant(bool left, uint8_t file) {
		if (left) {
			data |= 0b1000'0000;
		} else {
			data |= 0b0100'0000;
		}
		data |= file;
	}

	inline void reset_en_passant() { data &= ~0xff; }

	inline uint8_t get_en_passant() { return data & 0xff; }

	// Remove castling rights side specific.
	inline void rm_cr_bq() { data &= ~bq_mask; }
	inline uint8_t get_cr_bq() { return data & bq_mask; }
	inline void rm_cr_bk() { data &= ~bk_mask; }
	inline uint8_t get_cr_bk() { return data & bk_mask; }
	inline void rm_cr_wq() { data &= ~wq_mask; }
	inline uint8_t get_cr_wq() { return data & wq_mask; }
	inline void rm_cr_wk() { data &= ~wk_mask; }
	inline uint8_t get_cr_wk() { return data & wk_mask; }

	// Remove castling rights color specific.
	inline void rm_cr_b() {
		rm_cr_bk();
		rm_cr_bq();
	}

	inline void rm_cr_w() {
		rm_cr_wk();
		rm_cr_wq();
	}

	template <bool white>
	void rm_cr() {
		if constexpr (white)
			rm_cr_w();
		else
			rm_cr_b();
	}

	template <bool white>
	inline bool can_castle_queen() {
		return white ? get_cr_wq() : get_cr_bq();
	}

	template <bool white>
	inline bool can_castle_king() {
		return white ? get_cr_wk() : get_cr_bk();
	}

	uint32_t get_data() { return data; }
	void set_data(const uint32_t d) { data = d; }

private:
	uint32_t data = 0;
};

#endif
