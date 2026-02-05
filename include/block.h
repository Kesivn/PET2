#pragma once

#include <cstdint>
#include <vector>
#include<iostream>

#include<optional>

namespace pet {

	struct BLOCK_PARAMETER {
		static const int BLOCK_ROWS = 64;
		static const int HASH_NUMS = 5;
	};

	struct Block_Monitor {
		float load_factor = 0.0;
		int occupied_count = 0;
		uint64_t memory_used = 0;
		uint64_t memory_in_theory = 0;
	};

	struct Edge {
		uint32_t fp_src;
		uint32_t fp_dst;
		uint64_t value;
	};

	class Block {
	public://常用插入查询接口
		Block();
		Block(int _bits_used);
		~Block();

		bool insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value);
		std::optional<uint64_t> queryEdge(uint32_t fp_src, uint32_t fp_dst) const;
		std::optional<uint64_t> queryInDegree(uint32_t fp_dst) const;
		std::optional<uint64_t> queryOutDegree(uint32_t fp_src) const;
	private://内部数据结构
		static const BLOCK_PARAMETER info;
		int bits_used = 0;

		std::vector<uint64_t> fp_src_array;
		std::vector<uint64_t> fp_dst_array;
		std::vector<uint64_t> value_array;
		std::vector<char> occupied_array;

		std::vector<uint32_t> hash_row(uint32_t fp_src) const;
		std::vector<uint32_t> hash_col(uint32_t fp_dst) const;
		size_t offset(int row, int col) const;

	public://迭代器相关接口
		class Iterator; // forward declaration
		friend class Iterator; // allow iterator to access private members

		class Iterator {
		public:
			Iterator(const Block* parent = nullptr);
			Edge next();
			bool hasNext() const;
		private:
			const Block* parent;
			size_t index;
			size_t block_size;
		};

	public://再哈希相关接口
		bool is_low_load() const;
		void rehash();
	private:
		int occupied_count = 0;
		int hash_seed = 0;

	public://监控和调试接口
		Block_Monitor calculateAll() const;
		void debugPrint() const;
	

		/*float load_factor = 0.0;
		int occupied_count = 0;

		uint64_t memory_used = 0;
		uint64_t memory_in_theory = 0;
		*/
		//void calculateAll();
	};

}