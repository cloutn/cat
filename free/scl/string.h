////////////////////////////////////
//
//	new string implementation
//	2025.09.26 	caolei
//
////////////////////////////////////
#pragma once

#include "scl/stringdef.h"
#include "scl/type_traits.h"
#include "scl/assert.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

namespace scl {

template <typename char_t>
class char_traits;

struct fixed_storage_tag	{};
struct ptr_storage_tag		{};
struct dynamic_storage_tag	{};

template <typename char_t, typename storage>
class string_base;

template <typename char_t, int N>
class fixed_storage
{
public:
	using ct	= char_traits<char_t>;
	using tag	= fixed_storage_tag;

	fixed_storage() { clear(); }
	fixed_storage(const char_t* src, const int len = -1) 
	{
		clear();
		if (NULL == src)
			return;
		int copy_len = len;
		if (copy_len == -1)
			copy_len = ct::strlen(src);
		copy_len = copy_len > capacity() ? capacity() : copy_len;
		ct::strncpy(str, src, copy_len);
		safe_terminate(); 
	}
	
	inline const char_t*	c_str			() const	{ return str; }
	inline char_t*			c_str			()			{ return str; }
	inline int				max_size		() const	{ return N; }
	inline int				capacity		() const	{ return max_size() - 1; }
	inline int				length			() const	{ return static_cast<int>(ct::strnlen(str, static_cast<size_t>(max_size()))); }
	inline void				safe_terminate	()			{ str[max_size() - 1] = 0; }
	inline void				clear			()			{ ::memset(str, 0, max_size() * sizeof(char_t)); }
	inline bool				empty			() const	{ return str[0] == 0; }
	inline void				ensure_len		(const int t) { }
	inline void				init			(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL) {}
	inline void				alloc			(const int max_count, const char_t* init_string = NULL) { }
	inline void				free			() { }

private:
	char_t str[N];
};

template <typename char_t>
class ptr_storage
{
public:
	using ct	= char_traits<char_t>;
	using tag	= ptr_storage_tag;

	ptr_storage				() : str(nullptr), m_max_size(0) {}
	ptr_storage				(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL) { init(buffer, _max_size, init_string); }
	
	inline const char_t*	c_str			() const	{ return str; }
	inline char_t*			c_str			()			{ return str; }
	inline int				max_size		() const	{ return m_max_size; }
	inline int				capacity		() const	{ return max_size() - 1; }
	inline int				length			() const	{ return static_cast<int>(ct::strnlen(str, static_cast<size_t>(max_size()))); }
	inline void				safe_terminate	()			{ str[max_size() - 1] = 0; }
	inline void				clear			()			{ ::memset(str, 0, max_size() * sizeof(char_t)); }
	inline bool				empty			() const	{ return NULL == str || str[0] == 0; }
	inline void				ensure_len		(const int t) { }
	inline void				init			(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL)
	{
		str = buffer;
		m_max_size = _max_size;
		if (m_max_size == -1)
		{
			m_max_size = static_cast<int>(ct::strlen(str) + 1);
		}
		if (NULL != init_string)
		{
			clear();
			ct::strncpy(str, init_string, capacity());
		}
	}
	
	// 内存管理接口
	inline void				alloc			(const int max_count, const char_t* init_string = NULL) 
	{ 
		init(new char_t[max_count], max_count, init_string); 
	}

	inline void				free			()
	{
		if (str != nullptr) 
		{
			delete[] str;
			str = nullptr;
			m_max_size = 0;
		}
	}

private:
	char_t*	str;
	int		m_max_size;
};

struct dynamic_storage_const { static constexpr int level_size[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 65536,  262144, 1048576, 8388608, 134217728, 2147483646 }; };

template <typename char_t, int N>
class dynamic_storage
{
public:
	using ct = char_traits<char_t>;
	using tag = dynamic_storage_tag;
	using _const = dynamic_storage_const;

	dynamic_storage() : _long(NULL), m_long_max_size(0), m_level(0), m_autofree(1) { clear(); }

	dynamic_storage(const char_t* src, const int len = -1) : _long(NULL), m_long_max_size(0), m_level(0), m_autofree(1) 
	{
		_copy(src, len);
	}

	dynamic_storage(const dynamic_storage& other) : _long(NULL), m_long_max_size(0), m_level(0), m_autofree(1) 
	{
		_copy(other.c_str());
	}

	dynamic_storage(dynamic_storage&& other) noexcept : _long(NULL), m_long_max_size(0), m_level(0), m_autofree(1) 
	{
		_copy_rvalue(other);
	}

	~dynamic_storage()
	{
		if (m_autofree)
			this->free();
	}
	
