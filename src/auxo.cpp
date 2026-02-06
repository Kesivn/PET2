#include"auxo.h"

namespace pet {
	template<typename T>
	Auxo<T>::Auxo() : pet() {}

	template<typename T>
	Auxo<T>::~Auxo() {}

	template<typename T>
	void Auxo<T>::insert(const T& u, const T& v, uint64_t w) {
		uint32_t fp_u = hashNode(u);
		uint32_t fp_v = hashNode(v);
		pet.insert(fp_u, fp_v, w);
	}

	template<typename T>
	std::optional<uint64_t> Auxo<T>::queryEdge(const T& u, const T& v) const {
		uint32_t fp_u = hashNode(u);
		uint32_t fp_v = hashNode(v);
		return pet.queryEdge(fp_u, fp_v);
	}

	template<typename T>
	std::optional<uint64_t> Auxo<T>::queryInDegree(const T& v) const {
		uint32_t fp_v = hashNode(v);
		return pet.queryInDegree(fp_v);
	}

	template<typename T>
	std::optional<uint64_t> Auxo<T>::queryOutDegree(const T& u) const {
		uint32_t fp_u = hashNode(u);
		return pet.queryOutDegree(fp_u);
	}

	template<typename T>
	uint32_t Auxo<T>::hashNode(const T& node) const {
		std::hash<T> hasher;
		return static_cast<uint32_t>(hasher(node));
	}

	//显式实例化常用类型
	template class Auxo<std::string>;
	template class Auxo<uint32_t>;
	template class Auxo<uint64_t>;
	template class Auxo<int>;
}//namespace pet