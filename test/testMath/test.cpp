#include "./test.h"

#include "./util.h"

#include "scl/matrix.h"
#include "scl/quaternion.h"
#include "scl/vector.h"

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>		// glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp>	// glm::perspective
#include <glm/ext/scalar_constants.hpp>		// glm::pi
#include <glm/gtx/euler_angles.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace test {

using scl::matrix;
using scl::quaternion;
using scl::vector3;

void test_rotate()
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
	matrix		sRotate = sRotateX;
	sRotate.mul(sRotateY);
	sRotate.mul(sRotateZ);

	assert(compare_mat(sRotate, gRotate));

	quaternion q; 
	q.from_euler_angle(a[0], a[1], a[2]);
	matrix qmat;
	q.to_matrix(qmat);

	assert(compare_mat(qmat, sRotate));
}




void test_old()
{
	//scl::matrix::frustum(m_mvp, -3, 3, -3, 3, -3, 3);
	scl::matrix m_view;
	scl::matrix::lookat(m_view, 1, 1, 1, 0, 0, 0, 0, 1, 0);
	glm::mat4 glookat = glm::lookAtRH(glm::vec3{1, 1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});


	scl::matrix m_projection;
	float w = 1280;
	float h = 800;
	//scl::matrix::frustum(m_projection, -w/2, w/2, -h/2, h/2, -100, 100);
	scl::matrix::perspective(m_projection, 45, w/h, 0.1f, 100);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), w/h, 0.1f, 100.0f);


	compare_mat(m_view, glookat);


	//scl::matrix::ortho(m_projection, -w/2, w/2, -h/2, h/2, -10000, 10000);

	//m_mvp = scl::matrix::identity();
	//m_mvp = scl::matrix::scale(100, 100, 100);
	//m_mvp.mul(m_view);

	//m_mvp.mul(scl::matrix::scale(1000, 1000, 100));
	//m_mvp.mul(scl::matrix::rotate_x(30));
	//m_mvp.mul(scl::matrix::rotate_y(-45));
	//m_mvp.mul(scl::matrix::rotate_z(30));
	//m_mvp.mul(m_projection);
	//m_mvp.x1 = 1;
	//m_mvp.y2 = 1;
	//m_mvp.z3 = 1;
	//m_mvp.z4 = 0;
	//m_mvp.d3 = 0;
	//m_mvp.d4 = 1;
	//m_mvp.mul(scl::matrix::move(0, 0, -1.01));

	//scl::vector3 p0 = { 0, 0, 0 };
	//scl::vector3 p1 = { 1, 0, 0 };
	//scl::vector3 p2 = { 0, 1, 0 };
	//scl::vector3 p3 = { 1, 1, 0 };
	//p0.mul_matrix(m_mvp);
	//p1.mul_matrix(m_mvp);
	//p2.mul_matrix(m_mvp);
	//p3.mul_matrix(m_mvp);

	getchar();

}

void test_rotate_order()
{
	glm::vec3 euler = { 37, 97, 87};
	glm::vec3 radian = glm::radians(euler);
		//glm::mat4 all = glm::eulerAngleXYZ(euler.x, euler.y, euler.z);
		glm::mat4 all = glm::eulerAngleZYX(radian.z, radian.y, radian.x);
		print(all);

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(all, scale, rotation, translation, skew, perspective);
	glm::mat4 rotMat = glm::toMat4(rotation);
	print(rotMat);



	vector3 a = { 37, 90, 87 };
	matrix rx = matrix::rotate_x(a.x);
	matrix ry = matrix::rotate_y(a.y);
	matrix rz = matrix::rotate_z(a.z);

	matrix r = rx;
	r.mul(ry);
	r.mul(rz);

	matrix rxyz = matrix::rotate_xyz(a.x, a.y, a.z);

	compare_mat(r, all);

	getchar();
}



void test_decompose()
{
	//matrix t = matrix::move(10, 20, 30);
	//matrix s = matrix::scale(1, 2, 3);
	//matrix r = matrix::rotate_x(33);
	//r.mul(matrix::rotate_y(88));
	//r.mul(matrix::rotate_z(66));
	//matrix rxyz = matrix::rotate_xyz(33, 88, 66);
	//compare_matrix(r, rxyz);

	//using type_t = float;
	//type_t cosx = cosf(scl::radian(33));
	//type_t cosy = cosf(scl::radian(88));
	//type_t sinx = sinf(scl::radian(33));
	//type_t t1 = sinx*cosy;
	//type_t t2 = cosx*cosy;
	//type_t v = atan2f(t1, t2);
	//type_t av = scl::angle(v);

	//vector3 decomposeEuler = { 0,0,0 };
	//_decompose_rotation_xyz(r, decomposeEuler);
	//printf("decompose euler = %.3f, %.3f, %.3f\n", scl::angle(decomposeEuler.x), scl::angle(decomposeEuler.y), scl::angle(decomposeEuler.z));

	//getchar();
	//return 0;

	//matrix all = s;
	//all.mul(r);
	//all.mul(t);

	////printf("this is original rotate:\n");
	////print(r);

	//vector3 euler = { 0, 0, 0 };
	//const matrix& mat_rot = decompose(all, &euler);
	//printf("euler = %.3f, %.3f, %.3f\n", scl::angle(euler.x), scl::angle(euler.y), scl::angle(euler.z));
	//compare_matrix(mat_rot, r);

	//getchar();
	//if (mat_rot == r)
	//{
	//	printf("OK! rotate matrix is equal\n");
	//}
	//else
	//{
	//	printf("ERROR! rotate matrix is NOT equal\n");
	//}
	
	//test_old();
	//printf("aa\n");

	//getchar();	

}

}



