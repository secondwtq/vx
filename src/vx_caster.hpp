#ifndef HEADER_VX_CASTER
#define HEADER_VX_CASTER

#include "include/v8.h"

#include "vx_helper.hpp"

#include "vx_classinfo.hpp"

#include <string>
// #include <cstdio>

namespace vx {

template <typename T>
struct caster {
	using actualt = T;
	using backt = const T&;
	using v8t = L<obj>;

	static v8t tov8(iso *c, T src) {
		static H<v8::ObjectTemplate> t = L<ft>::New(c, class_info<T>::instance()->m_tempclass)->InstanceTemplate();

		// L<ft> lclasstemp = L<ft>::New(c, class_info<T>::instance()->m_tempclass);
		v8t obj = t->NewInstance();
		obj->SetAlignedPointerInInternalField(0, &src);
		
		return obj;
	}

	static backt back(iso *c, H<val> src) {
		H<obj> obj = src->ToObject();
		return *(reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0))); }
};

template <typename T>
struct caster<T *> {
	using actualt = T *;
	using backt = T *;
	using v8t = L<obj>;
// 1993": "cmysdxb
	static v8t tov8(iso *c, T *src) {
		// printf("getting a pointer ...\n");
		L<ft> lclasstemp = L<ft>::New(c, class_info<T>::instance()->m_tempclass);
		v8t obj = lclasstemp->InstanceTemplate()->NewInstance();
		obj->SetAlignedPointerInInternalField(0, src);
		return obj;
	}

	static backt back(iso *c, H<val> src) {
		H<obj> obj = src->ToObject();
		return reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0)); }
};

template <typename T>
struct caster<T&> {
	using actualt = T&;
	using backt = T&;
	using v8t = L<obj>;

	static v8t tov8(iso *c, T& src) {
		L<ft> lclasstemp = L<ft>::New(c, class_info<T>::instance()->m_tempclass);
		v8t obj = lclasstemp->InstanceTemplate()->NewInstance();
		obj->SetAlignedPointerInInternalField(0, &src);
		return obj;
	}

	static backt back(iso *c, H<val> src) {
		H<obj> obj = src->ToObject();
		return *(reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0)));
	}
};

template <typename T>
struct caster<const T&> {
	using actualt = const T&;
	using backt = const T&;
	using v8t = L<obj>;

	static backt back(iso *c, H<val> src) {
		H<obj> obj = src->ToObject();
		return *(reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0)));
	}
};

template <>
struct caster<int> {
	using actualt = int;
	using backt = int;
	using v8t = L<num>;

	static v8t tov8(iso *c, int src) {
		L<num> nobj = num::New(c, src);
		return nobj;
	}

	static backt back(iso *c, H<val> src) {
		return src->ToInt32()->Value(); }
};

template <>
struct caster<const char *> {
	using actualt = const char *;
	using backt = const char *;
	using v8t = L<str>;

	static v8t tov8(iso *c, const char *src) {
		L<str> str = str::NewFromUtf8(c, src);
		return str;
	}

	static backt back(iso *c, H<val> src) {
		return *str::Utf8Value(src); }
};

template <>
struct caster<std::string> {
	using v8t = L<str>;
	using actualt = std::string;
	using backt = std::string;

	static v8t tov8(iso *c, const std::string& src) {
		L<str> str = str::NewFromUtf8(c, src.c_str());
		return str;
	}

	static backt back(iso *c, H<val> src) {
		return std::string(*str::Utf8Value(src)); }
};

template <>
struct caster<std::string&> {
	using actualt = const std::string *;
	using backt = std::string;
	using v8t = L<str>;

	static v8t tov8(iso *c, std::string& src) {
		L<str> str = str::NewFromUtf8(c, src.c_str());
		return str; }

	static backt back(iso *c, H<val> src) {
		return std::string(*str::Utf8Value(src)); }
};

template <>
struct caster<const std::string&> {
	using actualt = const std::string *;
	using backt = std::string;
	using v8t = L<str>;

	static v8t tov8(iso *c, const std::string& src) {
		L<str> str = str::NewFromUtf8(c, src.c_str());
		return str; }

	static backt back(iso *c, H<val> src) {
		return std::string(*str::Utf8Value(src)); }
};

}

#endif
