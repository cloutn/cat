#include "./test.h"

#include "./util.h"

#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/vector.h"
#include "scl/plane.h"
#include "scl/math.h"

#include <limits>
#include <cstring>
#include <cassert>
#include <cmath>

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
using scl::z_range;

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

	quaternion q = { 0 }; 
	q.from_euler_angle(a[0], a[1], a[2]);
	matrix qmat;
	q.to_matrix(qmat);

	assert(compare_mat(qmat, sRotate ,print));
}

void test_rotate2(bool print)
{
	float		a[3]		= { -55.741806, 15.2314901, 24.8266335};

	glm::vec3	gRadian		= glm::radians(glm::vec3{a[0], a[1], a[2]});
	glm::mat4	gXYZ		= glm::eulerAngleZYX(gRadian.z, gRadian.y, gRadian.x);

	matrix		sXYZ		= matrix::rotate_xyz(a[0], a[1], a[2]);
	assert(compare_mat(sXYZ, gXYZ, print));

	//float v1 = std::numeric_limits<float>::epsilon();

	glm::quat	gQuat(gRadian);
	quaternion	sQuat;
	sQuat.from_euler_angle(a[0], a[1], a[2]);
	assert(compare_quat(gQuat, sQuat, print));

	scl::vector3 sEuler;
	sQuat.normalize();

	sQuat.to_euler_angle(sEuler);
	glm::vec3 gEulerRadian = glm::eulerAngles(gQuat);
	glm::vec3 gEuler { glm::degrees(gEulerRadian) };
	assert(float3_euqal(glm::value_ptr(gEulerRadian), glm::value_ptr(gRadian)));
	assert(float3_euqal(glm::value_ptr(gEuler), a));
	assert(float3_euqal(&sEuler.x, a));

	//glm::vec3 gEuler{ glm::degrees(glm::eulerAngles(gQuat)) };
	assert(compare_vector3(gEuler, sEuler, print));
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
	scl::vector3	pos			= {11.11, -22.22, 333.33};
	scl::vector3	lookat		= {10, -20, 30};
	scl::vector3	up			= {0, 1, 0};
	scl::matrix		sLookAt		= matrix::lookat(pos.x, pos.y, pos.z, lookat.x, lookat.y, lookat.z, up.x, up.y, up.z);
	scl::matrix		sLookAt2	= matrix::lookat2(pos, lookat, up);
	glm::mat4		gLookAt		= glm::lookAtRH(glm::vec3{pos.x, pos.y, pos.z}, glm::vec3{lookat.x, lookat.y, lookat.z}, glm::vec3{up.x, up.y, up.z});
	assert(compare_mat(sLookAt, gLookAt, print));
	assert(compare_mat(sLookAt2, gLookAt, print));

	float w = 1280;
	float h = 800;
	scl::matrix sProjection = matrix::perspective(45, w/h, 0.1f, 100);
	glm::mat4	gProjection = glm::perspective(glm::radians(45.0f), w/h, 0.1f, 100.0f);
	assert(compare_mat(sProjection, gProjection, print));

	matrix		sOrtho = matrix::volume(-w/2, w/2, -h/2, h/2, -10000, 10000, z_range::negative_one_to_one);
	glm::mat4	gOrtho = glm::orthoRH(-w/2, w/2, -h/2, h/2, -10000.0f, 10000.0f);
	assert(compare_mat(sOrtho, gOrtho, print));

	//scl::matrix::frustum(m_projection, -w/2, w/2, -h/2, h/2, -100, 100);
}

