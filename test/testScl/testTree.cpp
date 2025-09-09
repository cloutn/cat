
#include "testTree.h"

#include "scl/string.h"
#include "scl/ring_queue.h"
#include "scl/tree.h"

#include <stdio.h>

using scl::array;
using scl::tree;
using scl::string;
using scl::ring_queue;
using scl::tree_node;
using scl::single_allocator;
using scl::string256;
using scl::string16;
using scl::pstring;

typedef single_allocator<tree_node<int, int> > allocator_tree_node;

typedef tree<int, int, allocator_tree_node> tree_int;
typedef tree_int::node_T IntTreeNode;

 void testTree1(); //插入顺序不会造成旋转
 void testTree2(); //从10到1逆序插入
 void testTree3(); //从1到10顺序插入
 void testTree4(); //删除节点
 void testTree5(); //删除节点

//测试插入顺序为3,1,2，制造需要leftRight旋转的情况
 void testTree6();

//测试插入顺序为1,3,2，制造需要leftRight旋转的情况
 void testTree7();

//测试stl相关接口
 void testTree8();

//测试stl相关接口
 void testTree9();

//测试stl相关接口
 void testTree10();

void testTree11();
void testTree12();
void testTree_removeBug();

 void testTree()
{
	//测试插入一个不会发生旋转的序列
	testTree1();

	//测试插入从10到1的递减序列，主要用于测试右旋转
	testTree2();

	//测试插入从1到10的递增序列，主要用于测试左旋转
	testTree3();

	//测试节点的删除，必须保证Add接口测试完成的情况下才能运行
	testTree4();

	//删除节点
	testTree5();

	//测试插入顺序为3,1,2，制造需要leftRight旋转的情况
	testTree6();

	//测试插入顺序为1,3,2，制造需要leftRight旋转的情况
	testTree7();

	//测试stl相关接口
	testTree8();

	//测试stl相关接口
	testTree9();

	//测试stl相关接口
	testTree10();

	//测试非avl树的情况
	testTree11();

	//测试一个删除节点的bug
	testTree12();

	//测试一个remove相关的bug
	testTree_removeBug();

	printf("test tree \t\tOK!\n");
}

void _tree_to_string(tree_int& tree, string256& tree_values, bool withBf)
{
	ring_queue<IntTreeNode*, 256> q;

	//将根节点加入遍历队列
	q.push_back(tree.root());
	while (q.used() > 0)
	{
		//由于每层结束的时候需要打印回车，
		//所以遍历每一层的时候需要把节点先放入一个Array中，
		//等该层所有节点遍历完之后，一次性把Array中所有节点推入队列中
		array<IntTreeNode*, 100> t;

		//遍历取出当前层中所有的节点
		while (q.used() > 0)
		{
			IntTreeNode* pNode;
			q.pop_front(pNode);
			if (pNode)
			{
				string16 number;
				number.from_int(pNode->value);
				tree_values += number.c_str();
				if (withBf)
				{
					string16 bf;
					bf.from_int(pNode->bf);
					tree_values += "_";
					tree_values += bf.c_str();
				}
				tree_values += ",";
			}
			else
			{
				tree_values += "*,";
			}

			//当前层节点的子节点推入临时数组中
			if (pNode)
			{
				t.push_back(pNode->left);
				t.push_back(pNode->right);
			}
		}
		//当前层遍历结束

		//将临时数组中的节点推入队列中，准备下一层节点的遍历
		bool allNull = true;
		for (int i = 0; i < t.size(); ++i)
		{
			if (NULL != t[i])
			{
				allNull = false;
			}
			q.push_back(t[i]);
		}
		//如果下一层节点全部为NULL，则终止搜索
		if (allNull)
		{
			break;
		}
	}
}

void _trim_tree_string(string256& s)
{
	int i = s.length();
	while (i-- >= 0 && (s[i] == ',' || s[i] == '*'));

	s.erase(i + 1);
}

