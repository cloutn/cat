
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtx/euler_angles.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "./dxmath/DirectXMath.h"

#include "./client.h"



#include "scl/log.h"

#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
//#include "vld_runtime/include/vld.h"
//#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif

#include "scl/matrix.h"
#include "scl/vector.h"
#include "scl/quaternion.h"


using namespace scl;

void start_client()
{
	cat::Client* c = new cat::Client();
	c->init();
	c->run();
	delete c;
}

void print_matrix(const matrix& m)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			printf("%.2f\t", m.m[i][j]);
		printf("\n");
	}
}
void print_matrix(const glm::mat4& m)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			printf("%.2f\t", m[i][j]);
		printf("\n");
	}
}



bool matrix_equal(const float* m1, const float* m2)
{
	for (int i = 0; i < 16; ++i)
	{
		if (!scl::float_equal(m1[i], m2[i]))
			return false;
	}
	return true;
}


void compare_matrix(const scl::matrix& mat1, const glm::mat4& mat2)
{
	printf("scl : = \n");
	print_matrix(mat1);
	printf("glm : = \n");
	print_matrix(mat2);
	if (matrix_equal(mat1.ptr(), glm::value_ptr(mat2)))
		printf("aa : euqal");
	else
		printf("aa : NOT euqal");
}

void compare_matrix(const scl::matrix& mat1, const scl::matrix& mat2)
{
	printf("scl : = \n");
	print_matrix(mat1);
	printf("glm : = \n");
	print_matrix(mat2);
	if (matrix_equal(mat1.ptr(), mat2.ptr()))
		printf("aa : euqal");
	else
		printf("aa : NOT euqal");
}



class smat
{
public:
	using str = scl::string64;
	using pstr = const char* const;

	smat() {}
	union 
	{
		str m[3][3];
		struct 
		{
			str x1, y1, z1;
			str x2, y2, z2;
			str x3, y3, z3;
		};
	};

	str get(int x, int y) const
	{
		str v = m[x][y];
		if (v == "0")
			return "[0]";
		else if (v == "1")
			return "";
		return v;
	}

	void set(
		pstr _x1, pstr _y1, pstr _z1,
		pstr _x2, pstr _y2, pstr _z2,
		pstr _x3, pstr _y3, pstr _z3)
	{
		x1 = _x1; y1 = _y1; z1 = _z1;
		x2 = _x2; y2 = _y2; z2 = _z2;
		x3 = _x3; y3 = _y3; z3 = _z3;
	}

	inline void mul(const smat& other)
	{
		smat result;
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				result.m[row][col].format_append("*%s*%s + %s*%s + %s*%s + %s*%s",
					get(row, 0).c_str(), other.get(0, col).c_str(),
					get(row, 1).c_str(), other.get(1, col).c_str(),
					get(row, 2).c_str(), other.get(2, col).c_str(),
					get(row, 3).c_str(), other.get(3, col).c_str());
			}
		}
		*this = result;
	}
};


static void _decompose_rotation_xyz(const scl::matrix& m, scl::vector3& euler)
{
	euler.y		= atan2(-m.z1, sqrt(m.x1*m.x1 + m.y1*m.y1));
	float cosy	= cosf(euler.y);
	if (scl::float_equal(cosy, 0, 0.00001f))
	{
		// sinx*cosz - cosx*sinz,   sinx*sinz + cosx*cosz
		// cosx*cosz + sinx*sinz,	cosx*sinz - sinx*cosz

		//euler.x = atan2(m[2][1], m[1][1]);	// https://learnopencv.com/rotation-matrix-to-euler-angles/
		euler.x = atan2(-m[2][0], m[1][1]);		// glm code : matrix_decompose.inl:146
		euler.z = 0; 
	}
	else
	{
		float t1 = m[1][2];
		float t2 = m[2][2];
		euler.x		= atan2(m[1][2], m[2][2]);
		euler.z		= atan2(m[0][1], m[0][0]);
	}
}


matrix decompose(matrix m, vector3* euler)
{
	vector3 t = { m.x4, m.y4, m.z4 };
	printf("transform = %.2f, %.2f, %.2f\n", t.x, t.y, t.z);

	vector3 svx = { m.x1, m.y1, m.z1 };
	float sx = svx.length();

	vector3 svy = { m.x2, m.y2, m.z2 };
	float sy = svy.length();

	vector3 svz = { m.x3, m.y3, m.z3 };
	float sz = svz.length();

	vector3 s = { sx, sy, sz };
	printf("scale = %.2f, %.2f, %.2f\n", s.x, s.y, s.z);

	matrix mat_rot = 
	{
		m.x1/s.x,		m.y1/s.x,		m.z1/s.x,		0,
		m.x2/s.y,		m.y2/s.y,		m.z2/s.y,		0,
		m.x3/s.z,		m.y3/s.z,		m.z3/s.z,		0,
		0,				0,				0,				1,
	};
	//printf("this is mat_rot:\n");
	//print_matrix(mat_rot);
	if (NULL != euler)
	{
		_decompose_rotation_xyz(mat_rot, *euler);
	}
	return mat_rot;

}

void test_quaternion()
{
	//glm::vec3 eulerDegree = { 30, 60, 90 };
	glm::vec3 euler = glm::radians<>(glm::vec3({ 30, 60, 90 }));

	glm::mat4 transformX = glm::eulerAngleX(euler.x);
	glm::mat4 transformY = glm::eulerAngleY(euler.y);
	glm::mat4 transformZ = glm::eulerAngleZ(euler.z);

	glm::mat4 transform = transformZ * transformY * transformX;

	//XMMATRIX xmmat = XMMatrixRotationX(XMConvertToRadians(60));

	matrix rx = matrix::rotate_x(30);
	matrix ry = matrix::rotate_y(60);
	matrix rz = matrix::rotate_z(90);

	matrix mm = rx;
	mm.mul(ry);
	mm.mul(rz);

	//compare_matrix(mm, transform);

	quaternion q; 
	q.from_euler_angle(30, 60, 90);
	matrix qmat;
	q.to_matrix(qmat);

	compare_matrix(qmat, mm);

	getchar();
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


	compare_matrix(m_view, glookat);


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
		print_matrix(all);

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(all, scale, rotation, translation, skew, perspective);
	glm::mat4 rotMat = glm::toMat4(rotation);
	print_matrix(rotMat);



	vector3 a = { 37, 90, 87 };
	matrix rx = matrix::rotate_x(a.x);
	matrix ry = matrix::rotate_y(a.y);
	matrix rz = matrix::rotate_z(a.z);

	matrix r = rx;
	r.mul(ry);
	r.mul(rz);

	matrix rxyz = matrix::rotate_xyz(a.x, a.y, a.z);

	compare_matrix(r, all);

	getchar();
}

int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif

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
	////print_matrix(r);

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

	start_client();

	scl::log::release();

	return 0;
}