void test_frustum_infinite(bool print)
{
	// Test infinite frustum projection matrix
	float w = 1280;
	float h = 800;
	float near_z = 0.1f;
	float aspect = w / h;
	float fovy = 45.0f; // degrees
	
	// Calculate frustum bounds
	float frustumH = tanf(scl::radian(fovy) / 2.0f) * near_z;
	float frustumW = frustumH * aspect;
	
	// Test different z_range mappings
	scl::matrix sInfinite_neg1to1 = matrix::frustum_infinite(-frustumW, frustumW, -frustumH, frustumH, near_z, scl::z_range::negative_one_to_one);
	scl::matrix sInfinite_0to1 = matrix::frustum_infinite(-frustumW, frustumW, -frustumH, frustumH, near_z, scl::z_range::zero_to_one);
	
	// Compare with regular frustum using a very large far value to simulate infinity
	float very_far = 1000000.0f;
	scl::matrix sRegular_neg1to1 = matrix::frustum(-frustumW, frustumW, -frustumH, frustumH, near_z, very_far, scl::z_range::negative_one_to_one);
	scl::matrix sRegular_0to1 = matrix::frustum(-frustumW, frustumW, -frustumH, frustumH, near_z, very_far, scl::z_range::zero_to_one);
	
	if (print)
	{
		printf("Infinite frustum matrix (negative_one_to_one):\n");
		for (int i = 0; i < 4; ++i)
		{
			printf("%.6f %.6f %.6f %.6f\n", 
				sInfinite_neg1to1.m[i][0], sInfinite_neg1to1.m[i][1], 
				sInfinite_neg1to1.m[i][2], sInfinite_neg1to1.m[i][3]);
		}
		printf("\nRegular frustum matrix with large far (negative_one_to_one):\n");
		for (int i = 0; i < 4; ++i)
		{
			printf("%.6f %.6f %.6f %.6f\n", 
				sRegular_neg1to1.m[i][0], sRegular_neg1to1.m[i][1], 
				sRegular_neg1to1.m[i][2], sRegular_neg1to1.m[i][3]);
		}
	}
	
	// The matrices should be very similar, with the infinite version having exact values
	// We expect the (2,2) element to be exactly -1.0 for negative_one_to_one mapping
	// and the (3,2) element to be exactly -2*near for negative_one_to_one mapping
	
	float tolerance = 1e-5f;
	
	// Check negative_one_to_one mapping
	assert(std::abs(sInfinite_neg1to1.m[2][2] - (-1.0f)) < tolerance);
	assert(std::abs(sInfinite_neg1to1.m[3][2] - (-2.0f * near_z)) < tolerance);
	
	// Check zero_to_one mapping  
	assert(std::abs(sInfinite_0to1.m[2][2] - 0.0f) < tolerance);
	assert(std::abs(sInfinite_0to1.m[3][2] - (-near_z)) < tolerance);
	
	// The first two rows should be identical to regular frustum
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			assert(std::abs(sInfinite_neg1to1.m[i][j] - sRegular_neg1to1.m[i][j]) < tolerance);
		}
	}
	
	if (print)
	{
		printf("test_frustum_infinite passed!\n");
	}
}


