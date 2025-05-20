// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmd5.h
// Creator      : Wei Hua (魏华)
// Comment      : MD5校验（利用网上的源码，作者: Aladdin Enterprises）
//                对于长度为0的数据，计算出md5为16个0
// CreationDate : 2003-12
// ChangeLOG    : 2006-02-05 把whmd5的数据类型从const char *改为const void *这样很多地方会写的方便一些

/*
  本代码拷贝自：
  Copyright (C) 1999, 2000, 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

  $Id: whmd5.h,v 1.1 2005/06/17 03:49:14 wh Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
	http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.c is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2002-04-13 lpd Clarified derivation from RFC 1321; now handles byte order
	either statically or dynamically; added missing #include <string.h>
	in library.
  2002-03-11 lpd Corrected argument list for main(), and added int return
	type, in test program and T value program.
  2002-02-21 lpd Added missing #include <stdio.h> in test program.
  2000-07-03 lpd Patched to eliminate warnings about "constant is
	unsigned in ANSI C, signed in traditional"; made test program
	self-checking.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5).
  1999-05-03 lpd Original version.
*/

#ifndef __WHMD5_H__
#define __WHMD5_H__

#include <string.h>

namespace n_whcmn
{

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */
enum
{
	WHMD5LEN	= 16,
};

typedef unsigned char	md5_byte_t;	/* 8-bit byte */
typedef unsigned int	md5_word_t;	/* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;

// Initialize the algorithm.
void md5_init(md5_state_t *pms);

// Append a string to the message.
void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);

// Finish the message and return the digest.
void md5_finish(md5_state_t *pms, md5_byte_t digest[WHMD5LEN]);

// 获得MD5二进制串（szDst must be at least 16bytes）
unsigned char *	whmd5(const void *szSrc, size_t nSrcSize, unsigned char *szDst);
// 获得MD5字串（szDst must be at least 32+1bytes）
char *	whmd5str(const void *szSrc, size_t nSrcSize, char *szDst);
// 用函数里的静态字串变量
char *	whmd5str(const void *szSrc, size_t nSrcSize);

// 持续计算md5的对象
class	whmd5
{
private:
	md5_state_t		m_ms;
	unsigned char	m_MD5[WHMD5LEN];
public:
	whmd5()
	{
		md5_init(&m_ms);
	}
	inline const unsigned char *	GetMD5() const
	{
		return	m_MD5;
	}
	inline void	Add(const void *pData, int nBytes)
	{
		md5_append(&m_ms, (const md5_byte_t *)pData, nBytes);
	}
	inline const unsigned char *	Finish()
	{
		md5_finish(&m_ms, m_MD5);
		return	m_MD5;
	}
};

}	// EOF namespace n_whcmn

#endif	// EOF __WHMD5_H__
