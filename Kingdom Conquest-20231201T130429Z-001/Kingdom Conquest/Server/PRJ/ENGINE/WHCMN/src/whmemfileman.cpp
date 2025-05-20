// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfileman.cpp
// Creator      : Wei Hua (κ��)
// CreationDate : 2004-01-18

// 0 ԭ����whmemfileman::Open�á����ڲ����ˡ�

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
	// �������list������ڴ����ͷ��ƫ��
	m_pMemHdr->offFileSect	= sizeof(MEMINFO_T);
	m_pMemHdr->offFragSect	= m_pMemHdr->offFileSect
							+ whalist::CalcTotalSize(sizeof(FILEINFO_T), m_info.nMaxFile);
	// ��ʼ������list
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
	// �̳�����list
	m_alFileInfo.Inherit(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFileSect));
	m_alFragInfo.Inherit(wh_getoffsetaddr(m_pMemHdr, m_pMemHdr->offFragSect));
	m_info.pBuf				= pBuf;
	m_info.nMaxFile			= m_alFileInfo.GetHdr()->nMaxNum;
	m_info.nFragSize		= m_alFragInfo.GetHdr()->nDataUnitSize;
	m_info.nTotalFrag		= m_alFragInfo.GetHdr()->nMaxNum;
	//
	m_mapFileName2Idx.clear();
	// ����ĸ��Ӳ���
	Inherit_Ext();

	// ����ļ�list����ʼ��hashӳ��
	size_t	i;
	const whalist::HDR_T	*plHdr = m_alFileInfo.GetHdr();
	whvector<char>			vectbuf;
	for(i=0;i<plHdr->nMaxNum;i++)
	{
		FILEINFO_T	*pFileInfo = GetFileInfoByIdx(i);
		if( pFileInfo )
		{
			bool	bGood = true;
			// �ж��Ƿ����Ѿ�д�򿪵�
			if( pFileInfo->nProperty & FILEINFO_T::PROPERTY_WR )
			{
				// �����д�򿪵ģ�˵���ļ����ݿ����д�����Ҫ����һ��CRCУ��
				if( CalcCRC32(pFileInfo) != pFileInfo->nCRC32 )
				{
					// ����Ǵ�����(��д�Ĺ����г����ļ�����Ӧ��������)��Ӧ��ɾ�����ļ�
					bGood	= false;
				}
				else
				{
					// ������ȷ����property���
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
				// ɾ��������ƻ����ļ�
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
		// û���������ļ�
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
		// û���������ļ�
		return	NULL;
	}
	return	GetFileInfoByIdx(nIdx);
}

whmemfile *	whmemfileman::Open(int nFileName, int nMode)
{
	// �ȿ��ļ��Ƿ����Ѿ����ڵ�
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// û���������ļ�
		nIdx	= whalist::INVALIDIDX;
	}

	return	OpenByIdx(nIdx, nMode, (void *)nFileName);
}
int		whmemfileman::Close(whmemfile *pFile)
{
	if( !pFile )
	{
		// �Ѿ��ر��ˣ��������
		return	0;
	}
	// �ı�
	switch( pFile->m_nOpenMode )
	{
		case	MODE_READWRITE:
		case	MODE_CREATE:
			// ���д��־
			pFile->m_pFileInfo->nProperty	&= ~FILEINFO_T::PROPERTY_WR;
			// ���¼���CRC (Ӧ���Ǹı��ˣ���д�򿪲�д��SB)
			pFile->m_pFileInfo->nCRC32		= CalcCRC32(pFile->m_pFileInfo);
			// ��¼���ĵ�ʱ��
			pFile->m_pFileInfo->nLastModify	= wh_time();
		break;
	}
	// ɾ������
	delete	pFile;
	return	0;
}
int		whmemfileman::Delete(int nFileName)
{
	// �ҵ�nFileName��Ӧ��Idx
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// û���������ļ�
		return	-1;
	}
	// ɾ��֮
	return	DeleteByIdx(nIdx);
}
int		whmemfileman::MarkDelAndClean(int nFileName)
{
	// �ҵ�nFileName��Ӧ��Idx
	int	nIdx;
	if( !m_mapFileName2Idx.get(nFileName, nIdx) )
	{
		// û���������ļ�
		return	-1;
	}
	FILEINFO_T	*pFileInfo	= GetFileInfoByIdx(nIdx);
	if( pFileInfo )
	{
		// ɾ���ļ�����
		EmptyFile(pFileInfo);
		// ���ɾ��
		pFileInfo->nProperty	|= FILEINFO_T::PROPERTY_DEL;
		return	0;
	}
	// ��˵�����ݲ�ͬ����
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
	// �ܵ���һ��Ӧ���Ѿ��ǳ�ʼ�������ˣ�����hash�����
	m_mapFileName2Idx.exporttovector(vectList);
}

