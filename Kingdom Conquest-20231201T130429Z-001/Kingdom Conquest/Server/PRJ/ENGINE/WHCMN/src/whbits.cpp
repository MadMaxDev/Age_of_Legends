// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbits.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 位处理(可能效率不会太高，不过够用)
// CreationDate : 2001-03-11
// ChangeLOG    :

#include "../inc/whbits.h"
#include <string.h>
#include <assert.h>

namespace n_whcmn
{

void	whbit_rotl(unsigned char &c, int n)
{
	n	&=	3;
	if( n==0 )
	{
		return;
	}
	unsigned short	s	= c;
	s	<<= n;
	c	= (unsigned char)(s | (s>>8));
}
void	whbit_rotr(unsigned char &c, int n)
{
	n	&=	3;
	if( n==0 )
	{
		return;
	}
	unsigned short	s	= c<<8;
	s	>>= n;
	c	= (unsigned char)(s | (s>>8));
}

static int	_char_bit_get_mask(int __bit, int *__pos, unsigned char *__mask)
{
	// 2006-02-23 把获得mask的方法改为移位而不是查表
	if( __bit<0 )
	{
		return	-1;
	}
	*__pos	= __bit >> 3;
	*__mask	= 1 << (__bit&0x7);
	return	0;
}
void	whbit_set(void *__buf, int __bit)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__bit, &pos, &mask);

	buf				= (unsigned char *)__buf;
	buf[pos]		|= mask;
}
void	whbit_clr(void *__buf, int __bit)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__bit, &pos, &mask);

	buf				= (unsigned char *)__buf;
	buf[pos]		&= ~mask;
}
// return 1/0
bool	whbit_chk(void *__buf, int __bit)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__bit, &pos, &mask);

	buf		= (unsigned char *)__buf;
	return	(buf[pos] & mask) != 0;
}
void	whbit_set_cont(void *__buf, int __startbit, int __span)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__startbit, &pos, &mask);
	buf				= (unsigned char *)__buf + pos;
	int i=0;
	for(;i<__span;++i)
	{
		buf[0]		|= mask;
		mask		<<= 1;
		if( mask == 0 )
		{
			mask	= 1;
			++buf;
		}
	}
}
void	whbit_clr_cont(void *__buf, int __startbit, int __span)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__startbit, &pos, &mask);
	buf				= (unsigned char *)__buf + pos;
	int i=0;
	for(;i<__span;++i)
	{
		buf[pos]	&= ~mask;
		mask		<<= 1;
		if( mask == 0 )
		{
			mask	= 1;
			++buf;
		}
	}
}
int		whbit_chk_cont(void *__buf, int __startbit, int __span, bool __is1)
{
	int				pos;
	unsigned char	mask, *buf;
	_char_bit_get_mask(__startbit, &pos, &mask);
	buf				= (unsigned char *)__buf + pos;
	int i=0;
	for(;i<__span;++i)
	{
		if( __is1 )
		{
			if( (buf[0]&mask) == 0 )
			{
				break;
			}
		}
		else
		{
			if( (buf[0]&mask) != 0 )
			{
				break;
			}
		}
		mask		<<= 1;
		if( mask == 0 )
		{
			mask	= 1;
			++buf;
		}
	}

	return	i;
}

