#pragma once


#define TEST_VULKAN


#ifndef safe_delete
#define safe_delete(ptr) if (NULL != ptr) { delete ptr; ptr = NULL; }
#endif

#ifndef safe_delete_array
#define safe_delete_array(ptr) if (NULL != ptr) { delete[] ptr; ptr = NULL; }
#endif

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define ui_strcmp scl_strcasecmp

#ifndef OFFSET
#define OFFSET(type, member) ((unsigned char*)(&(((type*)0)->member)))
#endif

//
// 计算可变参数宏的参数数量，例如：
//	arg_count(1) = 1
//	arg_count(11, 22, 33, 44) = 4 
//
#ifdef SCL_WIN
#define _arg_expand(...) __VA_ARGS__
#define _arg_n(_1,_2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define arg_count(...) _arg_expand(_arg_n(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))
#else
#define _arg_n(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define arg_count(...) _arg_n(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#endif

namespace cat {

enum KEY_FRAME_TYPE
{
	KEY_FRAME_TYPE_INVALID = -1,

	KEY_FRAME_TYPE_ROTATE,
	KEY_FRAME_TYPE_MOVE,
	KEY_FRAME_TYPE_SCALE,
	//KEY_FRAME_TYPE_MOVE_RELATIVE, //KEY_FRAME_TYPE_MOVE_TO, //KEY_FRAME_TYPE_MOVE_TO_RELATIVE, //KEY_FRAME_TYPE_ALPHA, //KEY_FRAME_TYPE_GLOBAL_ALPHA, //KEY_FRAME_TYPE_COLOR,

	KEY_FRAME_TYPE_COUNT,
};

enum ELEM_TYPE
{
	ELEM_TYPE_INVALID		= 0,	
	ELEM_TYPE_INT8,		// byte
	ELEM_TYPE_UINT8,		// unsigned byte
	ELEM_TYPE_INT16,		// short
	ELEM_TYPE_UINT16,		// unsigned short
	ELEM_TYPE_UINT32,		// unsigned int
	ELEM_TYPE_FLOAT,	// float	
	ELEM_TYPE_INT32,		// int
	//DATA_TYPE_DOUBLE,	
};

enum OPERATE_TYPE
{
	OPERATE_TYPE_TRANSLATE,
	OPERATE_TYPE_ROTATE,
	OPERATE_TYPE_SCALE,
};

enum TRANSFORM_TYPE
{
	TRANSFORM_TYPE_LOCAL,
	TRANSFORM_TYPE_GLOBAL,
};

inline int elem_type_byte(const ELEM_TYPE& type) 
{
	switch (type)
	{
	case ELEM_TYPE_INT8		: case ELEM_TYPE_UINT8	: return 1;
	case ELEM_TYPE_INT16	: case ELEM_TYPE_UINT16	: return 2;
	case ELEM_TYPE_UINT32	: case ELEM_TYPE_INT32	: case ELEM_TYPE_FLOAT: return 4;
	case ELEM_TYPE_INVALID	: return 0;
	default: return 0;
	}
}

static const int MAX_OBJECT_COUNT				= 1024 * 128;

enum FIX_COMPONENT_TYPE
{
	FIX_COMPONENT_TYPE_TRANSFORM = 0,


	FIX_COMPONENT_TYPE_COUNT,
};

} // namespace cat 


