#include "petnode.h"

#include <algorithm>

namespace pet {

PetNode::PetNode() : depth(0), block(), children() {}

PetNode::PetNode(int _depth) : depth(_depth), block(_depth), children() {}

PetNode::~PetNode() {
    for (PetNode* child : children) {
        delete child;
    }
    children.clear();
}

bool PetNode::is_leaf() const {
    return children.empty();
}

int PetNode::get_depth() const {
    return depth;
}

PetNode* PetNode::get_child(int index) const {
    if (index < 0 || index > 3 || is_leaf()) return nullptr;
    return children[index];
}

Block& PetNode::get_block() {
    return block;
}

uint32_t PetNode::trim_fp(uint32_t fp) const {
    return fp & ((1u << (32 - depth)) - 1u);
}

bool PetNode::insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value) {
    uint32_t trimmed_src = trim_fp(fp_src);
    uint32_t trimmed_dst = trim_fp(fp_dst);
    return block.insert(trimmed_src, trimmed_dst, value);
}

std::optional<uint64_t> PetNode::queryEdge(uint32_t fp_src, uint32_t fp_dst) const {
    uint32_t trimmed_src = trim_fp(fp_src);
    uint32_t trimmed_dst = trim_fp(fp_dst);
    return block.queryEdge(trimmed_src, trimmed_dst);
}

std::optional<uint64_t> PetNode::queryInDegree(uint32_t fp_dst) const {
    uint32_t trimmed_dst = trim_fp(fp_dst);
    return block.queryInDegree(trimmed_dst);
}

std::optional<uint64_t> PetNode::queryOutDegree(uint32_t fp_src) const {
    uint32_t trimmed_src = trim_fp(fp_src);
    return block.queryOutDegree(trimmed_src);
}

Block_Monitor PetNode::getBlockMonitor() const {
	return block.calculateAll();
}

void PetNode::debugPrint(int idx) const {
    std::cout << idx << "ºÅ¿éÐÅÏ¢" << ":\n";
    block.debugPrint();
}
} // namespace pet
