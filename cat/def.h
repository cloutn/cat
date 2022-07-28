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
	//KEY_FRAME_TYPE_MOVE_RELATIVE,
	//KEY_FRAME_TYPE_MOVE_TO,
	//KEY_FRAME_TYPE_MOVE_TO_RELATIVE,
	KEY_FRAME_TYPE_SCALE,
	//KEY_FRAME_TYPE_ALPHA,
	//KEY_FRAME_TYPE_GLOBAL_ALPHA,
	//KEY_FRAME_TYPE_COLOR,

	KEY_FRAME_TYPE_COUNT,
};

static const int MAX_OBJECT_COUNT				= 1024 * 128;

} // namespace cat 


