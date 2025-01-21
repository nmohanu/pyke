
#include <cstdio>
#include <stdexcept>

#ifndef STACK_H
#define STACK_H

template <typename T>
struct MoveList;

template <typename T>
struct Stack;

// Sublist of a stack. Automatically sets the parent's stack pointer back when
// this list goes out of scope or deleted.
template <typename T>
struct MoveList {
	MoveList(T* start, T* last, Stack<T>& parent) : last(last), start(start), parent(parent) {};
	~MoveList() { parent.set_last(start); }
	size_t size() { return last - start; }
	T* begin() { return start; }
	T* end() { return last; }

private:
	T* start;
	T* last;
	Stack<T>& parent;
};

// Custom stack.
template <typename T>
struct Stack {
	T stack[64];
	Stack() : last(stack) {};
	void push(T val) { *last++ = val; }
	T pop() {
		if (last == stack) return *last;
		return *(--last);
	}
	T& top() { return (*last); }
	T& go_next() { return (*last++); }
	T& minustwo() { return *(last - 2); }
	MoveList<T> from(T* t) { return MoveList<T>(t, last, *this); }
	void destroy(MoveList<T>* sub) { last = sub->begin(); }
	void set_last(T* l) { last = l; }
	void point_prev() { last--; }
	void point_next() { last++; }

private:
	T* last;
};

#endif
