
#include "cat/wstring.h"

#include <string.h>

namespace cat {

#define countof(s) (sizeof(s)/sizeof(s[0]))
static const int g_level_size[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 65536,  262144, 1048576 };
static const int MAX_LENGTH = g_level_size[countof(g_level_size) - 1];	

WString::WString() : 
	m_level		(0), 
	m_autofree	(1)
{

}

WString::WString(const wchar* s) : 
	m_level		(0), 
	m_autofree	(1)
{
	_assign(s);
}

WString::WString(const WString& s) : 
	m_level		(0), 
	m_autofree	(s.m_autofree)
{
	_assign(s.c_str());
}

WString::~WString()
{
	if (m_autofree)
		m_long.free();
}

WString& WString::operator+=(const wchar* s)
{
	_grow(static_cast<int>(::wcsnlen(s, MAX_LENGTH)));
	pstring().append(s);
	return *this;
}

WString& WString::operator+=(const wchar c)
{
	_grow(1);
	pstring().append(c);
	return *this;
}

WString& WString::operator=(const wchar* s)
{
	_assign(s);
	return *this;
}

void WString::from_ansi(const char* s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().from_ansi(s);
}


void WString::clear()
{
	if (m_level)
	{
		m_long.free();
	}
	m_level = 0;
	m_short.clear();
}

void WString::insert(const int positionIndex, const wchar c)
{
	_grow(1);
	pstring().insert(positionIndex, c);
}

void WString::insert(const int positionIndex, const wchar* const s)
{
	_grow(static_cast<int>(::wcsnlen(s, MAX_LENGTH)));
	pstring().insert(positionIndex, s);
}

void WString::_grow(const int append_len, int cur_len)
{
	int len = cur_len;
	if (len < 0)
		len = length();
	if (len + append_len <= capacity())
		return;

	//create new WString
	int lv = m_level;
	while (g_level_size[lv] <= len + append_len)
		++lv;
	assert(lv < countof(g_level_size));
	int		new_max = g_level_size	[lv];
	wchar*	new_buf = new wchar		[new_max];
	memcpy(new_buf, pstring().c_str(), (len + 1) * sizeof(wchar)); // copy old string, add null-terminator

	//clear old string
	if (m_level)
		m_long.free();
	else
		m_short.clear();

	//assign new string
	m_long.init(new_buf, new_max);
	m_level = lv;
}

void WString::_assign(const wchar* s)
{
	_grow(static_cast<int>(::wcsnlen(s, MAX_LENGTH)));
	pstring().copy(s);
}

int WString::length() const
{
	return static_cast<int>(::wcsnlen(c_str(), capacity()));
}

bool WString::operator==(const wchar* s) const
{
	return pstring() == s;
}

} //namespace ui



