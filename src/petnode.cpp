#include "petnode.h"

#include <algorithm>

namespace pet {

PetNode::PetNode() : depth(0), children() {
    block = new Block;
}

PetNode::PetNode(int _depth) : depth(_depth), children() {
    block = new Block(_depth);
}

PetNode::~PetNode() {
    for (PetNode* child : children) {
        delete child;
    }
    children.clear();
	delete block;
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

Block* PetNode::get_block() {
    return block;
}

uint32_t PetNode::trim_fp(uint32_t fp) const {
    int kept = 32 - depth;
    if (kept <= 0) return 0u;
    if (kept >= 32) return fp;
    uint32_t mask = (kept == 32) ? ~0u : ((1u << kept) - 1u);
    return fp & mask;
}

void PetNode::expand() {
    //产生4个子节点
    for (int i = 0; i < 4; ++i) {
        children.push_back(new PetNode(depth + 1));
    }

    Block::Iterator it(block);
    size_t block_bits = static_cast<size_t>(32 - depth);

    while (it.hasNext()) {
        Edge e = it.next();
        //从block中取出的是没有前缀的
        uint32_t src = e.fp_src; 
        uint32_t dst = e.fp_dst;

        //路由子节点
        uint32_t src_bit = 0u;
        uint32_t dst_bit = 0u;
        if (block_bits > 0) {
            uint32_t shift = static_cast<uint32_t>(block_bits - 1);
            src_bit = (src >> shift) & 1u;
            dst_bit = (dst >> shift) & 1u;
        }
        uint32_t quadrant = (src_bit << 1) | dst_bit;
		children[quadrant]->insert(src, dst, e.value);//子节点的方法会实现裁剪
    }
    delete block;
    block = nullptr;
}

bool PetNode::insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value) {
    uint32_t trimmed_src = trim_fp(fp_src);
    uint32_t trimmed_dst = trim_fp(fp_dst);
    int fail_times = 0;
    //每次插入失败rehash，3次后上报至pet
    while (!block->insert(trimmed_src, trimmed_dst, value)) {
        fail_times++;
        if (fail_times >= 3) {
            return false;
        }
        block->rehash();

    }
    return true;
}

std::optional<uint64_t> PetNode::queryEdge(uint32_t fp_src, uint32_t fp_dst) const {
    uint32_t trimmed_src = trim_fp(fp_src);
    uint32_t trimmed_dst = trim_fp(fp_dst);
    return block->queryEdge(trimmed_src, trimmed_dst);
}

std::optional<uint64_t> PetNode::queryInDegree(uint32_t fp_dst) const {
    uint32_t trimmed_dst = trim_fp(fp_dst);
    return block->queryInDegree(trimmed_dst);
}

std::optional<uint64_t> PetNode::queryOutDegree(uint32_t fp_src) const {
    uint32_t trimmed_src = trim_fp(fp_src);
    return block->queryOutDegree(trimmed_src);
}

Block_Monitor PetNode::getBlockMonitor() const {
    return block->calculateAll();
}

void PetNode::debugPrint(int idx) const {
    std::cout << idx << "号块信息" << ":\n";
    block->debugPrint();
}
} // namespace pet
