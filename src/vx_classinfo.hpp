#ifndef HEADER_VX_CLS_INFO
#define HEADER_VX_CLS_INFO

#include "include/v8.h"

#include "vx_helper.hpp"

#include <string>

namespace vx {

template <typename T>
class class_info {
	public:

		class_info(iso *isolate) : m_isolate(isolate) {
			vx::L<vx::ft> func_class = vx::ft::New(m_isolate), func_ctor = vx::ft::New(m_isolate, ctor_callback);
			func_class->InstanceTemplate()->SetInternalFieldCount(1);
			func_ctor->InstanceTemplate()->SetInternalFieldCount(1);
			func_ctor->Inherit(func_class);

			m_tempclass.Reset(m_isolate, func_class);
			m_tempctor.Reset(m_isolate, func_ctor);
		}

		iso *m_isolate;
		v8::Persistent<ft> m_tempclass;
		v8::Persistent<ft> m_tempctor;

		static class_info<T> *instance(iso *isolate = nullptr) {
			static class_info<T> *instance_info = nullptr;
			if (isolate == nullptr)
				return instance_info;
			else {
				instance_info = new class_info<T>(isolate);
				return instance_info;
			}
		}

		void register_as(H<obj> global, const std::string& name) {
			vx::L<vx::ft> func_ctor = L<ft>::New(m_isolate, m_tempctor);
			global->Set(str::NewFromUtf8(m_isolate, name.c_str()), func_ctor->GetFunction());
		}

		static void ctor_callback(const callback_func<val>& args) {
			T *obj = new T();
			args.Holder()->SetAlignedPointerInInternalField(0, obj); }
};

}

#endif