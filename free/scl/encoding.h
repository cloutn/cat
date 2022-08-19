////////////////////////////////////////////////////////////////////////////////
//	string
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////////////

#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/stringdef.h"

namespace scl {

////////////////////////////////////
//	_ANSI
//	��������Ҫ������:
//		��Դ�����е�char*���ģ�ת��Ϊָ�������char*����
//		����ת��Ϊ����char*���룬��ο���_SCL_ENCODING_GBK_��˵��
////////////////////////////////////
#define _ANSI(s) scl::debug_to_ansi(s).c_str()


#define MAX_DEBUG_SOURCE_STRING_LENGTH 128

#ifdef SCL_WIN
//�μ�#define _ANSI(s)��˵��
inline string<MAX_DEBUG_SOURCE_STRING_LENGTH> debug_to_ansi(char* s)
{
#ifdef _SCL_ENCODING_GBK_
	return s;
#else
	wstring<MAX_DEBUG_SOURCE_STRING_LENGTH> ws;
	ws.from_gbk(s); //������windows�£�Դ��������gbk���룬��������from_gbk
	string<MAX_DEBUG_SOURCE_STRING_LENGTH> ss;
	ws.to_ansi(ss.c_str(), ss.capacity());
	return ss;
#endif
}
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
//�μ�#define _ANSI(s)��˵��
inline string<MAX_DEBUG_SOURCE_STRING_LENGTH> debug_to_ansi(char* s)
{
#ifdef _SCL_ENCODING_GBK_
	wstring<MAX_DEBUG_SOURCE_STRING_LENGTH> ws;
	ws.from_utf8(s); //������linux�£�Դ��������utf8���룬��������from_utf8
	string<MAX_DEBUG_SOURCE_STRING_LENGTH> ss;
	ws.to_ansi(ss.c_str(), ss.capacity());
	return ss;
#else
	return s;
#endif
}
#endif




} //namespace scl


