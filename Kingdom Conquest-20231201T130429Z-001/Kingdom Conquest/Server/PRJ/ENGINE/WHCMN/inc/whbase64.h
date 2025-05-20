// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbase64.h
// Creator      : Wei Hua (κ��)
// Comment      : base64����
// CreationDate : 2001-10
// Modify  Date : 2003-05-23

#ifndef __WHBASE64_H__
#define __WHBASE64_H__

namespace n_whcmn
{

/////////////////////////////////////////////////////////////////////////////
// ��������
/////////////////////////////////////////////////////////////////////////////
// ���ֽڱ�ɶ������ִ�
void	tobinary(unsigned char _in, char *_out);
// ������ʾ��base64�ַ����0~63
unsigned char	to_base64(unsigned char __c);
// ��0~63��ɿ���ʾ��base64�ַ�
unsigned char	to_base64_char(unsigned char __c);
// ���base64��Ӧ�����飬�μ�base64.cpp�е�base64_en��base64_de
void	outarray64(char *__file);

/////////////////////////////////////////////////////////////////////////////
// codec
/////////////////////////////////////////////////////////////////////////////
// base64
// ���룺_in�е��ַ���һ����4��������
// ���ؽ��볤��
int	base64_decode(char *_in, unsigned char *_out);
// ���룺_out�ĳ���һ����4��������
// ���ر��볤��
int	base64_encode(unsigned char *_in, int __inlen, char *_out);

// !!!!ע�⣺������������CDKEY������ʹ�ã����Բ������Ķ�!!!!
// ���е�base32��/����
// ���룺_in�е��ַ�������ν
// ���ؽ��볤��(�ֽ�)
int	base32_line_decode_old(const char *_in, unsigned char *_out);
int	base32_line_decode(const char *_in, unsigned char *_out, int __outlen);
// ���룺_out�ĳ���һ����4��������
// ���ر��볤��
int	base32_line_encode_old(const unsigned char *_in, int __inlen, char *_out);
int	base32_line_encode(const unsigned char *_in, int __inlen, char *_out, int __outlen);
// ���base32��Ӧ�����飬�μ�base64.cpp�е�base32_en��base32_de
void	outarray32(char *__file);

}	// EOF namespace n_whcmn

#endif	// EOF __WHBASE64_H__
