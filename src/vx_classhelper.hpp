#ifndef HEADER_VX_CLS_HELPER
#define HEADER_VX_CLS_HELPER

#include "include/v8.h"

#include "vx_helper.hpp"
#include "vx_heroes.hpp"

#include "vx_caster.hpp"
#include "vx_functionbind.hpp"
#include "vx_classinfo.hpp"

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

#include <cstdio>

namespace vx {

template <typename T>
class class_helper {
	private:

		using info_t = class_info<T>;

		template <typename PropT, PropT T::*AttrT>
		inline static void default_getter(v8::Local<v8::String> pro, const v8::PropertyCallbackInfo<v8::Value>& info) {
			T *obj = class_helper<T>::unwrap_object(info.Holder());
			info.GetReturnValue().Set(caster<PropT>::tov8(class_info<T>::instance()->m_isolate, obj->*AttrT));
		}

		template <typename PropT, PropT T::*AttrT>
		inline static void default_getterp(const v8::FunctionCallbackInfo<v8::Value>& args) {
			printf("calling getter\n");
			T *obj = class_helper<T>::unwrap_object(args.Holder());
			args.GetReturnValue().Set(caster<PropT>::tov8(class_info<T>::instance()->m_isolate, obj->*AttrT));
		}

		template <typename PropT, PropT T::*AttrT>
		inline static void default_setterp(const v8::FunctionCallbackInfo<v8::Value>& args) {
			printf("calling setter\n");
			T *obj = unwrap_object(args.Holder());
			obj->*AttrT = caster<PropT>::back(info_t::instance()->m_isolate, args[0]);
		}

		template <typename PropT, PropT T::*AttrT>
		inline static void default_setter(L<str> pro, L<val> value, const v8::PropertyCallbackInfo<void>& info) {
			T *obj = unwrap_object(info.Holder());
			obj->*AttrT = caster<PropT>::back(info_t::instance()->m_isolate, value);
		}

		template <typename PropT, PropT T::*AttrT>
		static void default_setter_readonly(L<str> pro, L<val> value, const v8::PropertyCallbackInfo<void>& info) {
			printf("Invalid set to readonly attribute %s.\n", *str::Utf8Value(pro)); }

	public:

		template<typename PropT, PropT T::*AttrT>
		static inline void property(const std::string& name, bool readonly = false) {
			info_t *info = info_t::instance();

			// NOT A NEW HANDLE! just convert from Persistent to Local
			// may be we should use v8pp to_local() to make it more semantic
			L<ft> classft = L<ft>::New(info->m_isolate, info->m_tempclass);

			v8::AccessorGetterCallback func_get = &default_getter<PropT, AttrT>;
			v8::AccessorSetterCallback func_set;

			// read only stuffs
			// TODO: the concurrence of readonly setter and v8::PropertyAttribute::ReadOnly
			if (!readonly) func_set = &default_setter<PropT, AttrT>;
			else func_set = &default_setter_readonly<PropT, AttrT>;
			v8::PropertyAttribute attr = v8::DontDelete;
			if (readonly) attr = static_cast<v8::PropertyAttribute>(attr | v8::ReadOnly);

			// H<ft> getterft = ft::New(info->m_isolate, &default_getterp<PropT, AttrT>);
			// H<ft> setterft = ft::New(info->m_isolate, &default_setterp<PropT, AttrT>);

			classft->InstanceTemplate()->SetAccessor(str::NewFromUtf8(info->m_isolate, name.c_str()), 
										func_get, func_set, H<val>(), v8::DEFAULT, attr);
			// classft->InstanceTemplate()->SetAccessorProperty(str::NewFromUtf8(info->m_isolate, name.c_str())
				// , getterft, setterft);
		}

		template <typename ProtoT, ProtoT func>
		struct method_callback_wrapper;

		template <typename ReturnT, typename ... Args, ReturnT (T::*func) (Args ...)>
		struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {
			template <size_t ... N>
			inline static ReturnT callback(T *self, std::tuple<Args ...> args, indices<N ...>) {
				return (self->*func)(std::get<N>(args) ...); }

			template <typename U = ReturnT>
			static typename std::enable_if<!(std::is_void<U>::value)>::type 
				callback(const callback_func<val>& info) {
					T *self = unwrap_object(info.Holder());
					std::tuple<Args ...> args = construct_args<Args ...>(info);
					info.GetReturnValue().Set(caster<ReturnT>::tov8(info.GetIsolate(), 
							callback(self, args, typename indices_builder<sizeof ... (Args)>::type())));
				}

			template <typename U = ReturnT>
			static typename std::enable_if<std::is_void<U>::value>::type 
				callback(const callback_func<val>& info) {
					T *self = unwrap_object(info.Holder());
					std::tuple<Args ...> args = construct_args<Args ...>(info);
					callback(self, args, typename indices_builder<sizeof ... (Args)>::type());
				}

			static void register_as(const std::string& name) {
				info_t *info = info_t::instance();
				L<ft> classft = L<ft>::New(info->m_isolate, info->m_tempclass);
				H<ft> methodft = ft::New(info->m_isolate, callback);
				classft->PrototypeTemplate()->Set(str::NewFromUtf8(info->m_isolate, name.c_str()), methodft->GetFunction());
			}

			// 0503: now you can define methods and static methods with ctor
			// 		insted of register_as() (in fact it calls register_as())
			method_callback_wrapper(const std::string& name) {
				method_callback_wrapper::register_as(name); }
		};

		// static function support
		// 		try to reuse structures in 'vx_functionbind'
		template <typename ProtoT, ProtoT& func>
		struct static_callback_wrapper;

		template <typename ReturnT, typename ... Args, ReturnT (&func) (Args ...)>
		struct static_callback_wrapper<ReturnT (Args ...), func> {
			static void register_as(const std::string& name) {
				info_t *info = info_t::instance();
				L<ft> classft = L<ft>::New(info->m_isolate, info->m_tempclass);
				H<ft> methodft = ft::New(info->m_isolate, function_callback_wrapper<ReturnT (Args ...), func>::callback);
				classft->PrototypeTemplate()->Set(str::NewFromUtf8(info->m_isolate, name.c_str()), methodft->GetFunction());
			}

			static_callback_wrapper(const std::string& name) {
				static_callback_wrapper::register_as(name); }
		};

		// template<typename ReturnT, typename ... Args, ReturnT (T::*FuncT)(Args ...)>
		// static inline void method(const std::string& name, bool readonly = false) {
		// 	info_t *info = info_t::instance();
		// }

		// 0503: NOW, you are suggested to construct and operate VX classes definition
		// 		all through 'classhelper'
		// 		and definitely we need another helper that wraps 'classhelper'
		template <typename ... Args>
		struct ctor_wrapper {
			template <size_t ... N>
			inline static T *callback_internal(std::tuple<Args ...> args, indices<N ...>) {
				// printf("ctor_wrapper::callback_internal\n");
				return new T(std::get<N>(args) ...); }

			static void callback(const callback_func<val>& info) {
				std::tuple<Args ...> args = construct_args<Args ...>(info);
				T *obj = callback_internal(args, typename indices_builder<sizeof ... (Args)>::type());
				info.Holder()->SetAlignedPointerInInternalField(0, obj);
			}

			inline static void define_with_isolate(iso *isolate) {
				class_info<T> *info = new class_info<T>(isolate, callback);
				class_info<T>::instance_wrapper::set(info);
			}
		};

		inline static T *unwrap_object(L<obj> obj) {
			return reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0)); }

		inline static void register_as(H<obj> global, const std::string& name) {
			info_t::instance()->register_as(global, name); }
};

}

#endif