	inline const char_t*	c_str			() const	{ return m_level ? _long: _short; }
	inline char_t*			c_str			()			{ return m_level ? _long: _short; }
	inline int				max_size		() const	{ return m_level ? m_long_max_size : N; }
	inline int				capacity		() const	{ return max_size() - 1; }
	inline int				length			() const	{ return static_cast<int>(ct::strnlen(c_str(), static_cast<size_t>(max_size()))); }
	inline void				safe_terminate	()			{ c_str()[max_size() - 1] = 0; }
	inline void				clear			()			{ ::memset(c_str(), 0, max_size() * sizeof(char_t)); }
	inline bool				empty			() const	{ return NULL == c_str() || c_str()[0] == 0; }
	inline void				init			(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL) {}
	inline void				alloc			(const int max_count, const char_t* init_string = NULL) {  }
	inline void				free			()			{ delete[] _long; _long = NULL; }

	inline dynamic_storage& operator=(dynamic_storage&& other) 
	{
		_copy_rvalue(other);
		return *this;
	}

	inline dynamic_storage& operator=(const dynamic_storage& other) 
	{
		_copy(other.c_str(), other.length());
		m_autofree = other.m_autofree;
		return *this;
	}

	inline void				ensure_len		(const int target_len) 
	{ 
		if (target_len <= capacity())
			return;

		int lv = m_level;
		while (_const::level_size[lv] <= target_len)
			++lv;
		assert(lv < scl::array_count(_const::level_size));

		int new_max = _const::level_size[lv];
		char_t*	new_buf = new char_t[new_max];
		memset(new_buf, 0, new_max * sizeof(char_t));
		memcpy(new_buf, c_str(), (length() + 1) * sizeof(char_t));

		//clear old string
		if (m_level)
		{
			if (NULL != _long) 
			{ 
				delete[] _long; 
				_long = NULL; 
			}
		}
		else
		{
			::memset(_short, 0, N * sizeof(char_t));
		}

		//assign new string
		_long = new_buf; 
		m_long_max_size = new_max; 
		m_level = lv;
	}

private:

	void _copy(const char_t* src, const int len = -1)
	{
		int copy_len = len;
		if (copy_len == -1)
			copy_len = ct::strlen(src);

		ensure_len(copy_len);
		clear(); // after ensure, we must clear again

		ct::strncpy(c_str(), src, copy_len);
		safe_terminate();
	}

	void _copy_rvalue(dynamic_storage& other)
	{
		m_level = other.m_level;
		m_autofree = other.m_autofree;
		if (other.m_level == 0)
		{
			_copy(other.c_str(), other.length());
			free();
			m_long_max_size = 0;
		}
		else
		{
			_long = other._long;
			m_long_max_size = other.m_long_max_size;

			other._long = NULL;
			other.m_long_max_size = 0;
			other.m_level = 0;
			other.clear();
		}
	}

private:
	char_t			_short[N];
	char_t*			_long;
	int				m_long_max_size;
	char			m_level;		//defined in g_level_size in vstring.cpp
	char			m_autofree;		//auto call pstring::free() in destructor.
};

template <>
class char_traits<char>
{
public:
	using char_t = char;

#pragma warning (disable: 4996)
	static char_t*				strncpy			(char_t* dst, const char_t* src, size_t sz)				{ return ::strncpy(dst, src, sz); }
	static char*				strncat			(char* dst, const char* src, size_t sz)					{ return ::strncat(dst, src, sz); }
#pragma warning (default: 4996)
	static size_t				strlen			(const char_t* s)										{ return ::strlen(s); }
	static size_t				strnlen			(const char* s, size_t maxlen)							{ return ::strnlen(s, maxlen); }
#ifdef SCL_WIN
	static int					strncasecmp		(const char* s1, const char* s2, size_t n)				{ return ::_strnicmp(s1, s2, n); }
#else
	static int					strncasecmp		(const char* s1, const char* s2, size_t n)				{ return ::strncasecmp(s1, s2, n); }
#endif
	static int					strncmp			(const char* s1, const char* s2, size_t n)				{ return ::strncmp(s1, s2, n); }
	static int					vsnprintf		(char* str, size_t size, const char* format, va_list ap){ return ::vsnprintf(str, size, format, ap); }
	static const char*			strchr			(const char* s, int c)									{ return ::strchr(s, c); }
	static const char*			strrchr			(const char* s, int c)									{ return ::strrchr(s, c); }
	static const char*			strstr			(const char* haystack, const char* needle)				{ return ::strstr(haystack, needle); }
	static int					toupper			(int c)													{ return ::toupper(c); }
	static int					tolower			(int c)													{ return ::tolower(c); }
	static int					isspace			(int c)													{ return ::isspace(c); }
	static long					strtol			(const char* nptr, char** endptr, int base)				{ return ::strtol(nptr, endptr, base); }
	static long long			strtoll			(const char* nptr, char** endptr, int base)				{ return ::strtoll(nptr, endptr, base); }
	static unsigned long		strtoul			(const char* nptr, char** endptr, int base)				{ return ::strtoul(nptr, endptr, base); }
	static unsigned long long	strtoull		(const char* nptr, char** endptr, int base)				{ return ::strtoull(nptr, endptr, base); }
	static double				strtod			(const char* nptr, char** endptr)						{ return ::strtod(nptr, endptr); }
	static int					vsscanf			(const char* ws, const char* format, va_list arg)		{ return ::vsscanf(ws, format, arg); }

