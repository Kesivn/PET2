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

		//Block& get_block();

		//to do
		void expand(){}


	private:
		int depth;
		Block block;
		std::vector<PetNode*> children;
		//四叉，分别代表嵌入00，01，10，11

		Block& get_block();

	public:
		bool insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value);
		std::optional<uint64_t> queryEdge(uint32_t fp_src, uint32_t fp_dst) const;
		std::optional<uint64_t> queryInDegree(uint32_t fp_dst) const;
		std::optional<uint64_t> queryOutDegree(uint32_t fp_src) const;

		Block_Monitor getBlockMonitor() const;
		void debugPrint(int idx) const;

	private:
		//裁剪指纹高depth位
		uint32_t trim_fp(uint32_t fp) const;
	};

}//namespace pet