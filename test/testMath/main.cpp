#include <stdio.h>

#include "./test.h"

int main()
{
	test::test_rotate					(false);
	test::test_rotate2					(false);
	test::test_rotate_order				(false);
	test::test_matrix_mul				(false);
	test::test_camera					(false);
	test::test_camera_look_at			(false);
	test::test_frustum_infinite			(false);
	test::test_decompose				(false);
	test::test_quaternion				(false);
	test::test_plane					(false);
	test::test_plane2					(false);
	test::test_plane3					(false);
	test::test_scl_limits_vs_std_limits	(false);
	test::test_scl_special_value_detection(false);
	test::test_vector3_equality			(false);
	test::test_float_equal_special_cases();

	printf("finished.\n");
	getchar();
	return 0;
}





