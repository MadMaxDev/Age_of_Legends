// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbits.h
// Creator      : Wei Hua (魏华)
// Comment      : 位处理(可能效率不会太高，不过够用)
// CreationDate : 2001-03-11
// ChangeLOG    :
// 2003-05-23    bits的顺序从第0位开始数。!!!!!!!!
// 2006-02-22    whbit_vn1_set中if( nNum & 0x80 )修改为if( nNum >= 0x80 )，原来那么写会导致有的超过0x80的数被变成1字节

#ifndef __WHBITS_H__
#define __WHBITS_H__

#include "./whcmn_def.h"

namespace n_whcmn
{

// 普通的bit操作
// _Ty应该是unsigned char/short/int等
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
	// 用异或还需要先判断是否非0才行，即这样
	// __var	^= ((*__buf)&__bit);
	__var	&= ~__bit;
}

// 循环移位
void	whbit_rotl(unsigned char &c, int n);
void	whbit_rotr(unsigned char &c, int n);
// __bit的取值是非负值
void	whbit_set(void *__buf, int __bit);
void	whbit_clr(void *__buf, int __bit);
bool	whbit_chk(void *__buf, int __bit);
// 连续设置
void	whbit_set_cont(void *__buf, int __startbit, int __span);
// 连续清除
void	whbit_clr_cont(void *__buf, int __startbit, int __span);
// 检查连续为0或为1的个数，返回这个个数
int		whbit_chk_cont(void *__buf, int __startbit, int __span, bool __is1);
// 附加一段bits，返回新的offset值
int		whbit_append(void *__dst, int __doff, void *__src, int __soff, int __len);
// 转换为字串
void	whbit_tostr(void *__buf, int __bits, char *__str);

// 专门针对uchar数据的函数(__bit : 0~7)
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
// 专门针对ushort数据的函数(__bit : 0~15)
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

// 专门针对ulong数据的函数(__bit : 0~31)
// 必须保证__exp>=0
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
// 创建宽度为__w的掩码，比如__w==4，得到二进制的1111
unsigned int	whbit_ulong_make_widemask(int __w);
void	whbit_ulong_make_widemask(int __num, int *__w, unsigned int *__m);
// 通过widmask把ulong分段(这个是0表示低位)
// __src	要分段的长整数
// __num	段数
// __w		每段的长度(数组)
// __m		每段的掩码(数组)
// __dst	分段后每段的数值
void	whbit_ulong_separate(unsigned int __src, int __num, int *__w, unsigned int *__m, unsigned int *__dst);

// 属性匹配
struct	whbit_property_match
{
	unsigned int		nProperty;							// 属性期望
	unsigned int		nALLMask;							// 全部匹配的掩码(对于掩码部分的意愿全部成立)
	unsigned int		nAnyMask;							// 只需要部分匹配的掩码(对于掩码部分的意愿至少成立一个)
	whbit_property_match()
	: nProperty(0), nALLMask(0), nAnyMask(0xFFFFFFFF)
	{
	}
	inline bool	IsOK(unsigned int nPropToChk) const
	{
		nPropToChk		^= ~nProperty;						// 这样满足的位就变成1
		if( (nALLMask&nPropToChk) == nALLMask				// nALLMask为0则表示不需要完全匹配的部分
		&&  (nAnyMask&nPropToChk) != 0						// nAnyMask不能为0，默认应该为0xFFFFFFFF
		)
		{
			return		true;
		}
		return			false;
	}
};

// 变长数字（variable number, VN）
// 1bit版：最低一位表示长度字节数：0~1表示1~2字节，代表7bit/15bit
// 设置数字，返回长度
int		whbit_vn1_set(void *pBuf, unsigned short nNum);
// 获取数字，返回长度
int		whbit_vn1_get(const void *pBuf, unsigned short *pnNum);
// 判断数据长度是否正确，如果正确才返回长度，否则返回0
int		whbit_vn1_get(const void *pBuf, int nSize, unsigned short *pnNum);
// 2bit版：最低两位表示长度字节数：0~3表示1~4字节，代表6bit/14bit/22bit/30bit
// 设置数字，返回长度
int		whbit_vn2_set(void *pBuf, unsigned int nNum);
// 计算长度
int		whbit_vn2_calclen(unsigned int nNum);
// 获取数字，返回长度
int		whbit_vn2_get(const void *pBuf, unsigned int *pnNum);
// 判断数据长度是否正确，如果正确才返回长度，否则返回0
int		whbit_vn2_get(const void *pBuf, int nSize, unsigned int *pnNum);

// 下面两个函数摘自Hacker's Delight
// （原理就是让1的个数成2倍逐级扩张）
// 获得不比x小的最小的二的整数幂
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
// 获得不比x大的最大的二的整数幂
inline unsigned int	whbit_flp2(unsigned int x)
{
	x = x | (x >> 1); 
	x = x | (x >> 2); 
	x = x | (x >> 4); 
	x = x | (x >> 8); 
	x = x | (x >>16); 
	return x - (x >> 1); 
}

// 把256进制的数据变成32进制数据字串
// p256		来源的256进制数据
// n256bit	来源的总bit数(如果bit数不是5的倍数，则内部最后自动补0)
// p32		存放32进制字串数据（最后要有0结尾）
// pn32len	输入为p32的最大长度，返回为32进制数据字串的长度（不包含0结尾的长度）
// cszChars	模板字串，就是32个字符，分别表示32个数字（比如下面这个样子）
// static const char	*g_cszChars[]	=
// {
// '0', '1', '2', '3', '3', '4', '5', '7', '8', '9', 
// 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
// 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
// 'U', 'V'
// };
int	whbit_256_to_32(const unsigned char *p256, int n256bit, char *p32, int *pn32len, const char *cszChars);

// 上面编码的解码过程
// cszCharTo32是一个256的数组，每个位置代表一个字符对应0～31中的哪个数字，它可以是由前面的g_cszChars自动生成的（不过应该只在程序开始的时候生成一次）
int	whbit_32_to_256(const char *p32, int n32len, unsigned char *p256, int *pn256bit, const unsigned char *cszCharTo32);

}	// EOF namespace n_whcmn

#endif	// EOF __WHBITS_H__
