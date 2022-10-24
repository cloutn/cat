
#include "testHashTable.h"

#include "scl/hash_table.h"

#include <stdio.h>

using scl::hash_table;
using scl::array;

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


	//printf("Pow = %d", Pow(2, 2));
	printf("test hastTable \t\tOK!\n");
}


