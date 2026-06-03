////////////////////////////////////////////////////////////////////////////////
//	assert
//	
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include "scl/backtrace.h"
#endif

#ifdef SCL_WIN
#include <excpt.h>
#endif

namespace scl { 

void assert_write(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expresion);

void assert_writef(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression,
	const char* const format,
	...);

// ensure 失败时调用：写日志（走 scl::log 正常通道，不走 urgency_log）。
// 返回值表示"是否建议宏侧 break"：debug 期且调试器附着时为 true，其它为 false。
// ensure 与 assert 的区别：assert 是不可恢复（release 走 throw 1），ensure 是可恢复（永不抛、永不终止），由 caller 看返回值决定后续。
bool ensure_failed(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression);

//undef system assert
#ifdef assert
#undef assert
#endif

#ifdef ensure
#undef ensure
#endif

//////////////////////////////////////////////////////////////////
//	win32 
//////////////////////////////////////////////////////////////////
#ifdef SCL_WIN32

#ifdef _DEBUG
	#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); __asm { int 3 }; } } while(0)
	//assert format
	#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); __asm { int 3 }; } } while(0)
	#define SCL_ASSERT_TRY 
	#define SCL_ASSERT_CATCH while(0)
#else
	#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); throw 1; } } while(0)
	#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); throw 1; } } while(0)
	#define SCL_ASSERT_TRY __try 
	#define SCL_ASSERT_CATCH __except(scl::except_handler(GetExceptionInformation())) 
#endif  // _DEBUG

#endif // SCL_WIN32


//////////////////////////////////////////////////////////////////
//	win64
//////////////////////////////////////////////////////////////////
#ifdef SCL_WIN64
#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); throw 1; } } while(0)

//assert format
#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, __VA_ARGS__); throw 1; } } while(0)

#define SCL_ASSERT_TRY __try 
#define SCL_ASSERT_CATCH __except(scl::except_handler(GetExceptionInformation())) 

#endif //SCL_WIN64


//////////////////////////////////////////////////////////////////
//linux assert
//////////////////////////////////////////////////////////////////
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)

#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); scl::print_stack_to_file(); throw 1; } } while(0)
//#define assert(expr) do { if (!(expr)) { scl::assert_write(__FILE__, __FUNCTION__, __LINE__, #expr); __asm__("int3"); } } while(0)

//assert format
#define assertf(expr, format, ...) do { if (!(expr)) { scl::assert_writef(__FILE__, __FUNCTION__, __LINE__, #expr, format, ##__VA_ARGS__); scl::print_stack_to_file(); throw 1; } } while(0)

#define SCL_ASSERT_TRY try 
#define SCL_ASSERT_CATCH catch(...) 
#endif // SCL_LINUX


//////////////////////////////////////////////////////////////////
//	ensure — 可恢复的软 assert
//
//	用法：
//		if (!ensure(NULL != ptr))
//			return;
//
//	语义：
//		表达式为 true 时不做任何事，返回 true（hot path 无开销）。
//		表达式为 false 时，按 (file, line) 每个 callsite 只触发一次：
//			1. 调用 scl::ensure_failed 写日志（走 scl::log 正常通道）
//			2. debug 期且调试器附着时 __debugbreak，否则不 break
//		永远返回 bool（false 表示违反），caller 看返回值决定后续。
//
//	与 assert 的区别：
//		assert(expr)  — 不可恢复：debug int 3 / release throw 1 → 进程终止
//		ensure(expr)  — 可恢复  ：debug 期调试器附着时 break / release 仅日志 → 返回 false 让 caller 走兜底
//
//	单线程约定：
//		去重用 static bool 而不是 scl::compare_and_swap，刻意让 assert.h 不依赖 scl/thread.h。
//		多线程下 race 的后果是"几个线程几乎同时撞同一 callsite 各写一次日志 / 各 break 一次"，
//		属于良性退化，不破坏数据、不 UB；ensure 本身不要求严格的 once 语义。
//		如未来 scl 被多线程项目使用且想严格 once，把 s_logged 换成 volatile int + CAS 即可。
//////////////////////////////////////////////////////////////////
#ifdef SCL_WIN
#define ensure(expr) \
	(!!(expr) || ([&]() -> bool { \
		static bool s_logged = false; \
		if (!s_logged) \
		{ \
			s_logged = true; \
			if (scl::ensure_failed(__FILE__, __FUNCTION__, __LINE__, #expr)) \
				__debugbreak(); \
		} \
		return false; \
	}()))
#endif // SCL_WIN

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#define ensure(expr) \
	(!!(expr) || ([&]() -> bool { \
		static bool s_logged = false; \
		if (!s_logged) \
		{ \
			s_logged = true; \
			scl::ensure_failed(__FILE__, __FUNCTION__, __LINE__, #expr); \
		} \
		return false; \
	}()))
#endif // SCL_LINUX

} //namespace scl
