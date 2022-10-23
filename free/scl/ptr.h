#pragma once

namespace scl {

typedef void* ptr_t;

////////////////////////////////////
//class ptr的作用是保证无论程序以什么方式退出，指针都可以正确的被delete
////////////////////////////////////
template<typename T, bool IS_ARRAY = false>
class ptr
{
public:
	T* p;
	ptr				() : p(new T) {}
	ptr				(T* new_ptr) : p(new_ptr) {}
	~ptr			() 
	{ 
		if (NULL == p)
			return;
		if (IS_ARRAY) 
			delete[] p;
		else
			delete p; 
		p = NULL;
	}
	T& operator*	() { return ref(); }
	T* operator->	() { assert(p); return p; }
	T& ref			() { assert(p); return *p; }

private:
	//disallow copy and assign
	void operator=	(const ptr&);
	ptr				(const ptr&);
};


class __any_class { public : int _i; void _none__tagF() {} };

//定义类函数指针类型
//注意:		在linux32下，		sizeof(class_function) = 8
//			在linux64下，		sizeof(class_function) = 16
//			在windows32下，	sizeof(class_function) = 4
typedef void (__any_class::*class_function_ptr)();

typedef void (*function_ptr)();

} //namespace scl