int _calc_node_in_string(const string256& s)
{
	//统计check_values中的节点数量
	int sizeInCheckString = 0;
	string256 tempString = s;
	char* nextToken = NULL;
	char* token = scl_strtok(tempString.c_str(), ",", &nextToken);
	while (token)
	{
		if (pstring(token) == "*")
		{
			token = scl_strtok(NULL, ",", &nextToken);
			continue;
		}
		token = scl_strtok(NULL, ",", &nextToken);
		sizeInCheckString++;
	}
	return sizeInCheckString;
}


/*

返回树的广度优先遍历的字符串表示形式，节点之间用","分隔，空子树用"*"代替
例如:

		1
	   / \
      2   3
	 /	 / \
    4   5   6

表示为"1,2,3,4,*,5,6"
*/
 void checkTree(tree_int& tree, const string256& check_values, bool withBf = false)
{
	string256 tree_values;
	_tree_to_string(tree, tree_values, withBf);
	_trim_tree_string(tree_values);
	
	string256 tmp_check_values = check_values;
	_trim_tree_string(tmp_check_values);

	assert(tree_values == tmp_check_values);

	const int sizeInCheckString = _calc_node_in_string(check_values);
	assert(sizeInCheckString == tree.size());
}

 void testTree1()
{
 	tree_int t;
	//按照以下插入顺序不会造成旋转
	//根节点
	t.add(10, 10);

	//一层节点
	t.add(5, 5);
	t.add(15, 15);

	//二层节点
	t.add(2, 2);
	t.add(8, 8);
	t.add(12, 12);
	t.add(18, 18);

	//三层节点
	t.add(1, 1);
	t.add(4, 4);
	
	/*
	结果:

			10
		   /  \
		  5    15
		 / \   / \
		2   8 12  18
	   / \
	  1	  4
	
	*/
	checkTree(t, "10,5,15,2,8,12,18,1,4,*,*,*,*,*,*");
}

 void testTree2()
{
	tree_int t;
	t.add(10, 10); 	
	checkTree(t, "10");
	t.add(9, 9); 		
	checkTree(t, "10,9,*");

	t.add(8, 8);
	checkTree(t, "9,8,10");

	t.add(7, 7); 		
	checkTree(t, "9,8,10,7,*,*,*");

	t.add(6, 6); 		
	checkTree(t, "9,7,10,6,8,*,*");

	t.add(5, 5); 		
	checkTree(t, "7,6,9,5,*,8,10");

	t.add(4, 4); 		
	checkTree(t, "7,5,9,4,6,8,10");

	t.add(3, 3); 		
	checkTree(t, "7,5,9,4,6,8,10,3,*,*,*,*,*,*,*");

	t.add(2, 2); 		
	checkTree(t, "7,5,9,3,6,8,10,2,4,*,*,*,*,*,*");

	t.add(1, 1); 		
	checkTree(t, "7_-1,3_0,9_0,2_-1,5_0,8_0,10_0,1_0,*,4_0,6_0,*,*,*,*", true);
}

 void testTree3()
{
	tree_int t;

	/*
	只有一个根节点：
		1
	*/
	t.add(1, 1);	//没有旋转
	checkTree(t, "1_0", true);

	/*
			1
			 \
			  2
	*/
	t.add(2, 2);	//没有旋转
	checkTree(t, "1_1,*,2_0", true);

	/*
			2
		   / \
		  1	  3
	*/
 	t.add(3, 3);	//发生旋转
	checkTree(t, "2_0,1_0,3_0", true);


	/*
			2
		   / \
		  1	  3
			   \
			    4
	*/
 	t.add(4, 4);	//没有旋转
	checkTree(t, "2_1,1_0,3_1,*,*,*,4_0", true);

	/*
			2
		   / \
		  1	  4
			 / \
			3   5
	*/
 	t.add(5, 5);	//发生旋转
	checkTree(t, "2_1,1_0,4_0,*,*,3_0,5_0", true);

	/*
			  4
			 / \
			2   5
		   / \	 \
		  1	  3	  6
	*/
 	t.add(6, 6);	//发生旋转
	checkTree(t, "4_0,2_0,5_1,1_0,3_0,*,6_0", true);

	/*
			   4
			 /   \
			2     6
		   / \	 / \
		  1	  3	5   7
	*/
 	t.add(7, 7); //发生旋转
	checkTree(t, "4_0,2_0,6_0,1_0,3_0,5_0,7_0", true);

	/*
			   4
			 /   \
			2     6
		   / \	 / \
		  1	  3	5   7
					 \
					  8
	*/
 	t.add(8, 8);	//不发生旋转
	checkTree(t, "4_1,2_0,6_1,1_0,3_0,5_0,7_1,*,*,*,*,*,*,*,8_0", true);

	/*
			   4
			 /   \
			2     6
		   / \	 / \
		  1	  3	5   8
				   / \
				  7	  9
	*/
 	t.add(9, 9);	//发生旋转
	checkTree(t, "4_1,2_0,6_1,1_0,3_0,5_0,8_0,*,*,*,*,*,*,7_0,9_0", true);

	/*
			     4
			  /     \
			2        8
		   / \	    / \
		  1	  3	   6   9
				  / \   \
				 5   7   10
				
				
	*/
 	t.add(10, 10); //发生旋转
	checkTree(t, "4_1,2_0,8_0,1_0,3_0,6_0,9_1,*,*,*,*,5_0,7_0,*,10_0", true);
}

 void testTree4()
{
 	tree_int t;
 	//根节点
 	t.add(10, 10);
 
 	//一层节点
 	t.add(5, 5);
 	t.add(15, 15);
 
 	//二层节点
 	t.add(2, 2);
 	t.add(8, 8);
 	t.add(12, 12);
 	t.add(18, 18);

	//三层节点
 	t.add(1, 1);
 	t.add(4, 4);
	checkTree(t, "10,5,15,2,8,12,18,1,4,*,*,*,*,*,*");

	/*
				10
			   /  \
			  5   15
			 / \  / \
			2  8 12 18
		   / \
		  1   4
	*/

//	t.erase(3, 3);
	IntTreeNode* pFind = t.find_node(8);
	assert(NULL != pFind);
	assert(pFind->key == 8);
	assert(pFind->value == 8);


	t.erase(8);
	/*
				10
			   /  \
			  2   15
			 / \  / \
			1   5 12 18
		       /
		      4
	*/
	pFind = t.find_node(8);
	assert(NULL == pFind);

	checkTree(t, "10,2,15,1,5,12,18,*,*,4,*,*,*,*,*");
}

 void testTree5()
{
 	tree_int t;
 	//根节点
 	t.add(10, 10);
 
 	//一层节点
 	t.add(5, 5);
 	t.add(15, 15);
 
 	//二层节点
 	t.add(2, 2);
 	t.add(8, 8);
 	t.add(12, 12);
 	t.add(18, 18);

	//三层节点
 	t.add(1, 1);
 	t.add(4, 4);
	checkTree(t, "10,5,15,2,8,12,18,1,4,*,*,*,*,*,*");

	/*
				10
			   /  \
			  5   15
			 / \  / \
			2  8 12 18
		   / \
		  1   4
	*/
	assert(t.size() == 9);

//	t.erase(3, 3);

	t.erase(5);
	/*
				10
			   /  \
			  4   15
			 / \  / \
			2  8 12 18
		   / 
		  1   
	*/
	checkTree(t, "10,4,15,2,8,12,18,1,*,*,*,*,*,*,*");

	tree_int k;
	for (int i = 0; i < 10; ++i)
	{
		k.add(i + 1, i + 1);
	}
	checkTree(k, "4,2,8,1,3,6,9,*,*,*,*,5,7,*,10");

	//tree_int k;
	//for (int i = 0; i < 10; ++i)
	//{
	//	k.add(i, -i);
	//}
	//checkTree(k, "-3,-1,-7,0,-2,-5,-8,*,*,*,*,-4,-6,*,-9");

	k.erase(1);
	/*
		 4
	   /   \
	  2     8
	   \   / \
       3   6  9
	      / \  \
          5 7   10
	*/
	checkTree(k, "4,2,8,*,3,6,9,*,*,5,7,*,10");


 	k.erase(2);
	/*
		     8
	       /   \
		   4	9
         /  \	 \
		 3	  6	  10
			 / \ 
			 5 7
	*/
	checkTree(k, "8,4,9,3,6,*,10,*,*,5,7,*,*");

 	k.erase(3);
	/*
		     8
	       /   \
		  6	  	9
		 / \   	 \
		4	7    10
		 \		
		  5         
	*/
	checkTree(k, "8_-1,6_-1,9_1,4_1,7_0,*,10_0,*,5_0,*,*,*,*", true);


 	k.erase(4);
	/*
		     8
	       /   \
		  6	  	9
		 / \   	 \
		5	7    10
	*/
	checkTree(k, "8_0,6_0,9_1,5_0,7_0,*,10_0", true);

 	k.erase(5);
	/*
		     8
	       /   \
		  6	  	9
		   \   	 \
			7    10
	*/
	checkTree(k, "8,6,9,*,7,*,10");

 	k.erase(6);
	/*
		     8
	       /   \
		  7	  	9
		      	 \
		  		 10
	*/
	checkTree(k, "8,7,9,*,*,*,10");

 	k.erase(7);
	/*
		  	  	9
		      /	 \
		  	 8	 10
	*/
	checkTree(k, "9,8,10");

 	k.erase(8);
	checkTree(k, "9,*,10");

 	k.erase(9);
	checkTree(k, "10");

 	k.erase(10);
	checkTree(k, "*");
}

