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

} // namespace pet
