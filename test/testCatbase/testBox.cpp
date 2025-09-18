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
		assert(box.min() == vector3::infinity());
		assert(box.max() == -vector3::infinity());
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

	// Test center() method - basic case
	{
		Box box({0, 0, 0}, {2, 4, 6});
		vector3 center = box.center();
		assert((center == vector3{1, 2, 3}));
	}

	// Test center() method - negative coordinates
	{
		Box box({-4, -6, -8}, {0, 0, 0});
		vector3 center = box.center();
		assert((center == vector3{-2, -3, -4}));
	}

	// Test center() method - mixed positive/negative
	{
		Box box({-1, -2, -3}, {3, 2, 1});
		vector3 center = box.center();
		assert((center == vector3{1, 0, -1}));
	}

	// Test center() method - single point box
	{
		Box box({5, 5, 5}, {5, 5, 5});
		vector3 center = box.center();
		assert((center == vector3{5, 5, 5}));
	}

	// Test size() method - basic case
	{
		Box box({1, 2, 3}, {4, 6, 9});
		vector3 size = box.size();
		assert((size == vector3{3, 4, 6}));
	}

	// Test size() method - zero box
	{
		Box box;
		vector3 size = box.size();
		assert((size == vector3{0, 0, 0}));
	}

	// Test size() method - single point box
	{
		Box box({7, 7, 7}, {7, 7, 7});
		vector3 size = box.size();
		assert((size == vector3{0, 0, 0}));
	}

	// Test size() method - negative coordinates
	{
		Box box({-5, -3, -1}, {-2, 0, 2});
		vector3 size = box.size();
		assert((size == vector3{3, 3, 3}));
	}

	// Test center() and size() together
	{
		Box box({10, 20, 30}, {50, 80, 90});
		vector3 center = box.center();
		vector3 size = box.size();
		assert((center == vector3{30, 50, 60}));
		assert((size == vector3{40, 60, 60}));
		
		// Verify relationship: center = min + size/2
		vector3 expected_center = box.min() + size * 0.5f;
		assert((center == expected_center));
	}

	// Test encapsulate() method - point inside existing box
	{
		Box box({0, 0, 0}, {4, 4, 4});
		box.encapsulate({2, 2, 2});  // Point inside
		assert((box.min() == vector3{0, 0, 0}));
		assert((box.max() == vector3{4, 4, 4}));
	}

	// Test encapsulate() method - point outside existing box
	{
		Box box({1, 1, 1}, {3, 3, 3});
		box.encapsulate({5, 0, 4});  // Point extends box in x, y, z
		assert((box.min() == vector3{1, 0, 1}));
		assert((box.max() == vector3{5, 3, 4}));
	}

	// Test encapsulate() method - first point on zero box
	{
		Box box;  // Default zero box
		box.encapsulate({2, 3, 4});
		assert((box.min() == vector3{0, 0, 0}));  // Zero box min stays
		assert((box.max() == vector3{2, 3, 4}));
	}

	// Test encapsulate() method - negative coordinates
	{
		Box box({0, 0, 0}, {2, 2, 2});
		box.encapsulate({-1, -2, -3});
		assert((box.min() == vector3{-1, -2, -3}));
		assert((box.max() == vector3{2, 2, 2}));
	}

	// Test encapsulate() method - exactly on boundary
	{
		Box box({1, 1, 1}, {3, 3, 3});
		box.encapsulate({1, 3, 2});  // Point on boundary
		assert((box.min() == vector3{1, 1, 1}));
		assert((box.max() == vector3{3, 3, 3}));
	}

	// Test encapsulate() method - chain multiple points
	{
		Box box({2, 2, 2}, {3, 3, 3});
		box.encapsulate({0, 4, 1}).encapsulate({5, 0, 6});
		assert((box.min() == vector3{0, 0, 1}));
		assert((box.max() == vector3{5, 4, 6}));
	}

	// Test encapsulate() method - single point box expansion
	{
		Box box({1, 1, 1}, {1, 1, 1});  // Single point box
		box.encapsulate({3, -1, 2});
		assert((box.min() == vector3{1, -1, 1}));
		assert((box.max() == vector3{3, 1, 2}));
	}

	// Test encapsulate() method - mixed positive and negative extensions
	{
		Box box({-1, -1, -1}, {1, 1, 1});
		box.encapsulate({-3, 2, 0});
		assert((box.min() == vector3{-3, -1, -1}));
		assert((box.max() == vector3{1, 2, 1}));
	}

	// Test encapsulate(Box) method - box completely inside
	{
		Box box1({0, 0, 0}, {10, 10, 10});
		Box box2({2, 3, 4}, {5, 6, 7});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{10, 10, 10}));
	}

	// Test encapsulate(Box) method - box completely outside
	{
		Box box1({0, 0, 0}, {2, 2, 2});
		Box box2({5, 5, 5}, {8, 8, 8});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{8, 8, 8}));
	}

	// Test encapsulate(Box) method - overlapping boxes
	{
		Box box1({0, 0, 0}, {4, 4, 4});
		Box box2({2, 2, 2}, {6, 6, 6});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{6, 6, 6}));
	}

	// Test encapsulate(Box) method - partial overlap different directions
	{
		Box box1({1, 1, 1}, {3, 3, 3});
		Box box2({0, 2, 4}, {2, 5, 6});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{0, 1, 1}));
		assert((box1.max() == vector3{3, 5, 6}));
	}

	// Test encapsulate(Box) method - zero box with non-zero box
	{
		Box box1;  // Zero box
		Box box2({-1, -2, -3}, {4, 5, 6});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{-1, -2, -3}));
		assert((box1.max() == vector3{4, 5, 6}));
	}

	// Test encapsulate(Box) method - both negative coordinates
	{
		Box box1({-5, -4, -3}, {-2, -1, 0});
		Box box2({-8, -6, -7}, {-3, -2, -1});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{-8, -6, -7}));
		assert((box1.max() == vector3{-2, -1, 0}));
	}

	// Test encapsulate(Box) method - single point boxes
	{
		Box box1({1, 1, 1}, {1, 1, 1});
		Box box2({3, 3, 3}, {3, 3, 3});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{1, 1, 1}));
		assert((box1.max() == vector3{3, 3, 3}));
	}

	// Test encapsulate(Box) method - chain multiple boxes
	{
		Box box1({2, 2, 2}, {3, 3, 3});
		Box box2({0, 4, 1}, {1, 5, 2});
		Box box3({5, 0, 6}, {6, 1, 7});
		box1.encapsulate(box2).encapsulate(box3);
		assert((box1.min() == vector3{0, 0, 1}));
		assert((box1.max() == vector3{6, 5, 7}));
	}

	// Test operator+= for vector3
	{
		Box box({1, 1, 1}, {3, 3, 3});
		box += vector3{5, 0, 4};
		assert((box.min() == vector3{1, 0, 1}));
		assert((box.max() == vector3{5, 3, 4}));
	}

	// Test operator+= for Box
	{
		Box box1({0, 0, 0}, {2, 2, 2});
		Box box2({1, 3, 1}, {4, 4, 5});
		box1 += box2;
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{4, 4, 5}));
	}

	// Test encapsulate(Box) method - identical boxes
	{
		Box box1({1, 2, 3}, {4, 5, 6});
		Box box2({1, 2, 3}, {4, 5, 6});
		box1.encapsulate(box2);
		assert((box1.min() == vector3{1, 2, 3}));
		assert((box1.max() == vector3{4, 5, 6}));
	}

	// Test is_empty() method - default constructor box
	{
		Box box;
		assert(box.is_empty());  // Default box with min=max=zero should be empty
	}

	// Test is_empty() method - single point box
	{
		Box box({2, 3, 4}, {2, 3, 4});
		assert(box.is_empty());  // Box with min=max should be empty
	}

	// Test is_empty() method - normal box
	{
		Box box({0, 0, 0}, {1, 1, 1});
		assert(!box.is_empty());  // Box with min!=max should not be empty
	}

	// Test is_empty() method - negative coordinates single point
	{
		Box box({-5, -3, -1}, {-5, -3, -1});
		assert(box.is_empty());  // Single point with negative coords should be empty
	}

	// Test is_empty() method - negative coordinates normal box
	{
		Box box({-3, -3, -3}, {-1, -1, -1});
		assert(!box.is_empty());  // Box with negative coords but min!=max should not be empty
	}

	// Test is_empty() method - box created with setMinDirect/setMaxDirect
	{
		Box box;
		box.setMinDirect({1, 1, 1});
		box.setMaxDirect({1, 1, 1});
		assert(box.is_empty());  // Box set to single point should be empty
	}

	// Test is_empty() method - box after intersection with no overlap
	{
		Box box1({0, 0, 0}, {1, 1, 1});
		Box box2({2, 2, 2}, {3, 3, 3});
		Box result = Box::intersection(box1, box2);
		assert(result.is_empty());  // No intersection should result in empty box
	}

	// Test is_empty() method - tiny but not empty box
	{
		Box box({0, 0, 0}, {0.001f, 0.001f, 0.001f});
		assert(!box.is_empty());  // Very small box but min!=max should not be empty
	}

	// Test is_empty() method - box modified to be empty via set()
	{
		Box box({1, 2, 3}, {4, 5, 6});  // Start with non-empty box
		assert(!box.is_empty());
		box.set({7, 7, 7}, {7, 7, 7});  // Set to single point
		assert(box.is_empty());  // Should now be empty
	}

	// Test is_empty() method - box after encapsulating same point multiple times
	// Note: Current implementation includes default (0,0,0) point, so this creates a non-empty box
	{
		Box box;
		box.encapsulate({1, 2, 3});
		box.encapsulate({1, 2, 3});  // Encapsulate same point twice
		// Current behavior: box contains (0,0,0) and (1,2,3), so it's not empty
		assert(!box.is_empty());  // Current implementation behavior
		assert((box.min() == vector3{0, 0, 0}));
		assert((box.max() == vector3{1, 2, 3}));
	}

	// Test is_empty() method - mixed coordinates single point
	{
		Box box({-1, 0, 2}, {-1, 0, 2});
		assert(box.is_empty());  // Mixed sign coordinates but same point should be empty
	}

	// Test is_empty() method - box one dimension empty
	{
		Box box({0, 0, 0}, {1, 1, 0});  // Zero thickness in Z dimension
		assert(!box.is_empty());  // Still has area in XY plane, not empty
	}

	// Test demonstrating the "default point inclusion" behavior
	{
		Box box1;
		Box box2({5, 5, 5}, {5, 5, 5});  // Single point box
		
		box1.encapsulate({5, 5, 5});  // Should this include (0,0,0)?
		box2.encapsulate({5, 5, 5});  // This should remain single point
		
		// Current behavior: box1 includes both (0,0,0) and (5,5,5)
		assert(!box1.is_empty());
		assert((box1.min() == vector3{0, 0, 0}));
		assert((box1.max() == vector3{5, 5, 5}));
		
		// box2 remains a single point
		assert(box2.is_empty());
		assert((box2.min() == vector3{5, 5, 5}));
		assert((box2.max() == vector3{5, 5, 5}));
	}

	printf("test box finished.");
}