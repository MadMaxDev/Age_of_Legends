// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfileman.cpp
// Creator      : Wei Hua (魏华)
// CreationDate : 2004-01-18

// 0 原来给whmemfileman::Open用。现在不用了。

#include "../inc/whmemfile.h"
#include "../inc/whcmn_def.h"
#include "../inc/whcrc.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whmemfileman
////////////////////////////////////////////////////////////////////
size_t	whmemfileman::CalcMemSizeNeeded(INFO_T *pInfo)
{
	return	sizeof(MEMINFO_T)
			+ whalist::CalcTotalSize(sizeof(FILEINFO_T), pInfo->nMaxFile)
			+ whalist::CalcTotalSize(pInfo->nFragSize, pInfo->nTotalFrag)
			;
}

whmemfileman::whmemfileman()
: m_pMemHdr(NULL)
{
}
whmemfileman::~whmemfileman()
{
}

int	whmemfileman::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));

	m_pMemHdr				= (MEMINFO_T *)m_info.pBuf;
	// 获得两个list管理的内存相对头的偏移
	m_pMemHdr->offFileSect	= sizeof(MEMINFO_T);
	m_pMemHdr->offFragSect	= m_pMemHdr->offFileSect
							+ whalist::CalcTotalSize(sizeof(FILEINFO_T), m_info.nMaxFile);
	// 初始化两个list
	m_alFileInfo.Init(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFileSect), sizeof(FILEINFO_T), m_info.nMaxFile);
	m_alFragInfo.Init(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFragSect), m_info.nFragSize, m_info.nTotalFrag);
	//
	m_mapFileName2Idx.clear();

	Init_Ext();

	return	0;
}
int	whmemfileman::Inherit(void *pBuf)
{
	m_pMemHdr				= (MEMINFO_T *)pBuf;
	if( IsMarkedClean() )
	{
		return	-1;
	}
	// 继承两个list
	m_alFileInfo.Inherit(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFileSect));
	m_alFragInfo.Inherit(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFragSect));
	m_info.pBuf				= pBuf;
	m_info.nMaxFile			= m_alFileInfo.GetHdr()->nMaxNum;
	m_info.nFragSize		= m_alFragInfo.GetHdr()->nDataUnitSize;
	m_info.nTotalFrag		= m_alFragInfo.GetHdr()->nMaxNum;
	//
	m_mapFileName2Idx.clear();
	// 子类的附加操作
	Inherit_Ext();

	// 浏览文件list，初始化hash映射
	size_t	i;
	const whalist::HDR_T	*plHdr = m_alFileInfo.GetHdr();
	whvector<char>			vectbuf;
	for(i=0;i<plHdr->nMaxNum;i++)
	{
		FILEINFO_T	*pFileInfo = GetFileInfoByIdx(i);
		if( pFileInfo )
		{
			bool	bGood = true;
			// 判断是否是已经写打开的
			if( pFileInfo->nProperty & FILEINFO_T::PROPERTY_WR )
			{
				// 如果是写打开的，说明文件内容可能有错误，需要进行一下CRC校验
				if( CalcCRC32(pFileInfo) != pFileInfo->nCRC32 )
				{
					// 真的是错误了(在写的过程中出错，文件内容应该有问题)，应该删除该文件
					bGood	= false;
				}
				else
				{
					// 内容正确，将property清空
					pFileInfo->nProperty	= 0;
				}
			}
			if( bGood )
			{
				int	rst	= Inherit_AfterGood(pFileInfo, i);
				if( rst<0 )
				{
					return	-100 + rst;
				}
			}
			else
			{
				// 删除这个被破坏的文件
				int	rst	= DeleteFile(pFileInfo);
				if( rst<0 )
				{
					return	-200 + rst;
				}
			}
		}
	}
	return	0;
}

