// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_compress_interface.h
// Creator     : Wei Hua (κ��)
// Comment     : ѹ���ӿڵĶ���
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
	// compress��decompress���Ƿ���0�ɹ���<0ʧ�ܡ�pSrc��pDstһ�㶼��������ͬ�Ļ��壬������ܻᷢ�����ݻ���!!!!	
	enum
	{
		RST_OK							= 0,
		// ����Ĵ���������CryptoPP::Exception::ErrorType
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
	// *pnDstSize����pDst���������Գ��ص�������
	virtual	int	compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
																	= 0;
	virtual	int	decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize)
																	= 0;
	// ������������ǱȽϱ��յ�����
	virtual	void	SelfDestroy()									= 0;
};

// ����һ��zlib��ѹ������
WHCompress *	WHCompress_Create_ZLib();
WHCompress *	WHCompress_Create_GZip();

}		// EOF namespace n_whcmn

#endif	// EOF __WH_COMPRESS_INTERFACE_H__
