#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "pet.h"

namespace pet {

	template<typename T>

	class Auxo {
	public:
		Auxo();
		~Auxo();

		void insert(const T& u, const T& v, uint64_t w);

		std::optional<uint64_t> queryEdge(const T& u, const T& v) const;
		std::optional<uint64_t> queryInDegree(const T& v) const;
		std::optional<uint64_t> queryOutDegree(const T& u) const;

	private:
		Pet pet;
		uint32_t hashNode(const T& node) const;
	};

}//namespace pet