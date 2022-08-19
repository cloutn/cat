////////////////////////////////////////////////////////////////////////////////
//	pstring (a wrap of char*)
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////

#include "scl/pstring.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace scl {

#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif

pstring::pstring() : 
	m_string	(NULL),
	MAX_COUNT	(0)
{ 

}

pstring::pstring(char* s, const int maxCount, const char* initString) // maxCount = -1, initString = NULL
{ 
	init(s, maxCount, initString);
}

void pstring::init(char* s, const int maxCount, const char* initString) //maxCount = -1, initString = NULL
{ 
	m_string = s; 
	MAX_COUNT = maxCount; 
	if (maxCount == -1)
	{ 
		MAX_COUNT = static_cast<int>(::strlen(m_string) + 1);
	}
	if (NULL != initString)
	{
		clear();
		copy(initString);
	}
}

void pstring::copy(const char* const source)
{ 
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	::strncpy(m_string, source, MAX_COUNT - 1);

	safe_terminate();
}


void pstring::copy(const char* const source, const int copyCount)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int limitCopyCount = copyCount > capacity() ? capacity() : copyCount;
	::strncpy(m_string, source, limitCopyCount);

	safe_terminate();
}

void pstring::memcpy(const void* p, const int length)
{
	if (NULL == m_string)
		return;
	if (NULL == p)
		return;

	if (length > capacity())
	{
		assert(false);
		return;
	}
	::memcpy(m_string, p, length);

	safe_terminate();
}


void pstring::append(const char* source)							
{ 
	if (NULL == m_string)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	::strncat(m_string, source, freeLength);

	safe_terminate();
}


void pstring::append(const char* source, const int count)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	if (freeLength < count )
		::strncat(m_string, source, freeLength);
	else
		::strncat(m_string, source, count);

	safe_terminate();
}


void pstring::append(const char	c)							
{ 
	if (NULL == m_string)
		return;
	if (c == 0)
		return;

	int len = length();
	if (len >= capacity())
		return;

	m_string[len]		= c;
	m_string[len + 1]	= 0;

	safe_terminate();
}


int	pstring::compare(const char* const source, const int length, bool ignoreCase) const	
{ 
	if (m_string == source)
		return 0;
	if (NULL == m_string)
		return -1;
	if (NULL == source)
		return 1;

	int result = 0;
	if (ignoreCase)
		result = scl_strncasecmp(m_string, source, length);
	else
		result = ::strncmp(m_string, source, length);

	return result;	
}


void	pstring::erase(const int startIndex, const int removeLength) 
{ 
	//ȷ����Ҫɾ����ʵ�ʳ���
	int len = length();
	int realRemoveLength = removeLength;
	if (realRemoveLength > len - startIndex || realRemoveLength == -1)
	{
		realRemoveLength = len - startIndex;
	}
	if (realRemoveLength <= 0)
	{
		return;
	}

	//ִ��ɾ��
	int moveLength = len - startIndex - realRemoveLength;
	for (int i = startIndex; i < startIndex + moveLength; ++i)
	{
		m_string[i] = m_string[i + removeLength];
	}
	m_string[len - realRemoveLength] = 0;
}



int	pstring::format_arg(const char* const format, va_list argumentpointer)	
{ 
	//���ﲻ��ʹ��vsprintf_s�İ汾����Ϊvsprintf_s�汾��Debug�£��������������ȵ�ʱ��ᱨ�������޷�����

	//����ڶ���������windows��linux�º��岻ͬ
	//		��linux�£��ڶ�����������'\0'
	//		��windows�£��ڶ�������������'\0'
	//���ԣ�����ͳһ��MAX_COUNT�������������windows�»��������ʱsafe_terminate();�ᷢ������
	//linux��safe_terminate()�����Ƿ���ö�û��ϵ

	int writtenCount = ::vsnprintf(m_string, MAX_COUNT, format, argumentpointer);
	safe_terminate();
	return writtenCount;
}



int	pstring::format_arg_append(const char* const format, va_list arg)	
{ 
	int freeLength = capacity() - length(); 
	if (freeLength > 0 && freeLength <= capacity()) 
	{ 
		//����ڶ���������windows��linux�º��岻ͬ
		//		��linux�£��ڶ�����������'\0'
		//		��windows�£��ڶ�������������'\0'
		//���ԣ�����ͳһ��freeLength + 1�������������windows�»��������ʱsafe_terminate();�ᷢ������
		//linux��safe_terminate()�����Ƿ���ö�û��ϵ
		int writeCount = ::vsnprintf(m_string + length(), freeLength + 1, format, arg);
		safe_terminate();
		return writeCount;
	}
	return 0;
}