bool	whmemfileman::IsFileExist(int nFileName) const
{
	return	m_mapFileName2Idx.has(nFileName);
}
int		whmemfileman::GetFileSize(int nFileName)
{
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// 没有这样的文件
		return	-1;
	}
	FILEINFO_T	*pFileInfo	= GetFileInfoByIdx(nIdx);
	assert(pFileInfo);
	return	pFileInfo->nSize;
}
whmemfileman::FILEINFO_T *	whmemfileman::GetFileInfo(int nFileName)
{
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// 没有这样的文件
		return	NULL;
	}
	return	GetFileInfoByIdx(nIdx);
}

whmemfile *	whmemfileman::Open(int nFileName, int nMode)
{
	// 先看文件是否是已经存在的
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// 没有这样的文件
		nIdx	= whalist::INVALIDIDX;
	}

	return	OpenByIdx(nIdx, nMode, (void *)nFileName);
}
int		whmemfileman::Close(whmemfile *pFile)
{
	if( !pFile )
	{
		// 已经关闭了，不算错误
		return	0;
	}
	// 改变
	switch( pFile->m_nOpenMode )
	{
		case	MODE_READWRITE:
		case	MODE_CREATE:
			// 清除写标志
			pFile->m_pFileInfo->nProperty	&= ~FILEINFO_T::PROPERTY_WR;
			// 重新计算CRC (应该是改变了，用写打开不写是SB)
			pFile->m_pFileInfo->nCRC32		= CalcCRC32(pFile->m_pFileInfo);
			// 记录更改的时间
			pFile->m_pFileInfo->nLastModify	= wh_time();
		break;
	}
	// 删除对象
	delete	pFile;
	return	0;
}
int		whmemfileman::Delete(int nFileName)
{
	// 找到nFileName对应的Idx
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// 没有这样的文件
		return	-1;
	}
	// 删除之
	return	DeleteByIdx(nIdx);
}
int		whmemfileman::MarkDelAndClean(int nFileName)
{
	// 找到nFileName对应的Idx
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// 没有这样的文件
		return	-1;
	}
	FILEINFO_T	*pFileInfo	= GetFileInfoByIdx(nIdx);
	if( pFileInfo )
	{
		// 删除文件内容
		EmptyFile(pFileInfo);
		// 标记删除
		pFileInfo->nProperty	|= FILEINFO_T::PROPERTY_DEL;
		return	0;
	}
	// 这说明数据不同步了
	assert(0);
	return	-1;
}
void	whmemfileman::GetStatistic(STATISTIC_T *pStat) const
{
	pStat->nFileNum		= m_alFileInfo.GetHdr()->nCurNum;
	pStat->nFileMax		= m_alFileInfo.GetHdr()->nMaxNum;
	pStat->nFragNum		= m_alFragInfo.GetHdr()->nCurNum;
	pStat->nFragMax		= m_alFragInfo.GetHdr()->nMaxNum;
}
void	whmemfileman::GetFileNameList(whvector<int> &vectList) const
{
	// 能到这一步应该已经是初始化过的了，所以hash表存在
	m_mapFileName2Idx.exporttovector(vectList);
}

