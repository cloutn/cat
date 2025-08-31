#include "scl/ini_file.h"

#include "scl/file.h"
#include "scl/assert.h"
#include "scl/string.h"
#include "scl/array.h"

#include <memory.h>
#include <ctype.h>


namespace scl {

using scl::file;

////////////////////////////////////
//	other tool functions
////////////////////////////////////
enum INI_VALUE_END_MODE
{
	INI_VALUE_END_MODE_NEW_LINE,	//value以“换行”作为结束标志
	INI_VALUE_END_MODE_EMAIL_AT,	//value以“@”作为结束标志
};

inline bool	_is_section_end(char c)
{
	return c == ']'		||
		   c == 0		||	
		   c == '\r'	||
		   c == '\n';
}


inline bool	_is_key_end(char c)
{
	return c == '='		||
		   c == 0		||	
		   c == '\t'	||	
		   c == ' '		||	
		   c == '\r'	||
		   c == '\n';
}


////////////////////////////////////
//	class key_value define
////////////////////////////////////
class ini_parser::key_value
{
public:
	const char*				key;
	const char*				value;
	INI_VALUE_END_MODE		valueEndMode;

	key_value				() : key(NULL), value(NULL), valueEndMode(INI_VALUE_END_MODE_NEW_LINE) {}
	key_value				(const char* const key_name) : key (key_name), value(NULL), valueEndMode(INI_VALUE_END_MODE_NEW_LINE) {}

	void	copy_value		(char* dest, const int maxSize) const;
	int		value_length	() const;
	bool	operator==		(const key_value& other) const;

private:
	bool	_is_value_end	(const int i) const;
};

bool ini_parser::key_value::_is_value_end(const int i) const
{
	char c = value[i];
	if (valueEndMode == INI_VALUE_END_MODE_EMAIL_AT)
	{
		char cNext = value[i + 1];
		return c == 0 ||
			( c == '@' && (cNext == '\r' || cNext == '\n' || cNext == 0) );
	}
	else
	{
		return	c == 0		||	
			c == '\r'	||
			c == '\n'	||
			c == '#';		
	}
}

bool ini_parser::key_value::operator==(const key_value& other) const
{
	if (NULL == key || NULL == other.key)
		return false;
	const char* pSelf = key;
	const char* pOther = other.key;

	while (!_is_key_end(*pSelf) && !_is_key_end(*pOther))
	{
		if (*pSelf != *pOther)
			return false;
		pSelf++;
		pOther++;
	}
	return _is_key_end(*pSelf) && _is_key_end(*pOther);
}

void ini_parser::key_value::copy_value(char* dest, const int maxSize) const
{
	if (NULL == value)
		return;
	if (NULL == dest)
		return;
	if (maxSize <= 0)
		return;

	int i = 0;
	while (!_is_value_end(i) && i < maxSize - 1)  // Reserve space for '\0'
	{
		dest[i] = value[i];
		++i;
	}
	
	// Always null-terminate the string
	dest[i] = '\0';
	
	if (valueEndMode != INI_VALUE_END_MODE_EMAIL_AT)
	{
		pstring(dest, maxSize).trim();  // Pass maxSize to ensure bounds
	}
}

int ini_parser::key_value::value_length() const
{
	if (NULL == value)
		return 0;

	int i = 0;
	while (!_is_value_end(i))
	{
		++i;
	}
	return i;
}


////////////////////////////////////
//	class Section define
////////////////////////////////////
class ini_parser::section
{
public:
	static const int MAX_KEY_VALUE_COUNT = 1024; //TODO 自动确定keyValue的数量

	const char* name;
	array<key_value, MAX_KEY_VALUE_COUNT> elems;

