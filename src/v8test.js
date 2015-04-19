
function test_vm_prototype () {

	var new_ctor_test = function () { };
	new_ctor_test.prototype.footest = function () { print('footest'); };

	var ctor_test_obj = new new_ctor_test();
	ctor_test_obj.prototype = new_ctor_test.prototype;
	print(ctor_test_obj.footest);

}

function main () {

	var a = new ref_test();
	var b = new ref_test();
	a.prototype = ref_test.prototype;
	print(a.prototype.member, a.member);
	print(a.test);
	a.test = 3;
	a.member();
	print(a.test);

	print(b.test);
	b.test = 2;
	print(a.test, ' ', b.test);
}

print(vx_test);

var a = new vx_test();
a.test = 9;
print(a.test_func(1));
print(a.test_static_func(2));
a.objref = new vx_test();
a.objref.test = 10;
print(a.test, ' ', a.objref.test);
a.test_readonly = 2333;
print(a.test_readonly);
print("test_funbind_void: ", test_funbind_void);
print(test_funbind_void());
print("test_funbind: ", test_funbind);
print(test_funbind(1, 2, 4));
print("test_funbind_obj: ", test_funbind_obj);
print(test_funbind_obj(1, a));



function global_func_bind_to_obj () {

	var func_bind_to_object = function (val) { this.val = val; }
	func_bind_to_object.prototype.test_funbind = test_funbind;
	func_bind_to_object.prototype.test_jsfunction = function (x, y, z) {
		print('func_bind_to_object::test_jsfunction: ', x, ' ', y, ' ', z, ' self val: ', this.val)
	}

	var funobj_test = new func_bind_to_object(7);
	print(funobj_test.test_funbind(1, 2, 4));
	print(funobj_test.test_jsfunction(1, 2, 4));

}

global_func_bind_to_obj()

// print(a.test0);
// print(a.objref);
// print(a.test);

// main();