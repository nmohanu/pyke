
#include <cstdint>

// Custom stack.
template <typename T>
struct Stack {
	T stack[1024];
	void push(T val) { stack[index++] = val; }
	T pop() { return stack[index--]; }
	uint16_t index = 0;
};