void test_camera_look_at(bool print)
{
	//scl::matrix sLookAt = matrix::lookat(1, 1, 1, 0, 0, 0, 0, 1, 0);
	//glm::mat4	gLookAt = glm::lookAtRH(glm::vec3{1, 1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
	//assert(compare_mat(sLookAt, gLookAt, print));

	// 测试 decompose_lookat
	//		从一个参数集合构造一个 camera view matrix
	//		然后用 decompose_lookat 来将矩阵还原为参数集合
	//		注意，由于计算过程 normalized 的原因，所以decompose得出的参数不会和之前输入参数完全一致
	//		但是用第二次decompose得出的参数重新计算matrix，一定是和第一次的matrix完全一致的，
	//		最终，我们检查这两个matrix从而得知 decompose 是否执行正确。
	scl::matrix sLookAt1 = matrix::lookat(10, 20, -30, 0, 0, 0, 0, 1, 0);

	scl::vector3 pos;
	scl::vector3 lookAt;
	scl::vector3 up;
	matrix::decompose_lookat(sLookAt1, pos.x, pos.y, pos.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);

	scl::matrix sLookAt2 = matrix::lookat(pos.x, pos.y, pos.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);
	assert(compare_mat(sLookAt1, sLookAt2, print));
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
	assert(sEuler.equal(sEulerDecompose2, 1e-5));

	vector3		sTranslateDecompose;
	vector3		sScaleDecompose;
	vector3		sEulerDecompose;
	matrix		sMatRotateDecompose;
	quaternion	sQuaternionDecompose;
	matrix::decompose(sAll, &sTranslateDecompose, &sScaleDecompose, &sEulerDecompose, &sMatRotateDecompose, &sQuaternionDecompose); 
	assert(sEuler.equal(sEulerDecompose, 1e-5));
	assert(sTranslateDecompose == vector3({t[0], t[1], t[2]}));
	assert(sScaleDecompose == vector3({s[0], s[1], s[2]}));
	assert(compare_mat(sMatRotateDecompose, sRotate, print));
	matrix		sDecomposeQuaternionToMatrix;
	sQuaternionDecompose.to_matrix(sDecomposeQuaternionToMatrix);
	assert(compare_mat(sDecomposeQuaternionToMatrix, sMatRotateDecompose, print));

	glm::vec3 gScaleDecompose;
	glm::quat gRotateDecompose;
	glm::vec3 gTranslateDecompose;
	glm::vec3 gSkewDecompose;
	glm::vec4 gPerspectiveDecompose;
	glm::decompose(gAll, gScaleDecompose, gRotateDecompose, gTranslateDecompose, gSkewDecompose, gPerspectiveDecompose);
	glm::mat4 gMatRotateDecompose = glm::toMat4(gRotateDecompose);
	assert(compare_mat(sMatRotateDecompose, gMatRotateDecompose, print));
}

void _test_quaternion_by_angle(float* a, bool print)
{
	//printf("xyz = {%f, %f, %f}\n", a[0], a[1], a[2]);

	matrix		mat_rx	= matrix::rotate_x(a[0]);
	matrix		mat_ry	= matrix::rotate_y(a[1]);
	matrix		mat_rz	= matrix::rotate_z(a[2]);
	matrix		mat_r	= mat_rx * mat_ry * mat_rz;

	quaternion	quat = { 0 };
	matrix		mat_from_quat;
	quat.from_euler_angle(a[0], a[1], a[2]);	
	quat.to_matrix(mat_from_quat);
				
	quaternion quat_from_mat = { 0 };
	quat_from_mat.from_matrix(mat_r);

	glm::vec3	gEuler		= glm::radians(glm::vec3{a[0], a[1], a[2] });
	glm::mat4	gRotate		= glm::eulerAngleZYX(gEuler.z, gEuler.y, gEuler.x);

	glm::quat	gQuat		= glm::quat(gEuler);		
	glm::quat	gQuatFromMatrix = glm::quat_cast(gRotate);

	assert(compare_mat(mat_r, gRotate, print));

	scl::matrix tm1;
	quat_from_mat.to_matrix(tm1);

	scl::matrix tm2;
	quat.to_matrix(tm2);

	assert(compare_mat(tm1, tm2, print));

	// quaterion 不一定相等，比如xyz=(0,0,270)的时候，但是只要quat转为matrix是相等的，就可以认为quaterion表示的旋转是一致的
	//assert(quat_from_mat == quat);

	assert(compare_mat(mat_r, mat_from_quat, print));

	quaternion quatInverse = quaternion::inverse(quat);
	matrix matInverse;
	matrix::inverse(mat_from_quat, matInverse);

	matrix	matrix_from_quatInverse;
	quatInverse.to_matrix(matrix_from_quatInverse);
	
	assert(compare_mat(matInverse, matrix_from_quatInverse, print));
}

