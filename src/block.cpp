#include "block.h"

namespace pet{
	const BLOCK_PARAMETER Block::info;

	Block::Block(){
		bits_used = 0;
		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		fp_src_array.resize(block_size, 0);
		fp_dst_array.resize(block_size, 0);
		value_array.resize(block_size, 0);
		occupied_array.resize(block_size, 0);
	}
	Block::Block(int _bits_used){
		bits_used = _bits_used;
		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		size_t fp_array_size = (static_cast<size_t>(bits_used) * block_size + 63) / 64;

		fp_src_array.resize(fp_array_size, 0);
		fp_dst_array.resize(fp_array_size, 0);
		value_array.resize(block_size, 0);
		occupied_array.resize(block_size, 0);
	}

	Block::~Block() = default;

	std::vector<uint32_t> Block::hash_row(uint32_t fp_src) const {
		std::vector<uint32_t> rows;
		for(int i=0;i<info.HASH_NUMS;i++){
			uint32_t hash_value = (fp_src ^ 0x7c5526b3u) + static_cast<uint32_t>(i * 17);
			hash_value %= BLOCK_PARAMETER::BLOCK_ROWS;
			rows.push_back(hash_value);
		}
		return rows;
	}

	std::vector<uint32_t> Block::hash_col(uint32_t fp_dst) const {
		std::vector<uint32_t> cols;
		for(int i=0;i<info.HASH_NUMS;i++){
			uint32_t hash_value = (fp_dst ^ 0x919cd373u) + static_cast<uint32_t>(i * 13);
			hash_value %= BLOCK_PARAMETER::BLOCK_ROWS;
			cols.push_back(hash_value);
		}
		return cols;
	}

	size_t Block::offset(int row, int col) const {
		return static_cast<size_t>(row) * BLOCK_PARAMETER::BLOCK_ROWS + static_cast<size_t>(col);
	}

