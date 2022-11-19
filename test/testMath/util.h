#pragma once

#include <glm/mat4x4.hpp> // glm::mat4

namespace scl { 
class matrix; 
}

namespace test {

void	print			(const glm::mat4& m);
void	print			(const scl::matrix& m);
bool	mat_equal		(const float* m1, const float* m2);
bool	compare_mat		(const scl::matrix& mat1, const glm::mat4& mat2, bool needPrint = true);	//return if euqal
bool	compare_mat		(const scl::matrix& mat1, const scl::matrix& mat2, bool needPrint = true);

} // namespace test


