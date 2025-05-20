// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbase64.cpp
// Creator      : Wei Hua (魏华)
// Comment      : base64编码
// CreationDate : 2001-10
// ChangeLOG    : 2008-08-22 增加了王珺写的更高效的base32解码编码部分


#include <stdio.h>
#include <string.h>
#include "../inc/whbase64.h"
#include "../inc/whbits.h"

namespace n_whcmn
{

void	tobinary(unsigned char _in, char *_out)
{
	int		i;
	union
	{
		unsigned short	s;
		unsigned char	b[2];
	}box;
	_out[0]	= 0;
	memset(&box, 0, sizeof(box));
	box.b[0]	= _in;
	for(i=0;i<8;i++)
	{
		box.b[1]= 0;
		box.s	<<= 1;
		if( box.b[1] )
		{
			strcat(_out, "1");
		}
		else
		{
			strcat(_out, "0");
		}
	}
}

unsigned char	to_base64(unsigned char __c)
{
	if( __c >= 'A' && __c<= 'Z' )
	{
		return	__c - 'A';
	}
	else if( __c >= 'a' && __c<= 'z' )
	{
		return	__c - 'a' + 26;
	}
	else if( __c >= '0' && __c<= '9' )
	{
		return	__c - '0' + 52;
	}
	else
	{
		switch( __c )
		{
			case	'+':
				return	62;
			break;
			case	'/':
				return	63;
			break;
		}
	}

	// error
	return	0;
}

unsigned char	to_base64_char(unsigned char __c)
{
	//if( __c >= 0 && __c<= 25 )
	if( __c<= 25 )
	{
		return	__c + 'A';
	}
	else if( __c >= 26 && __c<= 51 )
	{
		return	__c - 26 + 'a';
	}
	else if( __c >= 52 && __c<= 61 )
	{
		return	__c - 52 + '0';
	}
	else
	{
		switch( __c )
		{
			case	62:
				return	'+';
			break;
			case	63:
				return	'/';
			break;
		}
	}

	// error
	return	255;
}

void	outarray64(char *__file)
{
	FILE	*fp;
	int	i, count;
	fp	= fopen(__file, "w");

	count	= 0;
	fprintf(fp, "static unsigned char base64_en[] =\n{");
	for(i=0;i<64;i++)
	{
		if(!count)
		{
			fprintf(fp, "\n\t");
		}
		count	= (count + 1) % 16;
		fprintf(fp, "'%c', ", to_base64_char(i));
	}
	fprintf(fp, "\n};\n\n");

	fprintf(fp, "static unsigned char base64_de[] =\n{");
	count	= 0;
	for(i=0;i<256;i++)
	{
		if(!count)
		{
			fprintf(fp, "\n\t");
		}
		count	= (count + 1) % 16;
		unsigned char	c;
		c	= to_base64(i);
		if(c==255)
		fprintf(fp, "   0, ");
		else
		fprintf(fp, "%4d, ", c);
	}
	fprintf(fp, "\n};\n");

	fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
// base64 codec
////////////////////////////////////////////////////////////////////////////
// 用数组是为了快！！！！
static unsigned char base64_en[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
	'='
};
static unsigned char base64_de[] =
{
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   62,    0,    0,    0,   63, 
	  52,   53,   54,   55,   56,   57,   58,   59,   60,   61,    0,    0,    0,    0,    0,    0, 
	   0,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14, 
	  15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,    0,    0,    0,    0,    0, 
	   0,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40, 
	  41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
};

// _in里一定是4个字符，变成_out里的最多三个字符
// _in[0],[1]是不会为‘＝’的
int	_base64_decode(char *_in, unsigned char *_out)
{
	int		len;
	unsigned char	a;
	unsigned char	*pin	= (unsigned char *)_in;

	_out[0]	= base64_de[pin[0]];
	_out[0]	<<= 2;
	_out[1] = a	= base64_de[pin[1]];
	a		>>= 4;
	_out[0]	|= a;
	len		= 1;
	if( pin[2] == '=' )	return	len;
	_out[1]	<<= 4;
	_out[2] = a	= base64_de[pin[2]];
	a		>>= 2;
	_out[1]	|= a;
	len		= 2;
	if( pin[3] == '=' )	return	len;
	_out[2]	<<= 6;
	// 注意，_out没有3
	a		= base64_de[pin[3]];
	_out[2]	|= a;
	len		= 3;

	return	len;
}
// _in中的字符数一定是4的整倍数
int	base64_decode(char *_in, unsigned char *_out)
{
	int	rst, i, inlen, outlen;

	inlen	= strlen(_in);
	if( inlen % 4 )
	{
		return	-1;
	}
	outlen	= 0;
	for(i=0;i<inlen;i+=4)
	{
		rst	= _base64_decode(_in, _out);
		if( rst<0 )	return	-1;
		_in	+= 4;
		_out	+= rst;
		outlen	+= rst;
	}

	return	outlen;
}

// __inlen一定<=3 && >=1
int	_base64_encode(unsigned char *_in, int __inlen, char *_out)
{
	unsigned char	*out	= (unsigned char *)_out;
	int		i;
	unsigned char	a;
	memset(out, 64, 4);

	out[0]	= _in[0];
	out[0]	>>= 2;				// 高6
	out[1]	= _in[0] & 0x03;
	out[1]	<<= 4;				// 低2
	if( __inlen==1 )	goto	End;
	a		= _in[1];
	a		>>= 4;				// 高4
	out[1]	|= a;
	out[2]	= _in[1] & 0x0F;		// 低4
	out[2]	<<=2;
	if( __inlen==2 )	goto	End;
	a		= _in[2];
	a		>>= 6;				// 高2
	out[2]	|= a;
	out[3]	= _in[2] & 0x3F;		// 低6
End:
	for(i=0;i<4;i++)
	{
		out[i]	= base64_en[(unsigned char)out[i]];
	}
	return	0;
}
// outlen一定是4的整倍数
int	base64_encode(unsigned char *_in, int __inlen, char *_out)
{
	int	rst, i, inlen, outlen;
	int	rest;

	inlen	= __inlen;
	rest	= inlen;
	outlen	= 0;
	for(i=0;i<inlen;i+=3)
	{
		rst	= _base64_encode(_in, 3>rest ? rest : 3, _out);
		if( rst<0 )	return	-1;
		rest	-= 3;
		_in	+= 3;
		_out	+= 4;
		outlen	+= 4;
	}
	// 现在_out指向的是最后一个字符，且这个字串一定都是可显示的，所以加上0结尾
	*_out	= 0;

	return	outlen;
}

// base32
////////////////////////////////////////////////////////////////////
// 只要1、0、I、O都不要，然后进行一次魏氏乱序
static char base32_en[] =
{
	'P', '7', 'X', 'E', 'L',
	'S', 'F', 'T',
	'W', 'R', 'K', 'H', 'A', 'D',
	'M', '8', 'N', '3', 'Y',
	'B', 'C', 'G', 'J', 'Q', 'U', 'V', 'Z',
	'2', '4', '5', '6', '9', 
};
// 这个可以通过outarray32自动生成
static unsigned char base32_de[] =
{
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,   27,   17,   28,   29,   30,    1,   15,   31,    0,    0,    0,    0,    0,    0, 
	   0,   12,   19,   20,   13,    3,    6,   21,   11,    0,   22,   10,    4,   14,   16,    0, 
	   0,   23,    9,    5,    7,   24,   25,    8,    2,   18,   26,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
};
unsigned char	to_base32(int n)
{
	unsigned char	i;
	for(i=0;i<32;i++)
	{
		if( base32_en[i] == n )
		{
			return	i;
		}
	}
	return	0;
}
void	outarray32(char *__file)
{
	FILE	*fp;
	int	i, count;
	fp	= fopen(__file, "w");

	fprintf(fp, "static unsigned char base32_de[] =\n{");
	count	= 0;
	for(i=0;i<256;i++)
	{
		if(!count)
		{
			fprintf(fp, "\n\t");
		}
		count	= (count + 1) % 16;
		unsigned char	c;
		c	= to_base32(i);
		if(c==255)
		fprintf(fp, "   0, ");
		else
		fprintf(fp, "%4d, ", c);
	}
	fprintf(fp, "\n};\n");

	fclose(fp);
}
int	base32_line_decode_old(const char *_in, unsigned char *_out)
{
	int		offset, i;
	unsigned char	a;

	offset	= 0;
	i		= 0;
	while( (a=_in[i])>0 )
	{
		offset	= whbit_append((char *)_out, offset, (char *)&base32_de[a], 0, 5);
		i ++;
	}
	return	offset/8;
}
int	base32_line_encode_old(const unsigned char *_in, int __inlen, char *_out)
{
	int		bits, i, totalbits;
	unsigned char	a;

	totalbits	= __inlen * 8;
	bits	= 0;
	i		= 0;
	a		= 0;
	while(bits<totalbits)
	{
		int	nBits	= totalbits-bits;
		if( nBits>5 )
		{
			nBits	= 5;
		}
		whbit_append((char *)&a, 0, (char *)_in, bits, nBits);
		bits	+= nBits;
		*_out	= base32_en[a];
		_out	++;
		i		++;
	}
	*_out		= 0;
	return		i;
}


// 从256进制向32进制转换
int whbit_256_to_32(
					const unsigned char*	p256,		//源串 
					int						n256len,	//源串长度(字节)
					char*					p32,		//目的串 
					int						n32len,		//目的串的长度(字节)
					const char*				cszChars	//转换对照模板
					)
{
	unsigned short	src_short = 0 ;

	int src_byte_no	= 1;					//下一个需要处理的源byte号

	int dst_byte_no	= 0 ;					//目的字节号数

	int	nBitLeft	= 8;					//剩余没有处理的bit数

	// 拷贝起始的数据到short
	src_short	= p256[0];

	for(;;)
	{
		p32[dst_byte_no]	=	cszChars[(src_short	& 0x1F)];
		dst_byte_no			++;
		if( dst_byte_no>=n32len )
		{
			// 目标缓冲不够了
			return	-1;
		}
		src_short			>>= 5;
		nBitLeft			-= 5;
		if( nBitLeft<8 )
		{
			if( nBitLeft<=0 )
			{
				// 说明所有的都已经转换好了
				break;
			}
			// 增加新字节进来
			if( src_byte_no<n256len )
			{
				src_short	|= ((unsigned short)p256[src_byte_no])<<nBitLeft;
				src_byte_no	++;
				nBitLeft	+= 8;
			}
		}
	}
	// 字串0结尾
	p32[dst_byte_no]	= 0;

	return n32len;
}

// 从32进制向256进制转换
int	whbit_32_to_256(
					const	char*	p32,
					int		n32len,
					unsigned	char*	p256,
					int		n256len,
					const unsigned	char*	cszCharsTo256
					)
{
	unsigned short	dst_short	= 0;

	int		nBitFinished		= 0;				//short中已经处理的bit数

	int		dst_byte_no			= 0;				//目标字节号
	int		src_byte_no			= 0;				//来源字节号

	for(;;)
	{
		dst_short				|= ((unsigned short)(cszCharsTo256[ p32[src_byte_no] ])) << nBitFinished;
		nBitFinished			+= 5;
		src_byte_no				++;
		if( nBitFinished>=8 )
		{
			p256[dst_byte_no]	= (unsigned char)dst_short;
			dst_byte_no			++;
			if( dst_byte_no>n256len )
			{
				// 缓冲不够了
				return	-1;
			}
			dst_short			>>= 8;
			nBitFinished		-= 8;
		}
		if( src_byte_no>=n32len )
		{
			// 数据已经处理完了
			break;
		}
	}
	if( nBitFinished>0 )
	{
		// 最后一点
		p256[dst_byte_no]		= (unsigned char)dst_short;
		dst_byte_no				++;
	}
	return	dst_byte_no;
}
int	base32_line_decode(const char *_in, unsigned char *_out, int __outlen)
{
//	return	base32_line_decode_old(_in, _out);
	return	whbit_32_to_256(_in, strlen(_in), _out, __outlen, base32_de);
}
int	base32_line_encode(const unsigned char *_in, int __inlen, char *_out, int __outlen)
{
//	return	base32_line_encode_old(_in, __inlen, _out);
	return	whbit_256_to_32(_in, __inlen, _out, __outlen, base32_en);
}

}	// EOF namespace n_whcmn
