#pragma once

#include "scl/string.h"
#include "scl/pstring.h"


namespace cat {

class String
{
public:
	String	();
	String	(const char* s);
	String	(const char* s, const int len);
	String	(const String& s);
	~String	();

	void		copy		(const char* const s, const int len);
	void		insert		(const int positionIndex, const char* const s);
	void		insert		(const int positionIndex, const char c);	
	void		erase		(const int startIndex = 0, const int length = -1) { pstring().erase(startIndex, length); }
	void		from_wchar	(const wchar* s);
	bool		replace		(const char* const oldString, const char* const newString)  { return pstring().replace(oldString, newString); }

	int			length		() const;
	int			capacity	() const	{ if (m_level) return m_long.capacity();	else return m_short.capacity(); }
	char*		c_str		()			{ if (m_level) return m_long.c_str();		else return m_short.c_str();	}
	const char* c_str		()	const	{ if (m_level) return m_long.c_str();		else return m_short.c_str();	}
	scl::pstring pstring	() const { if (m_level) return m_long; else return m_short.pstring(); }
	void		set_autofree(bool v) { m_autofree = v; }
	void		free		() { m_long.free(); }
	void		clear		();
	bool		empty		() const { return pstring().empty(); }
	void		trim		() { pstring().trim(); }
	void		reserve		(const int len) { _grow(len); }
	int			find		(const char c, const int startIndex = 0) const	{ return pstring().find(c, startIndex); }
	int			find		(const char* const s, const int startIndex = 0)	const { return pstring().find(s, startIndex); }
	void		from_int	(const int		value) { pstring().from_int(value); }
	void 		from_uint	(const uint		value) { pstring().from_uint(value); }
	void 		from_double	(const double	value) { pstring().from_double(value); }
	void 		from_int64	(const int64	value) { pstring().from_int64(value); }
	void 		from_uint64	(const uint64	value) { pstring().from_uint64(value); }
	int			to_int		() const { return pstring().to_int(); }
	uint		to_uint		() const { return pstring().to_uint(); }
	double		to_double	() const { return pstring().to_double(); }
	float		to_float	() const { return pstring().to_float(); }
	int64		to_int64	() const { return pstring().to_int64(); }
	uint64		to_uint64	() const { return pstring().to_uint64(); }
	uint		to_hex		() const { return pstring().to_hex(); }
	bool		to_bool		() const { return pstring().to_bool(); }

	String&		operator+=	(const char* s);
	String&		operator+=	(const char c);
	String&		operator=	(const char* s);
	String&		operator=	(const String& s);
	bool		operator!=	(const char* s) const;
	char&		operator[]	(const int index) { return pstring()[index]; }
	const char& operator[]	(const int index) const { return pstring()[index]; }
	bool 		operator==	(const char* s			) const { return pstring().compare(s) == 0; }
	bool 		operator==	(const String& s		) const { return pstring().compare(s.c_str()) == 0; }
	bool 		operator>	(const String& s		) const { return pstring().compare(s.c_str()) > 0; }
	bool 		operator<	(const String& s		) const { return pstring().compare(s.c_str()) < 0; }


private:
	void		_grow		(const int appendLength, int currentLength = -1);
	void		_assign		(const char* s);
	void		_assign		(const char* s, const int len);

private:
	string16		m_short;
	scl::pstring	m_long;
	char			m_level;		//defined in g_level_size in string.cpp
	char			m_autofree;		//auto call pstring::free() in destructor.
};

    

} //namespace ui

namespace cat {
    uint hash_function(const String& key);
}
