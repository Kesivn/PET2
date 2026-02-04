#include<queue>

#include "pet.h"

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
		return leaf->insert(src, dst, value);

		//to do
		//拓展逻辑的实现
	}

	std::optional<uint64_t> Pet::queryEdge(uint32_t src, uint32_t dst) const {
		PetNode* leaf = findLeafNode(src, dst);
		return leaf->queryEdge(src, dst);
	}

	std::optional<uint64_t> Pet::queryInDegree(uint32_t dst) const {
		uint64_t total_in_degree = 0;
		std::queue<PetNode*> q;
		q.push(root);
		while(!q.empty()){
			PetNode* current = q.front();
			q.pop();
			if(current->is_leaf()){
				//在叶子节点上查询入度
				auto res = current->queryInDegree(dst);
				if (res.has_value()) {
					total_in_degree += res.value();
				}
			}else{
				//将子节点加入队列
				int cur_depth = current->get_depth();
				int dst_bit = (dst >> (32 - cur_depth)) & 1;
				if (dst_bit == 1) {
					//01 或者 11
					q.push(current->get_child(1));
					q.push(current->get_child(3));
				}
				else {
					//00 或者 10
					q.push(current->get_child(0));
					q.push(current->get_child(2));
				}
			}
		}
		return total_in_degree == 0 ? std::nullopt : std::optional<uint64_t>(total_in_degree);
	}

	std::optional<uint64_t> Pet::queryOutDegree(uint32_t src) const {
		uint64_t total_out_degree = 0;
		std::queue<PetNode*> q;
		q.push(root);
		while(!q.empty()){
			PetNode* current = q.front();
			q.pop();
			if(current->is_leaf()){
				//在叶子节点上查询出度
				auto res = current->queryOutDegree(src);
				if (res.has_value()) {
					total_out_degree += res.value();
				}
			}else{
				//将子节点加入队列
				int cur_depth = current->get_depth();
				int src_bit = (src >> (32 - cur_depth)) & 1;
				if (src_bit == 1) {
					//10 或者 11
					q.push(current->get_child(2));
					q.push(current->get_child(3));
				}
				else {
					//00 或者 01
					q.push(current->get_child(0));
					q.push(current->get_child(1));
				}
			}
		}
		return total_out_degree == 0 ? std::nullopt : std::optional<uint64_t>(total_out_degree);
		
	}

	void Pet::debugPrint(bool show_details) const {
		float load_factor_sum = 0.0;
		uint64_t memory_used_sum = 0;
		uint64_t memory_in_theory_sum = 0;
		int leaf_count = 0;
		std::queue<PetNode*> q;
		q.push(root);
		while(!q.empty()){
			PetNode* current = q.front();
			q.pop();
			if(current->is_leaf()){
				leaf_count++;
				if(show_details){
					current->debugPrint(leaf_count);
				}
				Block_Monitor monitor = current->getBlockMonitor();
				load_factor_sum += monitor.load_factor;
				memory_used_sum += monitor.memory_used;
				memory_in_theory_sum += monitor.memory_in_theory;
			}else{
				for(int i = 0; i < 4; i++){
					q.push(current->get_child(i));
				}
			}
		}
		float average_load_factor = leaf_count == 0 ? 0.0 : load_factor_sum / leaf_count;
		printf("PET总览:\n");
		printf("叶子总数: %d\n", leaf_count);
		printf("平均负载: %.4f\n", average_load_factor);
		printf("内存使用: %llu bytes\n", memory_used_sum);
		printf("存储内容理论占用内存: %llu bytes\n", memory_in_theory_sum);

	}


}//namespace pet