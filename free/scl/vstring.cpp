////////////////////////////////////////////////////////////////////////////////
//	vstring (variable length string) implementation
//	Moved from cat/string.cpp and adapted to scl naming conventions
//	2024.01.xx 
////////////////////////////////////////////////////////////////////////////////

#include "scl/vstring.h"
#include "scl/wstring.h"
#include "scl/hash_table.h"
#include <cstring>
#include <cwchar>

// Basic type definitions for standalone compilation
namespace scl {
	typedef unsigned int uint;
}

namespace scl {

#define countof(s) (sizeof(s)/sizeof(s[0]))
static const int g_level_size[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 65536,  262144, 1048576 };
static const int MAX_LENGTH = g_level_size[countof(g_level_size) - 1];	

vstring::vstring() : 
	m_level		(0), 
	m_autofree	(1)
{

}

vstring::vstring(const char* s) : 
	m_level		(0), 
	m_autofree	(1)
{
	_assign(s);
}

vstring::vstring(const char* s, const int len) : 
	m_level		(0), 
	m_autofree	(1)
{
	_assign(s, len);
}

vstring::vstring(const vstring& s) : 
	m_level		(0), 
	m_autofree	(s.m_autofree)
{
	_assign(s.c_str());
}

vstring::~vstring()
{
	if (m_autofree)
		m_long.free();
}

void vstring::copy(const char* const s, const int len)
{
	_assign(s, len);
}

vstring& vstring::operator+=(const char* s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().append(s);
	return *this;
}

vstring& vstring::operator+=(const char c)
{
	_grow(1);
	pstring().append(c);
	return *this;
}

vstring& vstring::operator=(const char* s)
{
	_assign(s);
	return *this;
}

vstring& vstring::operator=(const vstring& s)
{
	_assign(s.c_str());
	m_autofree = s.m_autofree;		//auto call pstring::free() in destructor.
	return *this;
}

void vstring::insert(const int position_index, const char c)
{
	_grow(1);
	pstring().insert(position_index, c);
}

void vstring::insert(const int position_index, const char* const s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().insert(position_index, s);
}

void vstring::from_wchar(const wchar_t* s)
{
	_grow(static_cast<int>(::wcslen(s) * 4));
    
    scl::pstring self = pstring();
    self.clear();
    
	pwstring(const_cast<wchar_t*>(s)).to_ansi(self.c_str(), self.capacity());
}

void vstring::_grow(const int append_len, int cur_len)
{
	int len = cur_len;
	if (len < 0)
		len = length();
	if (len + append_len <= capacity())
		return;

	//create new vstring
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

void vstring::_assign(const char* s)
{
	_grow(static_cast<int>(::strnlen(s, MAX_LENGTH)));
	pstring().copy(s);
}

void vstring::_assign(const char* s, const int len)
{
	_grow(len);
	pstring().copy(s, len);
}

int vstring::length() const
{
	return static_cast<int>(::strnlen(c_str(), capacity()));
}

bool vstring::operator!=(const char* s) const
{
	return pstring() != s;
}

void vstring::clear()
{
	if (m_level)
	{
		m_long.free();
	}
	m_level = 0;
	m_short.clear();
}

uint hash_function(const vstring& key)
{
    return hash_function(key.pstring());
}

} //namespace scl
