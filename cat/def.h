#pragma once

#ifndef safe_delete
#define safe_delete(ptr) if (NULL != ptr) { delete ptr; ptr = NULL; }
#endif

#ifndef safe_delete_array
#define safe_delete_array(ptr) if (NULL != ptr) { delete[] ptr; ptr = NULL; }
#endif

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define TEST_VULKAN

#define ui_strcmp scl_strcasecmp

#ifndef OFFSET
#define OFFSET(type, member) ((unsigned char*)(&(((type*)0)->member)))
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

} // namespace cat 


