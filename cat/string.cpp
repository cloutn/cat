
#include "cat/string.h"

#include "scl/wstring.h"
#include "scl/hash_table.h"

namespace cat {

#define countof(s) (sizeof(s)/sizeof(s[0]))
static const int g_level_size[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 65536,  262144, 1048576 };
static const int MAX_LENGTH = g_level_size[countof(g_level_size) - 1];	

String::String() : 
	m_level		(0), 
	m_autofree	(1)
{

}

String::String(const char* s) : 
	m_level		(0), 
	m_autofree	(1)
{
	_assign(s);
}

String::String(const char* s, const int len) : 
	m_level		(0), 
	m_autofree	(1)
{
	_assign(s, len);
}

String::String(const String& s) : 
	m_level		(0), 
	m_autofree	(s.m_autofree)
{
	_assign(s.c_str());
}

String::~String()
{
	if (m_autofree)
		m_long.free();
}

void String::copy(const char* const s, const int len)
{
	_assign(s, len);
}

String& String::operator+=(const char* s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().append(s);
	return *this;
}

String& String::operator+=(const char c)
{
	_grow(1);
	pstring().append(c);
	return *this;
}

String& String::operator=(const char* s)
{
	_assign(s);
	return *this;
}

String& String::operator=(const String& s)
{
	_assign(s.c_str());
	m_autofree = s.m_autofree;		//auto call pstring::free() in destructor.
	return *this;
}

void String::insert(const int positionIndex, const char c)
{
	_grow(1);
	pstring().insert(positionIndex, c);
}

void String::insert(const int positionIndex, const char* const s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().insert(positionIndex, s);
}

void String::from_wchar(const wchar* s)
{
	_grow(static_cast<int>(::wcsnlen(s, MAX_LENGTH) * 4));
    
    scl::pstring self = pstring();
    self.clear();
    
	pwstring(const_cast<wchar*>(s)).to_ansi(self.c_str(), self.capacity());
}

void String::_grow(const int append_len, int cur_len)
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
	char*	new_buf = new char		[new_max];
	memcpy(new_buf, pstring().c_str(), (len + 1) * sizeof(char)); // copy old string, add null-terminator

	//clear old string
	if (m_level)
		m_long.free();
	else
		m_short.clear();

	//assign new string
	m_long.init(new_buf, new_max);
	m_level = lv;
}

void String::_assign(const char* s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().copy(s);
}

void String::_assign(const char* s, const int len)
{
	_grow(len);
	pstring().copy(s, len);
}

int String::length() const
{
	return static_cast<int>(::strnlen(c_str(), capacity()));
}

bool String::operator!=(const char* s) const
{
	return pstring() != s;
}

void String::clear()
{
	if (m_level)
	{
		m_long.free();
	}
	m_level = 0;
	m_short.clear();
}

} //namespace ui

namespace cat {
    uint hash_function(const String& key)
    {
        return scl::hash_function(key.pstring());
    }
}