    	// 字符串常量定义
	static constexpr const char* true_str		= "true";
	static constexpr const char* false_str		= "false";
	static constexpr const char* zero_str		= "0";
	static constexpr const char* one_str		= "1";
	static constexpr const char* yes_str		= "yes";
	static constexpr const char* no_str			= "no";
	
	// 格式化字符串常量
	static constexpr const char* format_int		= "%d";
	static constexpr const char* format_uint	= "%u";
	static constexpr const char* format_double	= "%f";
	static constexpr const char* format_int64	= "%I64d";
	static constexpr const char* format_uint64	= "%I64u";
};

template <>
class char_traits<wchar_t>
{
public:
	using char_t = wchar_t;
#pragma warning (disable: 4996)
	static char_t*				strncpy			(char_t* dst, const char_t* src, size_t sz)					{ return ::wcsncpy(dst, src, sz); }
	static wchar_t*				strncat			(wchar_t* dst, const wchar_t* src, size_t sz)				{ return ::wcsncat(dst, src, sz); }
#pragma warning (default: 4996)
	static size_t				strlen			(const char_t* s)											{ return ::wcslen(s); }
	static size_t				strnlen			(const wchar_t* s, size_t maxlen)							{ return ::wcsnlen(s, maxlen); }
#ifdef SCL_WIN
	static int					strncasecmp		(const wchar_t* s1, const wchar_t* s2, size_t n)			{ return ::_wcsnicmp(s1, s2, n); }
#else
	static int					strncasecmp		(const wchar_t* s1, const wchar_t* s2, size_t n)			{ return ::wcsncasecmp(s1, s2, n); }
#endif
	static int					strncmp			(const wchar_t* s1, const wchar_t* s2, size_t n)			{ return ::wcsncmp(s1, s2, n); }
	static int					vsnprintf		(wchar_t* str, size_t size, const wchar_t* format, va_list ap){ return ::vswprintf(str, size, format, ap); }
	static const wchar_t*		strchr			(const wchar_t* s, wchar_t c)								{ return ::wcschr(s, c); }
	static const wchar_t*		strrchr			(const wchar_t* s, wchar_t c)								{ return ::wcsrchr(s, c); }
	static const wchar_t*		strstr			(const wchar_t* haystack, const wchar_t* needle)			{ return ::wcsstr(haystack, needle); }
	static wchar_t				toupper			(wchar_t c)													{ return ::towupper(c); }
	static wchar_t				tolower			(wchar_t c)													{ return ::towlower(c); }
	static int					isspace			(wchar_t c)													{ return ::iswspace(c); }
	static long					strtol			(const wchar_t* nptr, wchar_t** endptr, int base)			{ return ::wcstol(nptr, endptr, base); }
	static long long			strtoll			(const wchar_t* nptr, wchar_t** endptr, int base)			{ return ::wcstoll(nptr, endptr, base); }
	static unsigned long		strtoul			(const wchar_t* nptr, wchar_t** endptr, int base)			{ return ::wcstoul(nptr, endptr, base); }
	static unsigned long long	strtoull		(const wchar_t* nptr, wchar_t** endptr, int base)			{ return ::wcstoull(nptr, endptr, base); }
	static double				strtod			(const wchar_t* nptr, wchar_t** endptr)						{ return ::wcstod(nptr, endptr); }
	static int					vsscanf			(const wchar_t* ws, const wchar_t* format, va_list arg)		{ return ::vswscanf(ws, format, arg); }


		// 字符串常量定义
	static constexpr const wchar_t* true_str		= L"true";
	static constexpr const wchar_t* false_str		= L"false";
	static constexpr const wchar_t* zero_str		= L"0";
	static constexpr const wchar_t* one_str			= L"1";
	static constexpr const wchar_t* yes_str			= L"yes";
	static constexpr const wchar_t* no_str			= L"no";
	
	// 格式化字符串常量
	static constexpr const wchar_t* format_int		= L"%d";
	static constexpr const wchar_t* format_uint		= L"%u";
	static constexpr const wchar_t* format_double	= L"%f";
	static constexpr const wchar_t* format_int64	= L"%I64d";
	static constexpr const wchar_t* format_uint64	= L"%I64u";
};

template <typename char_t, typename storage>
class string_base
{
public:
	using ct = char_traits<char_t>;
	using pstring_t = string_base<char_t, ptr_storage<char_t> >;

	string_base() : d()						{ }
	string_base(const char_t* s, const int len = -1) : d(s, len)		{ }

	// constructor only for pstring
	explicit string_base(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL) : d(buffer, _max_size, init_string) { }

	// copy constructor
	string_base(const string_base& other) : d()
	{ 
		is_same_v<storage::tag, ptr_storage_tag> ? d.init(const_cast<char_t*>(other.c_str()), other.max_size(), nullptr) : copy(other.c_str());
	}