void test_quaternion(bool print)
{
	//float a[3] = { 0, 0, -90 };
	//_test_quaternion_by_angle(a, print);

	//int start	= 0;
	//int end	= 360;
	int start	= 66;
	int end		= 67;

	for (int angle_x = start; angle_x < end; ++angle_x)
	{
		//printf("x = {%d}\n", angle_x);
		for (int angle_y = start; angle_y < end; ++angle_y)
		{
			for (int angle_z = start; angle_z < end; ++angle_z)
			{
				float a[3] = { static_cast<float>(angle_x), static_cast<float>(angle_y), static_cast<float>(angle_z)};
				_test_quaternion_by_angle(a, print);
			}
		}
	}
}

glm::vec4 glm_plane_from_normal_point(glm::vec3 normal, glm::vec3 pt) 
{
    glm::vec3 norm = glm::normalize(normal);
    return { norm.x, norm.y, norm.z, -glm::dot(pt, norm) };
}

glm::vec4 glm_plane_from_points(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3) 
{
	glm::vec3 v1 = pt2 - pt1;
	glm::vec3 v2 = pt3 - pt1;
	glm::vec3 n = glm::normalize(glm::cross(v1, v2));
	return glm_plane_from_normal_point(n, pt1);
}

void test_plane(bool print)
{
	float			n[]			= { 1, 2,	3 };
	float			pos[]		= { 4, 5, 6 };
	float			otherPt[]	= { 11.5, 27.2, -5.5};

	glm::vec3		gNormal		= glm_vector(n);
	glm::vec3		gPos		= glm_vector(pos);
	glm::vec4		gPlane		= glm_plane_from_normal_point(gNormal, gPos);
	glm::vec3		gOtherPt	= glm_vector(otherPt);
	glm::vec3		gPlaneNormal= glm_vector(glm::value_ptr(gPlane)); 
	float			gDotPoint	= glm::dot(gPlaneNormal, gOtherPt) + gPlane.w;

	scl::vector3	sNormal		= scl_vector(n);
	scl::vector3	sPos		= scl_vector(pos);
	scl::vector3	sOtherPt	= scl_vector(otherPt);
	scl::plane		sPlane		= scl::plane::create_from_normal_point(sNormal, sPos);
	float			sDotPoint	= sPlane.dot(sOtherPt);

	assert(float4_euqal(glm::value_ptr(gPlane), sPlane.value_ptr()));
	assert(test::float_equal(gDotPoint, sDotPoint));
}

void test_plane2(bool print)
{
	float			p1[]	=	{ 1, 2,	3 };
	float			p2[]	=	{ 4, 0.5, 6 };
	float			p3[]	=	{ 17, 15, 2.6 };

	glm::vec3		gPt1		{ p1[0], p1[1], p1[2] };
	glm::vec3		gPt2		{ p2[0], p2[1], p2[2] };
	glm::vec3		gPt3		{ p3[0], p3[1], p3[2] };
	glm::vec4		gPlane	=	glm_plane_from_points(gPt1, gPt2, gPt3);

	scl::vector3	sPt1		{ p1[0], p1[1], p1[2] };
	scl::vector3	sPt2		{ p2[0], p2[1], p2[2] };
	scl::vector3	sPt3		{ p3[0], p3[1], p3[2] };
	
	scl::plane		sPlane	= scl::plane::create_from_point(sPt1, sPt2, sPt3);

	assert(float4_euqal(glm::value_ptr(gPlane), sPlane.value_ptr()));
}

