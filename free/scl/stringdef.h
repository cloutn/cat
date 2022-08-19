////////////////////////////////////////////////////////////////////////////////
//	string define
//	2010.11.17 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"

////////////////////////////////////////////////////////////////////////
//	_SCL_ENCODING_GBK_ ˵����
//		_SCL_ENCODING_GBK_����������string��pstring��from_ansi��to_ansi��������Ϊ
//
//		���������_SCL_ENCODING_GBK_
//		��ôfrom_ansi��to_ansi�����wchar*��gbk�����char*�໥ת��
//
//		���û�ж���_SCL_ENCODING_GBK_��
//		��ôfrom_ansi��to_ansi�����wchar*��utf8�����char*�໥ת��
////////////////////////////////////////////////////////////////////////

//	#define _SCL_ENCODING_GBK_


namespace scl {

#ifdef SCL_WIN

#define scl_strncasecmp		_strnicmp
#define scl_strcasecmp		_stricmp
#define scl_strtoi64		_strtoi64
#define scl_strtoui64		_strtoui64
#define scl_strtok			strtok_s
#define	scl_wcsncasecmp		_wcsnicmp
#define	scl_wcstoi64		_wcstoi64
#define scl_wcstoui64		_wcstoui64
#define scl_wcstok			wcstok_s
#define scl_snprintf		scl::_snprintf

//win32�£�ֱ��ʹ��_snprintf�ᵼ��warning�����������ӵ���һ��
int _snprintf(
	char*		buffer,
	int			count,
	const char*	format,
	...);

#define SCL_STR_FORMAT_I64	"%I64d"
#define SCL_STR_FORMAT_UI64	"%I64u"
#define SCL_STR_NEW_LINE	"\r\n"
#define SCL_WCS_FORMAT_I64	L"%I64d"
#define SCL_WCS_FORMAT_UI64	L"%I64u"
#define SCL_WCS_NEW_LINE	L"\r\n"

#endif

#ifdef SCL_ANDROID
int					wcsncasecmp	(const wchar_t *, const wchar_t *, int);
unsigned long long	wcstoull	(const wchar_t* s, wchar_t** end, int base);
long long			wcstoll		(const wchar_t* s, wchar_t** end, int base);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)

#define scl_strncasecmp		strncasecmp
#define scl_strcasecmp		strcasecmp
#define scl_strtoi64		strtoll
#define scl_strtoui64		strtoull
#define scl_strtok			strtok_r
#define scl_wcstok			wcstok
#define scl_snprintf		snprintf

#define SCL_STR_FORMAT_I64	"%lld"
#define SCL_STR_FORMAT_UI64 "%llu"
#define SCL_STR_NEW_LINE	"\n"
#define SCL_WCS_FORMAT_I64	L"%lld"
#define SCL_WCS_FORMAT_UI64	L"%llu"
#define SCL_WCS_NEW_LINE	L"\n"

#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_HTML5)
#define scl_wcsncasecmp		wcsncasecmp
#define scl_wcstoi64		wcstoll
#define scl_wcstoui64		wcstoull
#endif

#if defined(SCL_ANDROID)
#define scl_wcsncasecmp		scl::wcsncasecmp
#define scl_wcstoi64		scl::wcstoll
#define scl_wcstoui64		scl::wcstoull
#endif

#define scl_strtohex(s) ::strtoul(s, NULL, 16)

void			strcpy			(char* dest, const int max_size, const char* const src);
void			strncpy			(char* dest, const int max_size, const char* const src, const int copy_count);
void			strncpy_unsafe	(char* dest, const char* const src, const int copy_count);
int				sprintf			(char* buff, const int bufflen, const char* const format, ...);
double			strtod			(const char* const s);
float			strtof			(const char* const s, float _default = 0);
unsigned long	strtoul			(const char* s, char** end, int base);
long			strtol			(const char* s, char** end, int base);

////////////////////////////////////
//	Encoding function
//	implemented in pwstring.cpp
////////////////////////////////////
enum Encoding
{
	Encoding_GBK,
	Encoding_UTF8,
	Encoding_UTF16,
};

