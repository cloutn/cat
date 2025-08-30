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

	bool	open				(const char* const fileName, const char* mode);
	bool	open_buffer			(const char* const buffer, const int buffer_len);
	void	get_string			(const char* section_name, const char* key_name, char* output, const int outputMaxSize);

	int		get_int				(const char* section_name, const char* key_name, const int		_default = 0) const { return get<int>	(section_name, key_name, _default); }
	uint	get_uint			(const char* section_name, const char* key_name, const uint		_default = 0) const { return get<uint>	(section_name, key_name, _default); }
	float	get_float			(const char* section_name, const char* key_name, const float	_default = 0) const { return get<float>	(section_name, key_name, _default); }
	int64	get_int64			(const char* section_name, const char* key_name, const int64	_default = 0) const { return get<int64>	(section_name, key_name, _default); }
	uint64	get_uint64			(const char* section_name, const char* key_name, const uint64	_default = 0) const { return get<uint64>(section_name, key_name, _default); }

	bool	get_bool			(const char* section_name, const char* key_name, const bool defaultvalue = false);
	bool	is_open				() const { return m_string != NULL; }

	template <typename T>
	T		get					(const char* section_name, const char* key_name, const T& defaultvalue = T{}) const;

private:
	class key_value;
	class section;

	void						_parse					(const char* buffer, const int bufferSize);
	const key_value*			_get_key_value			(const char* const section_name, const char* const key_name) const;
	scl::vstring				_get_value_string		(const char* section_name, const char* key_name) const;

public:
	static const char* const	GLOBAL_SECTION_NAME;

private:
	static const int			MAX_SECTION_COUNT		= 256;
	static const int			GLOBAL_SECTION_INDEX	= 0;
	
	char*						m_string;
	scl::varray<section>		m_sections;
};

template <typename T>
inline T ini_to_value(const char* str, const T& defaultValue)
{
	assert(false);
	return defaultValue;
}

template <>
inline int ini_to_value(const char* str, const int& _default)
{
	string32 s = str;
	return s.to_int(_default);
}

template <>
inline uint ini_to_value(const char* str, const uint& _default)
{
	string32 s = str;
	return s.to_uint(_default);
}

template <>
inline float ini_to_value(const char* str, const float& _default)
{
	string32 s = str;
	return s.to_float(_default);
}

template <>
inline int64 ini_to_value(const char* str, const int64& _default)
{
	string32 s = str;
	return s.to_int64(_default);
}

template <>
inline uint64 ini_to_value(const char* str, const uint64& _default)
{
	string32 s = str;
	return s.to_uint64(_default);
}

template <typename T>
T ini_parser::get(const char* section_name, const char* key_name, const T& defaultvalue) const
{
	scl::vstring value = _get_value_string(section_name, key_name);
	T v = ini_to_value(value.c_str(), defaultvalue);
	return v;
}

class ini_writer
{
public:
    explicit ini_writer			(const char* path) : m_path(path) {}

    ~ini_writer();

    bool	open				();
    void	close				();
    bool	write_section		(const char* section);
    bool	write_key			(const char* key, const char* value);
	bool	write				(const char* key, const int value);
	bool	write				(const char* key, const uint value);
	bool	write				(const char* key, const float value);
	bool	write				(const char* key, const int64 value);
	bool	write				(const char* key, const uint64 value);
	bool	write				(const char* key, const bool value);

private:
    const char*					m_path = nullptr;
    FILE*						m_file = nullptr;
};




} //namespace scl




