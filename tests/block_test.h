#include"block.h"

namespace pet {
	class BlockTest {
	public:
		BlockTest(int _bits_used, int _test_mode);
		void insert();
		void query();
		void runAllTests();
	private:
		Block block;
		int test_mode;

		void makeRegularInsert();
		void makeIrregularInsert();
	};
}