#pragma once

namespace scl {

#ifndef NULL
#define NULL 0
#endif

// there is 2 buffers in file_read, read buffer and line buffer
#define DEFAULT_MAX_BUFFER_SIZE (64 * 1024)	// max size of read buffer by bytes
#define DEFAULT_LINE_CACHE_SIZE (64 * 1024)	// max size of line buffer by bytes
#define RESERVE_LINEBREAK_SIZE	3			// see comment of "file_reader::reserve_linebreak"

//	NOTICE! this class used "new" to malloc memory, so do NOT use it on a frequently called function stack.

////////////////////////////////////
//	read file line by line, used cache to SPEED UP contrast to "fgets"
//		NOTICE! this class used "new" to malloc memory, so do NOT use it on a frequently called function stack.
//	
////////////////////////////////////
class file_reader
{
public:
	file_reader(const int maxBufferSize = DEFAULT_MAX_BUFFER_SIZE, const int m_maxLineCacheSize = DEFAULT_LINE_CACHE_SIZE);
	~file_reader();

	bool		open							(const char* const filename);	//��һ���ļ�
	bool		open_buffer						(const char* const buffer, const int len);		//��һ��������
	bool		nextline						();		//������һ��
	char*		currentline						();		//��ȡ��ǰ�е�����
	void		clear							();		//������л��������α�״̬�����ᵼ�»������ڴ��ͷ�
	void		reserve_linebreak				(char* c);	//���ý���ÿ�к󣬱����Ļ��з���ʽ����������ã�currentline() ��õ��оͲ��������з�

private:
	bool		_append_string_in_next_block	();		//��m_buffer���޷�������������ʱ��ʹ��m_line���浱ǰ��
	bool		_find_next_start				();		//�ҵ���һ�еĿ�ʼλ��
	bool		_find_end						(bool useLineCache = false);		//�ҵ��µ�ǰ�н����ĵط�,useLineCache��������ָʾ��m_buff�޷���������һ��ʱ���Ƿ���ʹ����չm_line������
	bool		_load_block						();		//���ļ��ж�ȡһ���µ�byte��
	int			_block_end						() { return m_readByteCount; }	//��ǰ��ȡ��Ľ���λ��
	bool		_is_line_end					(const char c) { return c == '\r' || c == '\n' || c == 0; } //���ص�ǰ�ַ��Ƿ�Ϊһ�еĽ���

private:

	////////////////////////////////////
	//
	//	file_read����ͨ�����ֲ�ͬ�ķ�ʽ����ȡ�ļ�
	//
	//	1.��ʹ��open(filename)ʱ��ʹ��m_file�ļ�ָ���fread()����ȡ�ļ�����
	//	2.��ʹ��open_buffer(buffer, len)���Ͳ���ʹ��fread����ȡ�ļ�������ʹ��m_filebuffer_len��m_filebuffer_pos����ȡ�ļ�����
	//
	////////////////////////////////////
	void*		m_file;							//fopen���ص��ļ����
	const char*	m_filebuffer;					//�ļ�������
	int			m_filebuffer_len;				//�ļ�����������
	int			m_filebuffer_pos;				//�ļ���������ǰָ��


	//���ڴ����ļ�����ʱ������
	char*		m_buffer;						//������
	char*		m_line;							//��ʱ���л���������m_buffer���޷�������������ʱ���м�����˻��У������޷�����һ�������У���ʹ��m_line���浱ǰ��

	int			m_start;						//m_buffer����ʼ���α꣬���һ�еĿ�ʼλ��
	int			m_end;							//m_buffer�Ľ������α꣬���һ�еĽ���λ��
	int			m_readByteCount;				//ĳ��fread���ص��ֽ���������_block_end()������ʶ��ǰ��ȡ��Ľ���Ϊֹ��ע�⣬��λ���뻻�з�û���κ��߼���ϵ
	bool		m_usingLineCache;				//������ǰ�Ƿ�����ʹ��m_line������
	char		m_linebreak[RESERVE_LINEBREAK_SIZE]; //����ÿ�к󣬱���Ļ��з���ʽ

	//����
	const int	m_maxBufferSize;				//һ�㻺�������������
	const int	m_maxLineCacheSize;				//��ʱ�õġ��л����������������
};

} //namespace scl


