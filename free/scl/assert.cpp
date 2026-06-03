#include "scl/assert.h"
#include "scl/log.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <string.h>
#endif

#ifdef SCL_WIN
#include <windows.h>
#define snprintf sprintf_s
#endif 

namespace scl {

void assert_write(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression)
{
	const int MAX_ASSERT_STRING_LENGTH = 1024 * 16;
	char logMessage[MAX_ASSERT_STRING_LENGTH] = { 0 };
	snprintf(
		logMessage, 
		MAX_ASSERT_STRING_LENGTH - 1, 
		"<file: %s>\n<function: %s>\n<line: %d>\n<expresion: %s>\n", 
		fileName, 
		functionName, 
		lineNumber, 
		expression);
	logMessage[MAX_ASSERT_STRING_LENGTH - 1] = 0;

#ifdef SCL_WIN
	OutputDebugStringA(logMessage);
#endif

	printf("%s", logMessage);

	scl::urgency_log(logMessage);
}


void assert_writef(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression,
	const char* const format,
	...)
{
	const int MAX_ASSERT_STRING_LENGTH = 1024 * 16;
	char formatStringBuffer[1024 * 4] = { 0 };
	if (NULL != format)
	{
		va_list args;
		va_start(args, format);

#ifdef SCL_WIN
		vsprintf_s(formatStringBuffer, sizeof(formatStringBuffer), format, args);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
		vsnprintf(formatStringBuffer, sizeof(formatStringBuffer), format, args);
#endif

		va_end(args);
	}
	char logMessage[MAX_ASSERT_STRING_LENGTH] = { 0 };
	snprintf(
		logMessage, 
		MAX_ASSERT_STRING_LENGTH - 1, 
		"<file: %s>\n<function: %s>\n<line: %d>\n<expresion: %s>\n<message: %s>\n", 
		fileName, 
		functionName, 
		lineNumber, 
		expression,
		formatStringBuffer);
	logMessage[MAX_ASSERT_STRING_LENGTH - 1] = 0;

#ifdef SCL_WIN
	OutputDebugStringA(logMessage);
#endif

	printf("%s", logMessage);

	scl::urgency_log(logMessage);
}


bool ensure_failed(
	const char* const fileName, 
	const char* const functionName, 
	const int lineNumber, 
	const char* const expression)
{
	// ensure 是软失败（可恢复，进程仍在运行），走 scl::log 正常通道即可，
	// 不走 urgency_log（那是给"日志系统也可能崩"的 assert 兜底用的）。
	// 用 LOG_LEVEL_ERROR 而非 LOG_LEVEL_FATAL：与 ensure"可继续"语义对齐，
	// LOG_LEVEL_FATAL 留给真正不可恢复的 assert 失败。
	scl::log::out(
		scl::LOG_LEVEL_ERROR, 
		-1, 
		fileName, 
		functionName, 
		lineNumber, 
		"ensure failed: %s", 
		expression);

#ifdef SCL_WIN
	// 返回是否建议宏侧 break：仅在调试器附着时为 true。
	// 无调试器（release 直接运行 / 命令行启动 / shipping）下不打扰，让 caller 走 ensure 返回 false 的兜底路径。
	return IsDebuggerPresent() != FALSE;
#else
	return false;
#endif
}

} //namespace scl
