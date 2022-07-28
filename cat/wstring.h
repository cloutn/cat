#pragma once

#include "scl/wstring.h"
#include "scl/pwstring.h"

namespace cat {

class WString
{
public:
	WString	();
	WString	(const wchar* s);
	WString	(const WString& s);
	~WString	();

	void		insert		(const int positionIndex, const wchar* const s);
	void		insert		(const int positionIndex, const wchar c);	
	void		erase		(const int startIndex = 0, const int length = -1) { pstring().erase(startIndex, length); }
	void		from_ansi	(const char* s);
	void		clear		();

	int			length		() const;
	int			capacity	() const	{ if (m_level) return m_long.capacity();	else return m_short.capacity(); }
	wchar*		c_str		()			{ if (m_level) return m_long.c_str();		else return m_short.c_str();	}
	const wchar*c_str		()	const	{ if (m_level) return m_long.c_str();		else return m_short.c_str();	}
	pwstring	pstring		() const { if (m_level) return m_long; else return m_short.pstring(); }
	void		set_autofree(bool v) { m_autofree = v; }
	void		free		() { m_long.free(); }

	WString&	operator+=	(const wchar* s);
	WString&	operator+=	(const wchar c);
	WString&	operator=	(const wchar* s);
	bool		operator==	(const wchar* s) const;
	wchar&		operator[]	(const int index) { return pstring()[index]; }
	const wchar&operator[]	(const int index) const { return pstring()[index]; }

private:
	void		_grow		(const int appendLength, int currentLength = -1);
	void		_assign		(const wchar* s);

private:
	wstring16		m_short;
	scl::pwstring	m_long;
	char			m_level;		//defined in g_level_size in string.cpp
	char			m_autofree;		//auto call pstring::free() in destructor.
};


} //namespace ui