unsigned	whmemfileman::CalcCRC32(FILEINFO_T * pFileInfo) const
{
	// һ��һ�μ���crc
	int			nIdx	= pFileInfo->nHeadIdx;
	unsigned	iseed	= (unsigned)0xffffffff;
	int			nSize	= (int)pFileInfo->nSize;
	int			nUnitSize	= m_alFragInfo.GetHdr()->nDataUnitSize;
	unsigned	nCRC32	= 0;
	while( nIdx!=whalist::INVALIDIDX )
	{
		// ��ȡһ��
		char	*pszBuf	= (char *)m_alFragInfo.GetDataUnitPtr(nIdx);
		if( nSize>=nUnitSize )
		{
			nCRC32	= custom_crc32(pszBuf, nUnitSize, iseed);
		}
		else
		{
			nCRC32	= custom_crc32(pszBuf, nSize, iseed);
		}
		// �ж��Ƿ��˽�β
		nSize	-= nUnitSize;
		if( nSize<=0 )
		{
			break;
		}
		// ��һ��
		nIdx	= m_alFragInfo.GetNextOf(nIdx);
	}
	return	nCRC32;
}
int			whmemfileman::EmptyFile(FILEINFO_T *pFileInfo)
{
	// �ܵõ�pFileInfoָ��˵������һ���Ϸ���ָ�룬�Ͳ��ö��ж�һ�κϷ�����
	m_alFragInfo.FreeChain(pFileInfo->nHeadIdx);
	// ����ļ�ͷ����
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
	// ɾ���ļ�����
	EmptyFile(pFileInfo);
	// ��ɾ�������ĸ��Ӷ���
	DeleteFile_BeforeRealDel(pFileInfo);
	// ɾ���ļ���Ϣ��Ԫ
	m_alFileInfo.Free(pFileInfo->nIdx);
	return	0;
}
whmemfile *	whmemfileman::OpenFile(whmemfileman::FILEINFO_T *pFileInfo, int nMode, void *pName)
{
	// �жϴ򿪷�ʽ
	if( !pFileInfo )
	{
		// �ļ�������(�޷���)
		switch(nMode)
		{
			case	MODE_READ:
			case	MODE_READWRITE:
				return	NULL;
			break;
			case	MODE_CREATE:
			{
				// ����һ�����ļ�
				int	nIdx	= m_alFileInfo.Alloc();
				pFileInfo	= GetFileInfoByIdx(nIdx);
				if( !pFileInfo )
				{
					// �޷��������ļ��������Ѿ����ﶥ����
					return	NULL;
				}

				// ��ʼ��
				pFileInfo->clear();
				// ���ֻ�ڴ�����ʱ���ֶ���һ�� !!!! 2004-05-18 �޸� !!!!
				memset(pFileInfo->aExtPtr, 0, sizeof(pFileInfo->aExtPtr));
				// ���������������clear���ǲ���ܵ�
				pFileInfo->nIdx		= nIdx;
				// ���Ӳ���
				// �磺����ļ���ӳ��
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
				// ԭ������д�򿪣����ܱ�д��
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
					// ԭ������д�򿪣������Դ����ķ�ʽ��
					return	NULL;
				}
				// ��ԭ�����ļ�������գ�����ͼٶ��Ѿ�������һ�����ļ�
				EmptyFile(pFileInfo);
			}
			break;
		}
	}
	// ����Ӧ���Ѿ���֤pFileInfo�ǿ���

	// �ж�Ȩ�ޣ���ӱ�־λ
	switch(nMode)
	{
		case	MODE_READWRITE:
		case	MODE_CREATE:
			pFileInfo->nProperty	|= FILEINFO_T::PROPERTY_WR;
		break;
		default:
		break;
	}

	// �����ļ����󣬲���ʼ��֮
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
	// �ȿ��ļ��Ƿ����Ѿ����ڵ�
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// û���������ļ�
		nIdx	= whalist::INVALIDIDX;
	}

	return	whmemfileman::OpenFile(GetFileInfoByIdx(nIdx), nMode, (void *)cszFileName);
}
int		whmemfilemanWithName::Delete(const char * cszFileName)
{
	// �ȿ��ļ��Ƿ����Ѿ����ڵ�
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// û���������ļ�
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
	// �ȿ��ļ��Ƿ����Ѿ����ڵ�
	int	nIdx;
	if( !m_mapFileNameStr2Idx.get(cszFileName, nIdx) )
	{
		// û���������ļ�
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
	// �ٳ�ʼ�������
	size_t	nFirstPartSize	= whmemfileman::CalcMemSizeNeeded();
	m_alFileName.Init(wh_getoffsetaddr(m_pMemHdr, nFirstPartSize), sizeof(FILENAME_T), m_info.nMaxFile);
	m_mapFileNameStr2Idx.clear();
	return	0;
}
int		whmemfilemanWithName::Inherit_Ext()
{
	// �ҵ��Լ��Ŀ�ͷ���ֲ��̳�
	size_t	nFirstPartSize	= whmemfileman::CalcMemSizeNeeded();
	m_alFileName.Inherit(wh_getoffsetaddr(m_pMemHdr, nFirstPartSize));
	m_mapFileNameStr2Idx.clear();
	return	0;
}
int		whmemfilemanWithName::Inherit_AfterGood(FILEINFO_T *pFInfo, int nIdx)
{
	// Ӧ�ÿ��Ա�֤nIdx == pFInfo->nIdx
	if( nIdx!=pFInfo->nIdx )
	{
		// �ļ���Ϣ��һ��
		assert(0);
		return	-1;
	}
	// �ҵ��ļ�����Ԫ
	FILENAME_T	*pFName	= (FILENAME_T *)m_alFileName.GetDataUnitPtr(nIdx);
	if( !pFName )
	{
		// �ļ���Ϣ��һ��
		assert(0);
		return	-2;
	}
	// ��������ӳ��
	if( !m_mapFileNameStr2Idx.put(pFName->szFName, nIdx) )
	{
		// �����Ա�ʹ�ã����:(
		assert(0);
		return	-3;
	}
	return	0;
}
int		whmemfilemanWithName::Open_SetNameMap(void *pName, FILEINFO_T *pFInfo)
{
	const char	*cszName	= (const char *)pName;
	// ������õ��µ�FILENAME_T�ṹ
	int	nIdx	= m_alFileName.Alloc();
	assert( nIdx == pFInfo->nIdx );
	FILENAME_T	*pFName	= GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		// �޷���������Ӧ���ǲ����ܵ�(��Ϊ�����ļ���Ϣ����һ�µ�)
		assert(0);
		return	-1;
	}
	pFName->clear();
	// ��д���ֵ�Ԫ
	strcpy(pFName->szFName, cszName);
	// ��������ӳ��
	m_mapFileNameStr2Idx.put(pFName->szFName, nIdx);
	return	0;
}
int		whmemfilemanWithName::DeleteFile_BeforeRealDel(FILEINFO_T *pFInfo)
{
	// ������ֵ�Ԫ
	FILENAME_T	*pFName	= (FILENAME_T *)m_alFileName.GetDataUnitPtr(pFInfo->nIdx);
	if( !pFName )
	{
		return	-1;
	}
	// ɾ������ӳ��
	m_mapFileNameStr2Idx.erase(pFName->szFName);
	// �ͷ����ֵ�Ԫ
	m_alFileName.Free(pFInfo->nIdx);
	return	0;
}