static const	unsigned char	scst_bits_1[8] =
{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static const	unsigned char	scst_bits_0[8] =
{0xFF^0x01, 0xFF^0x02, 0xFF^0x04, 0xFF^0x08, 0xFF^0x10, 0xFF^0x20, 0xFF^0x40, 0xFF^0x80};
#define	BC_BIT_INC(__i, __bi)	{__bi++; if(__bi==8){__i++; __bi=0;}}
int		whbit_append(void *__dst, int __doff, void *__src, int __soff, int __len)
{
	int		di, si, dbi, sbi;		// 正在处理的字节和bit序号
	int		count;

	di	= __doff / 8;
	dbi	= __doff % 8;
	si	= __soff / 8;
	sbi	= __soff % 8;
	count	= __len;
	unsigned char	*src	= (unsigned char *)__src;
	unsigned char	*dst	= (unsigned char *)__dst;
	while(count>0)
	{
		if( scst_bits_1[sbi] & src[si] )
		{
			// 该位为1
			dst[di]	|= scst_bits_1[dbi];
		}
		else
		{
			// 该位为0
			dst[di]	&= scst_bits_0[dbi];
		}
		count	--;
		BC_BIT_INC(di, dbi);
		BC_BIT_INC(si, sbi);
	}

	return	__doff + __len;
}

void	whbit_tostr(void *__buf, int __bits, char *__str)
{
	int	i, j;
	j	= 0;
	for(i=__bits-1;i>=0;i--)
	{
		if( whbit_chk(__buf, i) )
		{
			__str[j++]	= '1';
		}
		else
		{
			__str[j++]	= '0';
		}
	}
	__str[__bits]	= 0;
}

unsigned int	whbit_ulong_make_widemask(int __w)
{
	unsigned int	nMask	= 0;
	for(int i=0;i<__w;i++)
	{
		nMask	|= _ulong_bit_get_mask(i);
	}
	return	nMask;
}
void	whbit_ulong_make_widemask(int __num, int *__w, unsigned int *__m)
{
	for(int i=0;i<__num;i++)
	{
		__m[i]	= whbit_ulong_make_widemask(__w[i]);
	}
}
// 通过widmask把ulong分段
// __src	要分段的长整数
// __num	段数
// __w		每段的长度(数组)
// __m		每段的掩码(数组)
// __dst	分段后每段的数值
void	whbit_ulong_separate(unsigned int __src, int __num, int *__w, unsigned int *__m, unsigned int *__dst)
{
	int	i;
	__num	--;
	for(i=0;i<__num;i++)
	{
		__dst[i]	= __src & __m[i];
		__src		>>= __w[i];
	}
	__dst[i]		= __src & __m[i];
}

// 变长数字（variable number, VN）
// 1bit版：最高一位表示长度字节数：0~1表示1~2字节，代表7bit/15bit
// 设置数字，返回长度
union	PALS_T
{
	unsigned int	l;
	unsigned short	s;
	unsigned char	c[4];
	explicit PALS_T(unsigned int __l)
	: l(__l)
	{
	}
	explicit PALS_T(unsigned short __s)
	: s(__s)
	{
	}
};
int		whbit_vn1_set(void *pBuf, unsigned short nNum)
{
	if( nNum & 0x8000 )
	{
		// 说明没有设置成功
		assert(0);
		return	0;
	}
	PALS_T			p(nNum);
	unsigned char	*pucBuf	= (unsigned char *)pBuf;
	// 2字节
	if( nNum >= 0x80 )
	{
		p.s			<<= 1;
		pucBuf[0]	= p.c[0] | 0x01;
		pucBuf[1]	= p.c[1];
		return		2;
	}
	// 1字节
	pucBuf[0]		= p.c[0] << 1;
	return			1;
}
// 获取数字，返回长度
int		whbit_vn1_get(const void *pBuf, unsigned short *pnNum)
{
	unsigned char	*pucBuf	= (unsigned char *)pBuf;
	if( (pucBuf[0] & 0x01) == 0 )
	{
		// 字节
		*pnNum		= pucBuf[0] >> 1;
		return		1;
	}
	*pnNum			= (*(unsigned short *)pucBuf) >> 1;
	return			2;
}
// 判断数据长度是否正确，如果正确才返回长度，否则返回0
int		whbit_vn1_get(const void *pBuf, int nSize, unsigned short *pnNum)
{
	if( nSize<=0 )
	{
		return	0;
	}
	unsigned char	*pucBuf	= (unsigned char *)pBuf;
	if( (pucBuf[0] & 0x01) == 0 )
	{
		// 字节
		*pnNum		= pucBuf[0] >> 1;
		return		1;
	}
	if( nSize<2 )
	{
		return	0;
	}
	*pnNum			= (*(unsigned short *)pucBuf) >> 1;
	return			2;
}

// 2bit版：最低两位表示长度字节数：0~3表示1~4字节，代表6bit/14bit/22bit/30bit
// 设置数字，返回长度
int		whbit_vn2_set(void *pBuf, unsigned int nNum)
{
	if( nNum & 0xC0000000 )
	{
		// 说明没有设置成功
		return	0;
	}
	PALS_T			p(nNum);
	p.l				<<= 2;
	// 4字节
	if( p.l >= 0x1000000 )
	{
		p.c[0]		|= 0x03;
		memcpy(pBuf, p.c, 4);
		return		4;
	}
	// 3字节
	if( p.l >= 0x10000 )
	{
		p.c[0]		|= 0x02;
		memcpy(pBuf, p.c, 3);
		return		3;
	}
	// 2字节
	if( p.l >= 0x100 )
	{
		p.c[0]		|= 0x01;
		memcpy(pBuf, p.c, 2);
		return		2;
	}
	// 1字节
	memcpy(pBuf, p.c, 1);
	return		1;
}
int		whbit_vn2_calclen(unsigned int nNum)
{
	if( nNum & 0xC0000000 )
	{
		// 说明没有设置成功
		return	0;
	}
	PALS_T			p(nNum);
	p.l				<<= 2;
	// 4字节
	if( p.l >= 0x1000000 )
	{
		return		4;
	}
	// 3字节
	if( p.l >= 0x10000 )
	{
		return		3;
	}
	// 2字节
	if( p.l >= 0x100 )
	{
		return		2;
	}
	// 1字节
	return		1;
}
// 获取数字，返回长度
int		whbit_vn2_get(const void *pBuf, unsigned int *pnNum)
{
	unsigned char	*pucBuf	= (unsigned char *)pBuf;
	unsigned char	nIdx	= ((*pucBuf) & 0x3) + 1;

	// 清空没用的部分
	*pnNum			= 0;
	// 拷贝有用的部分
	memcpy(pnNum, pBuf, nIdx);
	// 移位恢复
	*pnNum			>>= 2;

	return			nIdx;
}
// 判断数据长度是否正确，如果正确才返回长度，否则返回0
int		whbit_vn2_get(const void *pBuf, int nSize, unsigned int *pnNum)
{
	if( nSize<=0 )
	{
		return	0;
	}
	unsigned char	*pucBuf	= (unsigned char *)pBuf;
	unsigned char	nIdx	= ((*pucBuf) & 0x3) + 1;
	if( nIdx>nSize )
	{
		return	0;
	}
	// 清空没用的部分
	*pnNum			= 0;
	// 拷贝有用的部分
	memcpy(pnNum, pBuf, nIdx);
	// 移位恢复
	*pnNum			>>= 2;

	return			nIdx;
}

}	// EOF namespace n_whcmn
