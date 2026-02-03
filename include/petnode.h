#pragma once

#include"block.h"

namespace pet {
	class PetNode {
	public:
		PetNode();
		PetNode(int _depth);

		~PetNode();

		bool is_leaf() const;
		int get_depth() const;
		PetNode* get_child(int index) const;

		Block& get_block();

		//to do
		void expand(){}


	private:
		int depth;
		Block block;
		std::vector<PetNode*> children;
		//四叉，分别代表嵌入00，01，10，11
	};
}//namespace pet