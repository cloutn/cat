#pragma once

#include "scl/type.h"
#include "scl/varray.h"
#include "scl/string.h"
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

	//wchar
	bool	open				(const char* const fileName, const char* mode);
	bool	open_buffer			(const char* const buffer, const int buffer_len);
	void	get_string			(const char* sectionName, const char* keyName, char* output, const int outputMaxSize);
	int		get_int				(const char* sectionName, const char* keyName, const int defaultvalue = 0);
	uint	get_uint			(const char* sectionName, const char* keyName, const uint defaultvalue = 0);
	float	get_float			(const char* sectionName, const char* keyName, const float defaultvalue = 0);
	int64	get_int64			(const char* sectionName, const char* keyName, const int64 defaultvalue = 0);
	uint64	get_uint64			(const char* sectionName, const char* keyName, const uint64 defaultvalue = 0);
	bool	get_bool			(const char* sectionName, const char* keyName, const bool defaultvalue = false);
	bool	is_open				() const { return m_string != NULL; }

private:
	class key_value;
	class section;

	void						_parse					(const char* buffer, const int bufferSize);
	const key_value*			_get_key_value_position	(const char* const sectionName, const char* const keyName);

public:
	static const char* const	GLOBAL_SECTION_NAME;

private:
	static const int			MAX_SECTION_COUNT		= 256;
	static const int			GLOBAL_SECTION_INDEX	= 0;
	
	char*						m_string;
	scl::varray<section>		m_sections;
};

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




