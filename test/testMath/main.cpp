#include <stdio.h>

#include "./test.h"

int main()
{
	test::test_rotate		(false);
	test::test_rotate2		(false);
	test::test_rotate_order	(false);
	test::test_matrix_mul	(false);
	test::test_camera		(false);
	test::test_decompose	(false);
	test::test_quaternion	(false);


	printf("finished.\n");
	getchar();
	return 0;
}