	section() : name (NULL) {}
	section(const char* const section_name) : name (section_name) {}
	bool operator==(const section& other) const
	{
		if (NULL == name || NULL == other.name)
			return false;
		const char* pSelf = name;
		const char* pOther = other.name;

		while (!_is_section_end(*pSelf) && !_is_section_end(*pOther))
		{
			if (*pSelf != *pOther)
				return false;
			pSelf++;
			pOther++;
		}
		return _is_section_end(*pSelf) && _is_section_end(*pOther);
	}
};

////////////////////////////////////
//	ini_parser member functions
////////////////////////////////////
const char* const ini_parser::GLOBAL_SECTION_NAME = "";

enum INI_PARSE_STATE
{
	SEARCHING,
	PARSING_SECTION,
	PARSING_KEY,
	SEARCHING_VALUE,
	PARSING_VALUE,
	COMMENT,
};


ini_parser::ini_parser() : m_string(NULL)
{

}

ini_parser::~ini_parser()
{
	delete[] m_string;
	m_string = NULL;
}


bool ini_parser::open(const char* const fileName, const char* mode)
{	
	if (NULL != m_string)
	{
		delete[] m_string;
		m_string = NULL;
		assert(!"NULL != m_pBuffer");
		return false;
	}

	file f;
	if (!f.open(fileName, mode))
	{
		//assert(false);
		return false;
	}

	f.skip_bom();

	//初始化buffer
	const int bufferSize = static_cast<int>(f.size()) / sizeof(char) + 1;
	m_string = new char[bufferSize];
	memset(m_string, 0, sizeof(char) * bufferSize);

	//读取文件内容
	f.read(m_string, bufferSize, sizeof(char));
	f.close();
	
	_parse(m_string, bufferSize);

	return true;
}


bool ini_parser::open_buffer(const char* const buffer, const int len)
{
	if (NULL != m_string)
	{
		delete[] m_string;
		m_string = NULL;
		assert(!"NULL != m_pBuffer");
		return false;
	}

	//初始化buffer
	const int bufferSize = len + 1;
	m_string = new char[bufferSize];
	memset(m_string, 0, sizeof(char) * bufferSize);

	//读取文件内容
	memcpy(m_string, buffer, len);
	
	_parse(m_string, bufferSize);

	return true;
}

void ini_parser::get_string(const char* section_name, const char* key_name, char* output, const int outputMaxSize)
{
	const key_value*	pFrom	=  _get_key_value(section_name, key_name);
	if (NULL == pFrom)
		return;
	pFrom->copy_value(output, outputMaxSize);
}

scl::vstring ini_parser::_get_value_string(const char* section_name, const char* key_name) const
{
	const key_value* kv =  _get_key_value(section_name, key_name);
	if (NULL == kv)
		return "";
	const int value_len = kv->value_length();
	vstring r;
	r.reserve(value_len + 1);
	kv->copy_value(r.c_str(), r.capacity());
	return r;
}

//uint ini_parser::get_uint(const char* section_name, const char* key_name, const uint _default)
//{
//	string<32> value;
//	const key_value*	pFrom	= _get_key_value(section_name, key_name);
//	if (NULL == pFrom)
//		return _default;
//	pFrom->copy_value(value.c_str(), value.max_sizeof());
//	return value.to_uint();
//}


//float ini_parser::get_float(const char* section_name, const char* key_name, const float _default)
//{
//	string<32> value;
//	const key_value*	pFrom	= _get_key_value(section_name, key_name);
//	if (NULL == pFrom)
//		return _default;
//	pFrom->copy_value(value.c_str(), value.max_sizeof());
//	return static_cast<float>(value.to_double());
//}



//int64 ini_parser::get_int64(const char* section_name, const char* key_name, const int64 _default)
//{
//	string<32> value;
//	const key_value*	pFrom	= _get_key_value(section_name, key_name);
//	if (NULL == pFrom)
//		return _default;
//	pFrom->copy_value(value.c_str(), value.max_sizeof());
//	return value.to_int64();
//}
//
//
//uint64 ini_parser::get_uint64(const char* section_name, const char* key_name, const uint64 _default)
//{
//	string<32> value;
//	const key_value*	pFrom	= _get_key_value(section_name, key_name);
//	if (NULL == pFrom)
//		return _default;
//	pFrom->copy_value(value.c_str(), value.max_sizeof());
//	return value.to_uint64();
//}


bool ini_parser::get_bool(const char* section_name, const char* key_name, const bool _default)
{
	string<32> value;
	const key_value*	pFrom	= _get_key_value(section_name, key_name);
	if (NULL == pFrom)
		return _default;
	pFrom->copy_value(value.c_str(), value.max_sizeof());
	bool r = _default;
	if (value.length() > 0)
	{
		if (value[0] == 't' || value[0] == 'T' || value[0] == '1') // "true, TRUE, 1" all is true
			r = true;
	}
	return r;
}


const ini_parser::key_value* ini_parser::_get_key_value(const char* const section_name, const char* const key_name) const
{
	int sectionIndex = m_sections.find(section(section_name));
	if (-1 == sectionIndex)
		return NULL;

	int keyIndex = m_sections[sectionIndex].elems.find(key_value(key_name));
	if (-1 == keyIndex)
		return NULL;

	return &(m_sections[sectionIndex].elems[keyIndex]);
}

//void ini_parser<char>::_copyValueString(const wchar* pFrom, wchar* dest, const int maxSize)
//{
//	if (NULL == pFrom)
//		return;
//	if (NULL == dest)
//		return;
//	if (maxSize <= 0)
//		return;
//
//	const wchar* copyPosition = pFrom;
//	wchar* pTo = dest;
//	while (!_isValueEnd(*copyPosition) && pTo - dest < maxSize)
//	{
//		*pTo = *copyPosition;
//		pTo++;
//		copyPosition++;
//	}
//}


void ini_parser::_parse(const char* buffer, const int bufferSize)
{
	//TODO，这里可以先统计section数量，然后再reserve
	m_sections.reserve(MAX_SECTION_COUNT);

	//新建一个global section
	section& globalSection = m_sections.push_back_fast();
	globalSection.name = GLOBAL_SECTION_NAME;

	INI_PARSE_STATE mode = SEARCHING;
	INI_VALUE_END_MODE valueEndMode = INI_VALUE_END_MODE_NEW_LINE;
	const char* pCurrentSection	= NULL;
	const char* pCurrentKey		= NULL;
	for (int i = 0; i < bufferSize; ++i)
	{
		if (buffer[i] == 0)
			break; //立即结束整个循环

		switch (mode)
		{
		case SEARCHING: //正在寻找一个section或者一个keyValue
			{
				if (buffer[i] == '[')
				{
					//找到了section的起始符，添加一个新的section
					mode = PARSING_SECTION;
					pCurrentSection = &(buffer[i + 1]);
				}
				else if (buffer[i] == '#') //注释行
				{
					mode = COMMENT;
				}
				else if (!scl::isspace(buffer[i]))
				{
					//找到了一个新的key，添加一对新的KeyValue
					mode = PARSING_KEY;
					pCurrentKey = &(buffer[i]);
				}
			}
			break;
		case PARSING_SECTION: //正在解析一个Section
			{
				if (buffer[i] == '\t' || buffer[i] == ' ')
				{
					pCurrentSection++;
				}
				if (buffer[i] == ']')
				{
					//解析结束，将section加入到sections数组中
					m_sections.push_back_fast();
					m_sections[m_sections.size() - 1].name = pCurrentSection;
					pCurrentSection = NULL;
					mode = SEARCHING;
				}
				if (buffer[i] == '\r' || buffer[i] == '\n') //在找到结束符“]”之前遇到换行符，丢弃该section
				{
					mode = SEARCHING;
				}
			}
			break;
		case PARSING_KEY: //正在解析一个Key
			{
				if (buffer[i] == '=') //找到等号，则开始寻找value
				{
					key_value& newElem = m_sections[m_sections.size() - 1].elems.push_back_fast();
					newElem.key = pCurrentKey;
					pCurrentKey = NULL;
					mode = SEARCHING_VALUE;

					//注意！紧跟着"="后面的第一个"@"表示value解析的时候不以换行作为结束，而是以"@"作为value的结束
					//在这种情况下，value最末位的@符号将被删除
					if (i + 1 < bufferSize && buffer[i + 1] == '@')
					{
						newElem.valueEndMode = INI_VALUE_END_MODE_EMAIL_AT;
						valueEndMode = INI_VALUE_END_MODE_EMAIL_AT;

						int elemCount = m_sections[m_sections.size() - 1].elems.size();
						key_value& elem = m_sections[m_sections.size() - 1].elems[elemCount - 1];
						elem.value = &(buffer[i + 2]); //+2是因为跳过@符号
						mode = PARSING_VALUE; //直接转入parsingValue模式，“不”再忽略@后面的\t和空格等空白字符
					}
				}
				if (buffer[i] == '\r' || buffer[i] == '\n') //在找到等号之前遇到换行符，丢弃该Key
				{
					mode = SEARCHING;
				}
			}
			break;
		case SEARCHING_VALUE: //正在寻找某个Key对应的Value
			{
				if (buffer[i] == '\r' || buffer[i] == '\n')
				{
					//寻找value的过程中遇到了换行符，这说明相应的key对应的value为空
					mode = SEARCHING;
				}
				else if (!scl::isspace(buffer[i])) //找到了value，转入解析value模式
				{
					mode = PARSING_VALUE;
					int elemCount = m_sections[m_sections.size() - 1].elems.size();
					key_value& elem = m_sections[m_sections.size() - 1].elems[elemCount - 1];
					elem.value = &(buffer[i]);
				}
			}
			break;
		case PARSING_VALUE: //解析value
			{
				switch (valueEndMode)
				{
				case INI_VALUE_END_MODE_NEW_LINE:
					{
						if (buffer[i] == '\r' || buffer[i] == '\n')
						{
							mode = SEARCHING;
						}
						else if (buffer[i] == '#')
						{
							mode = COMMENT;
						}
					}
					break;
				case INI_VALUE_END_MODE_EMAIL_AT:
					{
						if (buffer[i] == '@')
						{
							mode = SEARCHING;
							valueEndMode = INI_VALUE_END_MODE_NEW_LINE;
						}
					}
					break;
				default:
					{
						assert(0);
					}
					break;
				};

			}
			break;
		case COMMENT:
			{
				//遇到换行符后，退出注释模式
				if (buffer[i] == '\r' || buffer[i] == '\n') 
				{
					mode = SEARCHING;
				}
			}
			break;
		default:
			{
				assertf(false, "mode = %d", mode);
				return;
			}
			break;
		}
	}
}

ini_writer::~ini_writer()
{
	if (m_file != nullptr)
		fclose(m_file);
}

bool ini_writer::open()
{
	if (m_file != nullptr)
		return true;

	m_file = fopen(m_path, "w");
	return m_file != nullptr;
}

void ini_writer::close()
{
	if (m_file != nullptr)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

bool ini_writer::write_section(const char* section)
{
	if (!m_file)
		return false;

	fprintf(m_file, "[%s]\n", section);
	return true;
}

//bool ini_writer::write_key(const char* key, const char* value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %s\n", key, value);
//	return true;
//}

//bool ini_writer::write(const char* key, const int value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %d\n", key, value);
//	return true;
//}

//bool ini_writer::write(const char* key, const uint value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %u\n", key, value);
//	return true;
//}
//
//bool ini_writer::write(const char* key, const float value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %f\n", key, value);
//	return true;
//}
//
//bool ini_writer::write(const char* key, const int64 value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %lld\n", key, value);
//	return true;
//}
//
//bool ini_writer::write(const char* key, const uint64 value)
//{
//	if (!m_file)
//		return false;
//
//	fprintf(m_file, "%s = %llu\n", key, value);
//	return true;
//}


//bool ini_writer::write(const char* key, const bool value)
//{
//	return write_key(key, value ? "true" : "false");
//}


} //namespace scl





