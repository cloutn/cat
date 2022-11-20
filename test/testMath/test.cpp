#include "./test.h"

#include "./util.h"

#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/vector.h"
#include "scl/assert.h"

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <glm/ext/matrix_transform.hpp>		// glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp>	// glm::perspective
#include <glm/ext/scalar_constants.hpp>		// glm::pi
#include <glm/ext/matrix_clip_space.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <glm/gtc/type_ptr.hpp>


namespace test {

using scl::matrix;
using scl::quaternion;
using scl::vector3;

void test_rotate(bool print)
{
	float a[3] = { 33.f, 66.f, 88.f};

	glm::vec3	gEuler		= glm::radians(glm::vec3({ a[0], a[1], a[2] }));
	glm::mat4	gRotateX	= glm::eulerAngleX(gEuler.x);
	glm::mat4	gRotateY	= glm::eulerAngleY(gEuler.y);
	glm::mat4	gRotateZ	= glm::eulerAngleZ(gEuler.z);
	glm::mat4	gRotate		= gRotateZ * gRotateY * gRotateX;

	matrix		sRotateX	= matrix::rotate_x(a[0]);
	matrix		sRotateY	= matrix::rotate_y(a[1]);
	matrix		sRotateZ	= matrix::rotate_z(a[2]);
	matrix		sRotate		= sRotateX * sRotateY * sRotateZ;

	assert(compare_mat(sRotate, gRotate, print));

	quaternion q; 
	q.from_euler_angle(a[0], a[1], a[2]);
	matrix qmat;
	q.to_matrix(qmat);

	assert(compare_mat(qmat, sRotate ,print));
}

void test_matrix_mul(bool print)
{
	vector3 angle = { 22, 55, 77};
	matrix m1 = matrix::rotate_x(angle.x);
	matrix m2 = matrix::rotate_x(angle.y);
	matrix m3 = matrix::rotate_x(angle.z);

	matrix all1 = m1 * m2 * m3;
	matrix all2 = matrix::mul_all(m1, m2, m3);

	assert(compare_mat(all1, all2, print));
}

void test_camera(bool print)
{
	scl::matrix sLookAt = matrix::lookat(1, 1, 1, 0, 0, 0, 0, 1, 0);
	glm::mat4	gLookAt = glm::lookAtRH(glm::vec3{1, 1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
	assert(compare_mat(sLookAt, gLookAt, print));

	float w = 1280;
	float h = 800;
	scl::matrix sProjection = matrix::perspective(45, w/h, 0.1f, 100);
	glm::mat4	gProjection = glm::perspective(glm::radians(45.0f), w/h, 0.1f, 100.0f);
	assert(compare_mat(sProjection, gProjection, print));

	matrix		sOrtho = matrix::ortho(-w/2, w/2, -h/2, h/2, -10000, 10000);
	glm::mat4	gOrtho = glm::orthoRH(-w/2, w/2, -h/2, h/2, -10000.0f, 10000.0f);
	assert(compare_mat(sOrtho, gOrtho, print));

	//scl::matrix::frustum(m_projection, -w/2, w/2, -h/2, h/2, -100, 100);
}

void test_rotate_order(bool print)
{
	//float a[] = { 37, 97, 87 };
	float a[] = { 33, 88, 66 };

	glm::vec3	gRadian		= glm::radians(glm::vec3{a[0], a[1], a[2]});
	glm::mat4	gXYZ		= glm::eulerAngleZYX(gRadian.z, gRadian.y, gRadian.x);
	glm::mat4	gXZY		= glm::eulerAngleYZX(gRadian.y, gRadian.z, gRadian.x);
	glm::mat4	gYXZ		= glm::eulerAngleZXY(gRadian.z, gRadian.x, gRadian.y);
	glm::mat4	gYZX		= glm::eulerAngleXZY(gRadian.x, gRadian.z, gRadian.y);
	glm::mat4	gZXY		= glm::eulerAngleYXZ(gRadian.y, gRadian.x, gRadian.z);
	glm::mat4	gZYX		= glm::eulerAngleXYZ(gRadian.x, gRadian.y, gRadian.z);

	matrix		sXYZ		= matrix::rotate_xyz(a[0], a[1], a[2]);
	matrix		sXZY		= matrix::rotate_xzy(a[0], a[1], a[2]);
	matrix		sYXZ		= matrix::rotate_yxz(a[0], a[1], a[2]);
	matrix		sYZX		= matrix::rotate_yzx(a[0], a[1], a[2]);
	matrix		sZXY		= matrix::rotate_zxy(a[0], a[1], a[2]);
	matrix		sZYX		= matrix::rotate_zyx(a[0], a[1], a[2]);

	assert(compare_mat(sXYZ, gXYZ, print));
	assert(compare_mat(sXZY, gXZY, print));
	assert(compare_mat(sYXZ, gYXZ, print));
	assert(compare_mat(sYZX, gYZX, print));
	assert(compare_mat(sZXY, gZXY, print));
	assert(compare_mat(sZYX, gZYX, print));
}



void test_decompose(bool print)
{
	float s[]	= { 1,	2,	3	};
	float r[]	= { 33, 88, 66	};
	float t[]	= { 10, 20, 30	};

	glm::vec3	gEuler		= glm::radians		(glm::vec3{r[0], r[1], r[2]});
	glm::mat4	gScale		= glm::scale		(glm::vec3(s[0], s[1], s[2]));
	glm::mat4	gRotate		= glm::eulerAngleZYX(gEuler.z, gEuler.y, gEuler.x);
	glm::mat4	gTranslate	= glm::translate	(glm::vec3(t[0], t[1], t[2]));
	glm::mat4	gAll		= gTranslate * gRotate * gScale;

	matrix		sScale		= matrix::scale		(s[0], s[1], s[2]);
	matrix		sRotate		= matrix::rotate_xyz(r[0], r[1], r[2]);
	matrix		sTranslate	= matrix::translate	(t[0], t[1], t[2]);
	matrix		sAll		= matrix::mul_all	(sScale, sRotate, sTranslate);

	assert(compare_mat(sScale,		gScale,		print));
	assert(compare_mat(sRotate,		gRotate,	print));
	assert(compare_mat(sTranslate,	gTranslate, print));
	assert(compare_mat(sAll,		gAll,		print));

	vector3	sEulerDecompose2 = { 0 };
	vector3 sEuler	= { r[0], r[1], r[2] };
	matrix::decompose_rotation_xyz(sRotate, sEulerDecompose2);
	assert(sEuler == sEulerDecompose2);

	vector3	sTranslateDecompose;
	vector3 sScaleDecompose;
	vector3 sEulerDecompose;
	matrix	sMatRotateDecompose;
	matrix::decompose(sAll, &sTranslateDecompose, &sScaleDecompose, &sEulerDecompose, &sMatRotateDecompose); 
	assert(sEuler == sEulerDecompose);
	assert(sTranslateDecompose == vector3({t[0], t[1], t[2]}));
	assert(sScaleDecompose == vector3({s[0], s[1], s[2]}));
	assert(compare_mat(sMatRotateDecompose, sRotate, print));

	glm::vec3 gScaleDecompose;
	glm::quat gRotateDecompose;
	glm::vec3 gTranslateDecompose;
	glm::vec3 gSkewDecompose;
	glm::vec4 gPerspectiveDecompose;
	glm::decompose(gAll, gScaleDecompose, gRotateDecompose, gTranslateDecompose, gSkewDecompose, gPerspectiveDecompose);
	glm::mat4 gMatRotateDecompose = glm::toMat4(gRotateDecompose);
	assert(compare_mat(sMatRotateDecompose, gMatRotateDecompose, print));
}


} // namespace test