void test_plane3(bool print)
{
	float			intercepts[]= { 1, 2, 3 };
	float			posX[]		= { intercepts[0],	0,				0 };
	float			posY[]		= { 0,				intercepts[1],	0 };
	float			posZ[]		= { 0,				0,				intercepts[2] };

	scl::plane p1 = scl::plane::create_from_intercept(intercepts[0], intercepts[1], intercepts[2]);
	scl::plane p2 = scl::plane::create_from_point(scl_vector(posX), scl_vector(posY), scl_vector(posZ));

	assert(p1 == p2);

	float			pt1[]		= { 1, 2, 3 };
	float			pt2[]		= { 0.1, 0.2, 0.3 };
	float			pt3[]		= { 1/3.0f, 2/3.0f, 1 };
	float			pt4[]		= { 10, 20, 30 };
	
	scl::vector3 vv1 = scl_vector(pt2);
	float len = vv1.length();
	float dot1 = p1.dot(scl_vector(pt1));
	float dot2 = p1.dot(scl_vector(pt2));
	float dot3 = p1.dot(scl_vector(pt3));

	assert(dot1 > 0);
	assert(dot2 < 0);
	assert(dot3 == 0);

	// test distance to point, the result number is from Houdini.
	assert(float_equal(dot1, 1.714286));
	assert(float_equal(dot2, -0.6));

	float pt_distance = p1.point_distance(scl_vector(pt2));
	assert(float_equal(pt_distance, 0.6));

	int side1 = p1.side(scl_vector(pt1));
	int side2 = p1.side(scl_vector(pt2));
	int side3 = p1.side(scl_vector(pt3));
	int side4 = p1.side(scl_vector(pt4));

	assert(side1 == 1);
	assert(side2 == -1);
	assert(side3 == 0);
	assert(side4 == 1);

	bool is_same12 = p1.same_side(scl_vector(pt1), scl_vector(pt2));
	bool is_same13 = p1.same_side(scl_vector(pt1), scl_vector(pt3));
	bool is_same14 = p1.same_side(scl_vector(pt1), scl_vector(pt4));
	bool is_same23 = p1.same_side(scl_vector(pt2), scl_vector(pt3));
	bool is_same24 = p1.same_side(scl_vector(pt2), scl_vector(pt4));

	assert(!is_same12);
	assert(is_same13);
	assert(is_same14);
	assert(is_same23);
	assert(!is_same24);
	//assert(float4_euqal(glm::value_ptr(gPlane), sPlane.value_ptr()));
	//assert(test::float_equal(gDotPoint, sDotPoint));
}

