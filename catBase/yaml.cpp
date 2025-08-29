#include "catBase/yaml.h"

#include "scl/file_mapping.h"
#include "scl/file.h"


namespace yaml {

document::document() : m_tree()
{

}

document::document(c4::yml::Tree* tree) : m_tree(*tree)
{

}

node document::load(const char* const filename)
{
	scl::file_mapping fm;
	if (!fm.open(filename))
		return node();

	char* buffer = (char*)fm.map();

    m_tree = ryml::parse_in_arena(ryml::to_csubstr(buffer));
	return node(m_tree.rootref());
}

void document::save(const char* const filename)
{
#if SCL_WIN
	FILE* fp = NULL;
	fopen_s(&fp, filename, "wb");
#else
	FILE* fp = fopen(filename, "wb");
#endif

	if (NULL == fp)
		return;

	ryml::emit_yaml(m_tree, m_tree.root_id(), fp);

	fclose(fp);
}

node document::root()
{
	return m_tree.rootref();
}

int document::next_sibling(const int id)
{
	return m_tree.next_sibling(id);
}

int document::prev_sibling(const int id)
{
	return m_tree.prev_sibling(id);
}

node::node()
{
}

node::node(c4::yml::NodeRef node) : m_node(node), m_document(node.tree())
{

}

node::node(yaml::document* doc, const int id)
{
	m_document = *doc;
	m_node = c4::yml::NodeRef(&doc->tree(), id);
}

bool node::is_valid() const
{
	return !m_node.invalid();
}

int node::child_count() const
{
	return m_node.num_children();
}

node node::child(const int index)
{
	if (!m_node.is_seq())
	{
		assert(false);
		return node();
	}
	return m_node.child(index);
}

node node::child(const char* name)
{
	if (!m_node.is_map())
	{
		assert(false);
		return node();
	}
	return m_node[c4::to_csubstr(name)];
}

const char* node::value() const
{
	if (!m_node.has_val())
		return NULL;

	if (m_value.empty())
	{
		m_value.copy(m_node.val().data(), m_node.val().len);
	}
	return m_value.c_str();
}


//float node::to_float(float _default) const
//{
//	float v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//double node::to_double(double _default) const
//{
//	double v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//int node::to_int(int _default) const
//{
//	return to_value(_default);
//	//int v;
//	//if (!m_node.has_val())
//	//	return v;
//	//m_node >> v;
//	//return v;
//}
//
//uint node::to_uint(uint _default) const
//{
//	return to_value(_default);
//	//uint v;
//	//if (!m_node.has_val())
//	//	return v;
//	//m_node >> v;
//	//return v;
//}
//
//int16 node::to_int16(int16 _default) const
//{
//	int16 v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//uint16 node::to_uint16(uint16 _default) const
//{
//	uint16 v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//int8 node::to_int8(int8 _default) const
//{
//	int8 v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//uint8 node::to_uint8(uint8 _default) const
//{
//	uint8 v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//int64 node::to_int64(int64 _default) const
//{
//	int64 v;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//uint64 node::to_uint64(uint64 _default) const
//{
//	uint64 v = _default;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}
//
//bool node::to_bool(bool _default) const
//{
//	bool v = _default;
//	if (!m_node.has_val())
//		return v;
//
//	m_node >> v;
//	return v;
//}
//
//scl::vector3 node::to_vector3(scl::vector3 _default) const
//{
//	scl::vector3 v = _default;
//	if (!m_node.has_val())
//		return v;
//	m_node >> v;
//	return v;
//}

void node::set_type(NODE_TYPE type)
{
	switch (type)
	{
	case NODE_TYPE_MAP:
		m_node |= ryml::MAP;
		break;
	case NODE_TYPE_SEQ:
		m_node |= ryml::SEQ;
		break;
	case NODE_TYPE_SEQ_INLINE:
		{
			m_node |= ryml::SEQ;
			m_node |= ryml::FLOW_SL;
		}
		break;
	case NODE_TYPE_SCALAR:
		m_node |= ryml::VAL;
		break;
	default:
		assert(false);
		break;
	}
}

node node::add()
{
	return node(m_node.append_child());
}

node node::add(const char* const name)
{
	return node(m_node[c4::to_csubstr(name)]);
}

node node::add_map()
{
	node n = add();
	n.set_map();
	return n;
}

node node::add_map(const char* const name)
{
	node n = add(name);
	n.set_map();
	return n;
}

node node::add_seq(const char* const name)
{
	node n = add(name);
	n.set_seq();
	return n;
}

node node::add_seq_inline(const char* const name)
{
	node n = add(name);
	n.set_seq_inline();
	return n;
}

node node::add_seq()
{
	node n = add();
	n.set_seq();
	return n;
}

yaml::node_list node::children()
{
	node_iterator begin	(&m_document, m_node.first_child().id());
	node_iterator end	(&m_document, -1);
	node_list iter(begin, end);
	return iter;
}

} // namespace data