int pstring::format(const char* const format, ...) 
{ 
	va_list SFA_arg;
	va_start(SFA_arg, format);
	int writeCount = format_arg(format, SFA_arg);
	va_end(SFA_arg);
	return writeCount;
}


int pstring::format_append(const char* const format, ...) 
{ 
	va_list arg;
	va_start(arg, format);
	int writeCount = format_arg_append(format, arg);
	va_end(arg);
	return writeCount;
}


int pstring::find_first_of(const char c, const int startIndex) const
{
	const char* pFind = ::strchr(m_string + startIndex, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pstring::find_first_of(const char* s, const int startIndex) const
{
	const char* pFind = ::strstr(m_string + startIndex, s);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pstring::find_last_of(const char c) const
{
	const char* pFind = ::strrchr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pstring::find_last_of(const char* s) const
{
	const char* pFind = NULL;
	const char* pSearch = ::strstr(m_string, s);
	if (NULL == pSearch)
	{
		return -1;
	}
	while (pSearch)
	{
		pFind = pSearch;
		pSearch++;
		pSearch = ::strstr(pSearch, s);
	}
	return static_cast<int>(pFind - m_string);
}


bool pstring::contains(const char c) const 
{ 
	return find_first_of(c) != -1; 
}


bool pstring::contains(const char* s) const 
{ 
	return find_first_of(s) != -1; 
}


void pstring::substr(const int startIndex, const int subLength, char* output, const int outputMaxCount) const
{
	int copyCount = 0;
	int sourceLength = length();
	for (int i = startIndex; i < sourceLength; ++i)
	{
		if (copyCount >= subLength)
			break;
		if (copyCount >= outputMaxCount - 1)
			break;

		output[copyCount] = m_string[i];
		copyCount++;
	}
	output[copyCount] = 0;
}

bool pstring::start_with(const char* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<int>(::strlen(s));
	const int thisLength	= length();
	if (compareLength > thisLength)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	return 0 == compare(s, compareLength, ignoreCase);
}

bool pstring::end_with(const char* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<int>(::strlen(s));
	const int thisLength = length();
	const int startIndex = thisLength - compareLength;
	if (startIndex < 0)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	const pstring sub(m_string + startIndex, compareLength + 1);
	return 0 == sub.compare(s, ignoreCase);
}

bool pstring::replace(const char* const oldString, const char* const newString) 
{ 
	const int stringLength		= length();
	const int stringMaxLength	= MAX_COUNT - 1;

	//int changedCount = 0;
	//�ҵ����滻���ַ���
	const int oldIndex = find_first_of(oldString);
	if (-1 == oldIndex)
	{
		return false;
	}

	//oldString��newString�ĳ��ȶ���Ӧ�ó�����ǰstring�������Ĳ��ֲ�����
	const int oldLength = static_cast<int>(::strnlen(oldString, stringMaxLength));
	int newLength = static_cast<int>(::strnlen(newString, stringMaxLength));
	if (oldLength > newLength)
	{
		::memcpy(m_string + oldIndex, newString, newLength);
		//������ĵط�����
		const int moveCount = oldLength - newLength;
		erase(oldIndex + newLength, moveCount);
	}
	else if (oldLength < newLength)
	{
		//��oldIndex������ַ������ƣ��ճ��㹻�Ŀռ�
		const int diff = newLength - oldLength;
		int moveCount = stringLength - (oldIndex + oldLength);
		int newEnd = (stringLength + diff) - 1;
		//���newEndԽ�磬��ȫ�ض�
		if (newEnd > capacity() - 1)
		{
			const int overflowed = newEnd - (capacity() - 1);
			moveCount -= overflowed;
			newEnd = capacity() - 1;
		}
		for (int i = newEnd; i > newEnd - moveCount; --i)
		{
			m_string[i] = m_string[i - diff];
		}
		//���newLength̫������ȫ�ض�
		if (oldIndex + newLength >= MAX_COUNT)
		{
			newLength -= ((oldIndex + newLength - MAX_COUNT) + 1);
		}
		::memcpy(m_string + oldIndex, newString, newLength);
		m_string[newEnd + 1] = 0;
		//changedCount = moveCount + offset;
	}
	else if (oldLength == newLength)
	{
		::memcpy(m_string + oldIndex, newString, oldLength);
		//�ַ�������û�з����仯
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}



int	pstring::replace_all(const char* const oldString, const char* const newString)//�����Ƿ����滻�Ĵ���
{ 
	int replacedTimes = 0;  
	while (replace(oldString, newString))
	{
		replacedTimes++;
	}
	return replacedTimes;
} 


void pstring::insert(const int positionIndex, const char* const insertString) 
{ 
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	if (NULL == insertString)
		return;

	int insertLength = static_cast<int>(::strnlen(insertString, stringMaxLength));
	//�������ַ�������
	if (insertLength <= 0)
		return;

	//������λ��
	if (positionIndex > stringLength || positionIndex < 0)
		return;

	int moveCount	= stringLength - positionIndex;
	int newEnd		= stringLength + insertLength - 1;
	//��������ܳ���
	if (newEnd > stringMaxLength - 1)
	{
		const int overflowed = newEnd - (stringMaxLength - 1);
		moveCount -= overflowed;
		newEnd = stringMaxLength - 1;
	}
	
	for (int i = newEnd; i > newEnd - moveCount; --i)
	{
		m_string[i] = m_string[i - insertLength];
	}
	const int copyMaxIndex = positionIndex + insertLength;
	if (copyMaxIndex > MAX_COUNT - 1)
	{
		insertLength -= (copyMaxIndex - (MAX_COUNT - 1));
	}
	::memcpy(m_string + positionIndex, insertString, insertLength);
	m_string[newEnd + 1] = 0;
}



void pstring::insert(const int positionIndex, const char insertChar) 
{
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	const int insertLength = 1;

	//������λ��
	if (positionIndex > stringLength || positionIndex < 0)
		return;
	if (stringLength + insertLength >= MAX_COUNT)
		return;

	int newEnd = stringLength + insertLength - 1;
	//��������ܳ���
	if (newEnd > stringMaxLength)
	{
		return;
	}
	int moveCount	= stringLength - positionIndex;
	for (int i = newEnd; i > newEnd - moveCount; --i)
	{
		m_string[i] = m_string[i - insertLength];
	}
	m_string[positionIndex] = insertChar;
	m_string[stringLength + insertLength] = 0;
}



void pstring::toupper() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::toupper(m_string[i]);
	}	
}


void pstring::tolower() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::tolower(m_string[i]);
	}	
}

void pstring::toupper(const int index) 		
{ 
	if (index < 0 || index >= MAX_COUNT)
	{
		assert(0);
		return;
	}
	m_string[index] = ::toupper(m_string[index]);
}


void pstring::tolower(const int index) 		
{ 
	if (index < 0 || index >= MAX_COUNT)
	{
		assert(0);
		return;
	}
	m_string[index] = ::tolower(m_string[index]);
}


int pstring::trim()			
{ 
	int rightCount = trim_right();
	int leftCount = trim_left();
	return rightCount + leftCount;
}


int pstring::trim_left()		
{ 
	int stringLength = length();

	//������Ŀ��ַ�
	int emptyCount = 0;
	for (int i = 0; i < stringLength; ++i)
	{
		if (!scl::isspace(m_string[i]))
			break;

		emptyCount++;
	}
	//ɾ�����п��ַ�
	erase(0, emptyCount);
	return emptyCount;
}


int pstring::trim_right()		
{ 
	int stringLength = length();
	int deleteEndCount = 0;
	//�����ĩβ�Ŀ��ַ�
	for (int i = stringLength - 1; i > 0; --i)
	{
		if (!scl::isspace(m_string[i]))
			break;

		m_string[i] = 0;
		deleteEndCount++;
	}
	return deleteEndCount;
}



void pstring::from_int(const int value) 
{ 
	clear();
	format("%d", value);
}


void pstring::from_uint(const uint value) 
{ 	
	clear();
	format("%u", value);
}


void pstring::from_double(const double value)
{
	clear();
	format("%f", value);
}


void pstring::from_int64(const int64 value)
{
	clear();
	format(SCL_STR_FORMAT_I64, value);
}


void pstring::from_uint64(const uint64 value)
{
	clear();
	format(SCL_STR_FORMAT_UI64, value);
}


int pstring::to_int() const
{
	return static_cast<int>(::strtol(m_string, NULL, 10));
}


uint pstring::to_uint() const
{
	return static_cast<int>(::strtoul(m_string, NULL, 10));
}


double pstring::to_double() const
{
	return ::strtod(m_string, NULL);
}


float pstring::to_float() const
{
	return static_cast<float>(::strtod(m_string, NULL));
}


int64 pstring::to_int64() const
{
	return scl_strtoi64(m_string, NULL, 10);
}


uint64 pstring::to_uint64() const
{
	return scl_strtoui64(m_string, NULL, 10);
}


uint pstring::to_hex() const
{
	return static_cast<uint>(::strtoul(m_string, NULL, 16));
}

int pstring::length() const
{
	return static_cast<int>(::strnlen(m_string, max_size()));
}

bool pstring::to_bool() const
{
	if (NULL == m_string)
		return false;
	const char c = m_string[0];
	if (c == 0)
		return false;

	if (c == 'f' || c == 'F' || c == '0')
		return false;
	//if (c == 't' || c == 'T' || c == '1')
	//	return true;
	return true;
}


#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif

} //namespace scl



