#include "block_test.h"
#include <iostream>
#include <cassert>
#include <random>

namespace pet {

BlockTest::BlockTest(int _bits_used, int _test_mode)
: block(_bits_used), test_mode(_test_mode) {}

void BlockTest::insert() {
    if (test_mode == 0) makeRegularInsert();
    else makeIrregularInsert();
}

void BlockTest::query() {
    // Basic queries corresponding to inserts from makeRegularInsert
    // Query edge (1,2) and (1,3)
    auto v12 = block.queryEdge(1,2);
    if (v12) std::cout << "queryEdge(1,2) = " << *v12 << "\n";
    else std::cout << "queryEdge(1,2) not found\n";

    auto v13 = block.queryEdge(1,3);
    if (v13) std::cout << "queryEdge(1,3) = " << *v13 << "\n";
    else std::cout << "queryEdge(1,3) not found\n";

    auto out = block.queryOutDegree(1);
    if (out) std::cout << "queryOutDegree(1) = " << *out << "\n";
    else std::cout << "queryOutDegree(1) not found\n";

    auto in2 = block.queryInDegree(2);
    if (in2) std::cout << "queryInDegree(2) = " << *in2 << "\n";
    else std::cout << "queryInDegree(2) not found\n";

    auto in3 = block.queryInDegree(3);
    if (in3) std::cout << "queryInDegree(3) = " << *in3 << "\n";
    else std::cout << "queryInDegree(3) not found\n";
}

void BlockTest::runAllTests() {
    std::cout << "Running Block tests (bits_used=" << /* not accessible directly; print placeholder */ "?" << ")\n";
    insert();
    query();
    block.debugPrint();
    std::cout << "Tests finished.\n";
}

void BlockTest::makeRegularInsert() {
    // Insert predictable edges and validate basic behaviors
    bool r;

    r = block.insert(1, 2, 10);
    assert(r && "Insert (1,2,10) failed");

    r = block.insert(1, 3, 20);
    assert(r && "Insert (1,3,20) failed");

    auto v = block.queryEdge(1,2);
    assert(v && *v == 10 && "queryEdge after first insert mismatch");

    // Update existing edge (same fingerprint pair)
    r = block.insert(1, 2, 30);
    assert(r && "Update insert (1,2,30) failed");

    v = block.queryEdge(1,2);
    assert(v && *v == 30 && "queryEdge after update mismatch");

    // Validate out-degree and in-degree sums
    auto out = block.queryOutDegree(1);
    if (out) {
        // out should be 30 (for 1->2) + 20 (for 1->3) = 50 in normal conditions
        std::cout << "Out degree for 1 = " << *out << "\n";
    }

    auto in2 = block.queryInDegree(2);
    if (in2) {
        std::cout << "In degree for 2 = " << *in2 << "\n";
    }

    std::cout << "makeRegularInsert finished\n";
}

void BlockTest::makeIrregularInsert() {
    // Attempt many random inserts to exercise collision handling and eventual failures
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<uint32_t> dist_fp(0, 0xFFFFFFFFu);
    size_t success = 0;
    size_t fail = 0;

    for (size_t i = 0; i < 6000; ++i) {
        uint32_t s = dist_fp(rng);
        uint32_t d = dist_fp(rng);
        uint64_t val = i + 1;
        if (block.insert(s, d, val)) ++success;
        else ++fail;
    }

    std::cout << "makeIrregularInsert: success=" << success << " fail=" << fail << "\n";
    assert(success > 0 && "No successful inserts in irregular test");
}

} // namespace pet
