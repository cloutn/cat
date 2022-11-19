
#include "./util.h"

#include "scl/math.h"
#include "scl/matrix.h"


//#include <glm/vec3.hpp> // glm::vec3
//#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
//#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
//#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
//#include <glm/ext/scalar_constants.hpp> // glm::pi
//#include <glm/gtx/euler_angles.hpp>
//#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtx/quaternion.hpp>

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

void print(const scl::matrix& m)
{
	_print(m.ptr());
}

void print(const glm::mat4& m)
{
	_print(glm::value_ptr(m));
}

bool mat_equal(const float* m1, const float* m2)
{
	for (int i = 0; i < 16; ++i)
	{
		if (!scl::float_equal(m1[i], m2[i]))
			return false;
	}
	return true;
}

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

} // namespace test 