unsigned	whmemfileman::CalcCRC32(FILEINFO_T * pFileInfo) const
{
	// 一段一段计算crc
	int			nIdx	= pFileInfo->nHeadIdx;
	unsigned	iseed	= (unsigned)0xffffffff;
	int			nSize	= (int)pFileInfo->nSize;
	int			nUnitSize	= m_alFragInfo.GetHdr()->nDataUnitSize;
	unsigned	nCRC32	= 0;
	while( nIdx!=whalist::INVALIDIDX )
	{
		// 获取一段
		char	*pszBuf	= (char *)m_alFragInfo.GetDataUnitPtr(nIdx);
		if( nSize>=nUnitSize )
		{
			nCRC32	= custom_crc32(pszBuf, nUnitSize, iseed);
		}
		else
		{
			nCRC32	= custom_crc32(pszBuf, nSize, iseed);
		}
		// 判断是否到了结尾
		nSize	-= nUnitSize;
		if( nSize<=0 )
		{
			break;
		}
		// 下一段
		nIdx	= m_alFragInfo.GetNextOf(nIdx);
	}
	return	nCRC32;
}
int			whmemfileman::EmptyFile(FILEINFO_T *pFileInfo)
{
	// 能得到pFileInfo指针说明这是一个合法的指针，就不用多判断一次合法性了
	m_alFragInfo.FreeChain(pFileInfo->nHeadIdx);
	// 清空文件头内容
	pFileInfo->clear();
	return	0;
}
int			whmemfileman::DeleteFile(FILEINFO_T * pFileInfo)
{
	if( !pFileInfo )
	{
		assert(0);
		return	-1;
	}
	// 删除文件内容
	EmptyFile(pFileInfo);
	// 先删除其他的附加东西
	DeleteFile_BeforeRealDel(pFileInfo);
	// 删除文件信息单元
	m_alFileInfo.Free(pFileInfo->nIdx);
	return	0;
}
whmemfile *	whmemfileman::OpenFile(whmemfileman::FILEINFO_T *pFileInfo, int nMode, void *pName)
{
	// 判断打开方式
	if( !pFileInfo )
	{
		// 文件不存在(无法读)
		switch(nMode)
		{
			case	MODE_READ:
			case	MODE_READWRITE:
				return	NULL;
			break;
			case	MODE_CREATE:
			{
				// 申请一个新文件
				int	nIdx	= m_alFileInfo.Alloc();
				pFileInfo	= GetFileInfoByIdx(nIdx);
				if( !pFileInfo )
				{
					// 无法创建新文件，可能已经到达顶峰了
					return	NULL;
				}

				// 初始化
				pFileInfo->clear();
				// 这个只在创建的时候手动清一次 !!!! 2004-05-18 修改 !!!!
				memset(pFileInfo->aExtPtr, 0, sizeof(pFileInfo->aExtPtr));
				// 设置索引，这个在clear中是不会管的
				pFileInfo->nIdx		= nIdx;
				// 附加操作
				// 如：添加文件名映射
				Open_SetNameMap(pName, pFileInfo);
			}
			break;
		}
	}
	else
	{
		switch(nMode)
		{
			case	MODE_READWRITE:
			{
				// 原来属于写打开，则不能被写打开
				if( pFileInfo->nProperty & FILEINFO_T::PROPERTY_WR )
				{
					return	NULL;
				}
			}
			break;
			case	MODE_CREATE:
			{
				if( pFileInfo->nProperty & FILEINFO_T::PROPERTY_WR )
				{
					// 原来属于写打开，则不能以创建的方式打开
					return	NULL;
				}
				// 将原来的文件内容清空，后面就假定已经申请了一个空文件
				EmptyFile(pFileInfo);
			}
			break;
		}
	}
	// 上面应该已经保证pFileInfo非空了

	// 判断权限，添加标志位
	switch(nMode)
	{
		case	MODE_READWRITE:
		case	MODE_CREATE:
			pFileInfo->nProperty	|= FILEINFO_T::PROPERTY_WR;
		break;
		default:
		break;
	}

	// 生成文件对象，并初始化之
	whmemfile	*pFile		= new whmemfile;
	pFile->m_pMan			= this;
	pFile->m_pFileInfo		= pFileInfo;
	pFile->m_nOpenMode		= nMode;
	pFile->m_nCurOffset		= 0;
	pFile->m_nCurBlockIdx	= pFileInfo->nHeadIdx;
	return		pFile;
}

////////////////////////////////////////////////////////////////////
// whmemfilemanWithName
////////////////////////////////////////////////////////////////////
size_t	whmemfilemanWithName::CalcMemSizeNeeded(INFO_T *pInfo)
{
	return	whmemfileman::CalcMemSizeNeeded(pInfo)
			+ whalist::CalcTotalSize(sizeof(FILENAME_T), pInfo->nMaxFile)
			;
}

