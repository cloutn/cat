#include "testBigInt.h"

#include "scl/big_int.h"
#include "scl/string.h"

using namespace scl;

void test_add()
{
	int1024 i1(99919901);
	int1024 i2(99919901 * 2);
	i1.add(i2);
	assert(i1.to_int64() == 99919901 * 3);

	i1.from(-99919901);
	i2.from(-99919901 * 2);
	i1.add(i2);
	assert(i1.to_int64() == -99919901 * 3);

	i1.from(99919901);
	i2.from(-99919901 * 2);
	i1.add(i2);
	assert(i1.to_int() == -99919901);

	i1.from(99919901);
	i2.from(-99919901 / 2);
	i1.add(i2);
	assert(i1.to_int() == 49959951);

	i1.from(-99919901);
	i2.from(99919901 * 2);
	i1.add(i2);
	assert(i1.to_int() == 99919901);

	i1.from(-99919901);
	i2.from(99919901 / 2);
	i1.add(i2);
	assert(i1.to_int() == -49959951);
}

void test_dec()
{
	int1024 i1(999919962);
	int1024 i2(999919962ull * 7);
	i2.dec(i1);
	assert(i2.to_int64() == 999919962ull * 6);

	i1.from(-999919962);
	i2.from(-999919962ll * 7);
	i2.dec(i1);
	assert(i2.to_int64() == -999919962ll * 6);

	i1.from(999919962);
	i2.from(-999919962ll * 7);
	i2.dec(i1);
	assert(i2.to_int64() == -999919962ll * 8);

	i1.from(-999919962);
	i2.from(999919962ull * 7);
	i2.dec(i1);
	assert(i2.to_int64() == 999919962ull * 8);

	i1.from(999919962);
	i2.from(-999919962ll / 7);
	i2.dec(i1);
	assert(i2.to_int64() == -1142765670);

	i1.from(-999919962);
	i2.from(999919962ull / 7);
	i2.dec(i1);
	assert(i2.to_int64() == 1142765670);
}

void test_mul()
{
	//int1024 i(9993992132ull);
	//i._mul_byte(127);
	//assert(i.to_int64() == 9993992132ull * 127);

	int1024 i2(6993992132ull);
	int1024 i3(257);
	i2.mul(i3);
	assert(i2.to_uint64() == 257 * 6993992132ull);

	i2.from(-6993992132ll);
	i3.from(-257);
	i2.mul(i3);
	assert(i2.to_uint64() == 257 * 6993992132ull);

	i2.from(6993992132ull);
	i3.from(-257);
	i2.mul(i3);
	assert(i2.to_int64() == 257 * -6993992132ll);


	i2.from(-6993992132ll);
	i3.from(257);
	i2.mul(i3);
	assert(i2.to_int64() == 257 * -6993992132ll);

	int1024 i4(6993992132ull);
	int1024 i5(193992132ull);
	i4.mul(i5);
	assert(i4.to_uint64() == 193992132ull * 6993992132ull);
}

void test_div()
{
	int1024 bi(65536 * 32767);
	int1024 bi2(65536 * 3);
	bi.div(bi2);
	assert(bi.to_int64() == 10922);

	bi.from(65536 * 32767);
	bi2.from(-65536 * 3);
	bi.div(bi2);
	assert(bi.to_int64() == -10922);

	bi.from(-65536 * 32767);
	bi2.from(65536 * 3);
	bi.div(bi2);
	assert(bi.to_int64() == -10922);

	int1024 bi3(9);
	int1024 bi4(3);
	bi3.div(bi4);
	assert(bi3.to_int64() == 3);

	big_int<8> bi5(18446744073709551615ull);	//2^64 - 1
	big_int<8> bi6(4611686018427387904ull);	//2^62
	bi5.div(bi6);
	assert(bi5.to_int64() == 3);
}

void test_base()
{
	int128 i;
	i.from(1024);
	assert(i.to_uint() == 1024);

	i.from(-1024);
	assert(i.to_int() == -1024);

	i.from(4611686018427387904ull);
	assert(i.to_uint64() == 4611686018427387904ull);

	i.from(-4611686018427387904ll);
	assert(i.to_int64() == -4611686018427387904ll);

	i.from("-99921332");
	assert(i.to_int64() == -99921332);

	i.from("-98765432");
	string128 s;
	i.to_string(s.c_str(), 128);
	assert(s == "-98765432");

	i.from("-0");
	s.clear();
	i.to_string(s.c_str(), s.capacity());
	assert(s == "0");

	i.from("+0");
	s.clear();
	i.to_string(s.c_str(), s.capacity());
	assert(s == "0");

	i.from("+1234567");
	s.clear();
	i.to_string(s.c_str(), s.capacity());
	assert(s == "1234567");
}

void testBigInt()
{
	test_base();

	test_add();

	test_dec();

	test_mul();

	test_div();
}
