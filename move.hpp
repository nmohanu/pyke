#include <cstdint>

#include "defaults.hpp"

#ifndef MOVE_H
#define MOVE_H

struct Move {
	Move() {};
	/*
	Move(MoveType t, Piece p) {
		set_type(t);
		set_piece(p);
	}
	Move(Piece p) { set_piece(p); }
	Move(MoveType t, Piece p, Square f, Square to) {
		set_type(t);
		set_piece(p);
		set_from(f);
		set_to(to);
	}
	inline void set_to(Square s) { data = (data & ~(0xFD << 16)) | ((s) << 16); }
	inline void set_piece(Piece p) { data = (data & ~(0b101 << 13)) | ((p) << 13); }
	inline void set_from(Square f) { data = data & ~(0xFD << 24) | (f) << 24; }
	inline void set_type(uint8_t t) { data = data & ~(0b101) | t; }
	inline void set_content(uint8_t c) { data = data & ~(0b101 << 8) | c << 8; }

	inline Square get_to() const { return (data >> 14) & 0b1111'1111; }
	inline Square get_from() const { return (data >> 22) & 0b1111'1111; }
	inline MoveType get_type() const { return data & 0b101; }
	inline uint8_t get_content() const { return (data >> 6) & 0b1111; }
	inline Piece get_piece() const { return (data >> 11) & 0b111; }
*/
private:
	uint32_t data = 0b0;
};

#endif
