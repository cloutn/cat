
#include "./util.h"

#include "scl/math.h"
#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/vector.h"

#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

namespace test {

void _print(const float* m)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			printf("%.2f\t", m[i * 4 + j]);
		printf("\n");
	}
}

bool float_equal(const float a, const float b)
{
	return scl::float_equal(a, b, 1e-5);
}

//void _print_float4(const float* m)
//{
//	for (int i = 0; i < 4; ++i)
//	{
//		printf("%.2\t", m[i]);
//	}
//	printf("\n");
//}

void print(const scl::matrix& m)
{
	_print(m.ptr());
}

void print(const glm::mat4& m)
{
	_print(glm::value_ptr(m));
}

void print(const glm::quat& q)
{
	printf("xyzw = %.3f\t%.3f\t%.3f\t%.3f\n", q.x, q.y, q.z, q.w);
}

void print(const scl::quaternion& q)
{
	printf("xyzw = %.3f\t%.3f\t%.3f\t%.3f\n", q.x, q.y, q.z, q.w);
}

void print(const glm::vec3& v)
{
	printf("xyz = %.3f\t%.3f\t%.3f\n", v.x, v.y, v.z);
}

void print(const scl::vector3& v)
{
	printf("xyz = %.3f\t%.3f\t%.3f\n", v.x, v.y, v.z);
}

bool mat_equal(const float* m1, const float* m2)
{
	for (int i = 0; i < 16; ++i)
	{
		if (!test::float_equal(m1[i], m2[i]))
			return false;
	}
	return true;
}

bool quat_equal(const glm::quat& q1, const scl::quaternion& q2)
{
	if (!test::float_equal(q1.x, q2.x))
		return false;
	if (!test::float_equal(q1.y, q2.y))
		return false;
	if (!test::float_equal(q1.z, q2.z))
		return false;
	if (!test::float_equal(q1.w, q2.w))
		return false;
	return true;
}

bool vector3_equal(const glm::vec3& q1, const scl::vector3& q2)
{
	if (!test::float_equal(q1.x, q2.x))
		return false;
	if (!test::float_equal(q1.y, q2.y))
		return false;
	if (!test::float_equal(q1.z, q2.z))
		return false;
	return true;
}

//bool float_equal4(const float* f1, const float* f2)
//{
//	for (int i = 0; i < 4; ++i)
//	{
//		if (!test::float_equal(f1[i], f2[i]))
//			return false;
//	}
//	return true;
//}

bool compare_mat(const scl::matrix& mat1, const glm::mat4& mat2, bool needPrint)
{
	bool equal = mat_equal(mat1.ptr(), glm::value_ptr(mat2));
	if (needPrint)
	{
		printf("scl mat1: \n");
		print(mat1);
		printf("------------------\n");
		printf("glm mat2: \n");
		print(mat2);
		printf("------------------\n");
		printf("%s\n", equal ? "Equal." : "NOT equal.");
	}
	return equal;
}

bool compare_mat(const scl::matrix& mat1, const scl::matrix& mat2, bool needPrint)
{
	bool equal = mat_equal(mat1.ptr(), mat2.ptr());
	if (needPrint)
	{
		printf("scl mat1: \n");
		print(mat1);
		printf("------------------\n");
		printf("scl mat2: \n");
		print(mat2);
		printf("------------------\n");
		printf("%s\n", equal ? "Equal." : "NOT equal.");
	}
	return equal;
}

bool compare_quat(const glm::quat& quat1, const scl::quaternion& quat2, bool needPrint)
{
	bool equal = quat_equal(quat1, quat2);
	if (needPrint)
	{
		printf("glm quat: \n");
		print(quat1);
		printf("------------------\n");
		printf("scl quat: \n");
		print(quat2);
		printf("------------------\n");
		printf("%s\n", equal ? "Equal." : "NOT equal.");
	}
	return equal;
}

bool compare_vector3(const glm::vec3& vec1, const scl::vector3& vec2, bool needPrint)
{
	bool equal = vector3_equal(vec1, vec2);
	if (needPrint)
	{
		printf("glm vec3: \n");
		print(vec1);
		printf("------------------\n");
		printf("scl vec3: \n");
		print(vec2);
		printf("------------------\n");
		printf("%s\n", equal ? "Equal." : "NOT equal.");
	}
	return equal;
}

bool float3_euqal(const float* v1, const float* v2)
{
	for (int i = 0; i < 3; ++i)
		if (!test::float_equal(v1[i], v2[i]))
			return false;
	return true;
}

bool float4_euqal(const float* v1, const float* v2)
{
	for (int i = 0; i < 4; ++i)
		if (!test::float_equal(v1[i], v2[i]))
			return false;
	return true;
}

glm::vec3 glm_vector(float* f3)
{
	return { f3[0], f3[1], f3[2] };
}

scl::vector3 scl_vector(float* v)
{
	return { v[0], v[1], v[2] };
}

} // namespace test 