/*
测试插入顺序为3,1,2，制造需要leftRight旋转的情况
		3
	   /
	  1
	   \
		2
*/
 void testTree6()
{
	tree_int t;
	t.add(3, 3);
	t.add(1, 1);
	t.add(2, 2);
	checkTree(t, "2,1,3");
}

/*
测试插入顺序为1,3,2，制造需要leftRight旋转的情况
		1
		 \
		  3
		 /
	    2
*/
 void testTree7()
{
	tree_int t;
	t.add(1, 1);
	t.add(3, 3);
	t.add(2, 2);
	checkTree(t, "2,1,3");

	//测试Find方法
	int find1 = t.find_value(1);
	assert(find1 == 1);
	int find2 = t.find_value(2);
	assert(find2 == 2);
	int find3 = t.find_value(3);
	assert(find3 == 3);
	assert(t.count(4) == 0);
}

//测试stl相关接口
 void testTree8()
{
	tree_int t;
	for (int i = 0; i < 10; ++i)
	{
		t.add(i, -i);
	}
	tree_int::iterator it = t.begin();
	for (int c = 0; it != t.end(); ++it, ++c)
	{
		assert((*it).first == c);
		assert((*it).second == -c);
	}
}

//测试stl相关接口
 void testTree9()
{
	tree_int t;
	for (int i = 0; i < 10; ++i)
	{
		t.insert(scl::make_pair(i, -i));
	}
	assert(t.size() == 10);
	//t.add(3, 2);
	tree_int::iterator find3 = t.find(3);
	assert((*find3).first == 3);
	assert((*find3).second == -3);
	t.clear();
	assert(t.empty());
	tree_int::iterator it = t.begin();
	for (; it != t.end(); ++it)
	{
		assert(false);
	}
}

