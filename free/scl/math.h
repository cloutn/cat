////////////////////////////////////////////////////////////////////////////////
//	mathematical
//	2010.05.01 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

#include <assert.h>

namespace scl {

const float PI			= 3.1415926535897932384626433832795f;
const float PI2			= 6.283185307179586476925286766559f;
const float PI_HALF		= 1.5707963267948966192313216916398f;
const float PI_QUATER	= 0.78539816339744830961566084581988f;

const	int		MAX_INT		= 0x7FFFFFFF;
const	uint	MAX_UNIT	= 0xFFFFFFFF;
const	int64	MAX_INT64	= 0x7FFFFFFFFFFFFFFF;
const	uint64	MAX_UINT64	= 0xFFFFFFFFFFFFFFFF;

const	int		MIN_INT		= 0x80000000;
const	int64	MIN_INT64	= 0x8000000000000000;

//const float FLOAT_EPSILON = 0.0001f;
const float MATH_FLOAT_EPSILON = 1e-6;
const double MATH_DOUBLE_EPSILON = 1e-6;

// ===============  Integer Limits  =============== 
constexpr int32		SCL_INT8_MIN			= -128;											// -128
constexpr int32		SCL_INT8_MAX			= 127;											//  127
constexpr uint32	SCL_UINT8_MAX			= 255;											//  255
constexpr int32		SCL_INT16_MIN			= -32768;										// -32768
constexpr int32		SCL_INT16_MAX			= 32767;										//  32767
constexpr uint32	SCL_UINT16_MAX			= 65535;										//  65535
constexpr int32		SCL_INT32_MIN			= static_cast<int32>(0x80000000);				// -2147483648
constexpr int32		SCL_INT32_MAX			= 0x7FFFFFFF;									//  2147483647
constexpr uint32	SCL_UINT32_MAX			= 0xFFFFFFFFu;									//  4294967295
constexpr int64		SCL_INT64_MIN			= static_cast<int64>(0x8000000000000000ULL);	// -9223372036854775808
constexpr int64		SCL_INT64_MAX			= 0x7FFFFFFFFFFFFFFFLL;							//  9223372036854775807
constexpr uint64	SCL_UINT64_MAX			= 0xFFFFFFFFFFFFFFFFuLL;						// 18446744073709551615


// ===============  Floating-Point Limits  ===============
constexpr float		SCL_FLOAT_MIN				= 1.1754943508222875e-38f;						// smallest positive normal
constexpr float		SCL_FLOAT_MAX				= 3.4028234663852886e+38f;						// largest finite value
constexpr float		SCL_FLOAT_EPSILON			= 1.1920928955078125e-7f;						// 1 ULP
constexpr double	SCL_DOUBLE_MIN				= 2.2250738585072014e-308;
constexpr double	SCL_DOUBLE_MAX				= 1.7976931348623157e+308;
constexpr double	SCL_DOUBLE_EPSILON			= 2.2204460492503131e-16;

// ===============  Floating-Point Special Values  =============== 
// Bit patterns are IEEE-754 compliant
union				FloatUnion				{ uint32 u; float f; };
union				DoubleUnion				{ uint64 u; double f; };
constexpr float		SCL_FLOAT_INFINITY		() noexcept { return FloatUnion{ 0x7F800000u }.f; }	// positive infinity
constexpr float		SCL_FLOAT_NEG_INFINITY	() noexcept { return -SCL_FLOAT_INFINITY(); }
constexpr float		SCL_FLOAT_QNAN			() noexcept { return FloatUnion{ 0x7FC00000u }.f; }	// quiet NaN: silent on use; 
constexpr float		SCL_FLOAT_SNAN			() noexcept { return FloatUnion{ 0x7FA00000u }.f; }	// signaling NaN: raises exception when touched
constexpr double	SCL_DOUBLE_INFINITY		() noexcept { return DoubleUnion{ 0x7FF0000000000000uLL }.f; }
constexpr double	SCL_DOUBLE_NEG_INFINITY	() noexcept { return -SCL_DOUBLE_INFINITY(); }
constexpr double	SCL_DOUBLE_QNAN			() noexcept { return DoubleUnion{ 0x7FF8000000000000uLL }.f; }	// quiet NaN: silent on use; 
constexpr double	SCL_DOUBLE_SNAN			() noexcept { return DoubleUnion{ 0x7FF4000000000000uLL }.f; }	// signaling NaN: raises exception when touched

// Helper functions for special float values (no std library dependency)
inline bool is_nan(float x) 
{
	// NaN has the property that it's not equal to itself
	return x != x;
}

inline bool is_inf(float x)
{
	// Use bit manipulation to detect infinity
	FloatUnion u;
	u.f = x;
	// Infinity: exponent = 0x7F800000 (all 1s in exponent), mantissa = 0
	return (u.u & 0x7F800000) == 0x7F800000 && (u.u & 0x007FFFFF) == 0;
}

inline bool is_nan(double x)
{
	return x != x;
}

inline bool is_inf(double x)
{
	DoubleUnion u;
	u.f = x;
	// Infinity: exponent = 0x7FF0000000000000 (all 1s in exponent), mantissa = 0
	return (u.u & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL && (u.u & 0x000FFFFFFFFFFFFFULL) == 0;
}


//求绝对值
template<typename T> 
inline T absolute(T x)
{
	return x > 0 ? x : -x;
}

//交换两个值
#ifdef SCL_WIN //windows下面的<utility>文件写的有问题，导致对swap的调用会歧义
template<typename T> 
inline void swap(T& a, T& b)
{
	T t = a;
	a = b;
	b = t;
}
#else
template<typename T> 
inline void swap(T& a, T& b)
{
	T t = a;
	a = b;
	b = t;
}
#endif

//角度转为弧度
inline float radian(const float a)
{
	return PI * a / 180;
}

//弧度转为角度
inline float angle(const float r)
{
	return 180 * r / PI;
}

//求较大值
#ifdef max
#undef max
#endif
template<typename T> 
inline T max(const T a, const T b)
{
	return a > b ? a : b;
}

#ifdef min 
#undef min 
#endif
template<typename T> 
inline T min(const T a, const T b)
{
	return a < b ? a : b;
}

#ifdef clamp
undef clamp
#endif
template<typename T> 
inline T clamp(const T v, const T _min, const T _max)
{
	if (v < _min)
		return _min;
	else if (v > _max)
		return _max;
	return v;
}


//判断浮点数是否相等
inline bool float_equal(const float a, const float b, const float precision = MATH_FLOAT_EPSILON)
{
	assert(precision < 0.1f);
	
	// Handle special cases first
	// If both are NaN, consider them equal
	if (is_nan(a) && is_nan(b))
		return true;
	
	// If only one is NaN, they are not equal
	if (is_nan(a) || is_nan(b))
		return false;
	
	// If both are infinity with same sign, they are equal
	if (is_inf(a) && is_inf(b))
		return (a > 0) == (b > 0);  // Same sign check
	
	// If only one is infinity, they are not equal
	if (is_inf(a) || is_inf(b))
		return false;
	
	// Normal case: use precision comparison
	return absolute(a - b) <= precision;
}

//判断浮点数是否为0
inline bool float_is_zero(const float a, const float precision = MATH_FLOAT_EPSILON)
{
	return float_equal(a, 0, precision);
}

//判断双精度浮点数是否相等
inline bool double_equal(const double a, const double b, const double precision = MATH_DOUBLE_EPSILON)
{
	assert(precision < 0.1);
	
	// Handle special cases first
	// If both are NaN, consider them equal
	if (is_nan(a) && is_nan(b))
		return true;
	
	// If only one is NaN, they are not equal
	if (is_nan(a) || is_nan(b))
		return false;
	
	// If both are infinity with same sign, they are equal
	if (is_inf(a) && is_inf(b))
		return (a > 0) == (b > 0);  // Same sign check
	
	// If only one is infinity, they are not equal
	if (is_inf(a) || is_inf(b))
		return false;
	
	// Normal case: use precision comparison
	return absolute(a - b) <= precision;
}


//浮点数大于
inline bool float_greater(const float a, const float b, const float precision = MATH_FLOAT_EPSILON)
{
	return a - b > precision;
}

//浮点数小于
inline bool float_smaller(const float a, const float b, const float precision = MATH_FLOAT_EPSILON)
{
	return b - a > precision;
}

//圆整浮点数为整数
template <typename T>
inline int round(const T f)
{
	return f > 0 ? static_cast<int>(f + 0.5f) : -static_cast<int>(-f + 0.5f);
}

//判断一个int是否为素数
bool is_prime(uint n);

//找出小于n的最大素数
uint min_prime(uint n);

//求base的times次方
int pow(int base, int times);

// Computes a fully accurate inverse square root
float inv_sqrt(float f);


//对2π求模
inline double mod2PI(const double angel)
{
	double r = angel - static_cast<int>(angel / PI2) * PI2;
	return r;
}

//线性插值
float	lerpf	(float from, float to, float t);
int		lerp	(int from, int to, float t);

template <typename T>
inline bool is_mul_overflow(const int v1, const T v2)
{
	if (v1 == 0 || v1 == 1)
		return false;
	double max_v2 = scl::MAX_INT / static_cast<double>(v1);
	return v2 >= max_v2;
}

inline bool is_add_overflow(const int v1, const int v2)
{
	if (v1 == 0 || v2 == 0)
		return false;
	int64 t1 = static_cast<int64>(v1) + v2;
	return t1 > scl::MAX_INT || t1 < scl::MIN_INT;
}

inline bool is_add_overflow(const int64 v1, const int64 v2)
{
	if (v1 == 0 || v2 == 0)
		return false;
	if (v1 < 0 && v2 > 0)
		return false;
	if (v1 > 0 && v2 < 0)
		return false;
	int64 left = scl::MAX_INT64 - absolute(v1);
	if (absolute(v2) > left)
		return true;
	return false;
}

template <typename T>
inline int safe_mul(const int v1, const T v2)
{
	if (is_mul_overflow(v1, v2))
		return scl::MAX_INT;
	else
		return static_cast<int>(v1 * v2);
}

template <typename T>
int binary_search(const T* array, const int from, const int to, const T& key) 
{
	if (from > to)
		return -1;

	int		left	= from;
	int		right	= to;
	int		mid		= -1;
	while (left <= right)
	{
		mid = (left + right) / 2; // mid = left + (right - left) / 2 ==>  mid = (left + right) / 2
		const T& cur = array[mid];
		if (key < cur)
			right = mid - 1;
		else if (cur < key)
			left = mid + 1;
		else
			break;
	}
	return mid;
}

template <typename T, typename CompareT>
int binary_search(const T* array, const int from, const int to, const T& key, CompareT compare) 
{
	if (from > to)
		return -1;

	int		left	= from;
	int		right	= to;
	int		mid		= -1;
	while (left <= right)
	{
		mid = (left + right) / 2; // mid = left + (right - left) / 2 ==>  mid = (left + right) / 2
		const T& cur = array[mid];
		if (compare(key, cur))
			right = mid - 1;
		else if (compare(cur, key))
			left = mid + 1;
		else
			break;
	}
	return mid;
}

template <typename T>
bool array_find(const T* array, const int arraySize, const T& key)
{
	if (NULL == array)
		return false;
	if (0 == arraySize)
		return false;
	for (int i = 0; i < arraySize; ++i)
	{
		if (array[i] != key)
			continue;
		return true;
	}
	return false;
}

class rander
{
public:
	static const unsigned int MAX16 = 0x7fff;
	static const unsigned int MAX32 = 0x7fffffff;
	rander();

	void			srand(const int seed);

	int				rand	(const int min, const int max);
	unsigned int	rand16	();		//rand 16 bit number, [0 ~ 32767] (0x7fff)
	unsigned int	rand32	();		//rand 32 bit number, [0 ~ 2147483647] (0x7fffffff)

	bool			rand_percent(const float percent);	//rand percent, return is success;

	unsigned int	seed	() {return m_seed;};
	void			set_seed(unsigned int seed) { m_seed = seed; }

private:
	unsigned int m_seed;
};

////////////////////////////////////
//全局rand函数，线程不安全
////////////////////////////////////
unsigned int rand	(const int min, const int max); //random in [min, max]
void		 srand	(int);							//设置全局随机种子 
unsigned int rand16	();								//random 16 bit number, [0 ~ 32767] (0x7fff)
unsigned int rand32	();								//random 32 bit number, [0 ~ 2147483647] (0x7fffffff)

} //namespace scl
