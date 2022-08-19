////////////////////////////////////////////////////////////
//	buffer.h
//  caolei 2012.05.27
////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

namespace scl {

class buffer
{
public:
	buffer();
	buffer(void* p, const int max_length, const int start = 0, const int end = 0);
	virtual ~buffer();

	//��ʼ��buffer
	void	alloc	(const int maxLength);				//����һ�鳤��ΪmaxLength���ڴ�,���ڴ�����������ʱ��ᱻdelete��
	void	init	(void* pData, const int maxLength, const int start = 0, const int end = 0);	//��pDataָ����ڴ���Ϊ�����������ʹ�ó���ΪmaxLength�����ڴ治�ᱻ��delete

	uint8*	data	()					{ return &(m_buffer[m_start]); }
	int		length	() const			{ return m_end - m_start; }			//endռһ��λ��
	int		read	(void* p, const int length);
	int		peek	(void* p, const int length);
	void	write	(const void* const p, const int length);
	int		free	() const			{ return m_maxLength - m_end;  }
	int		used	() const			{ return length(); }
	void	drop	(const int count) 	{ m_start += count; }
	int		max_length() const			{ return m_maxLength; }
	void	set_length(const int v)		{ m_end = m_start + v; }

private:
	uint8*			m_buffer;		//���ݻ�����
	int				m_start;		//��ʼλ�ã����λ�ú����ݵĵ�һ��byte�غ�
	int				m_end;			//����λ�ã����λ��λ�����ݵ����һ��byte����
	int				m_maxLength;	//�����������������ɵ����ݳ��ȣ�Ϊm_buffer�ĳ��ȼ�ȥm_endռ�õ�1
	bool			m_autoDelete;	//�Ƿ���Ҫ��BinaryStreamer����ʱdelete[] m_buffer
};

} //namespace scl 

