// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbits.h
// Creator      : Wei Hua (κ��)
// Comment      : λ����(����Ч�ʲ���̫�ߣ���������)
// CreationDate : 2001-03-11
// ChangeLOG    :
// 2003-05-23    bits��˳��ӵ�0λ��ʼ����!!!!!!!!
// 2006-02-22    whbit_vn1_set��if( nNum & 0x80 )�޸�Ϊif( nNum >= 0x80 )��ԭ����ôд�ᵼ���еĳ���0x80���������1�ֽ�

#ifndef __WHBITS_H__
#define __WHBITS_H__

#include "./whcmn_def.h"

namespace n_whcmn
{

// ��ͨ��bit����
// _TyӦ����unsigned char/short/int��
template<typename _Ty>
inline void	whcmnbit_set(_Ty &__var, const _Ty __bit)
{
	__var	|= __bit;
}
template<typename _Ty>
inline bool	whcmnbit_chk(const _Ty &__var, const _Ty __bit)			// for all bits
{
	return	(__var & __bit) == __bit;
}
template<typename _Ty>
inline bool	whcmnbit_chk_any(const _Ty &__var, const _Ty __bit)		// for any bits
{
	return	(__var & __bit) != 0;
}
template<typename _Ty>
inline void	whcmnbit_clr(_Ty &__var, const _Ty __bit)
{
	// �������Ҫ���ж��Ƿ��0���У�������
	// __var	^= ((*__buf)&__bit);
	__var	&= ~__bit;
}

// ѭ����λ
void	whbit_rotl(unsigned char &c, int n);
void	whbit_rotr(unsigned char &c, int n);
// __bit��ȡֵ�ǷǸ�ֵ
void	whbit_set(void *__buf, int __bit);
void	whbit_clr(void *__buf, int __bit);
bool	whbit_chk(void *__buf, int __bit);
// ��������
void	whbit_set_cont(void *__buf, int __startbit, int __span);
// �������
void	whbit_clr_cont(void *__buf, int __startbit, int __span);
// �������Ϊ0��Ϊ1�ĸ����������������
int		whbit_chk_cont(void *__buf, int __startbit, int __span, bool __is1);
// ����һ��bits�������µ�offsetֵ
int		whbit_append(void *__dst, int __doff, void *__src, int __soff, int __len);
// ת��Ϊ�ִ�
void	whbit_tostr(void *__buf, int __bits, char *__str);

// ר�����uchar���ݵĺ���(__bit : 0~7)
inline const unsigned char	_uchar_bit_get_mask(int __bit)
{
	const unsigned char	mask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	return	mask[__bit];
}
inline const unsigned char	_uchar_bit_get_imask(int __bit)
{
	const unsigned char	mask[8] = {0xFF^0x01, 0xFF^0x02, 0xFF^0x04, 0xFF^0x08, 0xFF^0x10, 0xFF^0x20, 0xFF^0x40, 0xFF^0x80};
	return	mask[__bit];
}
inline void	whbit_uchar_set(unsigned char *__buf, int __bit)
{
	*__buf	|= _uchar_bit_get_mask(__bit);
}
inline void	whbit_uchar_clr(unsigned char *__buf, int __bit)
{
	*__buf	&= _uchar_bit_get_imask(__bit);
}
inline bool	whbit_uchar_chk(unsigned char *__buf, int __bit)
{
	return	((*__buf) & _uchar_bit_get_mask(__bit)) != 0;
}
// ר�����ushort���ݵĺ���(__bit : 0~15)
inline const unsigned short	_ushort_bit_get_mask(int __bit)
{
	const unsigned short	mask[16] =
	{
		0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
		0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000,
	};
	return	mask[__bit];
}
inline const unsigned short	_ushort_bit_get_imask(int __bit)
{
	const unsigned short	mask[16] =
	{
		0xFFFF^0x0001, 0xFFFF^0x0002, 0xFFFF^0x0004, 0xFFFF^0x0008, 0xFFFF^0x0010, 0xFFFF^0x0020, 0xFFFF^0x0040, 0xFFFF^0x0080,
		0xFFFF^0x0100, 0xFFFF^0x0200, 0xFFFF^0x0400, 0xFFFF^0x0800, 0xFFFF^0x1000, 0xFFFF^0x2000, 0xFFFF^0x4000, 0xFFFF^0x8000,
	};
	return	mask[__bit];
}
inline void	whbit_ushort_set(unsigned short *__buf, int __bit)
{
	*__buf	|= _ushort_bit_get_mask(__bit);
}
inline void	whbit_ushort_clr(unsigned short *__buf, int __bit)
{
	*__buf	&= _ushort_bit_get_imask(__bit);
}
inline bool	whbit_ushort_chk(const unsigned short *__buf, int __bit)
{
	return	((*__buf) & _ushort_bit_get_mask(__bit)) != 0;
}

// ר�����ulong���ݵĺ���(__bit : 0~31)
// ���뱣֤__exp>=0
inline unsigned int		_ulong_2exp(int __exp)
{
	if( __exp<1 )
	{
		return	1;
	}
	return	((unsigned int)2)<<(__exp-1);
}
inline const unsigned int	_ulong_bit_get_mask(int __bit)
{
	const unsigned int	mask[32] =
	{
		0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
		0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
		0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000,
		0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000,
	};
	return	mask[__bit];
}
inline const unsigned int	_ulong_bit_get_imask(int __bit)
{
	const unsigned int	mask[32] =
	{
		~0x00000001, ~0x00000002, ~0x00000004, ~0x00000008, ~0x00000010, ~0x00000020, ~0x00000040, ~0x00000080,
		~0x00000100, ~0x00000200, ~0x00000400, ~0x00000800, ~0x00001000, ~0x00002000, ~0x00004000, ~0x00008000,
		~0x00010000, ~0x00020000, ~0x00040000, ~0x00080000, ~0x00100000, ~0x00200000, ~0x00400000, ~0x00800000,
		~0x01000000, ~0x02000000, ~0x04000000, ~0x08000000, ~0x10000000, ~0x20000000, ~0x40000000, ~0x80000000,
	};
	return	mask[__bit];
}
inline void	whbit_ulong_set(unsigned int *__buf, int __bit)
{
	*__buf	|= _ulong_bit_get_mask(__bit);
}
inline void	whbit_ulong_clr(unsigned int *__buf, int __bit)
{
	*__buf	&= _ulong_bit_get_imask(__bit);
}
inline bool	whbit_ulong_chk(const unsigned int *__buf, int __bit)
{
	return	((*__buf) & _ulong_bit_get_mask(__bit)) != 0;
}

inline const whuint64	_u64_bit_get_mask(int __bit)
{
	const whuint64	mask[64] =
	{
		0x0000000000000001LL, 0x0000000000000002LL, 0x0000000000000004LL, 0x0000000000000008LL, 0x0000000000000010LL, 0x0000000000000020LL, 0x0000000000000040LL, 0x0000000000000080LL,
		0x0000000000000100LL, 0x0000000000000200LL, 0x0000000000000400LL, 0x0000000000000800LL, 0x0000000000001000LL, 0x0000000000002000LL, 0x0000000000004000LL, 0x0000000000008000LL,
		0x0000000000010000LL, 0x0000000000020000LL, 0x0000000000040000LL, 0x0000000000080000LL, 0x0000000000100000LL, 0x0000000000200000LL, 0x0000000000400000LL, 0x0000000000800000LL,
		0x0000000001000000LL, 0x0000000002000000LL, 0x0000000004000000LL, 0x0000000008000000LL, 0x0000000010000000LL, 0x0000000020000000LL, 0x0000000040000000LL, 0x0000000080000000LL,
		0x0000000100000000LL, 0x0000000200000000LL, 0x0000000400000000LL, 0x0000000800000000LL, 0x0000001000000000LL, 0x0000002000000000LL, 0x0000004000000000LL, 0x0000008000000000LL,
		0x0000010000000000LL, 0x0000020000000000LL, 0x0000040000000000LL, 0x0000080000000000LL, 0x0000100000000000LL, 0x0000200000000000LL, 0x0000400000000000LL, 0x0000800000000000LL,
		0x0001000000000000LL, 0x0002000000000000LL, 0x0004000000000000LL, 0x0008000000000000LL, 0x0010000000000000LL, 0x0020000000000000LL, 0x0040000000000000LL, 0x0080000000000000LL,
		0x0100000000000000LL, 0x0200000000000000LL, 0x0400000000000000LL, 0x0800000000000000LL, 0x1000000000000000LL, 0x2000000000000000LL, 0x4000000000000000LL, 0x8000000000000000LL,
	};
	return	mask[__bit];
}
inline const whuint64	_u64_bit_get_imask(int __bit)
{
	const whuint64	mask[64] =
	{
		~0x0000000000000001LL, ~0x0000000000000002LL, ~0x0000000000000004LL, ~0x0000000000000008LL, ~0x0000000000000010LL, ~0x0000000000000020LL, ~0x0000000000000040LL, ~0x0000000000000080LL,
		~0x0000000000000100LL, ~0x0000000000000200LL, ~0x0000000000000400LL, ~0x0000000000000800LL, ~0x0000000000001000LL, ~0x0000000000002000LL, ~0x0000000000004000LL, ~0x0000000000008000LL,
		~0x0000000000010000LL, ~0x0000000000020000LL, ~0x0000000000040000LL, ~0x0000000000080000LL, ~0x0000000000100000LL, ~0x0000000000200000LL, ~0x0000000000400000LL, ~0x0000000000800000LL,
		~0x0000000001000000LL, ~0x0000000002000000LL, ~0x0000000004000000LL, ~0x0000000008000000LL, ~0x0000000010000000LL, ~0x0000000020000000LL, ~0x0000000040000000LL, ~0x0000000080000000LL,
		~0x0000000100000000LL, ~0x0000000200000000LL, ~0x0000000400000000LL, ~0x0000000800000000LL, ~0x0000001000000000LL, ~0x0000002000000000LL, ~0x0000004000000000LL, ~0x0000008000000000LL,
		~0x0000010000000000LL, ~0x0000020000000000LL, ~0x0000040000000000LL, ~0x0000080000000000LL, ~0x0000100000000000LL, ~0x0000200000000000LL, ~0x0000400000000000LL, ~0x0000800000000000LL,
		~0x0001000000000000LL, ~0x0002000000000000LL, ~0x0004000000000000LL, ~0x0008000000000000LL, ~0x0010000000000000LL, ~0x0020000000000000LL, ~0x0040000000000000LL, ~0x0080000000000000LL,
		~0x0100000000000000LL, ~0x0200000000000000LL, ~0x0400000000000000LL, ~0x0800000000000000LL, ~0x1000000000000000LL, ~0x2000000000000000LL, ~0x4000000000000000LL, ~0x8000000000000000LL,
	};
	return	mask[__bit];
}
inline void	whbit_u64_set(whuint64 *__buf, int __bit)
{
	*__buf	|= _u64_bit_get_mask(__bit);
}
inline void	whbit_u64_clr(whuint64 *__buf, int __bit)
{
	*__buf	&= _u64_bit_get_imask(__bit);
}
inline bool	whbit_u64_chk(const whuint64 *__buf, int __bit)
{
	return	((*__buf) & _u64_bit_get_mask(__bit)) != 0;
}
// �������Ϊ__w�����룬����__w==4���õ������Ƶ�1111
unsigned int	whbit_ulong_make_widemask(int __w);
void	whbit_ulong_make_widemask(int __num, int *__w, unsigned int *__m);
// ͨ��widmask��ulong�ֶ�(�����0��ʾ��λ)
// __src	Ҫ�ֶεĳ�����
// __num	����
// __w		ÿ�εĳ���(����)
// __m		ÿ�ε�����(����)
// __dst	�ֶκ�ÿ�ε���ֵ
void	whbit_ulong_separate(unsigned int __src, int __num, int *__w, unsigned int *__m, unsigned int *__dst);

// ����ƥ��
struct	whbit_property_match
{
	unsigned int		nProperty;							// ��������
	unsigned int		nALLMask;							// ȫ��ƥ�������(�������벿�ֵ���Ըȫ������)
	unsigned int		nAnyMask;							// ֻ��Ҫ����ƥ�������(�������벿�ֵ���Ը���ٳ���һ��)
	whbit_property_match()
	: nProperty(0), nALLMask(0), nAnyMask(0xFFFFFFFF)
	{
	}
	inline bool	IsOK(unsigned int nPropToChk) const
	{
		nPropToChk		^= ~nProperty;						// ���������λ�ͱ��1
		if( (nALLMask&nPropToChk) == nALLMask				// nALLMaskΪ0���ʾ����Ҫ��ȫƥ��Ĳ���
		&&  (nAnyMask&nPropToChk) != 0						// nAnyMask����Ϊ0��Ĭ��Ӧ��Ϊ0xFFFFFFFF
		)
		{
			return		true;
		}
		return			false;
	}
};

// �䳤���֣�variable number, VN��
// 1bit�棺���һλ��ʾ�����ֽ�����0~1��ʾ1~2�ֽڣ�����7bit/15bit
// �������֣����س���
int		whbit_vn1_set(void *pBuf, unsigned short nNum);
// ��ȡ���֣����س���
int		whbit_vn1_get(const void *pBuf, unsigned short *pnNum);
// �ж����ݳ����Ƿ���ȷ�������ȷ�ŷ��س��ȣ����򷵻�0
int		whbit_vn1_get(const void *pBuf, int nSize, unsigned short *pnNum);
// 2bit�棺�����λ��ʾ�����ֽ�����0~3��ʾ1~4�ֽڣ�����6bit/14bit/22bit/30bit
// �������֣����س���
int		whbit_vn2_set(void *pBuf, unsigned int nNum);
// ���㳤��
int		whbit_vn2_calclen(unsigned int nNum);
// ��ȡ���֣����س���
int		whbit_vn2_get(const void *pBuf, unsigned int *pnNum);
// �ж����ݳ����Ƿ���ȷ�������ȷ�ŷ��س��ȣ����򷵻�0
int		whbit_vn2_get(const void *pBuf, int nSize, unsigned int *pnNum);

// ������������ժ��Hacker's Delight
// ��ԭ�������1�ĸ�����2�������ţ�
// ��ò���xС����С�Ķ���������
inline unsigned int	whbit_clp2(unsigned int x)
{
	x = x - 1; 
	x = x | (x >> 1); 
	x = x | (x >> 2); 
	x = x | (x >> 4); 
	x = x | (x >> 8); 
	x = x | (x >>16); 
	return x + 1; 
}
// ��ò���x������Ķ���������
inline unsigned int	whbit_flp2(unsigned int x)
{
	x = x | (x >> 1); 
	x = x | (x >> 2); 
	x = x | (x >> 4); 
	x = x | (x >> 8); 
	x = x | (x >>16); 
	return x - (x >> 1); 
}

// ��256���Ƶ����ݱ��32���������ִ�
// p256		��Դ��256��������
// n256bit	��Դ����bit��(���bit������5�ı��������ڲ�����Զ���0)
// p32		���32�����ִ����ݣ����Ҫ��0��β��
// pn32len	����Ϊp32����󳤶ȣ�����Ϊ32���������ִ��ĳ��ȣ�������0��β�ĳ��ȣ�
// cszChars	ģ���ִ�������32���ַ����ֱ��ʾ32�����֣���������������ӣ�
// static const char	*g_cszChars[]	=
// {
// '0', '1', '2', '3', '3', '4', '5', '7', '8', '9', 
// 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
// 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
// 'U', 'V'
// };
int	whbit_256_to_32(const unsigned char *p256, int n256bit, char *p32, int *pn32len, const char *cszChars);

// �������Ľ������
// cszCharTo32��һ��256�����飬ÿ��λ�ô���һ���ַ���Ӧ0��31�е��ĸ����֣�����������ǰ���g_cszChars�Զ����ɵģ�����Ӧ��ֻ�ڳ���ʼ��ʱ������һ�Σ�
int	whbit_32_to_256(const char *p32, int n32len, unsigned char *p256, int *pn256bit, const unsigned char *cszCharTo32);

}	// EOF namespace n_whcmn

#endif	// EOF __WHBITS_H__
