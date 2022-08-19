////////////////////////////////////////////////////////////////////////////////
//	log
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/file.h"
#include "scl/pstring.h"

namespace scl {

//�ڴ滺����־����־���ݻ������ڴ��У�
//ֻ����Ҫ����д���ļ����͵���־��������ʱ���ִ��
//�÷����ڲ�ʹ���˶�̬�ڴ����
class log_file
{
public:
	log_file() : m_cache_len(0), m_quickflush(true) {}
	log_file(const int cacheBufferSize);
	~log_file();

	void	init	(const int cacheBufferSize);
	void	open	(const char* const fileName);
	void	write	(const char* format, ...);
	void	write	(bool quick_flush, const char* format, ...);
	void	flush	(bool quick_flush);
	void	close	();
	bool	is_open	() const { return m_file.is_open(); }
	void	set_quick_flush(bool quickflush) { m_quickflush = quickflush; }
	uint64	size	() { return m_file.size(); }
	const char* const filename() { return m_filename.c_str(); }

	////ע�⣡�ú����ڲ��ж�̬new
	//void resetCacheBuffer(const int cacheBufferSize);

private:
	file			m_file;
	stringPath		m_filename;
	pstring			m_cache;
	int				m_cache_len;
	bool			m_quickflush;
};


} //namespace scl

