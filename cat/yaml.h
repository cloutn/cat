#pragma once

#include "cat/string.h"

#include "rapidyaml/ryml.hpp"
#include "c4/format.hpp"

#include "scl/vector.h"
#include "scl/assert.h"

namespace scl {
inline size_t	to_chars	(ryml::substr	buf, scl::vector3	v) { return ryml::format(buf, "{x : {}, y : {}, z : {}}", v.x, v.y, v.z); }
inline bool		from_chars	(ryml::csubstr	buf, scl::vector3*	v) { size_t ret = ryml::unformat(buf, "{x : {}, y : {}, z : {}}", v->x, v->y, v->z); return ret != ryml::yml::npos; }
}

namespace yaml {

class node;
class node_list;

class document
{
public:
	document();
	document(c4::yml::Tree* tree);

	node			load			(const char* const filename);
	void			save			(const char* const filename);
	node			root			();

	// for node iterator
	int				next_sibling	(const int id);
	int				prev_sibling	(const int id);
	c4::yml::Tree&	tree			() { return m_tree; }

private:
	c4::yml::Tree	m_tree;

}; // class document

enum NODE_TYPE
{
	NODE_TYPE_SCALAR,
	NODE_TYPE_MAP,
	NODE_TYPE_SEQ,
	NODE_TYPE_SEQ_INLINE,
};

class node
{
public:
	node();
	node(yaml::document* doc, const int id); // for node iterator
	node(c4::yml::NodeRef node);

	bool			is_valid		() const;
	int				child_count		() const;
	node			child			(const int index);
	node			child			(const char* const);
	node_list		children		();
	node			operator[]		(const int index)			{ return child(index);	}
	node			operator[]		(const char* const name)	{ return child(name);	}
	const char*		value			() const;
	const char*		to_string		(const char* _default = "") const { return to_value(_default); }
	float			to_float		(float	_default = 0) const { return to_value(_default); }
	double			to_double		(double _default = 0) const { return to_value(_default); } 
	int				to_int			(int	_default = 0) const { return to_value(_default); }
	uint			to_uint			(uint	_default = 0) const { return to_value(_default); }
	int16			to_int16		(int16	_default = 0) const { return to_value(_default); }
	uint16			to_uint16		(uint16	_default = 0) const { return to_value(_default); }
	int8			to_int8			(int8	_default = 0) const { return to_value(_default); }
	uint8			to_uint8		(uint8	_default = 0) const { return to_value(_default); }
	int64			to_int64		(int64	_default = 0) const { return to_value(_default); }
	uint64			to_uint64		(uint64	_default = 0) const { return to_value(_default); }
	bool			to_bool			(bool	_default = false) const { return to_value(_default); }
	scl::vector3	to_vector3		(scl::vector3 _default = { 0, 0, 0}) const { return to_value(_default); }

	template <typename T>
	T				to_value(T _default) const
	{
		T v = _default;
		if (!m_node.has_val())
			return v;
		m_node >> v;
		return v;
	}

	void			set_type		(NODE_TYPE type);
	node			set_map			() { set_type(NODE_TYPE_MAP); return *this; }
	node			set_seq			() { set_type(NODE_TYPE_SEQ); return *this; }
	node			set_seq_inline	() { set_type(NODE_TYPE_SEQ_INLINE); return *this; }
	node			add				();
	node			add				(const char* const name);
	node			add_map			();
	node			add_map			(const char* const name);
	node			add_seq			();
	node			add_seq			(const char* const name);
	node			add_seq_inline	(const char* const name);
	node			add				(const char* const name, char* value) { return add(name, (const char*)value); }

	template <typename T>
	node			add				(const char* const name, T value)
	{
		node n = add(name);
		n.set_value(value);
		return n;
	}

	template <typename T>
	node			add_val			(T value)
	{
		node n = add();
		n.set_value(value);
		return n;
	}

	template <typename T>
	void			set_value		(T v) { m_node << v;  }

private:
	c4::yml::NodeRef	m_node;
	yaml::document		m_document;
	mutable cat::String m_value;

}; // class node


class node_iterator
{
public:
    node_iterator	(yaml::document* doc, int id) : m_document(doc), m_childID(id) { }

    node_iterator&	operator++ () { assert(m_childID != -1); m_childID = m_document->next_sibling(m_childID); return *this; }
    node_iterator&	operator-- () { assert(m_childID != -1); m_childID = m_document->prev_sibling(m_childID); return *this; }
    yaml::node		operator*  () const { return yaml::node(m_document, m_childID); }
    yaml::node		operator-> () const { return yaml::node(m_document, m_childID); }
    bool			operator!= (node_iterator that) const { assert(m_document == that.m_document); return m_childID != that.m_childID; }
    bool			operator== (node_iterator that) const { assert(m_document == that.m_document); return m_childID == that.m_childID; }

private:
    yaml::document*	m_document;
    size_t			m_childID;

}; // class node_iterator


class node_list
{
public:
    inline node_list(const node_iterator & b_, const node_iterator & e_) : b(b_), e(e_) {}
    inline node_iterator begin() const { return b; }
    inline node_iterator end  () const { return e; }

private:
    node_iterator b;
	node_iterator e;

}; // class node_list



} // namespace yaml





