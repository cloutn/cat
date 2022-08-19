#pragma once

namespace scl {

#define div8(n) (((n) & 0xFFFFFFF8) >> 3)					//	div8(n) = n / 8;
#define mod8(n) ((n) & 0x00000007)						//	mode8(n) = n % 8
#define mod8_flag(n) (((n) & 0x00000007) == 0 ? 0 : 1)	//	mode_flag(n) = (n % 8 == 0 ? 0 : 1)

//bool _get_bit(const unsigned char& c, const int bit_index);
//void _set_bit(unsigned char& c, const int bit_index);
//void _reset_bit(unsigned char& c, const int bit_index);


//�����Ϊ1,����λΪ0��char��������λ
static const unsigned char ONE = 0x80;

//���ĳ��char�е�posλ���ϵ�bit��,����true��ʾ1��false��ʾ2
//ע�⣡pos�Ǵ�char��࿪ʼ���������
inline bool _get_bit(const unsigned char& c, const int bit_index)
{
	unsigned char v = (c & (ONE >> bit_index));
	return (v != 0);
}

//����ĳ��char�е�posλ���ϵ�bitΪ1
//ע�⣡pos�Ǵ�char��࿪ʼ���������
inline void _set_bit(unsigned char& c, const int bit_index)
{
	c = (c  | (ONE >> bit_index));
}

//����ĳ��char�е�posλ���ϵ�bitΪ0
//ע�⣡pos�Ǵ�char��࿪ʼ���������
inline void _reset_bit(unsigned char& c, const int bit_index)
{
	c = (c  & (~(ONE >> bit_index)));
}

//��תĳ��char�е�posλ���ϵ�bit
//ע�⣡pos�Ǵ�char��࿪ʼ���������
inline void _flip_bit(unsigned char& c, const int bit_index)
{
	if (_get_bit(c, bit_index))
		_reset_bit(c, bit_index);
	else
		_set_bit(c, bit_index);
}


//���ش���߿�ʼ�ĵ�һ�� 1 ��λ��
inline int _get_first_set_bit(unsigned char _a)
{
	static char table[] =
	{
		-1, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	return table[_a];
}


//���ش���߿�ʼ�ĵ�һ�� 0 ��λ��
inline int _get_first_reset_bit(unsigned char _a)
{
	static char table[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, -1
	};
	return table[_a];
}

} //namespace scl

