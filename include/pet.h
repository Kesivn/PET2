#pragma once

#include"petnode.h"

namespace pet {
	class Pet{
	public:
		Pet();
		~Pet();

		bool insert(uint32_t src, uint32_t dst, uint64_t value);
		std::optional<uint64_t> queryEdge(uint32_t src, uint32_t dst) const;
		std::optional<uint64_t> queryInDegree(uint32_t dst) const;
		std::optional<uint64_t> queryOutDegree(uint32_t src) const;



	private:
		PetNode* root;
		int tree_height = 0;

		PetNode* findLeafNode(uint32_t src, uint32_t dst) const;

	private:
		enum class State {
			nomal,
			expand,
			error
		};
		State state = State::nomal;

		void do_expand() {};


	public:
		void debugPrint(bool show_details = false) const;
	};
}//namespace pet