////////////////////////////////////////////////////////////////////////////////
//	tree.h
//	Treeģ�嶨����ʵ�֣�AVL��
//	2010.05.04 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/pair.h"
#include "scl/allocator.h"

namespace scl {

////////////////////////////////////////////////////////////////////////////////
// enum TreeChildType
////////////////////////////////////////////////////////////////////////////////
enum TREE_CHILD_TYPE
{
	TREE_CHILD_TYPE_RIGHT		= -1,
	TREE_CHILD_TYPE_INVALID		= 0,
	TREE_CHILD_TYPE_LEFT		= 1,
	TREE_CHILD_TYPE_ROOT		= 2,
};

////////////////////////////////////////////////////////////////////////////////
// class tree_node
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T>
class tree_node
{
public:
	key_T			key;
	value_T			value;
	tree_node*		left;
	tree_node*		right;
	tree_node*		parent;
	
	//ƽ������(blanceFactor)������Ϊ���Ҳ������ֵ����//���������Ϊ��ֵ���Ҳ�����Ϊ��ֵ
	int			bf;	
	
	tree_node() : left(NULL), right(NULL), parent(NULL), bf(0) {}
};

////////////////////////////////////////////////////////////////////////////////
// class tree
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc = default_allocator<tree_node<key_T, value_T> > >
class tree
{
public:
	typedef tree_node<key_T, value_T>	node_T;
	typedef pair<key_T&, value_T&>		tree_value_T;
	typedef pair<key_T, value_T>		tree_value_insert_T;
	class	iterator;

public:
	tree(): m_pRoot(NULL), m_size(0), m_is_avl(true)  {};
	virtual ~tree() { clear(); };

	//stl���ݽӿ�
	pair<iterator, bool>	insert	(const tree_value_insert_T& val);
	iterator				begin	()	const	{ return iterator(_left_most(m_pRoot));	}
	iterator				end		()	const	{ return iterator(NULL);				}
	iterator				rbegin	()	const	{ return iterator(_right_most(m_pRoot));}
	iterator				rend	()	const	{ return iterator(NULL);				}
	void					clear	()			{ _free_node_recursion(m_pRoot); m_pRoot = NULL;	}
	bool					empty	() const	{ return m_pRoot == NULL;				}
	void					erase	(iterator where)		{ _remove(where.p->key); }	//TODO ���ر�ɾ���ڵ����һ���ڵ�
	void					erase	(const key_T& key)		{ _remove(key); _checkbf(m_pRoot); }					//TODO ���ر�ɾ���ڵ����һ���ڵ�
	iterator				find	(const key_T& key) const	{ return iterator(find_node(key)); }
	int						count	(const key_T& key) const { return find(key) == end() ? 0 : 1; }
	int						size	() const { return m_size; }
	value_T&				operator[](const key_T& key);
	const value_T&			operator[](const key_T& key) const;
	
	//��stl���ݽӿ�
	iterator	add			(const key_T& key, const value_T& value)	{ return iterator(add_node(key, value)); _checkbf(m_pRoot); }
	node_T*		add_node	(const key_T& key, const value_T& value);	//Add ���������ӵĽڵ�
	node_T*		find_node	(const key_T& key) const;
	value_T&	find_value	(const key_T& key) const;
	node_T*		root		() const { return m_pRoot; }
	int			height		() const { return height(m_pRoot); }
	int			height		(const node_T* const pNode) const;
	void		set_is_avl	(bool avl) { m_is_avl = avl; }
	bool		is_avl		()	const { return m_is_avl; }

private:
	void		_checkbf	(node_T* node) const;

	//Copy
	//TODO COPY

public:
	class iterator
	{
	public:
		node_T* p;
		tree_value_T* _pair;
		
		iterator	() : p (NULL), _pair(NULL) {}
		iterator	(node_T* pNode) : _pair(NULL) { p = pNode; }
		~iterator	() { if (_pair) delete _pair; }

		iterator&			operator++();
		iterator&			operator++(const int) { return ++(*this); }
		iterator&			operator--();
		iterator&			operator--(const int) { return --(*this); }
		bool				operator==(const iterator& other) const { return this->p == other.p; }
		bool				operator!=(const iterator& other) const { return this->p != other.p; }
		tree_value_T		operator*() { assert(NULL != p); return tree_value_T(p->key, p->value); }
		const tree_value_T	operator*() const { assert(NULL != p); return tree_value_T(p->key, p->value); }
		tree_value_T*		operator->() 
		{ 
			assert(NULL != p); 
			if (NULL != _pair)
				delete _pair;
			_pair = new tree_value_T(p->key, p->value);
			return _pair;
		}
		const tree_value_T* operator->() const
		{ 
			assert(NULL != p); 
			if (NULL != _pair)
				delete _pair;
			_pair = new tree_value_T(p->key, p->value);
			return _pair;
		}
	};

private:
	//����ɾ��
	node_T*	_insert				(const key_T& key, const value_T& elem, node_T* pPosition, const TREE_CHILD_TYPE& child_type);
	node_T*	_insert_recursion	(const key_T& key, const value_T& elem, node_T* pPosition, bool& needChangeBlanceFactor);
	node_T*	_insert_recursion_not_avl(const key_T& key, const value_T& elem, node_T* pPosition);
	void	_remove				(const key_T& key);
	void	_free_node			(node_T* pNode);
	void	_free_node_recursion(node_T* pNode);

