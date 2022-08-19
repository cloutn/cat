#pragma once

namespace scl {

typedef void* ptr_t;

////////////////////////////////////
//class ptr�������Ǳ�֤���۳�����ʲô��ʽ�˳���ָ�붼������ȷ�ı�delete
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


class _None__TagClass { public : int _i; void _none__tagF(); };

//�����ຯ��ָ������
//ע��:		��linux32�£�		sizeof(class_function) = 8
//			��linux64�£�		sizeof(class_function) = 16
//			��windows32�£�	sizeof(class_function) = 4
typedef void (_None__TagClass::*class_function)();

typedef void (*normal_function)();

} //namespace scl


