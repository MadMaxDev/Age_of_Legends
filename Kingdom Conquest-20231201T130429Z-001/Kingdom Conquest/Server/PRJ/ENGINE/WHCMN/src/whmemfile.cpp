// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfile.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的内存文件功能
// CreationDate : 2004-01-19
// ChangeLOG    : 2007-04-27 修正了whmemfile::Write中没有判断内存块是否申请成功，导致文件内容丢失的错误。

#include "../inc/whmemfile.h"
#include "../inc/whcmn_def.h"
#include <stdio.h>

using namespace n_whcmn;

whmemfile::whmemfile()
: m_pMan(NULL), m_pFileInfo(NULL)
, m_nOpenMode(0), m_nCurOffset(0), m_nCurBlockIdx(whalist::INVALIDIDX)
{
}
whmemfile::~whmemfile()
{
}
size_t	whmemfile::Read(void *pBuf, size_t nSize)
{
	// 用char*好计算偏移
	char	*szBuf	= (char *)pBuf;
	// 记录已经读到的尺寸
	size_t	nReadSize	= 0;
	// nSize不能超过当前位置到文件结尾的距离
	int		nDistFromTail = m_pFileInfo->nSize - m_nCurOffset;
	assert( nDistFromTail>=0 );
	if( nDistFromTail < (int)nSize )
	{
		nSize	= nDistFromTail;
	}

	while( nSize>0 )
	{
		const char	*szFrag		= (const char *)m_pMan->m_alFragInfo.GetDataUnitPtr(m_nCurBlockIdx);
		if( !szFrag )
		{
			// 应该已经到了文件尾了
			break;
		}
		// 局部偏移
		int		nLocalOffset	= m_nCurOffset%m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
		// 判断在当前块中可以拷贝的最大长度
		size_t	nSizeToCopy		= m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize - nLocalOffset;
		if( nSize < nSizeToCopy )
		{
			// 这就说明不需要下一块了
			// 注意：上面的“<”不能是“<=”，因为==的时候文件指针刚好会指向下个块的开头
			nSizeToCopy	= nSize;
		}
		else	// nSize >= nSizeToCopy
		{
			// 说明本块用完了，而且需要下一块
			m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		}
		// 获取拷贝的源指针
		szFrag					+= nLocalOffset;
		// 拷贝之
		memcpy(szBuf, szFrag, nSizeToCopy);

		// 新的总偏移
		m_nCurOffset			+= nSizeToCopy;
		// 有待拷贝的数据的起始指针
		szBuf					+= nSizeToCopy;
		// 已经读入的长度
		nReadSize				+= nSizeToCopy;
		// 得到还需要发送的长度
		nSize					-= nSizeToCopy;
	}

	return	nReadSize;
}
size_t	whmemfile::Write(const void *pBuf, size_t nSize)
{
	// 0比较特殊，就直接判断了
	if( nSize==0 )
	{
		return	0;
	}

	// 判断文件是否是可写打开的
	switch( m_nOpenMode )
	{
		case	whmemfileman::MODE_CREATE:
		case	whmemfileman::MODE_READWRITE:
		break;
		default:
			// 非写打开，不能写入
			assert(0);
			return	0;
		break;
	}

	// 用char*好计算偏移
	const char	*szBuf	= (const char *)pBuf;
	// 记录已经写入的尺寸
	size_t	nWriteSize	= 0;

	while( nSize>0 )
	{
		if( m_nCurBlockIdx == whalist::INVALIDIDX )
		{
			// 如果m_nCurBlockIdx指向INVALIDIDX则需要申请新的块
			m_nCurBlockIdx	= m_pMan->m_alFragInfo.Alloc();
			// 2007-04-27，判断申请是否成功
			if( m_nCurBlockIdx == whalist::INVALIDIDX )
			{
				// 返回错误
				return	0;
			}
			if( m_pFileInfo->nTailIdx == whalist::INVALIDIDX )
			{
				// 说明原来文件是空的，所以要把头也置上
				m_pFileInfo->nHeadIdx	= m_nCurBlockIdx;
			}
			else
			{
				// 将新块添加到原文件块链尾
				 m_pMan->m_alFragInfo.SetNextOf(m_pFileInfo->nTailIdx, m_nCurBlockIdx);
			}
			// 置新的尾
			m_pFileInfo->nTailIdx		= m_nCurBlockIdx;
		}
		// 找到当前块
		char	*szFrag			= (char *)m_pMan->m_alFragInfo.GetDataUnitPtr(m_nCurBlockIdx);
		if( !szFrag )
		{
			// 应该是前面无法分配了
			break;
		}
		// 局部偏移
		int		nLocalOffset	= m_nCurOffset%m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
		// 判断在当前块中可以拷贝的最大长度
		size_t	nSizeToCopy		= m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize - nLocalOffset;
		if( nSize < nSizeToCopy )
		{
			// 这就说明不需要下一块了
			// 注意：上面的“<”不能是“<=”，因为==的时候文件指针刚好会指向下个块的开头
			nSizeToCopy	= nSize;
		}
		else	// nSize >= nSizeToCopy
		{
			// 说明本块用完了，而且需要下一块
			m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		}
		// 获取拷贝的目的指针
		szFrag					+= nLocalOffset;
		// 拷贝之
		memcpy(szFrag, szBuf, nSizeToCopy);

		// 新的总偏移
		m_nCurOffset			+= nSizeToCopy;
		// 有待拷贝的数据的起始指针
		szBuf					+= nSizeToCopy;
		// 已经写入的长度
		nWriteSize				+= nSizeToCopy;
		// 得到还需要发送的长度
		nSize					-= nSizeToCopy;
	}

	// 如果offset超过了当前文件尺寸则改变文件尺寸为这个数
	if( m_nCurOffset > (int)m_pFileInfo->nSize )
	{
		m_pFileInfo->nSize	= m_nCurOffset;
	}

	// 返回
	return	nWriteSize;
}
int		whmemfile::Seek(int nOffset, int nOrigin)
{
	// 如果文件尺寸为0，则不能seek
	if( m_pFileInfo->nTailIdx == whalist::INVALIDIDX )
	{
		return	0;
	}

	// 计算目标文件指针的位置
	int		nDstOffset	= 0;

	switch(nOrigin)
	{
		case	SEEK_CUR:
			nDstOffset	= m_nCurOffset + nOffset;
		break;
		case	SEEK_SET:
			nDstOffset	= nOffset;
		break;
		case	SEEK_END:
			nDstOffset	= m_pFileInfo->nSize + nOffset;
		break;
		default:
			// 错误的参数，很可能是nOffset和nOrigin写反了
			assert(0);
		break;
	}
	if( nDstOffset<0 )
	{
		nDstOffset	= 0;
	}
	else if( nDstOffset>int(m_pFileInfo->nSize) )
	{
		nDstOffset	= m_pFileInfo->nSize;
	}

	m_nCurOffset	= nDstOffset;

	// 找到相应的block
	int				nBlock	= m_nCurOffset / m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
	m_nCurBlockIdx	= m_pFileInfo->nHeadIdx;
	for(int i=0;i<nBlock;i++)
	{
		m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		// 如果文件长度刚好是单个块长度的倍数，则在结束时m_nCurBlockIdx将指向INVALIDIDX
		// 这个刚好表示EOF，而且在Write的时候可以用通用的方法申请新块
	}

	return			0;
}
size_t	whmemfile::ReadToVector(whvector<char> &vect)
{
	vect.resize( GetFileSize() - Tell() );
	return	Read(vect.getbuf(), vect.size());
}
int		whmemfile::CmpWithData(const void *pData, int nSize)
{
	char	szBuf[4096];
	char	*pszData	= (char *)pData;

	// 如果长度不同则可以直接返回
	int		nRst		= GetFileSize() - nSize;
	if( nRst!=0 )
	{
		return	nRst;
	}

	while( !IsEOF() && nSize>0 )
	{
		int	nReadSize	= Read(szBuf, sizeof(szBuf));
		if( nReadSize==0 )
		{
			break;
		}
		int	nRst		= memcmp(szBuf, pszData, nReadSize);
		if( nRst!=0 )
		{
			// 这样返回还可以反映一些内存差异的状况，呵呵，不过没有太大必要
			return	nRst;
		}
		// 还需要比较字节数
		nSize			-= nReadSize;
		pszData			+= nReadSize;
	}
	if( nSize!=0 )
	{
		return	-1;
	}
	return	0;
}