	bool Block::insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value) {

		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;
		size_t words_per_cell = (bits_per_cell + 63) / 64;

		auto rows = hash_row(fp_src);
		auto cols = hash_col(fp_dst);

		uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1ULL);
		uint64_t src_val = static_cast<uint64_t>(fp_src) & fp_mask;
		uint64_t dst_val = static_cast<uint64_t>(fp_dst) & fp_mask;

		for(int i=0;i<info.HASH_NUMS;i++){
			for(int j=0;j<info.HASH_NUMS;j++){
				//生成m*m个候选桶
				size_t off = offset(rows[i], cols[j]);
				if(occupied_array[off]==0){
					
					occupied_array[off] = 1;
					value_array[off] = value;

					//针对bits_used打包指纹
					size_t start_bit = off * bits_per_cell;
					size_t word_idx = start_bit / 64;
					size_t bit_offset = start_bit % 64;

					// 不跨位和跨位两种情况
					if(bit_offset + bits_per_cell <= 64){
						fp_src_array[word_idx] |= (src_val & fp_mask) << bit_offset;
					} else {
						fp_src_array[word_idx] |= (src_val & fp_mask) << bit_offset;
						fp_src_array[word_idx + 1] |= (src_val & fp_mask) >> (64 - bit_offset);
					}

					if(bit_offset + bits_per_cell <= 64){
						fp_dst_array[word_idx] |= (dst_val & fp_mask) << bit_offset;
					} else {
						fp_dst_array[word_idx] |= (dst_val & fp_mask) << bit_offset;
						fp_dst_array[word_idx + 1] |= (dst_val & fp_mask) >> (64 - bit_offset);
					}

					return true;
				} else {
					//桶已占用，尝试比对，成功则更新权重
					size_t start_bit = off * bits_per_cell;
					size_t word_idx = start_bit / 64;
					size_t bit_offset = start_bit % 64;

					uint64_t extracted_src;
					if(bit_offset + bits_per_cell <= 64){
						extracted_src = (fp_src_array[word_idx] >> bit_offset) & fp_mask;
					} else {
						uint64_t low = fp_src_array[word_idx] >> bit_offset;
						uint64_t high = fp_src_array[word_idx + 1] << (64 - bit_offset);
						extracted_src = (low | high) & fp_mask;
					}

					uint64_t extracted_dst;
					if(bit_offset + bits_per_cell <= 64){
						extracted_dst = (fp_dst_array[word_idx] >> bit_offset) & fp_mask;
					} else {
						uint64_t low = fp_dst_array[word_idx] >> bit_offset;
						uint64_t high = fp_dst_array[word_idx + 1] << (64 - bit_offset);
						extracted_dst = (low | high) & fp_mask;
					}

					if(extracted_src == src_val && extracted_dst == dst_val){
						value_array[off] = value;
						return true;
					}
					// else continue to next candidate
				}
			}
		}

		return false;
	}

	std::optional<uint64_t> Block::queryEdge(uint32_t fp_src, uint32_t fp_dst) const {

		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;
		size_t words_per_cell = (bits_per_cell + 63) / 64;

		auto rows = hash_row(fp_src);
		auto cols = hash_col(fp_dst);

		uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1ULL);
		uint64_t src_val = static_cast<uint64_t>(fp_src) & fp_mask;
		uint64_t dst_val = static_cast<uint64_t>(fp_dst) & fp_mask;

		for (int i = 0; i < info.HASH_NUMS; i++) {
			for (int j = 0; j < info.HASH_NUMS; j++) {
				//生成m*m个候选桶
				size_t off = offset(rows[i], cols[j]);
				if (occupied_array[off] == 1) {
					size_t start_bit = off * bits_per_cell;
					size_t word_idx = start_bit / 64;
					size_t bit_offset = start_bit % 64;

					uint64_t extracted_src;
					if (bit_offset + bits_per_cell <= 64) {
						extracted_src = (fp_src_array[word_idx] >> bit_offset) & fp_mask;
					}
					else {
						uint64_t low = fp_src_array[word_idx] >> bit_offset;
						uint64_t high = fp_src_array[word_idx + 1] << (64 - bit_offset);
						extracted_src = (low | high) & fp_mask;
					}

					uint64_t extracted_dst;
					if (bit_offset + bits_per_cell <= 64) {
						extracted_dst = (fp_dst_array[word_idx] >> bit_offset) & fp_mask;
					}
					else {
						uint64_t low = fp_dst_array[word_idx] >> bit_offset;
						uint64_t high = fp_dst_array[word_idx + 1] << (64 - bit_offset);
						extracted_dst = (low | high) & fp_mask;
					}

					if (extracted_src == src_val && extracted_dst == dst_val) {
						return value_array[off];
					}
				}
				
			}
		}

		return std::nullopt;
	}

	std::optional<uint64_t> Block::queryInDegree(uint32_t fp_dst) const {
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;

		auto cols = hash_col(fp_dst);

		uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((bits == 0) ? 0ULL : ((1ULL << bits) - 1ULL));
		uint64_t dst_val = static_cast<uint64_t>(fp_dst) & fp_mask;

		uint64_t total = 0;
		bool found = false;

		for (int j = 0; j < info.HASH_NUMS; j++) {
			int col = cols[j];
			for (int row = 0; row < BLOCK_PARAMETER::BLOCK_ROWS; row++) {
				size_t off = offset(row, col);
				if (occupied_array[off] == 0) continue;

				if (bits == 0) {
					total += value_array[off];
					found = true;
					continue;
				}

				size_t start_bit = off * bits_per_cell;
				size_t word_idx = start_bit / 64;
				size_t bit_offset = start_bit % 64;

				uint64_t extracted_dst;
				if (bit_offset + bits_per_cell <= 64) {
					extracted_dst = (fp_dst_array[word_idx] >> bit_offset) & fp_mask;
				} else {
					uint64_t low = fp_dst_array[word_idx] >> bit_offset;
					uint64_t high = fp_dst_array[word_idx + 1] << (64 - bit_offset);
					extracted_dst = (low | high) & fp_mask;
				}

				if (extracted_dst == dst_val) {
					total += value_array[off];
					found = true;
				}
			}
		}

		if (!found) return std::nullopt;
		return total;
	}

	std::optional<uint64_t> Block::queryOutDegree(uint32_t fp_src) const {
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;

		auto rows = hash_row(fp_src);

		uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((bits == 0) ? 0ULL : ((1ULL << bits) - 1ULL));
		uint64_t src_val = static_cast<uint64_t>(fp_src) & fp_mask;

		uint64_t total = 0;
		bool found = false;

		for (int i = 0; i < info.HASH_NUMS; i++) {
			int row = rows[i];
			for (int col = 0; col < BLOCK_PARAMETER::BLOCK_ROWS; col++) {
				size_t off = offset(row, col);
				if (occupied_array[off] == 0) continue;

				if (bits == 0) {
					total += value_array[off];
					found = true;
					continue;
				}

				size_t start_bit = off * bits_per_cell;
				size_t word_idx = start_bit / 64;
				size_t bit_offset = start_bit % 64;

				uint64_t extracted_src;
				if (bit_offset + bits_per_cell <= 64) {
					extracted_src = (fp_src_array[word_idx] >> bit_offset) & fp_mask;
				} else {
					uint64_t low = fp_src_array[word_idx] >> bit_offset;
					uint64_t high = fp_src_array[word_idx + 1] << (64 - bit_offset);
					extracted_src = (low | high) & fp_mask;
				}

				if (extracted_src == src_val) {
					total += value_array[off];
					found = true;
				}
			}
		}

		if (!found) return std::nullopt;
		return total;
	}

	void Block::debugPrint() {
		calculateAll();
		std::cout << "Block Debug Print:" << std::endl;
		std::cout << "存储边数量:" << occupied_count << std::endl;
		std::cout << "负载因子:" << load_factor << std::endl;
		std::cout << "内存使用:" << memory_used << "B" << std::endl;
		std::cout << "理论内存占用:" << memory_in_theory << "B" << std::endl;
	}

	void Block::calculateAll() {
		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		occupied_count = 0;
		for (size_t i = 0; i < block_size; i++) {
			if (occupied_array[i] != 0) {
				occupied_count++;
			}
		}
		load_factor = static_cast<float>(occupied_count) / static_cast<float>(block_size);
		size_t fp_array_size = (static_cast<size_t>(bits_used) * block_size + 63) / 64;
		memory_used = sizeof(char) * block_size + sizeof(uint64_t) * (2 * fp_array_size + block_size);
		memory_in_theory = sizeof(char) * occupied_count + sizeof(uint32_t) * (2 * occupied_count) + sizeof(uint64_t) * occupied_count;
	}
}