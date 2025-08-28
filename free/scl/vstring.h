////////////////////////////////////////////////////////////////////////////////
//	vstring (variable length string)
//	Moved from cat/string.h and adapted to scl naming conventions
//	2024.01.xx 
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/string.h"
#include "scl/pstring.h"

// Forward declarations and basic types
namespace scl {
	typedef unsigned int uint;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef wchar_t wchar;
	template<int MAX_COUNT> class string;
	typedef string<16> string16;
}

namespace scl {

class vstring
{
public:
	vstring	();
	vstring	(const char* s);
	vstring	(const char* s, const int len);
	vstring	(const vstring& s);
	~vstring	();

	void		copy		(const char* const s, const int len);
	void		insert		(const int position_index, const char* const s);
	void		insert		(const int position_index, const char c);	
	void		erase		(const int start_index = 0, const int length = -1) { pstring().erase(start_index, length); }
	void		from_wchar	(const wchar_t* s);
	bool		replace		(const char* const old_string, const char* const new_string)  { return pstring().replace(old_string, new_string); }

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
	int			find		(const char c, const int start_index = 0) const	{ return pstring().find(c, start_index); }
	int			find		(const char* const s, const int start_index = 0)	const { return pstring().find(s, start_index); }
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

	vstring&	operator+=	(const char* s);
	vstring&	operator+=	(const char c);
	vstring&	operator=	(const char* s);
	vstring&	operator=	(const vstring& s);
	bool		operator!=	(const char* s) const;
	char&		operator[]	(const int index) { return pstring()[index]; }
	const char& operator[]	(const int index) const { return pstring()[index]; }
	bool 		operator==	(const char* s			) const { return pstring().compare(s) == 0; }
	bool 		operator==	(const vstring& s		) const { return pstring().compare(s.c_str()) == 0; }
	bool 		operator>	(const vstring& s		) const { return pstring().compare(s.c_str()) > 0; }
	bool 		operator<	(const vstring& s		) const { return pstring().compare(s.c_str()) < 0; }

private:
	void		_grow		(const int append_length, int current_length = -1);
	void		_assign		(const char* s);
	void		_assign		(const char* s, const int len);

private:
	string16		m_short;
	scl::pstring	m_long;
	char			m_level;		//defined in g_level_size in vstring.cpp
	char			m_autofree;		//auto call pstring::free() in destructor.
};

uint hash_function(const vstring& key);

} //namespace scl
