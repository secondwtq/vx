#ifndef HEADER_VX_HEROES
#define HEADER_VX_HEROES

#include <cstddef>

namespace vx {

	template <size_t ... idxs>
	struct indices { };

	template <size_t N, size_t ... idxs>
	struct indices_builder : indices_builder<N-1, N-1, idxs ...> { };

	template <size_t ... idxs>
	struct indices_builder<0, idxs ...>
		{ using type = indices<idxs ...>; };

}

#endif