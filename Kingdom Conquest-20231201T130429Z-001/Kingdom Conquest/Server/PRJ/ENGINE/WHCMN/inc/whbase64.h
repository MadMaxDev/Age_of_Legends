// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbase64.h
// Creator      : Wei Hua (魏华)
// Comment      : base64编码
// CreationDate : 2001-10
// Modify  Date : 2003-05-23

#ifndef __WHBASE64_H__
#define __WHBASE64_H__

namespace n_whcmn
{

/////////////////////////////////////////////////////////////////////////////
// 辅助函数
/////////////////////////////////////////////////////////////////////////////
// 将字节变成二进制字串
void	tobinary(unsigned char _in, char *_out);
// 将可显示的base64字符变成0~63
unsigned char	to_base64(unsigned char __c);
// 将0~63变成可显示的base64字符
unsigned char	to_base64_char(unsigned char __c);
// 输出base64对应表数组，参见base64.cpp中的base64_en和base64_de
void	outarray64(char *__file);

/////////////////////////////////////////////////////////////////////////////
// codec
/////////////////////////////////////////////////////////////////////////////
// base64
// 解码：_in中的字符数一定是4的整倍数
// 返回解码长度
int	base64_decode(char *_in, unsigned char *_out);
// 编码：_out的长度一定是4的整倍数
// 返回编码长度
int	base64_encode(unsigned char *_in, int __inlen, char *_out);

// !!!!注意：这两个函数在CDKEY生成中使用，所以不能随便改动!!!!
// 单行的base32编/解码
// 解码：_in中的字符数无所谓
// 返回解码长度(字节)
int	base32_line_decode_old(const char *_in, unsigned char *_out);
int	base32_line_decode(const char *_in, unsigned char *_out, int __outlen);
// 编码：_out的长度一定是4的整倍数
// 返回编码长度
int	base32_line_encode_old(const unsigned char *_in, int __inlen, char *_out);
int	base32_line_encode(const unsigned char *_in, int __inlen, char *_out, int __outlen);
// 输出base32对应表数组，参见base64.cpp中的base32_en和base32_de
void	outarray32(char *__file);

}	// EOF namespace n_whcmn

#endif	// EOF __WHBASE64_H__