//测试stl相关接口
 void testTree10()
{
	tree_int t;
	for (int i = 0; i < 10; ++i)
	{
		t.add(i, -i);
	}
	tree_int::iterator it = t.rbegin();
	for (int c = 9; it != t.rend(); it--, --c)
	{
		//printf("%d, %d\n", (*it).first, (*it).second);
		assert((*it).first == c);
		assert((*it).second == -c);
	}
	t[12] = 33;
	assert(t.find_value(12) == 33);
	t[12] = 44;
	assert(t.find_value(12) == 44);

	const tree_int& constTree = t;
	assert(constTree[12] == 44);
}

//测试非avl树的情况
void testTree11()
{
	tree_int t;
	t.set_is_avl(false);

	t.add(30, 30);
	checkTree(t, "30");

	t.add(90, 90);
	checkTree(t, "30,*,90");

	t.add(80, 80);
	checkTree(t, "30,*,90,80,*");

	t.add(70, 70); 		
	checkTree(t, "30,*,90,80,*,70,*");

	t.add(60, 60);
	checkTree(t, "30,*,90,80,*,70,*,60,*");

	t.add(20, 20); 		
	checkTree(t, "30,20,90,*,*,80,*,70,*,60,*");

	t.add(-20, -20); 		
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,60,*");

	t.add(40, 40); 		
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,60,*,40,*");
	//////

	t.add(50, 50);
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,60,*,40,*,*,50");
	
	t.add(10, 10);
	checkTree(t, "30,20,90,-20,*,80,*,*,10,70,*,*,*,60,*,40,*,*,50");

	t.add(0, 0);
	checkTree(t, "30,20,90,-20,*,80,*,*,10,70,*,0,*,60,*,*,*,40,*,*,50");

	t.erase(0);
	checkTree(t, "30,20,90,-20,*,80,*,*,10,70,*,*,*,60,*,40,*,*,50");

	t.erase(10);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  70  *
	//		       / \   
	//			  60  *  
	//			 / \
	//			40  *
	//		   / \
	//		  *   50
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,60,*,40,*,*,50");

	t.erase(50);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  70  *
	//		       / \   
	//			  60  *  
	//			 / \
	//			40  *
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,60,*,40,*");

	t.erase(60);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  70  *
	//		       / \   
	//			  40  *  
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,40,*");

	t.add(75, 75);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  70  *
	//		       / \   
	//			  40  75  
	checkTree(t, "30,20,90,-20,*,80,*,*,*,70,*,40,75");

	t.erase(70);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  40 *
	//		        /\
	//			   *  75
	checkTree(t, "30,20,90,-20,*,80,*,*,*,40,*,*,75");

	t.add(85, 85);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  40  85
	//		        /\   /\
	//			   *  75 * *
	checkTree(t, "30,20,90,-20,*,80,*,*,*,40,85,*,75");

	t.erase(40);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  *
	//		   /\    / \
	//		  *  *  75  85
	checkTree(t, "30,20,90,-20,*,80,*,*,*,75,85");

	t.add(95, 95);
	//				30
	//			   /  \
	//			 20    90
	//			/  \   / \
	//		   -20 *  80  95
	//		   /\    / \
	//		  *  *  75  85
	checkTree(t, "30,20,90,-20,*,80,95,*,*,75,85");


	t.erase(90);
	//				30
	//			   /  \
	//			 20    85
	//			/  \   / \
	//		   -20 *  80  95
	//		   /\    / \
	//		  *  *  75  *
	checkTree(t, "30,20,85,-20,*,80,95,*,*,75,*");
}

