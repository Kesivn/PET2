#include <pet.h>

namespace pet {
	Pet::Pet(){
		root = new PetNode();
		tree_height = 1;
	}
	Pet::~Pet(){
		delete root;
	}

	PetNode* Pet::findLeafNode(uint32_t src, uint32_t dst) const {
		PetNode* current = root;
		int current_depth = 1;
		while(!current->is_leaf()){
			int index = 0;
			//根据src和dst的高位决定下一个子节点的索引
			uint32_t src_bit = (src >> (32 - current_depth)) & 1;
			uint32_t dst_bit = (dst >> (32 - current_depth)) & 1;
			index = (src_bit << 1) | dst_bit;
			current = current->get_child(index);
			current_depth++;
		}
		return current;
	}

	bool Pet::insert(uint32_t src, uint32_t dst, uint64_t value){
		PetNode* leaf = findLeafNode(src, dst);
		return leaf->get_block().insert(src, dst, value);
	}

	std::optional<uint64_t> Pet::queryEdge(uint32_t src, uint32_t dst) const {
		PetNode* leaf = findLeafNode(src, dst);
		return leaf->get_block().queryEdge(src, dst);
	}

	

}//namespace pet