#include "file_reader.h"

#include "scl/assert.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#ifdef SCL_WIN
#pragma warning(disable:4996)
#endif

namespace scl { 

file_reader::file_reader(const int maxBufferSize, const int maxLineCacheSize) :
	m_file				(NULL),
	m_filebuffer		(NULL),
	m_filebuffer_len	(0),
	m_filebuffer_pos	(0),
	m_maxBufferSize		(maxBufferSize),
	m_maxLineCacheSize	(maxLineCacheSize)
{
	m_buffer	= new char[m_maxBufferSize];
	m_line		= new char[m_maxLineCacheSize];
	memset(m_linebreak, 0, sizeof(m_linebreak));

	clear();
}

file_reader::~file_reader()
{
	if (NULL != m_file)
	{
		::fclose(static_cast<FILE*>(m_file));
	}
	clear();
	delete m_buffer;
	delete m_line;	
}

bool file_reader::open(const char* const filename)
{
	if (NULL != m_file)
	{
		//ִ�����
		clear();
	}
	m_file = ::fopen(filename, "rb");
	return m_file != NULL;
}

bool file_reader::open_buffer(const char* const buffer, const int len)
{
	if (NULL == buffer)
		return false;
	if (0 == len)
		return false;
	if (NULL != m_filebuffer)
	{
		//ִ�����
		clear();
	}
	m_filebuffer		= buffer;
	m_filebuffer_len	= len;
	m_filebuffer_pos	= 0;
	return true;
}

bool file_reader::nextline()
{
	if (NULL == m_file && NULL == m_filebuffer)
	{
		assert(false);
		return false;
	}
	m_usingLineCache = false;

	if (-1 == m_start)
	{
		//��һ�ζ�ȡ�ļ�
		if (!_load_block())
			return false;
	}
	else
	{
		//����ǰһ�еĻ��з�
		if (!_find_next_start())
			return false;
	}

	//������һ�����з�
	return _find_end(true);
}

char* file_reader::currentline()
{
	if (NULL == m_file && NULL == m_filebuffer)
	{
		assert(false);
		return NULL;
	}

	if (m_usingLineCache)
	{
		return m_line;
	}
	else
	{
		return m_buffer + m_start;
	}
}

bool file_reader::_load_block()
{
	::memset(m_buffer, 0, m_maxBufferSize);

	//��ȡһ���µĿ�
	if (m_filebuffer != NULL)	// file buffer mode
	{
		const int remain = m_filebuffer_len - m_filebuffer_pos;
		m_readByteCount = remain > m_maxBufferSize ? m_maxBufferSize : remain;
		memcpy(m_buffer, m_filebuffer + m_filebuffer_pos, m_readByteCount);
		m_filebuffer_pos += m_readByteCount;
	}
	else	// file handle mode
	{
		m_readByteCount = static_cast<int>(::fread(m_buffer, 1, m_maxBufferSize - 1, static_cast<FILE*>(m_file)));
	}
	if (0 >= m_readByteCount)
		return false;
	m_start = 0;
	m_end	= 0;
	return true;
}

bool file_reader::_find_next_start()
{
	//����ǰһ�еĻ��з�
	m_start = m_end + 1;
	while (m_start < _block_end() && (_is_line_end(m_buffer[m_start])))
		++m_start;

	if (m_start >= _block_end())
	{
		//����һ���¿飬���ٴγ����ҵ�start
		if (!_load_block())
			return false;

		while (m_start < _block_end() && (_is_line_end(m_buffer[m_start])))
			++m_start;

		if (m_start >= _block_end())
		{
			assert(false && "too long single line!");
			return false;
		}
	}

	m_end = m_start;

	return true;
}

bool file_reader::_find_end(bool useLineCache)
{
	//������һ�����з�
	while (m_end <= _block_end())
	{
		//���˵�ǰ�ļ�������ĩβ
		if (m_end == _block_end() - 1)
		{
			bool reach_end = false;
			if (NULL != m_filebuffer)
			{
				if (m_filebuffer_pos >= m_filebuffer_len)
					reach_end = true;
			}
			else 
			{
				if (feof(static_cast<FILE*>(m_file))) //�ļ��Ѿ���ĩβ�ˣ�����ȫ���ַ���
					reach_end = true;
			}

			if (reach_end)
				break;
			else if (!_is_line_end(m_buffer[m_end]))
			{
				if (useLineCache && !_append_string_in_next_block())
					return false;
				break;
			}
		}

		//�ҵ��˻��з�
		if (_is_line_end(m_buffer[m_end]))
		{
			--m_end;
			break;
		}

		++m_end;
	}
	//ĩβ��0������currentline()ֱ�ӷ���m_start����һ���ضϵ��ַ�����
	if (m_end >= _block_end())
	{
		assert(false && "too long line!");
		return false;
	}
	if (m_buffer[m_end] != 0 && _is_line_end(m_buffer[m_end]))
	{
		assert(m_end > 0);
		--m_end;
	}
	//���ָ���˱���Ļ��з�������Ҫ��������ϻ��з�
	const int linebreak_length = static_cast<int>(::strlen(m_linebreak));
	assert(linebreak_length < RESERVE_LINEBREAK_SIZE);
	if (linebreak_length > 0 && m_end + linebreak_length < m_maxBufferSize)
	{
		::strcat(m_buffer, m_linebreak);
		m_end += linebreak_length;
	}
	m_buffer[m_end + 1] = 0;
	return true;
}

bool file_reader::_append_string_in_next_block()
{
	m_usingLineCache = true;

	//�ض��ļ������
	m_buffer[_block_end()] = 0;
	const int old_len = m_end - m_start + 1;

	//��Ҫ�����¸��ļ��飬�Ȱѵ�ǰ��m_start��m_end���塣
	::memset(m_line, 0, m_maxLineCacheSize);

	::strcpy(m_line, m_buffer + m_start);

	//��ȡ�ļ�
	if (!_load_block())
		return false;

	if (!_find_end())
		return false;

	const int new_len = m_end - m_start + 1;
	if (old_len + new_len >= m_maxLineCacheSize)
	{
		assert(false && "too long line!");
		return false;
	}
	::strcat(m_line, m_buffer + m_start);
	return true;
}

void file_reader::clear()
{
	if (NULL != m_file)
	{
		::fclose(static_cast<FILE*>(m_file));
	}
	m_file				= NULL;
	m_filebuffer		= NULL;
	m_start				= -1;
	m_end				= -1;
	m_usingLineCache	= false;
	m_readByteCount		= 0;

	memset(m_buffer,	0, m_maxBufferSize);
	memset(m_line,		0, m_maxLineCacheSize);
}

void file_reader::reserve_linebreak( char* c )
{
	::strncpy(m_linebreak, c, RESERVE_LINEBREAK_SIZE - 1);
	m_linebreak[RESERVE_LINEBREAK_SIZE - 1] = 0;
}

} //namespace scl

#ifdef SCL_WIN
#pragma warning(default:4996)
#endif