class Test_TextureFile
{
public:
	char name[32];
	int* handle;
	int counter;
	Test_TextureFile() : handle(NULL), counter(0) { memset(name, 0, sizeof(name)); }
	Test_TextureFile(int* _handle, int _counter) : handle(_handle), counter(_counter) { memset(name, 0, sizeof(name)); }
};

void testTree12()
{
	tree_int t;
 	t.add(1, 1);
 	t.add(2, 2);
 	t.add(3, 3);
 
 	//二层节点
 	t.add(4, 4);
	t.erase(4);
	t.add(1001, 1001);
 	t.add(5, 5);
 	t.add(6, 6);
 	t.add(7, 7);

	//三层节点
 	t.add(8, 8);
 	t.add(9, 9);
 	t.add(10, 10);
 	t.add(11, 11);
	checkTree(t, "5,2,9,1,3,7,11,*,*,*,*,6,8,10,1001");

	/*
				5	
			   /  \
			  2    9
			 / \  / \
			1  3 7  11
				/ \  / \
			   6   8 10  1001
	*/
	assert(t.size() == 11);

	t.erase(2);
	checkTree(t, "5,1,9,*,3,7,11,*,*,6,8,10,1001");
	/*
				5	
			   /  \
			  1    9
			   \  / \
			   3 7  11
				/ \  / \
			   6   8 10  1001
	*/
	assert(t.size() == 10);

	t.erase(5);
	checkTree(t, "9,3,11,1,7,10,1001,*,*,6,8");
	/*
				 9	
			   /  \
			  3    11 
			 / \  / \
			1   7 10 1001
			   / \  
			  6   8 
	*/
	assert(t.size() == 9);

	t.erase(1001);
	checkTree(t, "9,3,11,1,7,10,*,*,*,6,8");
	/*
				 9	
			   /  \
			  3    11 
			 / \  / 
			1   7 10
			   / \  
			  6   8 
	*/
	assert(t.size() == 8);

	t.erase(7);
	checkTree(t, "9,3,11,1,6,10,*,*,*,*,8");
	/*
				 9	
			   /  \
			  3    11 
			 / \  / 
			1   6 10
			     \  
			      8 
	*/
	assert(t.size() == 7);

	t.erase(6);
	checkTree(t, "9,3,11,1,8,10");
	/*
				 9	
			   /  \
			  3    11 
			 / \  / 
			1   8 10
	*/
	assert(t.size() == 6);

	t.erase(9);
	checkTree(t, "8,3,11,1,*,10");
	/*
				 8	
			   /  \
			  3    11 
			 /    / 
			1     10
	*/
	assert(t.size() == 5);

	t.erase(8);
	checkTree(t, "3,1,11,*,*,10");
	/*
				 3	
			   /  \
			  1    11 
			      / 
			     10
	*/
	assert(t.size() == 4);

	t.erase(3);
	checkTree(t, "10,1,11");
	/*
				10	
			   /  \
			  1    11 
	*/
	assert(t.size() == 3);

	t.erase(10);
	checkTree(t, "1,*,11");
	/*
				1	
			     \  
			     11  
	*/
	assert(t.size() == 2);

	t.erase(1);
	checkTree(t, "11");
	/*
				11	
	*/
	assert(t.size() == 1);
}

