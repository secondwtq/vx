
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include <cstdio>
#include <string>

using namespace v8;

#include "vx.hpp"

// https://developers.google.com/v8/get_started

class ref_test {
	public:
		ref_test() {
			printf("ref_test constructing ...\n"); }

		~ref_test() {
			printf("ref_test descturcting ...\n"); }

		int test = 1;

		ref_test *test_ptr;

		int member() {
			printf("test: %d\n", this->test);
			return this->test+1;
		}

		static void getter_test(v8::Local<v8::String> pro, const PropertyCallbackInfo<v8::Value>& info) {
			v8::Local<v8::Object> self = info.Holder();
			v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
			ref_test *self_ptr = reinterpret_cast<ref_test *>(wrap->Value());
			info.GetReturnValue().Set(self_ptr->test);
		}

		static void setter_test(v8::Local<v8::String> pro, v8::Local<v8::Value> val, 
								const v8::PropertyCallbackInfo<void>& info) {
			v8::Local<v8::Object> self = info.Holder();
			v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
			ref_test *self_ptr = reinterpret_cast<ref_test *>(wrap->Value());
			self_ptr->test = val->Int32Value();
		}
};

class vx_test {
	public:

		vx_test() { printf("vx_test constructing ...\n"); }

		int test_readonly = 2;

		vx_test *objref;

		int test = 3;

		int test_func(int a) {
			printf("VXX: vx_test::test_func: %d %d\n", this->test, a);
			return a; }

		static int test_static_func(int a) {
			printf("VXX: vx_test::test_static_func.\n");
			return a;
		}
};

void ref_test_callback(const FunctionCallbackInfo<v8::Value>& args) {
	ref_test *ref_obj = new ref_test();
	args.Holder()->SetInternalField(0, v8::External::New(args.GetIsolate(), ref_obj));
}

void add_callback(const FunctionCallbackInfo<v8::Value>& args) {
	double arg1 = args[0]->NumberValue(), arg2 = args[1]->NumberValue();
	double ret = arg1 + arg2;
	args.GetReturnValue().Set(ret);
}

void print_callback(const FunctionCallbackInfo<v8::Value>& args) {
	size_t count = args.Length();
	for (size_t i = 0; i < count; i++) {
		v8::String::Utf8Value utf8(args[i]);
		printf("%s", *utf8);
	}
	printf("\n");
}

std::string readfile(int argc, const char *argv[]) {
	const char *filename = argv[1];
	FILE *fp = fopen(filename, "r");

	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	rewind(fp);

	char *ret = new char[fsize+1];
	fread(ret, fsize, 1, fp);
	ret[fsize] = '\0';
	std::string str_ret(ret);
	delete ret;
	fclose(fp);

	return str_ret;
}

void test_funbind_void() {
	printf("VXX: test_funbind_void\n");
}

int test_funbind(int a, int b, int c) {
	printf("VXX: test_funbind: %d %d %d\n", a, b, c);
	return 23333;
}

int test_funbind_obj(int a, vx_test *obj) {
	printf("VXX: test_funbind_obj: %d %d\n", a, obj->test);
	return 23333;
}

int main(int argc, const char *argv[]) {

	// initialize
	V8::InitializeICU();
	Platform *pfdef = platform::CreateDefaultPlatform();
	V8::InitializePlatform(pfdef);
	V8::Initialize();

	// VM with head
	Isolate *isolate = Isolate::New();
	{
		Isolate::Scope isolate_scope(isolate); // enter isolate
		HandleScope handle_scope(isolate); // alloc on stack and to be disposed exiting the block

		// v8::Local<v8::ObjectTemplate> add_objt = v8::ObjectTemplate::New(isolate);
		// add_objt->Set(String::NewFromUtf8(isolate, "add"), FunctionTemplate::New(isolate, add_callback));
		Local<Context> context = Context::New(isolate);
		Context::Scope context_scope(context); // enter context
		v8::Local<v8::FunctionTemplate> ref_test_func = v8::FunctionTemplate::New(isolate, ref_test_callback);
		ref_test_func->PrototypeTemplate()->Set(v8::String::NewFromUtf8(isolate, "member"), v8::FunctionTemplate::New(isolate, [ ] (const FunctionCallbackInfo<v8::Value>& info) {
			v8::Local<v8::Object> self = info.Holder();
			v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
			ref_test *self_ptr = reinterpret_cast<ref_test *>(wrap->Value());

			auto return_value = self_ptr->member();
			info.GetReturnValue().Set(return_value);
		})->GetFunction());
		ref_test_func->InstanceTemplate()->SetInternalFieldCount(1);
		ref_test_func->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "test"), ref_test::getter_test, ref_test::setter_test);

		v8::Handle<v8::Object> global = context->Global();

		global->Set(v8::String::NewFromUtf8(isolate, "ref_test"), ref_test_func->GetFunction());
		global->Set(v8::String::NewFromUtf8(isolate, "print"), v8::FunctionTemplate::New(isolate, print_callback)->GetFunction());

		v8::Handle<v8::Function> add_temp = FunctionTemplate::New(isolate, add_callback)->GetFunction();
		global->Set(v8::String::NewFromUtf8(isolate, "add"), add_temp);

		vx::class_info<vx_test>::instance(isolate);
		vx::class_helper<vx_test>::property<int, &vx_test::test>("test");
		vx::class_helper<vx_test>::property<int, &vx_test::test_readonly>("test_readonly", true);
		vx::class_helper<vx_test>::property<vx_test *, &vx_test::objref>("objref");
		vx::class_helper<vx_test>::method_callback_wrapper<decltype(&vx_test::test_func), &vx_test::test_func>::register_as("test_func");
		vx::class_helper<vx_test>::static_callback_wrapper<decltype(vx_test::test_static_func), vx_test::test_static_func>::register_as("test_static_func");
		vx::class_helper<vx_test>::register_as(global, "vx_test");

		global->Set(vx::str::NewFromUtf8(isolate, "test_funbind_void"), vx::ft::New(isolate, vx::function_callback_wrapper<void (), test_funbind_void>::callback)->GetFunction());
		global->Set(vx::str::NewFromUtf8(isolate, "test_funbind"), vx::ft::New(isolate, vx::function_callback_wrapper<int (int, int, int), test_funbind>::callback)->GetFunction());
		global->Set(vx::str::NewFromUtf8(isolate, "test_funbind_obj"), vx::ft::New(isolate, vx::function_callback_wrapper<int (int, vx_test *), test_funbind_obj>::callback)->GetFunction());

		// create, compile and run source
		// local handles, on stack, desturcted with the handlescope
		std::string text_script = readfile(argc, argv);
		// printf("%s\n", text_script.c_str());
		Local<String> source = String::NewFromUtf8(isolate, text_script.c_str());
		Local<Script> script = Script::Compile(source);
		Local<Value> result = script->Run();

		String::Utf8Value utf8(result); // convert to UTF-8 string
		printf("Return from %s: %s\n", argv[1], *utf8);
	}

	// dispose
	isolate->Dispose();
	V8::Dispose();
	V8::ShutdownPlatform();
	delete pfdef;
	
	return 0;
}