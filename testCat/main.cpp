
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

void test()
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


matrix decompose(matrix m)
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
	printf("this is mat_rot:\n");
	print_matrix(mat_rot);
	return mat_rot;

}

void test_quaternion()
{
	matrix rx = matrix::rotate_x(30);
	matrix ry = matrix::rotate_y(30);
	matrix rz = matrix::rotate_z(30);

	matrix mm = rz;
	mm.mul(ry);
	mm.mul(rx);
	printf("matrix rotate matrix : \n");
	print_matrix(mm);

	quaternion q; 
	q.from_euler_angle(30, 60, 90);
	matrix qmat;

	printf("quaternion rot matrix : \n");
	q.to_matrix(qmat);
	print_matrix(qmat);
}


int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif

	matrix t = matrix::move(10, 20, 30);
	matrix s = matrix::scale(1, 2, 3);
	matrix r = matrix::rotate_x(30);
	r.mul(matrix::rotate_y(60));
	r.mul(matrix::rotate_z(90));

	test_quaternion();

	matrix all = s;
	all.mul(r);
	all.mul(t);

	printf("this is original rotate:\n");
	print_matrix(r);

	const matrix& mat_rot = decompose(all);
	if (mat_rot == r)
	{
		printf("OK! rotate matrix is equal\n");
	}
	else
	{
		printf("ERROR! rotate matrix is NOT equal\n");
	}
	
	//printf("aa\n");

	getchar();	

	//test();

	scl::log::release();

	return 0;
}

