#ifndef __MyMsger_H__
#define __MyMsger_H__

#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/tty_common_BD4Web.h"

extern bool g_bNBO;

using namespace n_whnet;

class MyMsger : public tcpmsgerbase
{
	virtual	int		CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
	{
		*pnUnitLen		= 0;
		if (nSize < MIN_BD_CMD_SIZE)
		{
			return CHECKUNIT_RST_NA;
		}
		BD_CMD_BASE_T*	pBaseCmd	= (BD_CMD_BASE_T*)pBegin;
		bd_size_t		nPacketSize	= 0;
// 		if (g_bNBO)
// 		{
// 			nPacketSize	= ntohl(pBaseCmd->nSize);
// 		}
// 		else
		{
			nPacketSize	= pBaseCmd->nSize;
		}
		if (nPacketSize > MAX_BD_CMD_SIZE)
		{
			return CHECKUNIT_RST_ERR;
		}
		if (nPacketSize <= nSize)
		{
			*pnUnitLen	= nPacketSize;
			return CHECKUNIT_RST_OK;
		}
		return CHECKUNIT_RST_NA;
	}
	virtual size_t	GetPackUnitSize(size_t nDSize)
	{
		return nDSize;
	}
	virtual	int		PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
	{
		memcpy(pDstBuf, pRawData, nDSize);
		return 0;
	}
	virtual	const void*	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
	{
		*pnDSize	= nTotalSize;
		return pPackedData;
	}
};

#endif
