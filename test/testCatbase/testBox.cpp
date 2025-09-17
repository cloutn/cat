#include "./testBox.h"

#include "cat/box.h"
#include "scl/vector.h"

#include <assert.h>
#include <stdio.h>

using namespace cat;
using namespace scl;

void testBox()
{
	// 测试默认构造函数
	{
		Box box;
		assert(box.min() == vector3::zero());
		assert(box.max() == vector3::zero());
	}

	// 测试带参数构造函数
	{
		Box box({1, 2, 3}, {4, 5, 6});
		assert((box.min() == vector3{1, 2, 3}));
		assert((box.max() == vector3{4, 5, 6}));
	}
	{
		Box box({3, 3, 3}, {2, 2, 2});
		assert((box.min() == vector3{2, 2, 2}));
		assert((box.max() == vector3{3, 3, 3}));
	}

	// 测试setter/getter - 直接设置版本
	{
		Box box;
		box.setMinDirect({0, 1, 2});
		box.setMaxDirect({3, 4, 5});
		assert((box.min() == vector3{0, 1, 2}));
		assert((box.max() == vector3{3, 4, 5}));
	}

	// 测试set函数（会自动调用ensureValid）
	{
		Box box;
		box.set({3, 3, 3}, {1, 1, 1}); 
		assert((box.min() == vector3{1, 1, 1}));
		assert((box.max() == vector3{3, 3, 3}));
	}

	// 测试相交检测 - 相交情况
	{
		Box box1({0, 0, 0}, {2, 2, 2});
		Box box2({1, 1, 1}, {3, 3, 3});
		assert(box1.intersect(box2));
		assert(box2.intersect(box1));
	}

	// 测试相交检测 - 不相交情况
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({2, 2, 2}, {3, 3, 3});
		assert(!box1.intersect(box2));
		assert(!box2.intersect(box1));
	}

	// 测试相交检测 - 边界触碰
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({1, 1, 1}, {2, 2, 2});
		assert(box1.intersect(box2));
	}

	// 测试相交检测 - 完全包含
	{
		Box box1({0, 0, 0}, {4, 4, 4});
		Box box2({1, 1, 1}, {3, 3, 3});
		assert(box1.intersect(box2));
		assert(box2.intersect(box1));
	}

	// 测试静态交集函数 - 有交集
	{
		Box box1({0, 0, 0}, {2, 2, 2});
		Box box2({1, 1, 1}, {3, 3, 3});
		Box result = Box::intersection(box1, box2);
		assert((result.min() == vector3{1, 1, 1}));
		assert((result.max() == vector3{2, 2, 2}));
	}

	// 测试静态交集函数 - 无交集
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({2, 2, 2}, {3, 3, 3});
		Box result = Box::intersection(box1, box2);
		// 无交集时返回默认Box（min=max=0）
		assert(result.min() == vector3::zero());
		assert(result.max() == vector3::zero());
	}

	// 测试成员交集函数
	{
		Box box1({0, 0, 0}, {2, 2, 2});
		Box box2({1, 1, 1}, {3, 3, 3});
		box1.intersection(box2);
		assert((box1.min() == vector3{1, 1, 1}));
		assert((box1.max() == vector3{2, 2, 2}));
	}

	// 测试静态并集函数
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({2, 2, 2}, {3, 3, 3});
		Box result = Box::combine(box1, box2);
		assert((result.min() == vector3{0, 0, 0}));
		assert((result.max() == vector3{3, 3, 3}));
	}

	// 测试成员并集函数
	{
		Box box1({1, 1, 1}, {2, 2, 2});
		Box box2({0, 0, 0}, {3, 3, 3});
		box1.combine(box2);
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{3, 3, 3}));
	}

	// 测试边界条件 - 单点Box
	{
		Box box1({1, 1, 1}, {1, 1, 1});
		Box box2({1, 1, 1}, {2, 2, 2});
		assert(box1.intersect(box2)); 
	}

	// 测试边界条件 - 无效Box（min > max）
	{
		Box box;
		box.setMinDirect({2, 2, 2});
		box.setMaxDirect({1, 1, 1}); // 直接设置，不会自动修正
		// 验证setMinDirect/setMaxDirect不会自动修正
		assert((box.min() == vector3{2, 2, 2}));
		assert((box.max() == vector3{1, 1, 1}));
		
		// 需要手动调用ensureValid来修正
		box.ensureValid();
		assert((box.min() == vector3{1, 1, 1}));
		assert((box.max() == vector3{2, 2, 2}));
	}

	// 测试链式调用
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({2, 2, 2}, {3, 3, 3});
		Box box3({4, 4, 4}, {5, 5, 5});
		
		Box result = box1;
		result.combine(box2).combine(box3);
		assert((result.min() == vector3{0, 0, 0}));
		assert((result.max() == vector3{5, 5, 5}));
	}

	// 测试负坐标
	{
		Box box1({-2, -2, -2}, {-1, -1, -1});
		Box box2({-1.5, -1.5, -1.5}, {0, 0, 0});
		assert(box1.intersect(box2));
		
		Box result = Box::intersection(box1, box2);
		assert((result.min() == vector3{-1.5, -1.5, -1.5}));
		assert((result.max() == vector3{-1, -1, -1}));
	}

	printf("test box finished.");
}