	string_base(string_base&& other) : d(scl::move(other.d)) {}

public:

	const char_t*		c_str			()  const	{ return d.c_str(); }
	char_t*				c_str			()			{ return d.c_str(); }
	int					capacity		()	const	{ return d.capacity(); }
	int					max_size		()	const	{ return d.max_size(); }
	int					sizeof_char		()	const	{ return sizeof(char_t); }
	int					max_sizeof		()	const	{ return max_size() * sizeof(char_t); }
	int					length			()	const	{ return d.length(); }
	void				safe_terminate	()			{ d.safe_terminate(); }
	void				clear			()			{ d.clear(); }
	bool				empty			()	const	{ return d.empty(); }
	bool				is_empty		()	const	{ return d.empty(); }

	// interface only for pstring
	void				init			(char_t* buffer, int _max_size = -1, const char_t* init_string = NULL)	{ return d.init(buffer, _max_size, init_string); }
	void				alloc			(const int max_count, const char_t* init_string = NULL)					{ d.alloc(max_count, init_string); }
	void				free			()																		{ d.free(); }

	// interface only for string
	pstring_t			pstring			() { return pstring_t(c_str(), max_size()); }
	const pstring_t		pstring			() const { return pstring_t(const_cast<char_t*>(c_str()), max_size()); }

	// interface only for vstring
	void				reserve			(const int len) { d.ensure_len(len); }

	//operators
	string_base& 		operator=	(const string_base&	str	)		
	{  
		d = str.d;
		return *this;
	}
	string_base& 		operator=	(const char_t* s		)		{ copy(s); return *this; }
	string_base& 		operator=	(const int value		)		{ from_int(value); return *this; }
	string_base& 		operator=	(const uint value		)		{ from_uint(value); return *this; }
	string_base& 		operator=	(const double value		)		{ from_double(value); return *this; }
	string_base& 		operator=	(const int64 value		)		{ from_int64(value); return *this; }
	string_base& 		operator=	(const uint64 value		)		{ from_uint64(value); return *this; }
	string_base& 		operator+=	(const char_t* s		)		{ append(s); return *this; }
	string_base& 		operator+=	(const string_base& s	)		{ append(s.c_str()); return *this; }
	string_base& 		operator+=	(const char_t c			)		{ append(c); return *this; }
	bool 				operator==	(const char_t* s		) const { return compare(s) == 0; }
	bool 				operator==	(const string_base& s	) const { return compare(s.c_str()) == 0; }
	bool 				operator>	(const string_base& s	) const { return compare(s.c_str()) > 0; }
	bool 				operator<	(const string_base& s	) const { return compare(s.c_str()) < 0; }
	bool 				operator!=	(const char_t* s		) const { return compare(s) != 0; }
	bool 				operator!=	(const string_base& s	) const { return compare(s.c_str()) != 0; }
	char_t&				operator[]	(const int index		)		{ if (index > capacity()) {assert(0); throw 1; } return _str()[index]; }
	const char_t&		operator[]	(const int index		) const { if (index > capacity()) { assert(0); throw 1; } return _str()[index]; }

	// 注意：转换方法需要在类外定义以避免循环依赖

	//// 序列化接口 (兼容 scl::buffer 系统)
	//template <typename StreamerT> 
	//inline void map(StreamerT& s) 
	//{ 
	//	int l = length();
	//	// 注意：这里需要假设 StreamerT 支持类似 scl::buffer 的接口
	//	// 实际使用时可能需要根据具体的序列化库进行适配
	//	s.write(_str(), l);  // 简化实现，具体根据 StreamerT 接口调整
	//	safe_terminate();
	//}

	void copy(const char_t* const src)
	{ 
		if (NULL == src)
			return;
	
		d.ensure_len(is_same_v<storage::tag, dynamic_storage_tag> ? static_cast<int>(ct::strlen(src)) : 0);

		ct::strncpy(_str(), src, capacity());
	
		safe_terminate();
	}

	void copy(const char_t* const src, const int copy_count)
	{
		if (NULL == src)
			return;

		int limit_copy_count = copy_count > capacity() ? capacity() : copy_count;

		d.ensure_len(limit_copy_count);

		ct::strncpy(_str(), src, limit_copy_count);
		
		// 在拷贝内容末尾添加 null terminator
		if (limit_copy_count < max_size())
			_str()[limit_copy_count] = 0;

		safe_terminate();
	}

	void memcpy(const void* p, const int len)
	{
		if (NULL == _str())
			return;
		if (NULL == p)
			return;

		d.ensure_len(len / sizeof_char());

		if (len > max_sizeof())
		{
			assert(false);
			return;
		}

		::memcpy(_str(), p, len);

		safe_terminate();
	}

	void append(const char_t* src)
	{
		const int len = length();

		//if constexpr (is_same_v<storage::tag, dynamic_storage_tag>)

		d.ensure_len(is_same_v<storage::tag, dynamic_storage_tag> ? static_cast<int>(ct::strlen(src)) + len : 0);

		int free_length = capacity() - len;
		if (free_length <= 0)
			return;


		ct::strncat(_str(), src, free_length);

		safe_terminate();
	}