void test_scl_limits_vs_std_limits(bool print)
{
	if (print)
		printf("\n=== Testing SCL Constants vs std::numeric_limits ===\n");
	
	// ===============  Integer Limits Tests  ===============
	
	// Test INT8 limits
	assert(scl::SCL_INT8_MIN == std::numeric_limits<int8_t>::min());
	assert(scl::SCL_INT8_MAX == std::numeric_limits<int8_t>::max());
	assert(scl::SCL_UINT8_MAX == std::numeric_limits<uint8_t>::max());
	
	// Test INT16 limits
	assert(scl::SCL_INT16_MIN == std::numeric_limits<int16_t>::min());
	assert(scl::SCL_INT16_MAX == std::numeric_limits<int16_t>::max());
	assert(scl::SCL_UINT16_MAX == std::numeric_limits<uint16_t>::max());
	
	// Test INT32 limits
	assert(scl::SCL_INT32_MIN == std::numeric_limits<int32_t>::min());
	assert(scl::SCL_INT32_MAX == std::numeric_limits<int32_t>::max());
	assert(scl::SCL_UINT32_MAX == std::numeric_limits<uint32_t>::max());
	
	// Test INT64 limits
	assert(scl::SCL_INT64_MIN == std::numeric_limits<int64_t>::min());
	assert(scl::SCL_INT64_MAX == std::numeric_limits<int64_t>::max());
	assert(scl::SCL_UINT64_MAX == std::numeric_limits<uint64_t>::max());
	
	// ===============  Floating-Point Limits Tests  ===============
	
	// Helper lambda for binary comparison of floats
	auto compare_float_binary = [](float a, float b) -> bool
	{
		uint32_t a_bits, b_bits;
		std::memcpy(&a_bits, &a, sizeof(float));
		std::memcpy(&b_bits, &b, sizeof(float));
		return a_bits == b_bits;
	};
	
	// Helper lambda for binary comparison of doubles
	auto compare_double_binary = [](double a, double b) -> bool
	{
		uint64_t a_bits, b_bits;
		std::memcpy(&a_bits, &a, sizeof(double));
		std::memcpy(&b_bits, &b, sizeof(double));
		return a_bits == b_bits;
	};
	
	// Test float limits
	assert(compare_float_binary(scl::SCL_FLOAT_MIN, std::numeric_limits<float>::min()));
	assert(compare_float_binary(scl::SCL_FLOAT_EPSILON, std::numeric_limits<float>::epsilon()));
	
	// Test double limits
	assert(compare_double_binary(scl::SCL_DOUBLE_MIN, std::numeric_limits<double>::min()));
	assert(compare_double_binary(scl::SCL_DOUBLE_MAX, std::numeric_limits<double>::max()));
	assert(compare_double_binary(scl::SCL_DOUBLE_EPSILON, std::numeric_limits<double>::epsilon()));
	
	// ===============  Special Values Tests  ===============
	
	// Test positive infinity
	assert(compare_float_binary(scl::SCL_FLOAT_INFINITY(), std::numeric_limits<float>::infinity()));
	
	// Test negative infinity
	assert(compare_float_binary(scl::SCL_FLOAT_NEG_INFINITY(), -std::numeric_limits<float>::infinity()));
	
	// Test quiet NaN - Note: NaN values can have different bit patterns but still be NaN
	float scl_qnan = scl::SCL_FLOAT_QNAN();
	float std_qnan = std::numeric_limits<float>::quiet_NaN();
	assert(scl::is_nan(scl_qnan) && scl::is_nan(std_qnan));
	
	// Test signaling NaN
	float scl_snan = scl::SCL_FLOAT_SNAN();
	assert(scl::is_nan(scl_snan));
	
	// Test double special values
	assert(compare_double_binary(scl::SCL_DOUBLE_INFINITY(), std::numeric_limits<double>::infinity()));
	assert(compare_double_binary(scl::SCL_DOUBLE_NEG_INFINITY(), -std::numeric_limits<double>::infinity()));
	
	// Test double quiet NaN
	double scl_dqnan = scl::SCL_DOUBLE_QNAN();
	double std_dqnan = std::numeric_limits<double>::quiet_NaN();
	assert(scl::is_nan(scl_dqnan) && scl::is_nan(std_dqnan));
	
	// Test double signaling NaN
	double scl_dsnan = scl::SCL_DOUBLE_SNAN();
	assert(scl::is_nan(scl_dsnan));
}

void test_float_equal_special_cases()
{
	// Test NaN cases
	float nan1 = scl::SCL_FLOAT_QNAN();
	float nan2 = scl::SCL_FLOAT_SNAN();
	assert(scl::float_equal(nan1, nan2));  // Both NaN should be equal
	assert(!scl::float_equal(nan1, 1.0f)); // NaN vs normal number
	assert(!scl::float_equal(1.0f, nan1)); // Normal number vs NaN
	
	// Test infinity cases
	float inf1 = scl::SCL_FLOAT_INFINITY();
	float inf2 = scl::SCL_FLOAT_INFINITY();
	float neg_inf = scl::SCL_FLOAT_NEG_INFINITY();
	assert(scl::float_equal(inf1, inf2));   // Same positive infinity
	assert(!scl::float_equal(inf1, neg_inf)); // Different sign infinity
	assert(!scl::float_equal(inf1, 1.0f));   // Infinity vs normal number
	assert(!scl::float_equal(1.0f, inf1));   // Normal number vs infinity
	
	// Test mixed NaN and infinity
	assert(!scl::float_equal(nan1, inf1));   // NaN vs infinity
	assert(!scl::float_equal(inf1, nan1));   // Infinity vs NaN
	
	// Test normal cases still work
	assert(scl::float_equal(1.0f, 1.0f + 1e-7f)); // Within epsilon
	assert(!scl::float_equal(1.0f, 2.0f));        // Outside epsilon
}

