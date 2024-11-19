
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
	T stack[1024];
	Stack() : last(stack) {};
	void push(T val) {
		if (last >= stack + 1024) throw std::out_of_range("Stack overflow");
		*last++ = val;
	}
	T pop() {
		if (last == stack) return *last;
		return *(--last);
	}
	T* top() { return (last); }
	MoveList<T> from(T* t) {
		if (t < stack || t > last) throw std::out_of_range("Invalid substack range");
		return MoveList<T>(t, last, *this);
	}
	void destroy(MoveList<T>* sub) { last = sub->begin(); }
	void set_last(T* l) { last = l; }

private:
	T* last;
};

#endif
