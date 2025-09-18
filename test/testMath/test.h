#pragma once

namespace test {

extern bool g_verbose;

void test_rotate					(bool print = false);
void test_rotate2					(bool print = false);
void test_matrix_mul				(bool print = false);
void test_camera					(bool print = false);
void test_camera_look_at			(bool print = false);
void test_rotate_order				(bool print = false);
void test_decompose					(bool print = false);
void test_quaternion				(bool print = false);
void test_plane						(bool print = false);
void test_plane2					(bool print = false);
void test_plane3					(bool print = false);
void test_scl_limits_vs_std_limits	(bool print = false);
void test_scl_special_value_detection(bool print = false);
void test_vector3_equality			(bool print = false);
void test_float_equal_special_cases	();

} // namespace test



