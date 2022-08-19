////////////////////////////////////////////////////////////////////////////////
//	file
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/string.h"

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class file
////////////////////////////////////////////////////////////////////////////////
class file
{
public:
	enum SEEK_POSITION
	{
		SEEK_POSITION_BEGIN		= 0,
		SEEK_POSITION_CURRENT	= 1,
		SEEK_POSITION_END		= 2,
	};

	enum LINEFEED_TYPE
	{
		LINEFEED_TYPE_INVALID	= -1,
		LINEFEED_TYPE_LINUX		= 0,
		LINEFEED_TYPE_MACOS		= 1,
		LINEFEED_TYPE_WINDOWS	= 2,
	};

public:
	file();
	virtual ~file();

	//"r"	Opens for reading. If the file does not exist or cannot be found, the fopen_s call fails.
	//"w"	Opens an empty file for writing. If the given file exists, its contents are destroyed.
	//"a"	Opens for writing at the end of the file (appending) without removing the EOF marker before writing new data to the file; creates the file first if it doesn't exist.
	//"r+"	Opens for both reading and writing. (The file must exist.)
	//"w+"	Opens an empty file for both reading and writing. If the given file exists, its contents are destroyed.
	//"a+"	Opens for reading and appending;  creates the file first if it doesn't exist. the appending operation includes the removal of the EOF marker before new data is written to the file and the EOF marker is restored after writing is complete; 
	bool			open				(const char* name, const char* const mode = "a+b");
	//bool			open				(const wchar* name, const wchar* const mode = L"a+b");
	int				seek				(int shift, SEEK_POSITION startPosition);
	int 			read				(void* data,		const int elementCount, const int elementSize = 1) const; //���ض�ȡ�����ݵ�byte��
	int				write				(const void* data,	const int elementCount, const int elementSize = 1);
	bool			close				();
	bool			eof					();
	bool			is_open				() const { return m_file != NULL; }
	void			flush				();	//ǿ���ļ�д�����
	uint64			size				();
	int				tell				();

	//���º������ں�string��ʽ��أ�Ŀǰֻ������UTF16�ļ��Ĵ���
	bool			skip_bom			();
	int				line_count			();
	bool			get_line			(char* data, const int maxCount);
	int 			write_string		(const char* message, const int stringLength);
	int				read_string			(wchar* message, const int maxCount); //����ֵ�Ƕ�ȡ�ַ��ĸ���
	LINEFEED_TYPE	get_linefeed_type	();

public:
	static bool		exists				(const wchar* const filename);
	static bool		exists				(const char* const filename);
	static bool		is_directory		(const char* const filename);
	static int		get_linefeed_size	(LINEFEED_TYPE type);
	static bool		remove				(const char* const filename);
	static bool		rename				(const char* const oldname, const char* const newname);

protected:
	void			_add_size			(const int size) { m_size += size; };
	int				_inner_write		(const void* data, const int elementSize, const int elementCount);
	void			_changed			() { m_size = 0; m_line_count = 0; }

protected:
	void*			m_file;
	uint64			m_size;
	int				m_line_count;
	LINEFEED_TYPE	m_linefeed_type;
};


//�淶��һ��·����
//�����е�'\'�滻Ϊ'/',ͬʱ,��autoAddSlashΪtrue, �ұ�Ҫ��ʱ�򣬻���ĩβ���'/'
void normalize_path			(pstring path, bool auto_add_slash = true);

//��·������ȡ�ļ�����withExt������ʾ�Ƿ�����չ����
void extract_filename		(pstring fullname, bool with_ext = true);

//��·������ȡ�ļ�������������չ��
void extract_filename_no_ext(pstring fullname);

//��·������ȡ��չ��
void extract_fileext		(pstring fullname);

//��·������ȡ��չ��
void extract_fileext_to		(pstring fullname, pstring ext);

//��·������ȡ·�������������ļ���
void extract_path			(pstring fullname, bool add_current_path = true);

//��ȡ��ǰ���̿�ִ���ļ�������
void get_exe_name			(pstring exename);

} // namespace scl 