	void append(const char_t* src, const int copy_count)
	{
		if (NULL == _str())
			return;
		if (NULL == src)
			return;
	
		const int src_len = ct::strlen(src);
		const int count = src_len > copy_count ? copy_count : src_len;
		const int len = length();
		d.ensure_len(len + count);

		int free_length = capacity() - length();
		if (free_length <= 0)
			return;
		if (free_length < count )
			ct::strncat(_str(), src, free_length);
		else
			ct::strncat(_str(), src, count);
	
		safe_terminate();
	}

	void append(const char_t c)							
	{ 
		if (NULL == _str())
			return;
		if (c == 0)
			return;

		int len = length();

		d.ensure_len(len + 1);

		if (len >= capacity())
			return;

		_str()[len]		= c;
		_str()[len + 1]	= 0;

		safe_terminate();
	}

	void substract(const int n)
	{
		if (NULL == _str())
			return;

		int pos = length() - n;
		if (pos < 0)
			pos = 0;
		_str()[pos] = 0;

		safe_terminate();
	}

	void erase(const int start_index = 0, const int remove_length = -1)
	{
		//确定需要删除的实际长度
		int len = length();
		int real_remove_length = remove_length;
		if (real_remove_length > len - start_index || real_remove_length == -1)
			real_remove_length = len - start_index;
		if (real_remove_length <= 0)
			return;

		//执行删除
		int move_length = len - start_index - real_remove_length;
		for (int i = start_index; i < start_index + move_length; ++i)
		{
			_str()[i] = _str()[i + remove_length];
		}
		_str()[len - real_remove_length] = 0;
	}

	int compare(const char_t* s, bool ignore_case = false) const { return compare(s, max_size(), ignore_case); }

	int compare(const char_t* const src, const int len, bool ignore_case = false) const
	{
		if (_str() == src)
			return 0;
		if (NULL == src)
			return 1;

		int result = 0;
		if (ignore_case)
			result = ct::strncasecmp(_str(), src, len);
		else
			result = ct::strncmp(_str(), src, len);

		return result;	
	}

	int compare(const char_t* const src, const int len, const int start_pos, bool ignore_case = false) const
	{
		if (_str() == src)
			return 0;
		if (NULL == src)
			return 1;

		int result = 0;
		if (ignore_case)
			result = ct::strncasecmp(_str() + start_pos, src, len);
		else
			result = ct::strncmp(_str() + start_pos, src, len);

		return result;	
	}

	int	compare(const string_base& s, bool ignoreCase = false) const { return compare(s.c_str(), ignoreCase); }

	int	format_arg(const char_t* const format, va_list arg_list)
	{
		va_list arg_list_copy;
		va_copy(arg_list_copy, arg_list);
		int need_len = ct::vsnprintf(nullptr, 0, format, arg_list_copy);
		va_end(arg_list_copy);
		d.ensure_len(need_len);

		int written_count = ct::vsnprintf(_str(), max_size(), format, arg_list);
		safe_terminate();
		return written_count;
	}

	int	format_arg_append(const char_t* const format, va_list arg_list)
	{
		const int len = length();

		va_list arg_list_copy;
		va_copy(arg_list_copy, arg_list);
		int need_length = ct::vsnprintf(nullptr, 0, format, arg_list_copy);
		va_end(arg_list_copy);
		d.ensure_len(len + need_length);

		int free_len = capacity() - len;
		if (free_len > 0)
		{
			int write_count = ct::vsnprintf(_str() + len, free_len + 1, format, arg_list);
			safe_terminate();
			return write_count;
		}
		return 0;
	}

	int format(const char_t* const format, ...)
	{
		va_list SFA_arg;
		va_start(SFA_arg, format);
		int write_count = format_arg(format, SFA_arg);
		va_end(SFA_arg);
		return write_count;
	}

	int format_append(const char_t* const format, ...)
	{
		va_list arg;
		va_start(arg, format);
		int write_count = format_arg_append(format, arg);
		va_end(arg);
		return write_count;
	}

	int scanf(const char_t* format, ...) const
	{
		va_list ap;
		va_start(ap, format);
		int ret = ct::vsscanf(c_str(), format, ap);
		va_end(ap);
		return ret;
	}

	int find(const char_t c, const int start_index = 0) const { return find_first_of(c, start_index); }

	int find(const char_t* const s, const int start_index = 0)	const { return find_first_of(s, start_index); }

	int find_first_of(const char_t c, const int start_index = 0) const
	{
		const char_t* p_find = ct::strchr(_str()+ start_index, c);
		if (NULL == p_find)
		{
			return -1;
		}
		return static_cast<int>(p_find - _str());
	}

	int find_first_of(const char_t* s, const int start_index = 0) const
	{
		const char_t* p_find = ct::strstr(_str() + start_index, s);
		if (NULL == p_find)
		{
			return -1;
		}
		return static_cast<int>(p_find - _str());
	}

