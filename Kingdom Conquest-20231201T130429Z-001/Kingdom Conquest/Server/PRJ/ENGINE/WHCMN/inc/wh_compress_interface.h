// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_compress_interface.h
// Creator     : Wei Hua (魏华)
// Comment     : 压缩接口的定义
// CreationDate: 2006-07-14
// ChangeLOG   :

#ifndef	__WH_COMPRESS_INTERFACE_H__
#define	__WH_COMPRESS_INTERFACE_H__

#include <stdlib.h>

namespace n_whcmn
{

class	WHCompress
{
protected:
	virtual	~WHCompress()											{};
public:
	// compress和decompress都是返回0成功，<0失败。pSrc和pDst一般都不能是相同的缓冲，否则可能会发生数据混乱!!!!	
	enum
	{
		RST_OK							= 0,
		// 下面的错误定义来自CryptoPP::Exception::ErrorType
		//! a method is not implemented
		RST_ERR_NOT_IMPLEMENTED			= -11,
		//! invalid function argument
		RST_ERR_INVALID_ARGUMENT		= -12,
		//! BufferedTransformation received a Flush(true) signal but can't flush buffers
		RST_ERR_CANNOT_FLUSH			= -13,
		//! data integerity check (such as CRC or MAC) failed
		RST_ERR_DATA_INTEGRITY_CHECK_FAILED	= -14,
		//! received input data that doesn't conform to expected format
		RST_ERR_INVALID_DATA_FORMAT		= -15,
		//! error reading from input device or writing to output device
		RST_ERR_IO_ERROR				= -16,
		//! some error not belong to any of the above categories
		RST_ERR_OTHER_ERROR				= -100
	};
	// *pnDstSize中是pDst缓冲最大可以承载的数据量
	virtual	int	compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
																	= 0;
	virtual	int	decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
																	= 0;
	// 调用这个函数是比较保险的做法
	virtual	void	SelfDestroy()									= 0;
};

// 创建一个zlib的压缩对象
WHCompress *	WHCompress_Create_ZLib();
WHCompress *	WHCompress_Create_GZip();

}		// EOF namespace n_whcmn

#endif	// EOF __WH_COMPRESS_INTERFACE_H__
