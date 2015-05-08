#ifndef HEADER_VX_BIND_FUNCTION
#define HEADER_VX_BIND_FUNCTION

#include "include/v8.h"

#include "vx_helper.hpp"
#include "vx_heroes.hpp"

#include "vx_caster.hpp"

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace vx {

	template <typename ... T, size_t ... N>
	inline std::tuple<T ...> construct_args(const callback_func<val>& info, indices<N ...>) {
		return std::forward_as_tuple<T ...>( caster<T>::back(info.GetIsolate(), info[N]) ... ); }

	template <typename ... T>
	std::tuple<T ...> construct_args(const callback_func<val>& info) {
		constexpr size_t count = sizeof ... (T);
		return construct_args<T ...>(info, typename indices_builder<count>::type());
	}
	
	template <typename ProtoT, ProtoT& func>
	struct function_callback_wrapper;

	template <typename ReturnT, typename ... Args, ReturnT (&func) (Args ...)>
	struct function_callback_wrapper<ReturnT (Args ...), func> {

		template <size_t ... N>
		inline static ReturnT callback(std::tuple<typename caster<Args>::backt ...> args, indices<N ...>) {
			return func(std::get<N>(args) ...); }

		template <typename U = ReturnT>
		static typename std::enable_if<!(std::is_void<U>::value)>::type 
			callback(const callback_func<val>& info) {
				auto args = construct_args<typename caster<Args>::backt ...>(info);
				info.GetReturnValue().Set(caster<ReturnT>::tov8(info.GetIsolate(), 
						callback(args, typename indices_builder<sizeof ... (Args)>::type())));
			}

		template <typename U = ReturnT>
		static typename std::enable_if<std::is_void<U>::value>::type 
			callback(const callback_func<val>& info) {
				std::tuple<Args ...> args = construct_args<Args ...>(info);
				callback(args, typename indices_builder<sizeof ... (Args)>::type());
			}

	};

}

#endif