	int find_last_of(const char_t c) const
	{
		const char_t* p_find = ct::strrchr(_str(), c);
		if (NULL == p_find)
		{
			return -1;
		}
		return static_cast<int>(p_find - _str());
	}

	int find_last_of(const char_t* s) const
	{
		const char_t* p_find = NULL;
		const char_t* p_search = ct::strstr(_str(), s);
		if (NULL == p_search)
		{
			return -1;
		}
		while (p_search)
		{
			p_find = p_search;
			p_search++;
			p_search = ct::strstr(p_search, s);
		}
		return p_find - _str();
	}

	bool contains(const char_t c) const { return find_first_of(c) != -1; }

	bool contains(const char_t* s) const { return find_first_of(s) != -1; }

	bool start_with(const char_t* const s, bool ignore_case = false) const
	{
		const int compare_length = static_cast<int>(ct::strlen(s));
		const int this_length = length();
		if (compare_length > this_length)
			return false;
		if (compare_length == 0 && this_length != 0)
			return false;
		return 0 == compare(s, compare_length, ignore_case);
	}

	bool end_with(const char_t* const s, bool ignore_case = false) const
	{
		const int compare_length = static_cast<int>(ct::strlen(s));
		const int this_length = length();
		const int start_index = this_length - compare_length;
		if (start_index < 0)
			return false;
		if (compare_length == 0 && this_length != 0)
			return false;
		return 0 == compare(s, compare_length, start_index, ignore_case);
	}

	void substr(const int start_index, const int sub_length, char_t* output, const int output_max_count) const
	{
		int copy_count = 0;
		int src_length = length();
		for (int i = start_index; i < src_length; ++i)
		{
			if (copy_count >= sub_length)
				break;
			if (copy_count >= output_max_count - 1)
				break;

			output[copy_count] = _str()[i];
			copy_count++;
		}
		output[copy_count] = 0;
	}

	bool replace(const char_t* const old_string, const char_t* const new_string)
	{
		//const int string_max_length = max_size() - 1;
		const int len = length();

		//int changed_count = 0;
		//找到需替换的字符串
		const int old_index = find_first_of(old_string);
		if (-1 == old_index)
		{
			return false;
		}

		//old_string和new_string的长度都不应该超过当前string，超过的部分不处理
		const int old_len = static_cast<int>(ct::strlen(old_string));
		int new_len = static_cast<int>(ct::strlen(new_string));
		if (old_len > new_len)
		{
			::memcpy(_str() + old_index, new_string, new_len * sizeof_char());
			//将多余的地方补齐
			const int move_count = old_len - new_len;
			erase(old_index + new_len, move_count);
		}
		else if (old_len < new_len)
		{
			const int diff = new_len - old_len;

			d.ensure_len(len + diff);

			//将old_index后面的字符串右移，空出足够的空间
			int move_count = len - (old_index + old_len);
			int new_end = (len + diff) - 1;
			//如果new_end越界，安全截断
			if (new_end > capacity() - 1)
			{
				const int overflowed = new_end - (capacity() - 1);
				move_count -= overflowed;
				new_end = capacity() - 1;
			}
			for (int i = new_end; i > new_end - move_count; --i)
			{
				_str()[i] = _str()[i - diff];
			}
			//如果new_length太长，安全截断
			if (old_index + new_len >= max_size())
			{
				new_len -= ((old_index + new_len - max_size()) + 1);
			}
			::memcpy(_str() + old_index, new_string, new_len * sizeof_char());
			_str()[new_end + 1] = 0;
			//changedCount = moveCount + offset;
		}
		else if (old_len == new_len)
		{
			::memcpy(_str() + old_index, new_string, old_len * sizeof_char());
			//字符串长度没有发生变化
		}
		else
		{
			safe_terminate();
			assert(0);
			return false;
		}
		safe_terminate();
		return true;
	}

	//返回是发生替换的次数
	int	replace_all(const char_t* const old_string, const char_t* const new_string)	
	{
		int replaced_times = 0;
		while (replace(old_string, new_string))
		{
			replaced_times++;
		}
		return replaced_times;
	}

	void insert(const int position_index, const char_t* const insert_string)
	{
		int insert_len = static_cast<int>(ct::strlen(insert_string));
		if (insert_len <= 0)
			return;

		const int len = length();

		d.ensure_len(len + insert_len);

		const int string_capacity = capacity();

		if (NULL == insert_string)
			return;

		//检查插入位置
		if (position_index > len || position_index < 0)
			return;

		int move_count = len - position_index;
		int new_end = len + insert_len - 1;
		//检查插入后总长度
		if (new_end > string_capacity - 1)
		{
			const int overflowed = new_end - (string_capacity - 1);
			move_count -= overflowed;
			new_end = string_capacity - 1;
		}

		for (int i = new_end; i > new_end - move_count; --i)
		{
			_str()[i] = _str()[i - insert_len];
		}
		const int copy_max_index = position_index + insert_len;
		if (copy_max_index > max_size() - 1)
		{
			insert_len -= (copy_max_index - (max_size() - 1));
		}
		::memcpy(_str() + position_index, insert_string, insert_len * sizeof_char());
		_str()[new_end + 1] = 0;
	}

