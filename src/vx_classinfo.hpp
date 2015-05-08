#ifndef HEADER_VX_CLS_INFO
#define HEADER_VX_CLS_INFO

#include "include/v8.h"

#include "vx_helper.hpp"

#include <string>
#include <cstdlib>

namespace vx {

template <typename T>
class class_info {
	private:
		typedef void (*ctor_callback_type)(const callback_func<val>&);

	public:

		// 0503: why not move ctor arg out?
		class_info(iso *isolate, ctor_callback_type ctor) : 
				m_isolate(isolate), ctor_callback_store(ctor) {
					vx::L<vx::ft> func_class = vx::ft::New(m_isolate), func_ctor = vx::ft::New(m_isolate, ctor_callback_store);
					func_class->InstanceTemplate()->SetInternalFieldCount(1);
					func_ctor->InstanceTemplate()->SetInternalFieldCount(1);
					func_ctor->Inherit(func_class);

					m_tempclass.Reset(m_isolate, func_class);
					m_tempctor.Reset(m_isolate, func_ctor);
				}

		iso *m_isolate;
		// you constuct instances in JavaScript with 'new' syntax calling m_tempctor
		// 	and it's based on m_tempclass
		v8::Persistent<ft> m_tempclass;
		v8::Persistent<ft> m_tempctor;

		ctor_callback_type ctor_callback_store = nullptr;

		// 0503: instance() should be obsoleted, HOWEVER
		// 	it exists as a helper, but we'll need a shorter identifier for it.
		// 	
		// 	explained: as a Singleton, we construct 'class_info' for a certain class
		// 		as soon as the first time it's instance() is referred anywhere in the codebase
		// 		and it's implemented as a common static variable inside instance(),
		//		and you use vx::class_helper<$classname>::instance($isolate) to init it
		//		as soon as you want to define and use a VX class.
		//		
		//		but now, because we need ctor to be customized, and we would like to pass
		//		extra arguments to 'class_info', also we'll like more control to its construction,
		//		so we changed it to a property with a simple wrapper.
		//		Now define your class with
		//			class_helper<$classname>::ctor_wrapper<$ctor_sign>::define_with_isolate($isolate)
		//				(it creates 'class_info' and points it to the corresponding ctor_callback for you)
		//				
		//	TODO: so much things left ...
		//			we need asserts, logs .....
		//	
		struct instance_wrapper {
			private: static class_info<T> *_ret;

			public:
				inline static class_info<T> *get() { return _ret; }
				inline static class_info<T> *set(class_info<T> *src) { return (_ret = src); }
		};

		inline static class_info<T> *instance() {
			return instance_wrapper::get(); }

		void register_as(H<obj> global, const std::string& name) {
			vx::L<vx::ft> func_ctor = L<ft>::New(m_isolate, m_tempctor);
			global->Set(str::NewFromUtf8(m_isolate, name.c_str()), func_ctor->GetFunction());
		}
};

// stackoverflow.com/questions/3229883/static-member-initialization-in-a-class-template
// 	p.s.: that's pretty cool ..
template <typename T>
class_info<T> *class_info<T>::instance_wrapper::_ret = nullptr;

}

#endif