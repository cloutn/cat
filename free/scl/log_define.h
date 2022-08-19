#pragma once

#include "scl/string.h"

namespace scl {

//�߳���־���߳���ʧ�󣬵ȴ�THREAD_LOG_CLOSE_DELAY�����رգ�
//ԭ����buffer�п��ܻ���δ��д�����־����������־д����ٹر�thread_log
const uint THREAD_LOG_CLOSE_DELAY		= 3 * 1000;		//��λ������
const int  THREAD_LOG_CHECK_INTERVAL	= 5 * 1000;		//��λ������

//һ����־������ַ�������
const int MAX_LOG_STRING_LENGTH			= 1024 * 8;

//��־�������Ĵ�С��
//ÿ������Ҫ��ӡ���ļ�����־�ַ����ĳ��ȡ����������������Сʱ���ͻ����ļ�д����־
const int LOG_CACHE_BUFFER_SIZE			= 1024 * 32;

const int FREQUENT_LOG_MAX_COUNT		= 1024;	//����Ƶ����־�ĸ�������
const int FREQUENT_LOG_TIMES_LIMIT		= 16;	//������־ˢ�´������ޣ�����������ߣ���ΪƵ����־���������˵�
const int FREQUENT_LOG_CLEAR_INTERVAL	= 1 * 60 * 1000; //Ƶ����־��ռ������λ������

const int MAX_LOG_FILE_SIZE				= 512 * 1024 * 1024; //��־�ļ���С����

//һ���̵߳���־��������С��������������������С����־ϵͳ�ͻᱨ��
//#ifdef SCL_WIN
const int MAX_THREAD_LOG_BUFFER_SIZE	= 32 * 1024;
//#endif
//#ifdef SCL_LINUX
//const int MAX_THREAD_LOG_BUFFER_SIZE	= 128 * 1024 * 1024;
//#endif

//��־�߳���������������Ҫ��ӡ��־���߳������������������־ϵͳ�ͻᱨ��
#ifdef SCL_WIN
const int MAX_THREAD_LOG_COUNT			= 32;
#endif
#ifdef SCL_LINUX
const int MAX_THREAD_LOG_COUNT			= 256;
#endif
#ifdef SCL_APPLE
const int MAX_THREAD_LOG_COUNT			= 64;
#endif
#ifdef SCL_ANDROID
const int MAX_THREAD_LOG_COUNT			= 64;
#endif
#ifdef SCL_HTML5
const int MAX_THREAD_LOG_COUNT			= 32;
#endif

const int MAX_LOG_HANDLER_COUNT			= 256;

//�����ϸ��buffer���
const byte	BEGIN_MARK	= 0xFE;
const byte	END_MARK	= 0xFF;

struct log_header
{
	byte		begin_mark;
	byte		format;
	byte		level;
	int			len;
	int			line;
	uint64		time;
	int			millisecond;
	string256	filename;
	string256	function;
	log_header() : begin_mark(0), format(0), level(0), len(0), line(0), time(0), millisecond(0) {}
};

//��־�ȼ���ö��ֵԽ�����Խ����
enum LOG_LEVEL
{
	//���ϵ������س̶���������
	LOG_LEVEL_INVALID = -1,	//

	LOG_LEVEL_VERBOSE,	//��־�ȼ���������Ϣ
	LOG_LEVEL_DEBUG,	//��־�ȼ�������
	LOG_LEVEL_INFO,		//��־�ȼ�����ʾ
	LOG_LEVEL_WARN,		//��־�ȼ�������
	LOG_LEVEL_ERROR,	//��־�ȼ�������
	LOG_LEVEL_FATAL,	//��־�ȼ�������
	//LOG_LEVEL_NET,		//��־�ȼ�: net.

	LOG_LEVEL_USER1 = 11,//��־�ȼ����û�
	LOG_LEVEL_USER2,	//��־�ȼ����û�
	LOG_LEVEL_USER3,	//��־�ȼ����û�
	LOG_LEVEL_USER4,	//��־�ȼ����û�
	LOG_LEVEL_USER5,	//��־�ȼ����û�
	LOG_LEVEL_USER6,	//��־�ȼ����û�
	LOG_LEVEL_USER7,	//��־�ȼ����û�
	LOG_LEVEL_USER8,	//��־�ȼ����û�

	LOG_LEVEL_COUNT,		//��־��������
};

//��־�����ʽ
enum LOG_OUTPUT
{
	LOG_OUTPUT_CONSOLE	= 0x01,	//��ӡ����Ļ��
	LOG_OUTPUT_FILE 	= 0x02,	//��ӡ�������ļ���
	//LOG_OUTPUT_REMOTE	= 0x04,	//��ӡ��Զ��socket
};

//��־��ʽ
enum LOG_FORMAT
{
	LOG_FORMAT_NONE			= 0,

	LOG_FORMAT_TIME			= 0x01,
	LOG_FORMAT_LINE_FEED	= 0x02,
	LOG_FORMAT_LEVEL		= 0x04,
	LOG_FORMAT_FUNCTION		= 0x08,
	LOG_FORMAT_FILENAME		= 0x10,
	LOG_FORMAT_MILLISECOND	= 0x20,

	LOG_FORMAT_ALL			= 0xFF,
};

struct log_level
{
	byte		output;
	byte		format;
	bool		enable;
	bool		prevent_frequent;
	bool		quick_flush;
	string8		show;
	//string32	remote;
	//int			remote_socket;
	string64	filename;
	bool		date_dir;
	bool		split;

	log_level() : output(0), format(0), enable(false), prevent_frequent(true), quick_flush(false), date_dir(false), split(false) {}

	void clear	() { output = 0; format = 0; enable = false; show = "";  }
	void set	(
		const char* show, 
		byte		output, 
		byte		format, 
		bool		prevent_frequent	= true,
		bool		quick_flush			= false,
		bool		enable				= true,
		bool		date_dir			= false,
		const char* filename			= NULL,
		bool		split				= true) 
	{ 
		this->output				= output;
		this->format				= format; 
		this->enable				= enable;
		this->prevent_frequent	= prevent_frequent;
		this->quick_flush		= quick_flush;
		this->show				= show; 
		this->date_dir			= date_dir;
		if (NULL != filename)
			this->filename		= filename;
		this->split				= split;
	}
};

//����Ƶ������־
struct frequent_log
{
	string256	filename;
	int			line;
	int			times;
	
	frequent_log() { clear(); }

	void clear()
	{
		filename.clear();
		line		= 0;
		times	= 0;
	}
};



} //namespace scl
