#ifndef HEADER_VX_HELPER
#define HEADER_VX_HELPER

#include "include/v8.h"

namespace vx {

	template <typename T>
	using L = v8::Local<T>;
	template <typename T>
	using H = v8::Handle<T>;

	using obj = v8::Object;

	using val = v8::Value;
	
	using num = v8::Number;
	using str = v8::String;

	using iso = v8::Isolate;
	using ft = v8::FunctionTemplate;
	using ext = v8::External;

	template <typename T>
	using callback_func = v8::FunctionCallbackInfo<T>;

}

#endif