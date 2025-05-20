// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_compress_interface.cpp
// Creator     : Wei Hua (魏华)
// Comment     : 压缩接口的定义
// CreationDate: 2006-07-14
// ChangeLOG   :

#include "../inc/wh_compress_interface.h"
#include <cryptlib/zlib.h>
#include <cryptlib/gzip.h>

namespace n_whcmn
{

// 错误码转换
int	CryptoPP_Exception_ErrorType_Translate(CryptoPP::Exception::ErrorType nErr)
{
	switch(nErr)
	{
	case	CryptoPP::Exception::NOT_IMPLEMENTED:
		{
			return	WHCompress::RST_ERR_NOT_IMPLEMENTED;
		}
		break;
	case	CryptoPP::Exception::INVALID_ARGUMENT:
		{
			return	WHCompress::RST_ERR_INVALID_ARGUMENT;
		}
		break;
	case	CryptoPP::Exception::CANNOT_FLUSH:
		{
			return	WHCompress::RST_ERR_CANNOT_FLUSH;
		}
		break;
	case	CryptoPP::Exception::DATA_INTEGRITY_CHECK_FAILED:
		{
			return	WHCompress::RST_ERR_DATA_INTEGRITY_CHECK_FAILED;
		}
		break;
	case	CryptoPP::Exception::INVALID_DATA_FORMAT:
		{
			return	WHCompress::RST_ERR_INVALID_DATA_FORMAT;
		}
		break;
	case	CryptoPP::Exception::IO_ERROR:
		{
			return	WHCompress::RST_ERR_IO_ERROR;
		}
		break;
	default:
		return	WHCompress::RST_ERR_OTHER_ERROR;
	}
}

class	WHCompress_ZLib	: public WHCompress
{
private:
	CryptoPP::ZlibCompressor	m_zc;
	CryptoPP::ZlibDecompressor	m_zd;

public:
	WHCompress_ZLib()
		: m_zd(NULL, true)
	{
	}
	~WHCompress_ZLib()
	{
	}
	virtual	int	compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
	{
		if( nSrcSize==0 )
		{
			*pnDstSize	= 0;
			return	WHCompress::RST_OK;
		}
		assert(pSrc!=pDst);

		CryptoPP::ZlibCompressor	&zc	= m_zc;
		try
		{
			zc.Detach();
			zc.Put2( (const byte *)pSrc, nSrcSize, 1, true );
			*pnDstSize	= zc.Get( (byte *)pDst, *pnDstSize );
		}
		catch( CryptoPP::Exception &e )
		{
			return	CryptoPP_Exception_ErrorType_Translate(e.GetErrorType());
		}
		return	WHCompress::RST_OK;
	}
	virtual	int	decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
	{
		if( nSrcSize==0 )
		{
			*pnDstSize	= 0;
			return	WHCompress::RST_OK;
		}
		assert(pSrc!=pDst);

		CryptoPP::ZlibDecompressor	&zd	= m_zd;
		try
		{
			zd.Detach();
			zd.Put2( (const byte *)pSrc, nSrcSize, 1, true );
			*pnDstSize	= zd.Get( (byte *)pDst, *pnDstSize );
		}
		catch( CryptoPP::Exception &e )
		{
			return	CryptoPP_Exception_ErrorType_Translate(e.GetErrorType());
		}
		return	WHCompress::RST_OK;
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
};

class	WHCompress_GZip	: public WHCompress
{
public:
	virtual	int	compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
	{
		if( nSrcSize==0 )
		{
			*pnDstSize	= 0;
			return	WHCompress::RST_OK;
		}
		assert(pSrc!=pDst);
		CryptoPP::Gzip		zc;
		try
		{
			zc.Put2( (const byte *)pSrc, nSrcSize, -1, true );
			*pnDstSize	= zc.Get( (byte *)pDst, *pnDstSize );
		}
		catch( CryptoPP::Exception &e )
		{
			return	CryptoPP_Exception_ErrorType_Translate(e.GetErrorType());
		}
		return	WHCompress::RST_OK;
	}
	virtual	int	decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
	{
		if( nSrcSize==0 )
		{
			*pnDstSize	= 0;
			return	WHCompress::RST_OK;
		}
		assert(pSrc!=pDst);
		CryptoPP::Gunzip	zd;
		try
		{
			zd.Put2( (const byte *)pSrc, nSrcSize, -1, true );
			*pnDstSize	= zd.Get( (byte *)pDst, *pnDstSize );
		}
		catch( CryptoPP::Exception &e )
		{
			return	CryptoPP_Exception_ErrorType_Translate(e.GetErrorType());
		}
		return	WHCompress::RST_OK;
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
};

WHCompress *	WHCompress_Create_ZLib()
{
	return	new WHCompress_ZLib;
}
WHCompress *	WHCompress_Create_GZip()
{
	return	new WHCompress_GZip;
}

}		// EOF namespace n_whcmn
