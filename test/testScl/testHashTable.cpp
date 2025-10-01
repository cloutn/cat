
#include "testHashTable.h"

#include "scl/hash_table.h"

#include <stdio.h>

using scl::hash_table;
using scl::array;
using scl::pstring;
using scl::string32;

 void testHashTable1()
{
	array<int, 32> k1;
	k1.push_back(2);
	k1.push_back(3);
	k1.push_back(4);
	array<int, 32> k2;
	k2.push_back(2);
	k2.push_back(3);
	hash_table<array<int, 32>, int> ht;
	ht.init(32);
	ht.add(k1, 201);
	ht.add(k2, 202);
	assert(ht[k1] == 201);
	assert(ht[k2] == 202);
}

 void testHashTable2()
{
	char s1[] = "abcd";
	char s2[] = "efg";
	char s3[] = "";
	pstring p1(s1, 5);
	pstring p2(s2, 4);
	pstring p3(s3, 1);
	hash_table<pstring, int> ht;
	ht.init(10);
	ht.add(p1, 33);
	ht.add(p2, 34);
	ht.add(p3, 35);
	assert(ht[p1] == 33);
	assert(ht[p2] == 34);
	assert(ht[p3] == 35);
}

// Mock类用于测试，模拟ComponentArrayBase
class MockComponent
{
public:
	int value;
	MockComponent(int v) : value(v) {}
};

void testHashTableUInt64()
{
	// 测试uint64作为key，指针作为value (模拟World::m_componentArrayTable的使用场景)
	hash_table<uint64, MockComponent*> ht;
	ht.init(32);
	
	// 创建一些测试用的MockComponent对象
	MockComponent* comp1 = new MockComponent(100);
	MockComponent* comp2 = new MockComponent(200);
	MockComponent* comp3 = new MockComponent(300);
	
	// 测试一些典型的uint64值
	uint64 key1 = 12345678901234567ULL;	// 大整数
	uint64 key2 = 0x1234567890ABCDEFULL;	// 十六进制大数
	uint64 key3 = 1;						// 小整数
	uint64 key4 = 0;						// 零值
	
	// 添加键值对
	ht.add(key1, comp1);
	ht.add(key2, comp2);
	ht.add(key3, comp3);
	
	// 测试查找
	assert(ht[key1] == comp1);
	assert(ht[key2] == comp2);  
	assert(ht[key3] == comp3);
	assert(ht[key1]->value == 100);
	assert(ht[key2]->value == 200);
	assert(ht[key3]->value == 300);
	
	// 测试find方法
	MockComponent* found1 = ht.find(key1);
	MockComponent* found2 = ht.find(key2);
	assert(found1 == comp1);
	assert(found2 == comp2);
	
	// 测试不存在的key
	assert(ht.find_index(key4) == -1);
	
	// 测试count方法
	assert(ht.count(key1) == true);
	assert(ht.count(key2) == true);
	assert(ht.count(key3) == true);
	assert(ht.count(key4) == false);
	
	// 测试删除
	ht.erase(key2);
	assert(ht.count(key2) == false);
	assert(ht.find_index(key2) == -1);
	
	// 验证其他键值对仍然存在
	assert(ht.count(key1) == true);
	assert(ht.count(key3) == true);
	assert(ht[key1] == comp1);
	assert(ht[key3] == comp3);
	
	// 测试get_values功能（模拟World析构函数的使用）
	scl::varray<MockComponent*> values;
	ht.get_values(values);
	assert(values.size() == 2);  // key2被删除了，应该只剩2个
	
	// 验证values包含正确的指针
	bool foundComp1 = false, foundComp3 = false;
	for (int i = 0; i < values.size(); ++i)
	{
		if (values[i] == comp1) foundComp1 = true;
		if (values[i] == comp3) foundComp3 = true;
	}
	assert(foundComp1);
	assert(foundComp3);
	
	// 清理内存
	delete comp1;
	delete comp2;
	delete comp3;
	
	printf("testHashTableUint64 passed!\n");
}

 void testHashTable()
{
	//测试IsPrime函数
	assert(scl::is_prime(97));
	assert(!scl::is_prime(98));

	//测试MinPrime函数
	assert(scl::min_prime(100) == 97);

	hash_table<int, int> ht;
	ht.init(100);
	ht.add(12, 33);
	ht.add(13, 34);
	ht.add(14, 376);
	assert(ht[12] == 33);
	assert(ht[13] == 34);
	assert(ht[14] == 376);
	int q = ht.find(12);
	ht.erase(13);
	assert(!ht.count(13));
	assert(ht.count(12));
	assert(ht.count(14));
	q = q;

	hash_table<string32, int> sTable;
	sTable.init(100);
	sTable.add("buaa", 20);
	sTable.add("caolei", 30);

	int b1 = sTable.find("buaa");
	int b2 = sTable.find("caolei");
	assert(b1 == 20);
	assert(b2 == 30);

	testHashTable1();

	testHashTable2();
	
	testHashTableUInt64();

	//printf("Pow = %d", Pow(2, 2));
	printf("test hastTable \t\tOK!\n");
}


