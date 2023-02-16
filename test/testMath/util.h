#pragma once

#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/quaternion.hpp>

namespace scl { 

class matrix; 
class quaternion;
class vector3;

}

namespace test {

void	print			(const glm::mat4& m);
void	print			(const scl::matrix& m);
void	print			(const glm::quat& q);
void	print			(const scl::quaternion& q);
void	print			(const glm::vec3& v);
void	print			(const scl::vector3& v);
bool	mat_equal		(const float* m1, const float* m2);
bool	compare_mat		(const scl::matrix& mat1, const glm::mat4& mat2, bool needPrint = true);	//return if euqal
bool	compare_mat		(const scl::matrix& mat1, const scl::matrix& mat2, bool needPrint = true);
bool	compare_quat	(const glm::quat& quat1, const scl::quaternion& quat2, bool needPrint = true);
bool	compare_vector3	(const glm::vec3& vec1, const scl::vector3& vec2, bool needPrint = true);
bool	float3_euqal	(const float* v1, const float* v2);

} // namespace test