	void insert(const int position_index, const char_t insert_char)
	{
		const int len = length();

		d.ensure_len(len + 1);

		const int string_max_length = capacity();

		const int insert_length = 1;

		//检查插入位置
		if (position_index > len || position_index < 0)
			return;
		if (len + insert_length >= max_size())
			return;

		int new_end = len + insert_length - 1;
		//检查插入后总长度
		if (new_end > string_max_length)
		{
			return;
		}
		int move_count = len - position_index;
		for (int i = new_end; i > new_end - move_count; --i)
		{
			_str()[i] = _str()[i - insert_length];
		}
		_str()[position_index] = insert_char;
		_str()[len + insert_length] = 0;
	}
	
	void toupper()
	{
		int len = length();
		for (int i = 0; i < len; ++i)
		{
			_str()[i] = ct::toupper(_str()[i]);
		}
	}
	
	void tolower()
	{
		int len = length();
		for (int i = 0; i < len; ++i)
		{
			_str()[i] = ct::tolower(_str()[i]);
		}
	}
	
	void toupper(const int index)
	{
		if (index < 0 || index >= max_size())
		{
			assert(0);
			return;
		}
		_str()[index] = ct::toupper(_str()[index]);
	}
	
	void tolower(const int index)
	{
		if (index < 0 || index >= max_size())
		{
			assert(0);
			return;
		}
		_str()[index] = ct::tolower(_str()[index]);
	}
	
	int trim()
	{
		int right_count = trim_right();
		int left_count = trim_left();
		return right_count + left_count;
	}
	
	int trim_left()
	{
		int string_length = length();

		//清除左侧的空字符
		int empty_count = 0;
		for (int i = 0; i < string_length; ++i)
		{
			if (!ct::isspace(_str()[i]))
				break;

			empty_count++;
		}
		//删除所有空字符
		erase(0, empty_count);
		return empty_count;
	}
	
	int trim_right()
	{
		int string_length = length();
		int delete_end_count = 0;
		//先清除末尾的空字符
		for (int i = string_length - 1; i > 0; --i)
		{
			if (!ct::isspace(_str()[i]))
				break;

			_str()[i] = 0;
			delete_end_count++;
		}
		return delete_end_count;
	}
	
	void from_int(const int value)
	{
		clear();
		format(ct::format_int, value);
		safe_terminate();
	}
	
	void from_uint(const uint value)
	{
		clear();
		format(ct::format_uint, value);
		safe_terminate();
	}
	
	void from_double(const double value)
	{
		clear();
		format(ct::format_double, value);
		safe_terminate();
	}
	
	void from_int64(const int64 value)
	{
		clear();
		format(ct::format_int64, value);
		safe_terminate();
	}
	
	void from_uint64(const uint64 value)
	{
		clear();
		format(ct::format_uint64, value);
		safe_terminate();
	}

	// Safe conversion functions with default values for conversion failure
	int to_int(const int default_value = 0, const int base = 0) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		long result = ct::strtol(_str(), &endptr, base);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return static_cast<int>(result);
	}
	
	uint to_uint(const uint default_value = 0, const int base = 0) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		unsigned long result = ct::strtoul(_str(), &endptr, base);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return static_cast<uint>(result);
	}
	
	double to_double(const double default_value = 0.0f) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		double result = ct::strtod(_str(), &endptr);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return result;
	}
	
	float to_float(const float default_value = 0.0f) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		double result = ct::strtod(_str(), &endptr);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return static_cast<float>(result);
	}
	
	int64 to_int64(const int64 default_value = 0, const int base = 0) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		int64 result = ct::strtoll(_str(), &endptr, base);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return result;
	}
	
	uint64 to_uint64(const uint64 default_value = 0, const int base = 0) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		uint64 result = ct::strtoull(_str(), &endptr, base);
		if (*endptr != '\0')
		{
			// Conversion failed - non-numeric characters found
			return default_value;
		}
		return result;
	}
	
	uint to_hex(const uint default_value = 0) const
	{
		if (empty())
			return default_value;
		char_t* endptr;
		unsigned long result = ct::strtoul(_str(), &endptr, 16);
		if (*endptr != '\0')
		{
			// Conversion failed - non-hex characters found
			return default_value;
		}
		return static_cast<uint>(result);
	}
	
	bool to_bool(const bool default_value = false) const
	{
		if (empty())
			return default_value;

		// Check for common false values
		if (compare(ct::false_str, true) == 0 || 
			compare(ct::no_str, true) == 0 ||
			compare(ct::zero_str) == 0
			// || ct::tolower(_str()[0]) == ct::false_str[0] || 
			//ct::tolower(_str()[0]) == ct::no_str[0]
			)
		{
			return false;
		}

		// Check for common true values    
		if (compare(ct::true_str, true) == 0 || 
			compare(ct::yes_str, true) == 0 || 
			compare(ct::one_str) == 0
			//|| ct::tolower(_str()[0]) == ct::true_str[0] ||
			//ct::tolower(_str()[0]) == ct::yes_str[0]
			)
		{
			return true;
		}

		// If not recognizable, return default
		return default_value;
	}

	void from_utf8(const char* utf8)
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "from_utf8() only available for wchar_t-based strings");
		if (NULL == utf8) 
			return;

		// TODO 要先确定转换后的数组大小。
		d.ensure_len(is_wchar ? static_cast<int>(char_traits<char>::strlen(utf8)) : 0);

		clear();
		ansi_to_wchar(reinterpret_cast<wchar_t*>(_str()), max_size(), utf8, -1, Encoding_UTF8);
		safe_terminate();
	}

	void from_gbk(const char* gbk)
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "from_gbk() only available for wchar_t-based strings");
		if (NULL == gbk) 
			return;

		// TODO 要先确定转换后的数组大小。
		d.ensure_len(is_wchar ? static_cast<int>(char_traits<char>::strlen(gbk)) : 0);

		clear();
		ansi_to_wchar(reinterpret_cast<wchar_t*>(_str()), max_size(), gbk, -1, Encoding_GBK);
		safe_terminate();
	}

	void from_ansi(const char* ansi)
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "from_ansi() only available for wchar_t-based strings");
		if (NULL == ansi) 
			return;

		// TODO 要先确定转换后的数组大小。
		d.ensure_len(is_wchar ? static_cast<int>(char_traits<char>::strlen(ansi)) : 0);

		clear();