int wchar_to_ansi(
	char*			ansi, 
	const int		ansi_byte_size, 
	const wchar*	wide_string, 
	const int		wide_char_count,
	Encoding		ansi_encoding);

int ansi_to_wchar(
	wchar*			wide_string, 
	const int		wide_char_count,
	const char*		ansi, 
	const int		ansi_byte_size, 
	Encoding		ansi_encoding);

bool isalpha(char c);
bool iswalpha(wchar c);

bool isdigit(char c);
bool iswdigit(wchar c);

inline bool isspace(char c)
{
	//space char = 0x09 �C 0x0D or 0x20
	if (c >= 0x09 && c <= 0x0D) return true;
	if (c == 0x20) return true;
	return false;
}


inline bool iswspace(wchar c)
{
	//space char = 0x09 �C 0x0D or 0x20
	if (c >= 0x09 && c <= 0x0D) return true;
	if (c == 0x20) return true;
	return false;
}

int trim		(char* s);
int trim_left	(char* s, const int l = -1);
int trim_right	(char* s, const int l = -1);

////////////////////////////////////////////////////////////////////////
// load_string_to_matrix
//
//		�����ǽ�һ������"1,2,3,4 \n 5,6,7,8 "���ַ���ת��Ϊһ����ά����
//		��ά����ĵ�һ�а���4������ 1 2 3 4
//		��ά����ĵڶ��а���4������ 5 6 7 6
//
// ������
//		str					��ת�����ַ���
//		str_len				��ת�����ַ�������
//		seperator			�ָ������ò���ֱ�Ӵ��ݸ�strtok����
//		output_array		��������ָ��
//		output_capacity		����������󳤶�
//		output_row_count	����ֵ���������һ���ж����У�����ά����ĸ߶�
//		output_column_count	����ֵ���������һ���ж����У�����ά����Ŀ��
//
////////////////////////////////////////////////////////////////////////
bool load_string_to_matrix(
	char*		str, 
	const int	str_len, 
	const char*	seperator,
	uint16*		output_array, 
	const int	output_capacity,
	uint16*		output_height,
	uint16*		output_width);

////////////////////////////////////////////////////////////////////////
// load_string_to_array
//
//		�����ǽ�һ������"1,2,3,4"���ַ���ת��Ϊһ��int����
//
// ������
//		str					��ת�����ַ���
//		str_len				��ת�����ַ�������
//		seperator			�ָ������ò���ֱ�Ӵ��ݸ�strtok����
//		output_array		��������ָ��
//		output_capacity		����������󳤶�
//		output_length		����ֵ���������һ���ж��ٸ�int
//
////////////////////////////////////////////////////////////////////////
bool load_string_to_array(
	char*		str, 
	const int	str_len, 
	const char*	seperator,
	int*		output_array, 
	const int	output_capacity,
	uint16*		output_length = NULL);

////////////////////////////////////////////////////////////////////////
// string_to_float_array
//
//		�����ǽ�һ������"1.2 , 0.33 , 3.14f , 4"���ַ���ת��Ϊһ��float����
//		�ָ���Ϊ����"�����ڹ���float���ַ�"���ַ�, ����" 1.001, { 2.33 } [ 3.14f ]" Ҳ����
//
// ������
//		s					��ת�����ַ���
//		out					�����double����
//		out_max				����������󳤶�
////////////////////////////////////////////////////////////////////////
void string_to_float_array(char* s, double* out, const int out_max);
void string_to_uint_array(char* s, int base, unsigned int* out, const int out_max);

//������TypeName���ַ���ת��ΪTYPE_NAME����ʽ
void string_camel_to_all_upper(const char* const camel, char* const allupper, const int allupper_capacity, bool ignoreNumber = true, bool disableAdjacentUnderscore = true);

} //namespace scl


#ifdef SCL_ANDROID
inline int wcsnlen(const wchar_t *wcs, int maxsize)
{
    int n;
    for (n = 0; n < maxsize && *wcs; n++, wcs++);
    return n;
}
#endif


