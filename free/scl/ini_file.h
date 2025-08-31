#pragma once

#include "scl/type.h"
#include "scl/varray.h"
#include "scl/vstring.h"
#include "scl/file.h"

namespace scl {

////////////////////////////////////
//	ini文件解析
//	注意！内部使用了动态内存分配
////////////////////////////////////
class ini_parser
{
public:
	ini_parser();
	virtual ~ini_parser();

	bool			open				(const char* const fileName, const char* mode);
	bool			open_buffer			(const char* const buffer, const int buffer_len);
	bool			is_open				() const { return m_string != NULL; }

	int				get_int				(const char* section_name, const char* key_name, const int		_default = 0	) const { return get<int>		(section_name, key_name, _default); }
	uint			get_uint			(const char* section_name, const char* key_name, const uint		_default = 0	) const { return get<uint>		(section_name, key_name, _default); }
	float			get_float			(const char* section_name, const char* key_name, const float	_default = 0	) const { return get<float>		(section_name, key_name, _default); }
	double			get_double			(const char* section_name, const char* key_name, const double	_default = 0	) const { return get<double>	(section_name, key_name, _default); }
	int64			get_int64			(const char* section_name, const char* key_name, const int64	_default = 0	) const { return get<int64>		(section_name, key_name, _default); }
	uint64			get_uint64			(const char* section_name, const char* key_name, const uint64	_default = 0	) const { return get<uint64>	(section_name, key_name, _default); }
	bool			get_bool			(const char* section_name, const char* key_name, const bool		_default = false) const { return get<bool>		(section_name, key_name, _default); }
	scl::vstring	get_string			(const char* section_name, const char* key_name, const scl::vstring	_default = ""	) const { return get<scl::vstring>	(section_name, key_name, _default); }


	template <typename T>
	T get(const char* section_name, const char* key_name, const T& _default = T{}) const
	{
		scl::vstring value = _get_value_string(section_name, key_name);
		const T& v = ini_to_value(value, _default);
		return v;
	}

private:
	class key_value;
	class section;

	void								_parse					(const char* buffer, const int bufferSize);
	const key_value*					_get_key_value			(const char* const section_name, const char* const key_name) const;
	scl::vstring						_get_value_string		(const char* section_name, const char* key_name) const;

public:
	static const char* const			GLOBAL_SECTION_NAME;

private:
	static const int					MAX_SECTION_COUNT		= 256;
	static const int					GLOBAL_SECTION_INDEX	= 0;
	
	char*								m_string;
	scl::varray<section>				m_sections;
};



class ini_writer
{
public:
    explicit ini_writer			(const char* path) : m_path(path) {}

    ~ini_writer();

    bool	open				();
    void	close				();
    bool	write_section		(const char* section);
    //bool	write_key			(const char* key, const char* value);
	//bool	write				(const char* key, const int value);
	//bool	write				(const char* key, const int value) { _write(key, value); }
	//bool	write				(const char* key, const uint value);
	//bool	write				(const char* key, const float value);
	//bool	write				(const char* key, const int64 value);
	//bool	write				(const char* key, const uint64 value);
	//bool	write				(const char* key, const bool value);

public:
	template <typename T>
	bool	write(const char* key, const T& value)
	{
		if (!m_file)
			return false;

		const scl::vstring s = ini_from_value(value);
		fprintf(m_file, "%s = %s\n", key, s.c_str());
		return true;
	}

private:
    const char*					m_path = nullptr;
    FILE*						m_file = nullptr;
};

template <typename T> inline T				ini_to_value	(const scl::vstring& str, const T& _default) = delete;
template <typename T> inline scl::vstring	ini_from_value	(const T& v) = delete;

// int
template <> inline int						ini_to_value	(const scl::vstring& str, const int& _default) { return str.to_int(_default); }
template <> inline scl::vstring				ini_from_value	(const int& v) { scl::vstring s; s.from_int(v); return s; }

// uint
template <> inline uint						ini_to_value	(const scl::vstring& str, const uint& _default) { return str.to_uint(_default); }
template <> inline scl::vstring				ini_from_value	(const uint& v) { scl::vstring s; s.from_uint(v); return s; }

// float
template <> inline float					ini_to_value	(const scl::vstring& str, const float& _default) { return str.to_float(_default); }
template <> inline scl::vstring				ini_from_value	(const float& v) { scl::vstring s; s.from_double(v); return s; }

// int64
template <> inline int64					ini_to_value	(const scl::vstring& str, const int64& _default) { return str.to_int64(_default); }
template <> inline scl::vstring				ini_from_value	(const int64& v) { scl::vstring s; s.from_int64(v); return s; }

// uint64
template <> inline uint64					ini_to_value	(const scl::vstring& str, const uint64& _default) { return str.to_uint64(_default); }
template <> inline scl::vstring				ini_from_value	(const uint64& v) { scl::vstring s; s.from_uint64(v); return s; }

// double
template <> inline double					ini_to_value	(const scl::vstring& str, const double& _default) { return str.to_double(_default); }
template <> inline scl::vstring				ini_from_value	(const double& v) { scl::vstring s; s.from_double(v); return s; }

// bool
template <> inline bool						ini_to_value	(const scl::vstring& str, const bool& _default) { return str.length() > 0 ? (str[0] == 't' || str[0] == 'T' || str[0] == '1') : _default; }
template <> inline scl::vstring				ini_from_value	(const bool& v) { return v ? "true" : "false"; }

// string / const char* const / const char [N] 
template <> inline scl::vstring				ini_to_value	(const scl::vstring& str, const scl::vstring& _default) { return str; }
template <> inline scl::vstring				ini_from_value	(const char* const& v) { return scl::vstring(v); }
template <> inline scl::vstring				ini_from_value	(const scl::vstring& v) { return v; }
template <int N> inline scl::vstring		ini_from_value	(const char (&v)[N]) { return scl::vstring(v); }


} //namespace scl