whmemfile *	whmemfilemanWithName::Open(const char *cszFileName, int nMode)
{
	// 先看文件是否是已经存在的
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// 没有这样的文件
		nIdx	= whalist::INVALIDIDX;
	}

	return	whmemfileman::OpenFile(GetFileInfoByIdx(nIdx), nMode, (void *)cszFileName);
}
int		whmemfilemanWithName::Delete(const char * cszFileName)
{
	// 先看文件是否是已经存在的
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// 没有这样的文件
		return	-1;
	}

	return	DeleteFile(GetFileInfoByIdx(nIdx));
}
bool	whmemfilemanWithName::IsFileExist(const char * cszFileName) const
{
	return	m_mapFileNameStr2Idx.has(cszFileName);
}
whmemfileman::FILEINFO_T *	whmemfilemanWithName::GetFileInfo(const char * cszFileName)
{
	// 先看文件是否是已经存在的
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// 没有这样的文件
		return	NULL;
	}

	return	GetFileInfoByIdx(nIdx);
}
void	whmemfilemanWithName::GetFileIdxList(whvector<int> &vectList) const
{
	m_mapFileNameStr2Idx.exportvaltovector(vectList);
}

int		whmemfilemanWithName::Init_Ext()
{
	// 再初始化本类的
	size_t	nFirstPartSize	= whmemfileman::CalcMemSizeNeeded();
	m_alFileName.Init(wh_getoffsetaddr(m_pMemHdr, nFirstPartSize), sizeof(FILENAME_T), m_info.nMaxFile);
	m_mapFileNameStr2Idx.clear();
	return	0;
}
int		whmemfilemanWithName::Inherit_Ext()
{
	// 找到自己的开头部分并继承
	size_t	nFirstPartSize	= whmemfileman::CalcMemSizeNeeded();
	m_alFileName.Inherit(wh_getoffsetaddr(m_pMemHdr, nFirstPartSize));
	m_mapFileNameStr2Idx.clear();
	return	0;
}
int		whmemfilemanWithName::Inherit_AfterGood(FILEINFO_T *pFInfo, int nIdx)
{
	// 应该可以保证nIdx == pFInfo->nIdx
	if( nIdx!=pFInfo->nIdx )
	{
		// 文件信息不一致
		assert(0);
		return	-1;
	}
	// 找到文件名单元
	FILENAME_T	*pFName	= (FILENAME_T *)m_alFileName.GetDataUnitPtr(nIdx);
	if( !pFName )
	{
		// 文件信息不一致
		assert(0);
		return	-2;
	}
	// 加入名字映射
	if( !m_mapFileNameStr2Idx.put(pFName->szFName, nIdx) )
	{
		// 名字以被使用，奇怪:(
		assert(0);
		return	-3;
	}
	return	0;
}
int		whmemfilemanWithName::Open_SetNameMap(void *pName, FILEINFO_T *pFInfo)
{
	const char	*cszName	= (const char *)pName;
	// 先申请得到新的FILENAME_T结构
	int	nIdx	= m_alFileName.Alloc();
	assert( nIdx == pFInfo->nIdx );
	FILENAME_T	*pFName	= GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		// 无法创建，这应该是不可能的(因为它和文件信息块是一致的)
		assert(0);
		return	-1;
	}
	pFName->clear();
	// 填写名字单元
	strcpy(pFName->szFName, cszName);
	// 设置名字映射
	m_mapFileNameStr2Idx.put(pFName->szFName, nIdx);
	return	0;
}
int		whmemfilemanWithName::DeleteFile_BeforeRealDel(FILEINFO_T *pFInfo)
{
	// 获得名字单元
	FILENAME_T	*pFName	= (FILENAME_T *)m_alFileName.GetDataUnitPtr(pFInfo->nIdx);
	if( !pFName )
	{
		return	-1;
	}
	// 删除名字映射
	m_mapFileNameStr2Idx.erase(pFName->szFName);
	// 释放名字单元
	m_alFileName.Free(pFInfo->nIdx);
	return	0;
}