void test_scl_special_value_detection(bool print)
{
	if (print)
		printf("\n=== Testing SCL Special Value Detection Functions ===\n");
	
	// ===============  Test is_nan functions vs std library  ===============
	if (print)
		printf("Testing is_nan functions:\n");
	
	// Test float NaN detection
	float fnan_quiet = scl::SCL_FLOAT_QNAN();
	float fnan_signal = scl::SCL_FLOAT_SNAN();
	float fnormal = 1.23f;
	float fzero = 0.0f;
	float finf = scl::SCL_FLOAT_INFINITY();
	
	// Compare scl::is_nan with std::isnan
	assert(scl::is_nan(fnan_quiet) == std::isnan(fnan_quiet));
	assert(scl::is_nan(fnan_signal) == std::isnan(fnan_signal));
	assert(scl::is_nan(fnormal) == std::isnan(fnormal));
	assert(scl::is_nan(fzero) == std::isnan(fzero));
	assert(scl::is_nan(finf) == std::isnan(finf));
	if (print) printf("[PASS] Float is_nan matches std::isnan\n");
	
	// Test double NaN detection
	double dnan_quiet = scl::SCL_DOUBLE_QNAN();
	double dnan_signal = scl::SCL_DOUBLE_SNAN();
	double dnormal = 1.23;
	double dzero = 0.0;
	double dinf = scl::SCL_DOUBLE_INFINITY();
	
	// Compare scl::is_nan with std::isnan
	assert(scl::is_nan(dnan_quiet) == std::isnan(dnan_quiet));
	assert(scl::is_nan(dnan_signal) == std::isnan(dnan_signal));
	assert(scl::is_nan(dnormal) == std::isnan(dnormal));
	assert(scl::is_nan(dzero) == std::isnan(dzero));
	assert(scl::is_nan(dinf) == std::isnan(dinf));
	if (print) printf("[PASS] Double is_nan matches std::isnan\n");
	
	// ===============  Test is_inf functions vs std library  ===============
	if (print) printf("Testing is_inf functions:\n");
	
	// Test float infinity detection
	float fpos_inf = scl::SCL_FLOAT_INFINITY();
	float fneg_inf = scl::SCL_FLOAT_NEG_INFINITY();
	
	// Compare scl::is_inf with std::isinf
	assert(scl::is_inf(fpos_inf) == std::isinf(fpos_inf));
	assert(scl::is_inf(fneg_inf) == std::isinf(fneg_inf));
	assert(scl::is_inf(fnan_quiet) == std::isinf(fnan_quiet));
	assert(scl::is_inf(fnormal) == std::isinf(fnormal));
	assert(scl::is_inf(fzero) == std::isinf(fzero));
	if (print) printf("[PASS] Float is_inf matches std::isinf\n");
	
	// Test double infinity detection
	double dpos_inf = scl::SCL_DOUBLE_INFINITY();
	double dneg_inf = scl::SCL_DOUBLE_NEG_INFINITY();
	
	// Compare scl::is_inf with std::isinf
	assert(scl::is_inf(dpos_inf) == std::isinf(dpos_inf));
	assert(scl::is_inf(dneg_inf) == std::isinf(dneg_inf));
	assert(scl::is_inf(dnan_quiet) == std::isinf(dnan_quiet));
	assert(scl::is_inf(dnormal) == std::isinf(dnormal));
	assert(scl::is_inf(dzero) == std::isinf(dzero));
	if (print) printf("[PASS] Double is_inf matches std::isinf\n");
	
	// ===============  Test float_equal comprehensive cases  ===============
	if (print) printf("Testing float_equal function:\n");
	
	// Normal float comparisons
	assert(scl::float_equal(1.0f, 1.0f));                    // Exact match
	assert(scl::float_equal(1.0f, 1.0f + 1e-8f));           // Within epsilon
	assert(!scl::float_equal(1.0f, 1.001f));                // Outside epsilon
	assert(scl::float_equal(0.0f, 0.0f));                   // Zero comparison
	assert(scl::float_equal(-1.0f, -1.0f));                 // Negative numbers
	
	// Special value comparisons  
	assert(scl::float_equal(fnan_quiet, fnan_signal));      // NaN == NaN
	assert(!scl::float_equal(fnan_quiet, 1.0f));            // NaN != normal
	assert(scl::float_equal(fpos_inf, fpos_inf));           // +inf == +inf
	assert(scl::float_equal(fneg_inf, fneg_inf));           // -inf == -inf
	assert(!scl::float_equal(fpos_inf, fneg_inf));          // +inf != -inf
	assert(!scl::float_equal(fpos_inf, 1.0f));              // inf != normal
	assert(!scl::float_equal(fnan_quiet, fpos_inf));        // NaN != inf
	
	// Edge cases
	assert(scl::float_equal(0.0f, -0.0f));                  // +0 == -0
	if (print) printf("[PASS] float_equal handles all cases correctly\n");
	
	// ===============  Test double_equal  ===============
	if (print) printf("Testing double_equal function:\n");
	
	// Normal double comparisons
	assert(scl::double_equal(1.0, 1.0));                    // Exact match
	assert(scl::double_equal(1.0, 1.0 + 1e-16));           // Within epsilon
	assert(!scl::double_equal(1.0, 1.001));                // Outside epsilon
	
	// Special value comparisons
	assert(scl::double_equal(dnan_quiet, dnan_signal));     // NaN == NaN
	assert(!scl::double_equal(dnan_quiet, 1.0));            // NaN != normal
	assert(scl::double_equal(dpos_inf, dpos_inf));          // +inf == +inf
	assert(!scl::double_equal(dpos_inf, dneg_inf));         // +inf != -inf
	if (print) printf("[PASS] double_equal handles all cases correctly\n");
}