#if defined(_SCL_ENCODING_GBK_)
		from_gbk(ansi);
#else
		from_utf8(ansi);
#endif
	}

	void to_utf8(char* out, const int max_count) const
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "to_utf8() only available for wchar_t-based strings");

		if (NULL == out || max_count <= 0) 
			return;

		wchar_to_ansi(out, max_count, reinterpret_cast<const wchar_t*>(_str()), -1, Encoding_UTF8);
	}

	void to_gbk(char* out, const int max_count) const
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "to_gbk() only available for wchar_t-based strings");

		if (NULL == out || max_count <= 0)
			return;

		wchar_to_ansi(out, max_count - 1, reinterpret_cast<const wchar_t*>(_str()), -1, Encoding_GBK);
	}

	void to_ansi(char* out, const int max_count) const
	{
		constexpr bool is_wchar = is_same_v<char_t, wchar_t>;
		static_assert(is_wchar, "to_ansi() only available for wchar_t-based strings");

		if (NULL == out || max_count <= 0)
			return;

#if defined(_SCL_ENCODING_GBK_)
		to_gbk(out, max_count);
#else
		to_utf8(out, max_count);
#endif
	}

private:
	inline char_t*			_str()			{ return c_str(); }
	inline const char_t*	_str() const	{ return c_str(); }

private:
	storage d;
};

// string and wstring
template <int N>
using string = string_base<char, fixed_storage<char, N> >;
template <int N>
using wstring = string_base<wchar_t, fixed_storage<wchar_t, N> >;


// pstring and pwstring
typedef string_base<char, ptr_storage<char> > pstring;
typedef string_base<wchar_t, ptr_storage<wchar_t> > pwstring;

// vstring and vwstring
typedef string_base<char, dynamic_storage<char, 16> > vstring;
typedef string_base<wchar_t, dynamic_storage<wchar_t, 16> > vwstring;


////////////////////////////////////
// stringN
////////////////////////////////////
typedef 	string<8> 	string8;
typedef 	string<16> 	string16;
typedef		string<32> 	string32;
typedef 	string<64> 	string64;
typedef 	string<128> string128;
typedef 	string<256> string256;
typedef 	string<260> string260;
#ifdef SCL_WIN
typedef 	string<260> stringPath;
#else
typedef 	string<512> stringPath;
#endif
typedef 	string<512> string512;
typedef 	string<1024> string1024;
typedef 	string<2048> string2048;
typedef 	string<4096> string4096;
typedef 	string<8192> string8192;
typedef 	string<16384> string16384;
typedef		string<65536> string65536;

typedef 	string<1024> string1k;
typedef 	string<2048> string2k;
typedef 	string<4096> string4k;
typedef 	string<8192> string8k;
typedef 	string<16384> string16k;
typedef		string<65536> string64k;

////////////////////////////////////
// wstringN
////////////////////////////////////
typedef 	wstring<8> 		wstring8;
typedef 	wstring<16> 	wstring16;
typedef		wstring<32> 	wstring32;
typedef 	wstring<64> 	wstring64;
typedef 	wstring<128> 	wstring128;
typedef 	wstring<256> 	wstring256;
typedef 	wstring<260> 	wstring260;
typedef 	wstring<260> 	wstringPath;
typedef 	wstring<512> 	wstring512;
typedef 	wstring<1024>	wstring1024;


} // namespace scl



