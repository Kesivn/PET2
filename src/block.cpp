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
		uint32_t base = static_cast<uint32_t>(0x7c5526b3u ^ (static_cast<uint32_t>(hash_seed) * 0x9e3779b9u));
		for(int i=0;i<info.HASH_NUMS;i++){
			uint32_t hash_value = (fp_src ^ base) + static_cast<uint32_t>(i * 17 + hash_seed * 31);
			hash_value %= BLOCK_PARAMETER::BLOCK_ROWS;
			rows.push_back(hash_value);
		}
		return rows;
	}

	std::vector<uint32_t> Block::hash_col(uint32_t fp_dst) const {
		std::vector<uint32_t> cols;
		uint32_t base = static_cast<uint32_t>(0x919cd373u ^ (static_cast<uint32_t>(hash_seed) * 0x85ebca6bu));
		for(int i=0;i<info.HASH_NUMS;i++){
			uint32_t hash_value = (fp_dst ^ base) + static_cast<uint32_t>(i * 13 + hash_seed * 17);
			hash_value %= BLOCK_PARAMETER::BLOCK_ROWS;
			cols.push_back(hash_value);
		}
		return cols;
	}

	size_t Block::offset(int row, int col) const {
		return static_cast<size_t>(row) * BLOCK_PARAMETER::BLOCK_ROWS + static_cast<size_t>(col);
	}

	bool Block::insert(uint32_t fp_src, uint32_t fp_dst, uint64_t value) {

		//size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;
		//size_t words_per_cell = (bits_per_cell + 63) / 64;

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

					occupied_count++;
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
				}
			}
		}

		return false;
	}

	std::optional<uint64_t> Block::queryEdge(uint32_t fp_src, uint32_t fp_dst) const {

		//size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		size_t bits = static_cast<size_t>(bits_used);
		size_t bits_per_cell = bits;
		//size_t words_per_cell = (bits_per_cell + 63) / 64;

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

	void Block::debugPrint() const {
		Block_Monitor monitor =calculateAll();
		std::cout << "Block Debug Print:" << std::endl;
		std::cout << "存储边数量:" << monitor.occupied_count << std::endl;
		std::cout << "负载因子:" << monitor.load_factor << std::endl;
		std::cout << "内存使用:" << monitor.memory_used << "B" << std::endl;
		std::cout << "理论内存占用:" << monitor.memory_in_theory << "B" << std::endl;
	}

	Block_Monitor Block::calculateAll() const {
		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		/*int occupied_count = 0;
		for (size_t i = 0; i < block_size; i++) {
			if (occupied_array[i] != 0) {
				occupied_count++;
			}
		}*/
		float load_factor = static_cast<float>(occupied_count) / static_cast<float>(block_size);
		size_t fp_array_size = (static_cast<size_t>(bits_used) * block_size + 63) / 64;
		uint64_t memory_used = sizeof(char) * block_size + sizeof(uint64_t) * (2 * fp_array_size + block_size);
		uint64_t memory_in_theory = sizeof(char) * occupied_count + sizeof(uint32_t) * (2 * occupied_count) + sizeof(uint64_t) * occupied_count;
		Block_Monitor monitor{
			load_factor,
			occupied_count,
			memory_used,
			memory_in_theory
		};
		return monitor;

	}

	bool Block::is_low_load() const {
		size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
		float load_factor = static_cast<float>(occupied_count) / static_cast<float>(block_size);
		return load_factor < 0.5f;
	}

	void Block::rehash() {
	    size_t block_size = BLOCK_PARAMETER::BLOCK_ROWS * BLOCK_PARAMETER::BLOCK_ROWS;
	    if (block_size == 0) return;

	    size_t bits = static_cast<size_t>(bits_used);
	    size_t bits_per_cell = bits;
	    size_t fp_array_size = (bits_per_cell * block_size + 63) / 64;

	    // backup old storage
	    auto old_fp_src = fp_src_array;
	    auto old_fp_dst = fp_dst_array;
	    auto old_value = value_array;
	    auto old_occupied = occupied_array;

	    const int max_attempts = 8;
	    for (int attempt = 0; attempt < max_attempts; ++attempt) {
	        ++hash_seed; // change seed

	        std::vector<uint64_t> new_fp_src(fp_array_size, 0);
	        std::vector<uint64_t> new_fp_dst(fp_array_size, 0);
	        std::vector<uint64_t> new_value(block_size, 0);
	        std::vector<char> new_occupied(block_size, 0);

	        bool success = true;

	        for (size_t off = 0; off < block_size; ++off) {
	            if (old_occupied[off] == 0) continue;

	            uint64_t val = old_value[off];
	            uint32_t src_fp = 0, dst_fp = 0;

	            if (bits != 0) {
	                size_t start_bit = off * bits_per_cell;
	                size_t word_idx = start_bit / 64;
	                size_t bit_offset = start_bit % 64;

	                uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1ULL);

	                if (bit_offset + bits_per_cell <= 64) {
	                    src_fp = static_cast<uint32_t>((old_fp_src[word_idx] >> bit_offset) & fp_mask);
	                    dst_fp = static_cast<uint32_t>((old_fp_dst[word_idx] >> bit_offset) & fp_mask);
	                } else {
	                    uint64_t low_s = old_fp_src[word_idx] >> bit_offset;
	                    uint64_t high_s = old_fp_src[word_idx + 1] << (64 - bit_offset);
	                    src_fp = static_cast<uint32_t>((low_s | high_s) & fp_mask);

	                    uint64_t low_d = old_fp_dst[word_idx] >> bit_offset;
	                    uint64_t high_d = old_fp_dst[word_idx + 1] << (64 - bit_offset);
	                    dst_fp = static_cast<uint32_t>((low_d | high_d) & fp_mask);
	                }
	            }

	            // try to place into new arrays using new hash functions
	            auto rows = hash_row(src_fp);
	            auto cols = hash_col(dst_fp);

	            uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((bits == 0) ? 0ULL : ((1ULL << bits) - 1ULL));

	            bool placed = false;
	            for (int i = 0; i < info.HASH_NUMS && !placed; ++i) {
	                for (int j = 0; j < info.HASH_NUMS && !placed; ++j) {
	                    size_t new_off = offset(rows[i], cols[j]);
	                    if (new_occupied[new_off] == 0) {
	                        // place
	                        new_occupied[new_off] = 1;
	                        new_value[new_off] = val;

	                        if (bits != 0) {
	                            size_t start_bit2 = new_off * bits_per_cell;
	                            size_t word_idx2 = start_bit2 / 64;
	                            size_t bit_offset2 = start_bit2 % 64;

	                            if (bit_offset2 + bits_per_cell <= 64) {
	                                new_fp_src[word_idx2] |= (static_cast<uint64_t>(src_fp) & fp_mask) << bit_offset2;
	                                new_fp_dst[word_idx2] |= (static_cast<uint64_t>(dst_fp) & fp_mask) << bit_offset2;
	                            } else {
	                                new_fp_src[word_idx2] |= (static_cast<uint64_t>(src_fp) & fp_mask) << bit_offset2;
	                                new_fp_src[word_idx2 + 1] |= (static_cast<uint64_t>(src_fp) & fp_mask) >> (64 - bit_offset2);

	                                new_fp_dst[word_idx2] |= (static_cast<uint64_t>(dst_fp) & fp_mask) << bit_offset2;
	                                new_fp_dst[word_idx2 + 1] |= (static_cast<uint64_t>(dst_fp) & fp_mask) >> (64 - bit_offset2);
	                            }
	                        }

	                        placed = true;
	                    } else {
	                        // check match
	                        if (bits == 0) {
	                        // if bits == 0, treat as match only if both slots considered same (no fingerprints) -> we consider not match
	                        } else {
	                            size_t start_bit2 = new_off * bits_per_cell;
	                            size_t word_idx2 = start_bit2 / 64;
	                            size_t bit_offset2 = start_bit2 % 64;

	                            uint64_t existing_src;
	                            if (bit_offset2 + bits_per_cell <= 64) {
	                                existing_src = (new_fp_src[word_idx2] >> bit_offset2) & fp_mask;
	                            } else {
	                                uint64_t low = new_fp_src[word_idx2] >> bit_offset2;
	                                uint64_t high = new_fp_src[word_idx2 + 1] << (64 - bit_offset2);
	                                existing_src = (low | high) & fp_mask;
	                            }

	                            uint64_t existing_dst;
	                            if (bit_offset2 + bits_per_cell <= 64) {
	                                existing_dst = (new_fp_dst[word_idx2] >> bit_offset2) & fp_mask;
	                            } else {
	                                uint64_t low = new_fp_dst[word_idx2] >> bit_offset2;
	                                uint64_t high = new_fp_dst[word_idx2 + 1] << (64 - bit_offset2);
	                                existing_dst = (low | high) & fp_mask;
	                            }

	                            if (existing_src == src_fp && existing_dst == dst_fp) {
	                                // update value
	                                new_value[new_off] = val;
	                                placed = true;
	                            }
	                        }
	                    }
	                }
	            }

	            if (!placed) {
	                success = false;
	                break;
	            }
	        }

	        if (success) {
	            // commit
	            fp_src_array.swap(new_fp_src);
	            fp_dst_array.swap(new_fp_dst);
	            value_array.swap(new_value);
	            occupied_array.swap(new_occupied);
	            return;
	        }
	        // otherwise try next seed
	    }

	    // if all attempts failed, leave original arrays; hash_seed has been advanced a few times
	}
 
 Block::Iterator::Iterator(const Block* parent)
     : parent(parent)
     , index(0)
     , block_size(static_cast<size_t>(BLOCK_PARAMETER::BLOCK_ROWS) * static_cast<size_t>(BLOCK_PARAMETER::BLOCK_ROWS))
 {
     if (this->parent) {
         while (index < block_size && this->parent->occupied_array[index] == 0) ++index;
     }
 }
 
 bool Block::Iterator::hasNext() const {
     if (!parent) return false;
     size_t idx = index;
     while (idx < block_size && parent->occupied_array[idx] == 0) ++idx;
     return idx < block_size;
 }
 
 Edge Block::Iterator::next() {
     Edge e{0,0,0};
     if (!parent) return e;
 
     while (index < block_size && parent->occupied_array[index] == 0) ++index;
     if (index >= block_size) return e;
 
     size_t off = index;
     e.value = parent->value_array[off];
 
     size_t bits = static_cast<size_t>(parent->bits_used);
     if (bits == 0) {
         e.fp_src = 0;
         e.fp_dst = 0;
     } else {
         size_t bits_per_cell = bits;
         size_t start_bit = off * bits_per_cell;
         size_t word_idx = start_bit / 64;
         size_t bit_offset = start_bit % 64;
 
         uint64_t fp_mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1ULL);
 
         uint64_t extracted_src;
         if (bit_offset + bits_per_cell <= 64) {
             extracted_src = (parent->fp_src_array[word_idx] >> bit_offset) & fp_mask;
         } else {
             uint64_t low = parent->fp_src_array[word_idx] >> bit_offset;
             uint64_t high = parent->fp_src_array[word_idx + 1] << (64 - bit_offset);
             extracted_src = (low | high) & fp_mask;
         }
 
         uint64_t extracted_dst;
         if (bit_offset + bits_per_cell <= 64) {
             extracted_dst = (parent->fp_dst_array[word_idx] >> bit_offset) & fp_mask;
         } else {
             uint64_t low = parent->fp_dst_array[word_idx] >> bit_offset;
             uint64_t high = parent->fp_dst_array[word_idx + 1] << (64 - bit_offset);
             extracted_dst = (low | high) & fp_mask;
         }
 
         e.fp_src = static_cast<uint32_t>(extracted_src);
         e.fp_dst = static_cast<uint32_t>(extracted_dst);
     }

     ++index;
     while (index < block_size && parent->occupied_array[index] == 0) ++index;
 
     return e;
 }
 
 }