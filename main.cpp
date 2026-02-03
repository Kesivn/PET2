#include"../tests/block_test.h"

int main() {

	//pet::BlockTest test(16, 0); // Using 16 bits for fingerprints, regular insert mode
	pet::BlockTest test2(24, 1); // Using 16 bits for fingerprints, irregular insert mode
	test2.runAllTests();
	return 0;
}