void testTree_removeBug()
{
	scl::tree<string<32>, Test_TextureFile> t; 
	typedef scl::tree<string<32>, Test_TextureFile> TextureTree; 
	const int TEST_FILE_COUNT = 6;
	Test_TextureFile* files[TEST_FILE_COUNT] = { NULL };
	TextureTree::iterator it[TEST_FILE_COUNT];

	it[0] = t.add("background",			Test_TextureFile((int*)0, 0));
	it[1] = t.add("ButtonDown",			Test_TextureFile((int*)1, 1));
	it[2] = t.add("ButtonHighlight",	Test_TextureFile((int*)2, 2));
	it[3] = t.add("ButtonUp",			Test_TextureFile((int*)3, 3));
	it[4] = t.add("border",				Test_TextureFile((int*)4, 4));
	it[5] = t.add("loading",			Test_TextureFile((int*)5, 5));

	for (int i = 0; i < TEST_FILE_COUNT; ++i)
		files[i] = &(it[i]->second);

	TextureTree::iterator itdel = t.find("border");
	t.erase(itdel);
	assert(files[0]->counter == 0);
	assert(files[1]->counter == 1);
	assert(files[2]->counter == 2);
	assert(files[3]->counter == 3);
	assert(files[5]->counter == 5);

	itdel = t.find("background");
	t.erase(itdel);
	assert(files[1]->counter == 1);
	assert(files[2]->counter == 2);
	assert(files[3]->counter == 3);
	assert(files[5]->counter == 5);

	itdel = t.find("ButtonDown");
	t.erase(itdel);
	assert(files[2]->counter == 2);
	assert(files[3]->counter == 3);
	assert(files[5]->counter == 5);

	itdel = t.find("ButtonUp");
	t.erase(itdel);
	assert(files[2]->counter == 2);
	assert(files[5]->counter == 5);

	itdel = t.find("ButtonHighlight");
	t.erase(itdel);
	assert(files[5]->counter == 5);
}


