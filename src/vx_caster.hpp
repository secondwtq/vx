#ifndef HEADER_VX_CASTER
#define HEADER_VX_CASTER

#include "include/v8.h"

#include "vx_helper.hpp"

#include "vx_classinfo.hpp"

namespace vx {

template <typename T>
struct caster {
	using v8t = L<obj>;

	static v8t tov8(iso *c, T src) {
		L<ft> lclasstemp = L<ft>::New(c, class_info<T>::instance()->m_tempclass);
		v8t obj = lclasstemp->GetFunction()->NewInstance();
		obj->SetAlignedPointerInInternalField(0, &src);
		return obj;
	}
};

template <typename T>
struct caster<T *> {
	using v8t = L<obj>;

	static v8t tov8(iso *c, T *src) {
		// printf("getting a pointer ...\n");
		L<ft> lclasstemp = L<ft>::New(c, class_info<T>::instance()->m_tempclass);
		v8t obj = lclasstemp->GetFunction()->NewInstance();
		obj->SetAlignedPointerInInternalField(0, src);
		return obj;
	}

	static T *back(iso *c, H<val> src) {
		H<obj> obj = src->ToObject();
		return reinterpret_cast<T *>(obj->GetAlignedPointerFromInternalField(0)); }
};

template <>
struct caster<int> {
	using v8t = L<num>;

	static v8t tov8(iso *c, int src) {
		L<num> nobj = num::New(c, src);
		return nobj;
	}

	static int back(iso *c, H<val> src) {
		return src->ToInt32()->Value(); }
};

}

#endif