	//��ת����
	int		_left_rotate		(node_T* const oldRoot);	//��ת����������ֵΪ��ת��߶ȱ仯��ֻ��Ϊ-1��0�������ĸ߶ȼ�1�������ĸ߶Ȳ���
	int		_right_rotate		(node_T* const oldRoot);
	int		_left_right_rotate	(node_T* const oldRoot);
	int		_right_left_rotate	(node_T* const oldRoot);

	//�����ڵ������Ҳ�ڵ�
	node_T*	_left_most			(node_T* pNode) const	{ if (NULL != pNode && NULL != pNode->left)	return _left_most(pNode->left);		else return pNode;	}
	node_T*	_right_most			(node_T* pNode) const	{ if (NULL != pNode && NULL != pNode->right) return _right_most(pNode->right);	else return pNode;	}

	void	_adjust_remove_node	(node_T* pNode, int bfChanged);
	void	_attach_node		(node_T* pParent, node_T* pChild, const TREE_CHILD_TYPE childType);
	TREE_CHILD_TYPE _get_child_type(const node_T* const pNode) const; //����Node��Node-parent�����ӽڵ㻹�����ӽڵ�
	void	_swap_node			(node_T* n1, node_T* n2);
	
private:
	node_T*	m_pRoot;
	int		m_size;
	bool	m_is_avl;
};


////////////////////////////////////////////////////////////////////////////////
// treeʵ��
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>* tree<key_T, value_T, Alloc>::add_node(const key_T& key, const value_T& value)
{ 
	if (NULL != m_pRoot)
	{	
		if (is_avl())
		{
			bool needChangeBlanceFactor = false;
			return _insert_recursion(key, value, m_pRoot, needChangeBlanceFactor);
		}
		else
		{
			return _insert_recursion_not_avl(key, value, m_pRoot);
		}
	}
	else
	{
		m_pRoot = Alloc::alloc();
		m_pRoot->key	=	key;
		m_pRoot->value	=	value;
		m_size++;
		return m_pRoot;
	}
}

////////////////////////////////////////////////////////////
//	Removeʵ�֣�
//	�ҵ���Ҫɾ���Ľڵ㣬Ȼ����ݸýڵ��Ƿ�����ӽڵ����ֱ���
//	1.����ýڵ㲻�����ӽڵ㣬��ôֱ��ɾ������
//	2.����ýڵ������ڵ㣬���������ҽڵ㣬������ڵ�����ýڵ�
//	3.��2����֮��Ȼ
//	4.����ýڵ�ͬʱ�������ҽڵ㣬��ôҪ�ݹ���ҵ����ýڵ����ڵ����ҵ�Ҷ�ӽڵ㡱rightMost��
//	  Ȼ����rightMost�滻�ýڵ㣬���ɾ���ýڵ�
//	5.����ɾ���ڵ�ĸ��ڵ㴫�ݸ�adjustRemoveNode����avlƽ��
////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_remove(const key_T& key)
{
	node_T* pRemove = find_node(key);
	if (NULL == pRemove) //û���ҵ���Ҫɾ���Ľڵ�
		return;

	node_T*			pRemoveParent	= NULL;
	TREE_CHILD_TYPE	bfChanged		= TREE_CHILD_TYPE_INVALID;

	//step2.ɾ���ýڵ�
	if (NULL != pRemove->left && NULL != pRemove->right)
	{
		//step2.1 ��������ӽڵ����Ϊ�գ���ô������ɾ���ýڵ�
		node_T* pRightMost = _right_most(pRemove->left);

		//pRightMost���ٿ�����pRemove->left����pRemove->left != NULL�����pRightMost������ΪNULL
		assert(NULL != pRightMost);	

		bool adjacent = (pRightMost->parent == pRemove);
		assert(pRightMost->parent);

		//���pRemove��pRightMost�����ڽڵ㣬��Ҫ���⴦��
		if (adjacent)
		{
			//��������£�rightMost�Ǹ��Ҳ�ڵ�
			//���ǵ�ɾ���ڵ��������ֻ��һ���ڵ�ʱ������ڵ����rightMost��
			//��ʱ�ýڵ���pRemove�����ӽڵ㣬��Ҫ���⴦��
			pRemoveParent = pRightMost;
			bfChanged = TREE_CHILD_TYPE_LEFT;

			_swap_node(pRemove, pRightMost);

			assert(pRightMost->left);

			//pRightMost->left = NULL;


			if (pRemove->left != NULL)
			{
				pRemove->parent->left = pRemove->left;
				pRemove->left->parent = pRemove->parent;
			}
			else
				pRemove->parent->left = NULL;
		}
		else
		{
			pRemoveParent = pRightMost->parent;
			bfChanged = TREE_CHILD_TYPE_RIGHT;

			_swap_node(pRemove, pRightMost);

			assert(pRemove->parent->right);
			//pRemove->parent->right = NULL;

			if (pRemove->left != NULL)
			{
				pRemove->parent->right = pRemove->left;
				pRemove->left->parent = pRemove->parent;
			}
			else
				pRemove->parent->right = NULL;
		}

		//ɾ��RightMost�ڵ�
		_free_node(pRemove);
	}
	else if (NULL == pRemove->left)
	{
		//step2.2 �����ڵ�Ϊ�գ��������ӽڵ���浱ǰ�ڵ�
		bfChanged = _get_child_type(pRemove);
		_attach_node(pRemove->parent, pRemove->right, bfChanged);

		//������Ҫ����ƽ�����ӵĸ��ڵ�
		pRemoveParent = pRemove->parent;

		_free_node(pRemove);
	}
	else if (NULL == pRemove->right)
	{
		//step2.3 ����ҽڵ�Ϊ�գ��������ӽڵ���浱ǰ�ڵ�
		bfChanged = _get_child_type(pRemove);
		_attach_node(pRemove->parent, pRemove->left, bfChanged);

		//������Ҫ����ƽ�����ӵĸ��ڵ�
		pRemoveParent = pRemove->parent;

		_free_node(pRemove);
	}

	//step3 ����ƽ������
	if (is_avl() && NULL != pRemoveParent)
	{
		_adjust_remove_node(pRemoveParent, bfChanged);
	}
}

template<typename key_T, typename value_T, typename Alloc>
pair<typename tree<key_T, value_T, Alloc>::iterator, bool> tree<key_T, value_T, Alloc>::insert(const tree_value_insert_T& val) 
{  
	iterator findKey = find(val.first);
	if (findKey != end())
	{
		return pair<iterator, bool>(findKey, false);
	}
	iterator newNode = add_node(val.first, val.second);
	return pair<iterator, bool>(newNode, true);
}

template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>* tree<key_T, value_T, Alloc>::find_node(const key_T& key) const
{
	node_T* pNode = m_pRoot;
	node_T* pRemove = NULL;

	//step1.�ҵ���Ҫɾ���Ľڵ�
	while (pNode)
	{
		if (key < pNode->key)
			pNode = pNode->left;
		else if (pNode->key < key)
			pNode = pNode->right;
		else// if (pNode->key == key)
		{
			pRemove = pNode;
			break;
		}
	}
	return pRemove;
}

template<typename key_T, typename value_T, typename Alloc>
value_T& tree<key_T, value_T, Alloc>::find_value(const key_T& key) const
{
	node_T* pFoundNode = find_node(key);
	if (NULL == pFoundNode)
	{
		//return NULL;
		assert(0);
		throw 1;
	}
	return pFoundNode->value;
}


//����һ����ɾ���ڵ�ĸ��ڵ�
template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_adjust_remove_node(node_T* pNode, int bfChanged)
{
	if (NULL == pNode)
	{
		assert(false);
		return;
	}

	node_T* pOldParent = pNode->parent;

	//��ɾ���ڵ�ĸ��ڵ�ԭ����bfΪ0��ɾ�������ڵ��bfΪ1����-1�����������е����������ݹ�
	if (0 == pNode->bf)
	{
		pNode->bf += bfChanged;
		
		//�����ݹ�
		return;
	} 
	//��ɾ���ڵ�ĸ��ڵ�ԭ����bfΪ1�������ɾ�����ڵ����ܵ��²�ƽ��
	//ͬʱ��Ҫ�ݹ�ĵ����ýڵ�ĸ��ڵ�
	else if (1 == pNode->bf)
	{
		pNode->bf += bfChanged;
		int newType = _get_child_type(pNode);
		int heightChanged = (bfChanged != 0);
		if (pNode->bf >= 2)
		{
			//��Ҫ������ת
			if (pNode->right->bf >= 0)
			{
				heightChanged = _left_rotate(pNode);
			}
			else
			{
				heightChanged = _right_left_rotate(pNode);
			}
		}
		if (0 != heightChanged && NULL != pOldParent)
		{
			_adjust_remove_node(pOldParent, newType);
		}
		else
		{
			return;
		}
	} 
	//��ɾ���ڵ�ĸ��ڵ�ԭ����bfΪ-1�������ɾ���Ҳ�ڵ����ܵ��²�ƽ��
	//ͬʱ��Ҫ�ݹ�ĵ����ýڵ�ĸ��ڵ�
	else if (-1 == pNode->bf)
	{
		pNode->bf += bfChanged;
		int newType = _get_child_type(pNode);
		int heightChanged = (bfChanged != 0);
		if (pNode->bf <= -2)
		{
			//��Ҫ������ת
			if (pNode->left->bf <= 0)
			{
				heightChanged = _right_rotate(pNode);
			}
			else
			{
				heightChanged = _left_right_rotate(pNode);
			}
		}
		if (0 != heightChanged && NULL != pOldParent)
		{
			_adjust_remove_node(pOldParent, newType);
		}
		else
		{
			return;
		}
	}
	 //�����ɾ���ڵ㸸�ڵ��bf��Ϊ1,-1��0����ʾԭ�����Ѿ���ƽ�⣬����
	else
	{
		assert(false);
	}
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_right_rotate(node_T* const oldRoot)
{
	/*
	//				O (oldRoot) bf = -2
	//		h+2	   / \
	//	(newRoot) O   h
	//			 / \
	//		   h+1  a

	����	a <= h + 1;

	// ��ת��
	//		O (newRoot) 
	//	   / \
	//   h+1  O (oldRoot)   
	//		 / \
	//	    a   h

	�� newRoot->bf == 0 ����a = h + 1��ʱ����ת��߶Ȳ���
	������ת��߶ȼ�1
	*/

	//TODO ָ�밲ȫ�ж�
	//TODO ����parentָ��
	assert(oldRoot);
	assert(oldRoot->left);

	//oldRoot����ڵ���Ϊ���������µĶ���root
	node_T* const newRoot = oldRoot->left;

	//������root����תǰ���ҽڵ�
	node_T* const oldRight = newRoot->right;	

	//����ԭ��oldRoot��parent�ڵ�
	node_T* const oldParent = oldRoot->parent;

	//��oldRoot��ת��newRoot���ҽڵ�
	newRoot->right = oldRoot;
	oldRoot->parent = newRoot;
	newRoot->parent = oldParent;
	if (oldParent)
	{
		if (oldParent->left == oldRoot)
		{
			oldParent->left = newRoot;
		}
		else if (oldParent->right == oldRoot)
		{
			oldParent->right = newRoot;
		}
		else
		{
			assert(false);
		}
	}
	if (oldRoot == m_pRoot)
	{
		m_pRoot = newRoot;
	}

	//��newRoot��ԭ���ҽڵ��ƶ���oldRoot����ڵ�
	oldRoot->left = oldRight;
	if (oldRight)
	{	
		oldRight->parent = oldRoot;
	}

	int heightChanged = 0;
	//����ƽ�����ӣ���������Ƚϸ���
	if (newRoot->bf == 0)
	{
		newRoot->bf = 1;
		oldRoot->bf = -1;
		heightChanged = 0;
	}
	else
	{
		newRoot->bf = 0;
		oldRoot->bf = 0;
		heightChanged = -1;
	}
	return heightChanged;
}

//����ֵΪ������oldRoot���ڵ�λ����Ϊ���ڵ������������ת֮��߶ȵı仯���
template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_left_rotate(node_T* const oldRoot)
{
	/*
	//		O (oldRoot)
	//	   / \
	//	  �� O (newRoot) 
	//	     / \
	//		�� O  
	// ��ת��
	//				 O (newRoot) 
	//				/ \
	//	(oldRoot)  O   O  
	//			  / \
	//           �� ��

	����ƽ�����ӱ仯�͸߶ȱ仯�Ľ��Ͳμ�rightRotate

	*/

	//TODO ָ�밲ȫ�ж�
	//TODO ����parentָ��
	assert(oldRoot);
	assert(oldRoot->right);

	//oldRoot���ҽڵ���Ϊ���������µĶ���root
	node_T* const newRoot = oldRoot->right;

	//������root����תǰ����ڵ�
	node_T* const oldLeft = newRoot->left;	

	//����ԭ��oldRoot��parent�ڵ�
	node_T* const oldParent = oldRoot->parent;

	//��oldRoot��ת��newRoot����ڵ�
	newRoot->left = oldRoot;
	oldRoot->parent = newRoot;
	newRoot->parent = oldParent;
	if (oldParent)
	{
		if (oldParent->left == oldRoot)
		{
			oldParent->left = newRoot;
		}
		else if (oldParent->right == oldRoot)
		{
			oldParent->right = newRoot;
		}
		else
		{
			assert(false);
		}
	}
	if (oldRoot == m_pRoot)
	{
		m_pRoot = newRoot;
	}

	//��newRoot��ԭ����ڵ��ƶ���oldRoot���ҽڵ�
	oldRoot->right = oldLeft;
	if (oldLeft)
	{	
		oldLeft->parent = oldRoot;
	}

	int heightChanged = 0;
	//����ƽ�����ӣ���������Ƚϸ���
	if (newRoot->bf == 0)
	{
		newRoot->bf = -1;
		oldRoot->bf = 1;
		heightChanged = 0;
	}
	else
	{
		newRoot->bf = 0;
		oldRoot->bf = 0;
		heightChanged = -1;
	}
	return heightChanged;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_left_right_rotate(node_T* const oldRoot)
{
	/*
	//					O  (oldRoot)
	//				   / \
    //	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == 1
	//				   / \
	//				  h-1 h
	// ����
	//					O (oldRoot)
	//				   / \
	//	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == -1
	//				   / \
	//				  h  h-1

	//					O  (oldRoot)
	//				   / \
    //	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == 1
	//				   / \
	//				  a   b

	����	a = h �� h-1
			b = h �� h-1
			���������ʱ��������ͬʱ����a = h, b = h����Ȼ��a��b��ֵ��h��h-1�и�ȡһ��
			��ɾ������ʱ������ͬʱ���� a = h, b = h, ��ʱҪע����������ƽ������
			����������� a == b == 0��ʱ�����۲����ɾ�������ܳ��� a == b

	��ת��

	��һ��:
					O  (oldRoot)
				   / \
			      O   h  oldLeftRightBlanceFactor == 1
				 / \
	(oldLeft)   O	b	
			   / \   
			  h	  a 

	 �ڶ�����
			        O  oldLeftRightBlanceFactor == 1
				  /   \
	(oldLeft)   O		O	(oldRoot)
			   / \	   / \
			  h	  a    b  h
	

	*/

	assert(oldRoot);
	assert(oldRoot->left);

	//����ԭ״̬�ڵ㣬������ת�����bf
	node_T* oldLeft				= oldRoot->left;
	node_T* oldLeftRight			= oldRoot->left->right;
	const int oldLeftRightBlanceFactor	= oldLeftRight->bf;
	assert(oldLeftRight);
	//�� a == b == 0 ��ʱ�򣬿��ܳ��� oldLeftRightBlanceFactor == 0
	//assert(oldLeftRightBlanceFactor == 1 || oldLeftRightBlanceFactor == -1);
	
	_left_rotate(oldRoot->left);
	_right_rotate(oldRoot);

	//����ƽ������
	if (oldLeftRightBlanceFactor == 1)
	{
		oldLeft->bf = -1;
		oldRoot->bf = 0;
		oldLeftRight->bf = 0;
	}
	else if (oldLeftRightBlanceFactor == -1)
	{
		oldLeft->bf = 0;
		oldRoot->bf = 1;
		oldLeftRight->bf = 0;
	}
	else if (oldLeftRightBlanceFactor == 0)
	{
		//caolei 2010.09.29
		//����������߼�������oldLeftRightBlacneFactor = 0�����
		//������ɾ���߼���oldLeftRightBlacneFactor = 0 ��ʱ��ᵥ����ȡ����ת��������ת��
		//����Ŀǰ����ִ��������߼�
		//caolei 2011.02.12�� a == b == 0 ��ʱ�򣬿��ܳ��� oldLeftRightBlanceFactor == 0
		//assert(false);

		oldLeft->bf = 0;
		oldRoot->bf = 0;
		oldLeftRight->bf = 0;
	}
	//�����������ĸ߶ȱ�Ȼ����1
	int heightChange = -1;
	return heightChange;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_right_left_rotate(node_T* const oldRoot)
{
	/*
	//									O (oldRoot)
	//								   / \
	//								  h   O (oldRight)
	//									 / \
	//	oldLeftRightBlanceFactor = -1	O   h
	//								   / \
	//								  h	 h-1
	//����
	//									O (oldRoot)
	//								   / \
	//								  h   O (oldRight)
	//									 / \
	//	oldLeftRightBlanceFactor = 1	O   h
	//								   / \
	//								 h-1  h

	������̿��Բμ�leftRightRotate������ע��
	*/

	assert(oldRoot);
	assert(oldRoot->right);

	//����ԭ״̬�ڵ㣬������ת�����bf
	node_T* oldRight = oldRoot->right;
	node_T* oldRightLeft = oldRoot->right->left;
	const int oldRightLeftBlanceFactor = oldRightLeft->bf;
	assert(oldRightLeft);
	//caolei 2011.02.12�� a == b == 0 ��ʱ�򣬿��ܳ��� oldRightLeftBlanceFactor == 0
	//assert(oldRightLeftBlanceFactor == 1 || oldRightLeftBlanceFactor == -1);

	_right_rotate(oldRoot->right);
	_left_rotate(oldRoot);

	//����ƽ������
	if (oldRightLeftBlanceFactor == 1)
	{
		oldRight->bf = 0;
		oldRoot->bf = -1;
		oldRightLeft->bf = 0;
	}
	else if (oldRightLeftBlanceFactor == -1)
	{
		oldRight->bf = 1;
		oldRoot->bf = 0;
		oldRightLeft->bf = 0;
	}
	else if (oldRightLeftBlanceFactor == 0)
	{
		//caolei 2010.09.29
		//����������߼�������oldRightLeftBlanceFactor = 0�����
		//������ɾ���߼���oldRightLeftBlanceFactor = 0 ��ʱ��ᵥ����ȡ����ת��������ת��
		//����Ŀǰ����ִ��������߼�
		//caolei 2011.02.12�� a == b == 0 ��ʱ�򣬿��ܳ��� oldRightLeftBlanceFactor == 0
		//assert(false);

		oldRight->bf = 0;
		oldRoot->bf = 0;
		oldRightLeft->bf = 0;
	}
	//�����������ĸ߶ȱ�Ȼ����1
	int heightChange = -1;
	return heightChange;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::height(const node_T* const pNode) const
{
	if (!pNode)
		return 0;

	int _height = 1;
	int rightHeight = 0;
	int leftHeight = 0;
	if (pNode->left)
	{
		leftHeight = height(pNode->left);
	}

	if (pNode->right)
	{
		rightHeight = height(pNode->right);
	}
	_height += leftHeight > rightHeight ? leftHeight : rightHeight;

	return _height;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_free_node(node_T* pNode)
{
	if (NULL == pNode)
		return;

	m_size--;
	Alloc::free(pNode);
}

template<typename Key, typename Value, typename Alloc>
void tree<Key, Value, Alloc>::_free_node_recursion(node_T* pNode)
{
	if (NULL == pNode)
		return;
	if (pNode->left)
		_free_node_recursion(pNode->left);
	if (pNode->right)
		_free_node_recursion(pNode->right);
	m_size--;
	Alloc::free(pNode);
}

template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert(const key_T& key, const value_T& value, node_T* pPosition, const TREE_CHILD_TYPE& TreeChildType)
{
	if (NULL == pPosition)
	{
		pPosition = m_pRoot;
	}
	if (pPosition)
	{
		node_T* pNewNode = Alloc::alloc();
		pNewNode->key	= key;
		pNewNode->value = value;
		pNewNode->left = NULL;
		pNewNode->right = NULL;
		pNewNode->parent = pPosition;
		if (TreeChildType == TREE_CHILD_TYPE_LEFT)
			pPosition->left = pNewNode;
		else if (TreeChildType == TREE_CHILD_TYPE_RIGHT)
			pPosition->right = pNewNode;
		m_size++;
		return pNewNode;
	}
	return NULL;
}

//�ݹ����
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert_recursion(const key_T& key, const value_T& elem, node_T* pCurrent, bool& needChangeBlanceFactor)
{
	//ע�⣬��keyֵ��ȵ�ʱ�򣬲��뵽�Ҳ࣡
	if (key < pCurrent->key)
	{
		//������
		if (NULL != pCurrent->left)
		{
			node_T* pNewNode = _insert_recursion(key, elem, pCurrent->left, needChangeBlanceFactor);
			if (needChangeBlanceFactor)
			{
				pCurrent->bf--;	//���������ߣ�bf���٣�����bf�����ǡ��������߶ȡ������������߶ȡ���

				if (pCurrent->bf == -1) // ԭ����bf�� 0, ��ߺ��ұ�һ���ߣ�������߼���һ��(bf = bf - 1)��bf��Ϊ-1�ˣ�pCurrent������߶�������1������ needChangeBlanceFactor = true 
					needChangeBlanceFactor = true;
				else if (pCurrent->bf == 0) // ԭ����bf�� 1, �ұ߱���߸߶ȶ�1��������߼���һ��(bf = bf - 1)��bf��Ϊ0�ˣ�����pCurrent������߶�û�䣬���� needChangeBlanceFactor = false
					needChangeBlanceFactor = false;
			}

			if (pCurrent->bf == -2)
			{
				if (key < pCurrent->left->key)
					//������������е�������ת
					_right_rotate(pCurrent);
				else
					//�������������������ת
					_left_right_rotate(pCurrent);

				//������ӽڵ������¡�����ת�����ĸ߶ȱ�Ȼ��1����͵�����ת�����ĸ߶ȼ�1��������ˣ�
				//������ϵ��������Ƚڵ㶼�����޸�ƽ������
				needChangeBlanceFactor = false;
			}

			return pNewNode;
		}
		else
		{
			if (NULL == pCurrent->right)
				//����ڵ�û�������������������ᵼ�¸߶ȱ仯���Ӷ�����ƽ�����ӱ仯
				needChangeBlanceFactor = true;
			else
				//����ڵ��Ѿ�������������������󲻻ᵼ�¸߶ȱ仯����˲����޸�ƽ������
				needChangeBlanceFactor = false;

			pCurrent->bf--;
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_LEFT);
		}
	}
	else if (pCurrent->key < key)
	{
		//�Ҳ����
		if (NULL != pCurrent->right)
		{	
			node_T* pNewNode = _insert_recursion(key, elem, pCurrent->right, needChangeBlanceFactor);

			if (needChangeBlanceFactor)
			{
				pCurrent->bf++;
				if (pCurrent->bf == 1)  // ԭ����bf�� 0, ��ߺ��ұ�һ���ߣ������ұ߼���һ��(bf = bf + 1)��bf��Ϊ1�ˣ�pCurrent������߶�������1������ needChangeBlanceFactor = true 
					needChangeBlanceFactor = true;
				else if (pCurrent->bf == 0) // ԭ����bf�� -1, ��߱��ұ߸߶ȶ�1�������ұ߼���һ��(bf = bf + 1)��bf��Ϊ0�ˣ�����pCurrent������߶�û�䣬���� needChangeBlanceFactor = false
					needChangeBlanceFactor = false;
			}
			if (pCurrent->bf == 2)
			{
				if (key < pCurrent->right->key)
				{
					//�������������������ת
					_right_left_rotate(pCurrent);
				}
				else
				{
					//������������е�������ת
					_left_rotate(pCurrent);
				}
				//������ӽڵ������¡�����ת�����ĸ߶ȱ�Ȼ��1����͵�����ת�����ĸ߶ȼ�1��������ˣ�
				//������ϵ��������Ƚڵ㶼�����޸�ƽ������
				needChangeBlanceFactor = false;
			}

			return pNewNode;
		}
		else
		{
			if (NULL == pCurrent->left)
			{
				//����ڵ�û�������������Ҳ�����ᵼ�¸߶ȱ仯���Ӷ�����ƽ�����ӱ仯
				needChangeBlanceFactor = true;
			}
			else
			{
				//����ڵ��Ѿ��������������Ҳ����󲻻ᵼ�¸߶ȱ仯����˲����޸�ƽ������
				needChangeBlanceFactor = false;
			}
			pCurrent->bf++;
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_RIGHT);
		}
	}
	else // if (pCurrent->key == key)
	{
		//�����ظ�ֵ��
		assert(false);
		return NULL;
	}
}


//�ݹ���룬��������avl��ת
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert_recursion_not_avl(const key_T& key, const value_T& elem, node_T* pCurrent)
{
	if (key < pCurrent->key)
	{
		//������
		if (NULL != pCurrent->left)
			return _insert_recursion_not_avl(key, elem, pCurrent->left);
		else
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_LEFT);
	}
	else if (pCurrent->key < key)
	{
		//�Ҳ����
		if (NULL != pCurrent->right)
			return _insert_recursion_not_avl(key, elem, pCurrent->right);
		else
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_RIGHT);
	}
	else // if (pCurrent->key == key)
	{
		//�����ظ�ֵ��
		assert(false);
		return NULL;
	}
}

template<typename key_T, typename value_T, typename Alloc>
TREE_CHILD_TYPE tree<key_T, value_T, Alloc>::_get_child_type(const node_T* const pNode) const
{
	if (NULL == pNode->parent)
	{
		assert(pNode == m_pRoot);
		return TREE_CHILD_TYPE_ROOT;
	}
	else if (pNode->parent->left == pNode)
	{
		return TREE_CHILD_TYPE_LEFT;
	}
	else if (pNode->parent->right == pNode)
	{
		return TREE_CHILD_TYPE_RIGHT;
	}

	return TREE_CHILD_TYPE_INVALID;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_attach_node(node_T* pParent, node_T* pChild, const TREE_CHILD_TYPE childType)
{
	//����parent�ڵ�
	if (pParent == NULL) //��pChild attach��root
		m_pRoot = pChild;
	else
	{
		if (childType == TREE_CHILD_TYPE_LEFT)
			pParent->left = pChild;
		else if (childType == TREE_CHILD_TYPE_RIGHT)
			pParent->right = pChild;
		else
			assert(false);
	}

	//����child�ڵ�
	if (NULL != pChild)
		pChild->parent = pParent;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_swap_node(node_T* _n1, node_T* _n2)
{
	if (_n1 == _n2)
		return;
	assert(_n1->key < _n2->key || _n2->key < _n1->key);

	//make sure n1 is always on the right side of n2
	node_T* n1 = _n1;
	node_T* n2 = _n2;
	if (_n2->key < _n1->key)
	{
		n1 = _n2;
		n2 = _n1;
	}

	node_T* const n1_left	= n1->left;
	node_T* const n1_right	= n1->right;
	node_T* const n1_parent	= n1->parent;
	int		const n1_bf		= n1->bf;

	node_T* const n2_left	= n2->left;
	node_T* const n2_right	= n2->right;
	node_T* const n2_parent	= n2->parent;
	int		const n2_bf		= n2->bf;

	//swap n1 and n2
	n1->left		= n2_left;
	n1->right	= n2_right;
	n1->parent	= n2_parent;

	n2->left		= n1_left;
	n2->right	= n1_right;
	n2->parent	= n1_parent;

	//if n1 and n2 are adjacent, update special
	if (n1_right == n2)
	{
		n1->parent	= n2;
		n2->right	= n1;
	}
	else if (n2_left == n1)
	{
		n1->left		= n2;
		n2->parent	= n1;
	}

	//update n1.left/right and n2.left/right
	if (NULL != n1_left)
		n1_left->parent = n2;
	if (NULL != n1_right && n1_right != n2)	//only update n1_right when n1 and n2 are NOT adjacent  
		n1_right->parent = n2;
	if (NULL != n2_left && n2_left != n1)	//only update n2_left when n1 and n2 are NOT adjacent
		n2_left->parent = n1;
	if (NULL != n2_right)
		n2_right->parent = n1;

	// update n1.parent
	if (NULL != n1_parent && n2_left != n1) // only update n1.parent when n1.parent is NOT n2
	{
		if (n1_parent->left == n1)
			n1_parent->left = n2;
		else if (n1_parent->right = n1)
			n1_parent->right = n2;
		else
			assert(false);
	}
	// update n2.parent
	if (NULL != n2_parent && n1_right != n2) // only update n2.parent when n2.parent is NOT n1
	{
		if (n2_parent->left == n2)
			n2_parent->left = n1;
		else if (n2_parent->right = n2)
			n2_parent->right = n1;
		else
			assert(false);
	}

	n1->bf = n2_bf;
	n2->bf = n1_bf;

	// update root
	if (m_pRoot == n1)
		m_pRoot = n2;
	else if (m_pRoot == n2)
		m_pRoot = n1;
}

template<typename key_T, typename value_T, typename Alloc>
value_T& tree<key_T, value_T, Alloc>::operator[](const key_T& key) 
{ 
	if (count(key))
	{
		return find_value(key);
	}
	value_T v = value_T();
	pair<iterator, bool> insertResult = insert(make_pair(key, v));
	if (false == insertResult.second || NULL == insertResult.first.p)
	{
		assert(0);
		throw 1;
	}
	return insertResult.first.p->value;
}

template<typename key_T, typename value_T, typename Alloc>
const value_T& tree<key_T, value_T, Alloc>::operator[](const key_T& key) const
{
	return (*const_cast<tree<key_T, value_T, Alloc>*>(this))[key];
}



template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_checkbf(node_T* node) const
{
#ifdef SCL_DEBUG
	if (!is_avl())
		return;
	if (NULL == node)
		return;

	_checkbf(node->left);
	_checkbf(node->right);

	int lheight = height(node->left);
	int rheight = height(node->right);
	int bf = rheight - lheight;

	assert(bf == node->bf);
#else
	node = node;
#endif
}

template<typename key_T, typename value_T, typename Alloc>
typename tree<key_T, value_T, Alloc>::iterator& tree<key_T, value_T, Alloc>::iterator::operator++() 
{	
	if (NULL == p)
		return *this;

	node_T* next = NULL;
	if (NULL != p->right)
	{
		//��������ӽڵ㣬���ҵ����ӽڵ���ӽ�������Ľڵ�(�����ڵ�)
		next = p->right;
		while (next->left)
		{
			next = next->left;
		}
		p = next;
	}
	else if (NULL == p->right)
	{	
		//������ӽڵ�Ϊ�գ���׷�����Ƚڵ㣬ֱ���ҵ����Ƚڵ���ڵ�Ϊnext�Ľڵ�(next�ǵݹ���ҽڵ�)
		next = p;
		while (next->parent && next->parent->right == next)
		{
			next = next->parent;
		}
		p = next->parent;
	}
	return *this; 
}

template<typename key_T, typename value_T, typename Alloc>
typename tree<key_T, value_T, Alloc>::iterator& tree<key_T, value_T, Alloc>::iterator::operator--()
{	
	if (NULL == p)
		return *this;

	node_T* next = NULL;
	if (NULL != p->left)
	{
		//��������ӽڵ㣬���ҵ����ӽڵ���ӽ������ҵĽڵ�(����С�ڵ�)
		next = p->left;
		while (next->right)
		{
			next = next->right;
		}
		p = next;
	}
	else if (NULL == p->left)
	{	
		//������ӽڵ�Ϊ�գ���׷�����Ƚڵ㣬ֱ���ҵ����Ƚڵ��ҽڵ�Ϊnext�Ľڵ�(next�ǵݹ���ҽڵ�)
		next = p;
		while (next->parent && next->parent->left == next)
		{
			next = next->parent;
		}
		p = next->parent;
	}
	return *this; 
}

} //namespace scl