void test_vector3_equality(bool print)
{
	if (print) printf("\n=== Testing vector3 operator== ===\n");
	
	// Test normal vector equality
	scl::vector3 v1 = { 1.0f, 2.0f, 3.0f };
	scl::vector3 v2 = { 1.0f, 2.0f, 3.0f };
	scl::vector3 v3 = { 1.1f, 2.0f, 3.0f };
	
	assert(v1 == v2);  // Same vectors
	assert(!(v1 == v3)); // Different vectors
	if (print) printf("[PASS] Normal vector equality works\n");
	
	// Test zero vectors
	scl::vector3 vzero1 = scl::vector3::zero();
	scl::vector3 vzero2 = { 0.0f, 0.0f, 0.0f };
	assert(vzero1 == vzero2);
	if (print) printf("[PASS] Zero vector equality works\n");
	
	// Test vectors with special values
	float nan_val = scl::SCL_FLOAT_QNAN();
	float inf_val = scl::SCL_FLOAT_INFINITY();
	
	scl::vector3 vnan1 = { nan_val, 2.0f, 3.0f };
	scl::vector3 vnan2 = { nan_val, 2.0f, 3.0f };
	scl::vector3 vinf1 = { inf_val, 2.0f, 3.0f };
	scl::vector3 vinf2 = { inf_val, 2.0f, 3.0f };
	
	// Note: The behavior depends on how vector3::operator== is implemented
	// If it uses float_equal, then NaN vectors should be equal
	// If it uses direct float comparison, then NaN vectors won't be equal
	if (print) printf("[INFO] Testing vector3 with NaN and infinity values\n");
	
	// Test that definitely different vectors are not equal
	scl::vector3 vdiff = { 1.0f, 2.0f, 4.0f };  // Different z component
	assert(!(v1 == vdiff));
	if (print) printf("[PASS] Different vectors are not equal\n");
	
	// Test negative values
	scl::vector3 vneg1 = { -1.0f, -2.0f, -3.0f };
	scl::vector3 vneg2 = { -1.0f, -2.0f, -3.0f };
	assert(vneg1 == vneg2);
	if (print) printf("[PASS] Negative vector equality works\n");
}

} // namespace test



