// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whzlib
// File         : whzlib.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 功能函数的实现
// CreationDate : 2003-09-05
// ChangeLOG    : 2006-01-23 使用CryptoPP中带的zlib实现，比原来效率提高1倍。Thanks to Wei Dai :)
//              : 2006-01-24 把Put2的第三个参数从-1改为1，因为它应该只需要用到一个filter。

#define	USE_ZLIB_CRYPTOCPP
//#define	USE_GZIP_CRYPTOCPP
//#define	USE_ZLIB
//#define	USE_LZO
//#define	USE_BZ2

#include "../inc/whzlib_i.h"
#include "../inc/whfile.h"
#ifdef	USE_ZLIB_CRYPTOCPP
#include <cryptlib/zlib.h>
#endif
#ifdef	USE_GZIP_CRYPTOCPP
#include <cryptlib/gzip.h>
#endif
#ifdef	USE_ZLIB
#include <zlib/zlib.h>
#endif
#ifdef	USE_LZO
#include <lzo/minilzo.h>
#endif
#ifdef	USE_BZ2
#include <bzip2/bzlib.h>
#endif

using namespace n_whcmn;

namespace n_whzlib
{

// 最底层压缩函数函数
#ifdef	USE_ZLIB_CRYPTOCPP
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	CryptoPP::ZlibCompressor	zc;
	try
	{
		zc.Put2( (const byte *)pSrc, nSrcSize, 1, true );
		*pnDstSize	= zc.Get( (byte *)pDst, *pnDstSize );
	}
	catch( CryptoPP::Exception &e )
	{
		return	-100 - e.GetErrorType();
	}
	return	0;
}
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	CryptoPP::ZlibDecompressor	zd;
	try
	{
		zd.Put2( (const byte *)pSrc, nSrcSize, 1, true );
		*pnDstSize	= zd.Get( (byte *)pDst, *pnDstSize );
	}
	catch( CryptoPP::Exception &e )
	{
		return	-100 - e.GetErrorType();
	}
	return	0;
}
#endif
#ifdef	USE_GZIP_CRYPTOCPP
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	CryptoPP::Gzip		zc;
	try
	{
		zc.Put2( (const byte *)pSrc, nSrcSize, -1, true );
		*pnDstSize	= zc.Get( (byte *)pDst, *pnDstSize );
	}
	catch( CryptoPP::Exception &e )
	{
		return	-100 - e.GetErrorType();
	}
	return	0;
}
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	CryptoPP::Gunzip	zd;
	try
	{
		zd.Put2( (const byte *)pSrc, nSrcSize, -1, true );
		*pnDstSize	= zd.Get( (byte *)pDst, *pnDstSize );
	}
	catch( CryptoPP::Exception &e )
	{
		return	-100 - e.GetErrorType();
	}
	return	0;
}
#endif
#ifdef	USE_ZLIB
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	return	compress( (Bytef *)pDst, (uLongf *)pnDstSize, (const Bytef *)pSrc, (uLong)nSrcSize);
}
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	return	uncompress( (Bytef *)pDst, (uLongf *)pnDstSize, (const Bytef *)pSrc, (uLong)nSrcSize);
}
#endif
#ifdef	USE_LZO
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	static char	szWork[LZO1X_MEM_COMPRESS];
	int		rst;
	rst		= lzo1x_1_compress((lzo_byte *)pSrc, nSrcSize, (lzo_byte *)pDst, pnDstSize, szWork);
	return	rst;
}
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	return	lzo1x_decompress((lzo_byte *)pSrc, nSrcSize, (lzo_byte *)pDst, pnDstSize, NULL);
}
#endif
#ifdef	USE_BZ2
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	int	rst	= BZ2_bzBuffToBuffCompress(
		(char *)pDst, pnDstSize, (char *)pSrc, nSrcSize
		, 9, 0, 0
		);
	return	rst;
}
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
{
	int	rst	= BZ2_bzBuffToBuffDecompress(
		(char *)pDst, pnDstSize, (char *)pSrc, nSrcSize
		, 0, 0
		);
	return	rst;
}
#endif

int		whzlib_single_compress(whzlib_file *file, const char *cszDstFile, const char *cszPasswd)
{
	return	whzlib_single_compress_i(file, cszDstFile, cszPasswd);
}
bool	whzlib_issinglecompressed(whzlib_file *srcfile)
{
	sc_file_hdr_t	hdr;
	return	whzlib_issinglecompressed_i(srcfile, &hdr);
}

whzlib_file *	whzlib_OpenSingleCompress(whzlib_file *srcfile, const char *cszPasswd)
{
	whzlib_file_i_sc	*pFile = new whzlib_file_i_sc;
	if( pFile )
	{
		pFile->SetPassword(cszPasswd);

		if( pFile->InitFrom(srcfile)==0 )
		{
		}
		else
		{
			delete	pFile;
			pFile	= NULL;
		}
	}
	return	pFile;
}
void			whzlib_CloseSingleCompress(whzlib_file *singlefile)
{
	// 2006-02-05 改为直接删除了
	delete	singlefile;
}

////////////////////////////////////////////////////////////////////
// whzlib_fileman
////////////////////////////////////////////////////////////////////
whzlib_fileman *		whzlib_fileman::Create()
{
	return	new whzlib_fileman_i;
}

////////////////////////////////////////////////////////////////////
// whzlib_filemodifyman
////////////////////////////////////////////////////////////////////
whzlib_filemodifyman *	whzlib_filemodifyman::Create()
{
	return	new whzlib_filemodifyman_i;
}


}		// EOF namespace n_whzlib
