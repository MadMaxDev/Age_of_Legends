// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whzlib
// File         : whzlib_pck_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �;�������صĹ���
// CreationDate : 2003-09-04
// ChangeLOG    :
// 2004-03-04   : ������insertafter�ĵط������滹�ڼ���ʹ��֮ǰ�õ���ָ�룬����������ڶ���ΪGetUnitPtr�ˡ�Thanks to ����:)

#include "../inc/whzlib_pck_i.h"
#include "../inc/whfile.h"
#include "../inc/whstring.h"
#include <time.h>
#include <stdlib.h>

namespace n_whzlib
{

const char *infofile_hdr_t::CSZ_MAGIC	= "whzlib, 2003";

const char *cstIdxFileExt	= "idx";
const char *cstDatFileExt	= "dat";
const char *cstNamFileExt	= "nam";

bool	whzlib_ispckexist(const char *cszPck)
{
	char	szFilePath[WH_MAX_PATH];

	sprintf(szFilePath, "%s.%s", cszPck, cstIdxFileExt);
	if( !whfile_ispathexisted(szFilePath) )
	{
		return	false;
	}
	sprintf(szFilePath, "%s.%s", cszPck, cstDatFileExt);
	if( !whfile_ispathexisted(szFilePath) )
	{
		return	false;
	}
	sprintf(szFilePath, "%s.%s", cszPck, cstNamFileExt);
	if( !whfile_ispathexisted(szFilePath) )
	{
		return	false;
	}

	return		true;
}

int	whzlib_getpckfilenames(const char *cszPck, PCKFILENAMES_T *pNames)
{
	sprintf(pNames->szIdx, "%s.%s", cszPck, cstIdxFileExt);
	sprintf(pNames->szDat, "%s.%s", cszPck, cstDatFileExt);
	sprintf(pNames->szNam, "%s.%s", cszPck, cstNamFileExt);
	return	0;
}

int	whzlib_GetFilePartNum(int nTotalSize, int nUnitSize)
{
	int	nNum	= nTotalSize / nUnitSize;
	if( nTotalSize % nUnitSize )
	{
		nNum	++;
	}
	return		nNum;
}

int	whzlib_ReadFileName(FILE *fp, int nPos, char *szFileName, int nSize)
{
	int		rst;
	whzlib_filenamesize_t	nNameSize;

	// ���ŵص�
	::fseek(fp, nPos, SEEK_SET);
	// ���볤��
	rst		= ::fread(&nNameSize, 1, sizeof(nNameSize), fp);
	if( rst!=sizeof(nNameSize) )
	{
		assert(0);
		return	-1;
	}
	if( nNameSize>nSize )
	{
		// ���ճ��Ȳ���
		assert(0);
		return	-1;
	}
	// �����ִ�
	rst		= ::fread(szFileName, 1, nNameSize, fp);
	if( rst!=nNameSize ) 
	{
		assert(0);
		return	-1;
	}

	return	nNameSize;
}
int	whzlib_WriteFileName(FILE *fp, const char *szFileName)
{
	whzlib_filenamesize_t	nSize = strlen(szFileName) + 1;
	int		rst;

	// д�볤��
	rst		= ::fwrite(&nSize, 1, sizeof(nSize), fp);
	if( rst!=sizeof(nSize) )
	{
		assert(0);
		return	-1;
	}
	// д���ִ�
	rst		= ::fwrite(szFileName, 1, nSize, fp);
	if( rst!=nSize ) 
	{
		assert(0);
		return	-1;
	}

	return	0;
}

}

using namespace n_whcmn;
using namespace n_whzlib;

////////////////////////////////////////////////////////////////////
// whzlib_file_i_pck
////////////////////////////////////////////////////////////////////
whzlib_file_i_pck::whzlib_file_i_pck()
: m_buf(NULL)
, m_bufsize(0), m_leftsize(0), m_curpart(0), m_totalpart(0)
{
	memset(&m_info, 0, sizeof(m_info));
}
whzlib_file_i_pck::~whzlib_file_i_pck()
{
	Release();
}
int	whzlib_file_i_pck::FileSize() const
{
	return	m_info.totaldatasize;
}
time_t	whzlib_file_i_pck::FileTime() const
{
	EXTFILEUNIT_T	*pUnit = m_info.pReader->GetUnitPtr(m_info.nFileID);
	if( !pUnit )
	{
		return	-1;
	}
	return	pUnit->infile.time;
}
int	whzlib_file_i_pck::Read(void *pBuf, int nSize)
{
	char	*pcBuf = (char *)pBuf;
	int		nReadSize = 0;

	if( nSize<=0 )
	{
		return	0;
	}

Again:
	// �жϵ�ǰ�ڴ��еĳߴ��Ƿ���
	if( nSize<=m_leftsize ) 
	{
		memcpy(pcBuf, m_buf+(m_bufsize-m_leftsize), nSize);
		m_leftsize		-= nSize;
		nReadSize		+= nSize;			// ��Ϊ������again�������
	}
	else
	{
		if( m_leftsize>0 )
		{
			memcpy(pcBuf, m_buf+(m_bufsize-m_leftsize), m_leftsize);
			nSize		-= m_leftsize;
			pcBuf		+= m_leftsize;
			nReadSize	+= m_leftsize;
		}
		m_curpart	++;
		if( m_curpart>=m_totalpart )
		{
			// �����ļ���β(û����һ����)
			m_bufsize	= 0;
			m_leftsize	= 0;
			goto		End;
		}
		// ��reader��Ҫ��һ������
		m_bufsize		= m_info.pReader->GetDataPart(m_info.nFileID, m_curpart, m_buf);
		m_leftsize		= m_bufsize;
		assert( m_bufsize>0 );
		// ������
		goto			Again;
	}

End:
	return	nReadSize;
}
int	whzlib_file_i_pck::Seek(int nOffset, int nOrigin)
{
	int	curpos = m_curpart * m_info.nUnitSize + m_bufsize - m_leftsize;
	if( IsEOF() )
	{
		curpos	= FileSize();
	}
	int	newpos = curpos;
	int	curpart, curidx;
	switch(nOrigin)
	{
		case	SEEK_SET:
		{
			newpos	= nOffset;
		}
		break;
		case	SEEK_CUR:
		{
			newpos	+= nOffset;
		}
		break;
		case	SEEK_END:
		{
			newpos	= FileSize() + nOffset;
		}
		break;
	}
	// �����ж�
	if( newpos<0 )
	{
		newpos		= 0;
		curpart		= 0;
		curidx		= 0;
	}
	else if( newpos>=FileSize() )
	{
		// ��������EOF��
		curpart		= m_totalpart;
		curidx		= 0;
	}
	else
	{
		curpart		= newpos / m_info.nUnitSize;
		curidx		= newpos % m_info.nUnitSize;
	}
	if( curpart!=m_curpart )
	{
		m_curpart	= curpart;
		m_bufsize	= m_info.pReader->GetDataPart(m_info.nFileID, m_curpart, m_buf);
	}
	m_leftsize	= m_bufsize - curidx;

	return	0;
}
bool	whzlib_file_i_pck::IsEOF()
{
	if( m_curpart>=m_totalpart )
	{
		return	true;
	}
	return		false;
}
int		whzlib_file_i_pck::Tell()
{
	return	m_curpart * m_info.nUnitSize + m_bufsize-m_leftsize;
}
int	whzlib_file_i_pck::Init()
{
	assert(m_info.pReader);
	m_vectbuf.resize(m_info.nUnitSize);
	m_buf		= m_vectbuf.getbuf();

	// �����һ������(��һ��������һ����е�)
	m_curpart	= 0;
	m_totalpart	= whzlib_GetFilePartNum(m_info.totaldatasize, m_info.nUnitSize);
	m_bufsize	= m_info.pReader->GetDataPart(m_info.nFileID, m_curpart, m_buf);
	m_leftsize	= m_bufsize;

	return	0;
}
int	whzlib_file_i_pck::Release()
{
	// ����û�ÿ�����!!!
	return	0;
}
int	whzlib_file_i_pck::GetMD5(unsigned char *MD5)
{
	EXTFILEUNIT_T	*pUnit = m_info.pReader->GetUnitPtr(m_info.nFileID);
	if( !pUnit )
	{
		return	-1;
	}
	memcpy(MD5, pUnit->infile.md5, 16);
	return	0;
}

////////////////////////////////////////////////////////////////////
// whzlib_pck_reader_i
////////////////////////////////////////////////////////////////////
whzlib_pck_reader_i::whzlib_pck_reader_i()
: m_fileIdx(NULL), m_fileDat(NULL), m_fileNam(NULL)
, m_bAutoClosePackFile(true)
, m_bIC(true)
, m_buf(NULL)
{
	m_szPassword[0]	= 0;
}
whzlib_pck_reader_i::~whzlib_pck_reader_i()
{
	ClosePck();
}
void	whzlib_pck_reader_i::SetPassword(const char *szPassword)
{
	if( szPassword && szPassword[0] )
	{
		WH_STRNCPY0(m_szPassword, szPassword);
		WHDES_ConvertToFixed(szPassword, strlen(szPassword), (char *)m_des.m_key, sizeof(m_des.m_key));
		m_des.setmask(m_szPassword, strlen(m_szPassword));
	}
	else
	{
		m_des.clrmask();
	}
}
int	whzlib_pck_reader_i::OpenAllFileOnDisk(const char *szPckFile, const char *szMode)
{
	char	szFilePath[WH_MAX_PATH];

	m_bAutoClosePackFile	= true;

	// ���������ļ�
	sprintf(szFilePath, "%s.%s", szPckFile, cstIdxFileExt);
	m_fileIdx	= whfile_OpenCmnFile(szFilePath, szMode);
	if( !m_fileIdx )
	{
		return	-10;
	}
	sprintf(szFilePath, "%s.%s", szPckFile, cstDatFileExt);
	m_fileDat	= whfile_OpenCmnFile(szFilePath, szMode);
	if( !m_fileDat )
	{
		return	-20;
	}
	sprintf(szFilePath, "%s.%s", szPckFile, cstNamFileExt);
	m_fileNam	= whfile_OpenCmnFile(szFilePath, szMode);
	if( !m_fileNam )
	{
		return	-30;
	}

	return	0;
}
int	whzlib_pck_reader_i::OpenAllFileFromMemOrOnDisk(const char *szPckFile, whsmpfilecache *pCache)
{
	PCKFILENAMES_T	fs;
	whzlib_getpckfilenames(szPckFile, &fs);

	m_bAutoClosePackFile	= true;

	m_fileIdx		= pCache->OpenFile(fs.szIdx, true);
	if( !m_fileIdx )
	{
		return	-10;
	}
	m_fileDat		= pCache->OpenFile(fs.szDat, true);
	if( !m_fileDat )
	{
		return	-20;
	}
	m_fileNam		= pCache->OpenFile(fs.szNam, true);
	if( !m_fileNam )
	{
		return	-30;
	}
	return		0;
}
int	whzlib_pck_reader_i::OpenPack()
{
	int				rst;
	bool			bval;
	EXTFILEUNIT_T	*pUnit;

	// ������Ϣͷ
	rst		= m_fileIdx->Read(&m_infohdr);

	// �ж��Ƿ���֧�ֵ��ļ�
	if( rst!=sizeof(m_infohdr)
	||  !m_infohdr.IsVerGood()
	||  !m_infohdr.IsMagicGood()
	)
	{
		// ������ļ�
		return	-1;
	}

	// �����ļ������б�
	m_finfolist.clear();
	while( !m_fileIdx->IsEOF() )
	{
		pUnit	= m_finfolist.push_back();
		rst		= m_fileIdx->Read(&pUnit->infile);
		// ��������һ�������rst==0��״̬
		if( rst==0 )
		{
			m_finfolist.pop_back();
			break;
		}
		// �������0����˵���ߴ�һ������
		if( rst!=sizeof(pUnit->infile) )
		{
			// �ļ�����
			m_finfolist.pop_back();
			assert(0);
			return	-1;
		}
		pUnit->pDirInfo	= NULL;
	}

	m_vectbuf.resize(m_infohdr.hdr.nUnitSize);
	m_buf	= m_vectbuf.getbuf();

	// ���������ļ������ڴ�
	m_fileNam->Seek(0, SEEK_END);
	int	nAllNameSize = m_fileNam->Tell();
	m_vectnames.resize(nAllNameSize);
	m_fileNam->Seek(0, SEEK_SET);
	m_fileNam->Read(m_vectnames.getbuf(), nAllNameSize);

	// ��������Ŀ¼���ļ���������±��ӳ��
	// Ŀ¼����ָ��ջ
	whvector<EXTFILEUNIT_T::DIRINFO *>	vectDirPtrStack;
	// ��Ŀ¼������չ����ָ��
	EXTFILEUNIT_T::DIRINFO				*pFatherDirInfo;

	// ��һ������Ǹ�Ŀ¼
	pUnit	= m_finfolist.getptr(0);
	assert(pUnit->infile.type == infofile_fileunit_t::TYPE_DIR);
	pUnit->pDirInfo	= new EXTFILEUNIT_T::DIRINFO;
	assert(pUnit->pDirInfo);
	vectDirPtrStack.push_back(pUnit->pDirInfo);
	pFatherDirInfo	= pUnit->pDirInfo;
	// ���������ļ���Ϣ����������Ŀ¼���ļ���ӳ��
	for(size_t i=1;i<m_finfolist.size();i++)
	{
		pUnit	= m_finfolist.getptr(i);
		// ���ļ�������hash��
		switch( pUnit->infile.type )
		{
			case	infofile_fileunit_t::TYPE_DIR:
			case	infofile_fileunit_t::TYPE_FILE:
			{
				bval	= pFatherDirInfo->mapName2ID.put(
						GetFileNamePtr(pUnit->infile.namepos)
						, i
						);
				if( !bval )
				{
					assert(0);	// �����ظ����ļ���????!!!!
					return	-1;
				}
			}
			break;
		}
		// Ŀ¼��ز���
		switch( pUnit->infile.type )
		{
			case	infofile_fileunit_t::TYPE_DIR:
			{
				// ������չ��Ϣ��
				pUnit->pDirInfo	= new EXTFILEUNIT_T::DIRINFO;
				if( !pUnit->pDirInfo )
				{
					assert(0);
					return	-1;
				}
				vectDirPtrStack.push_back(pUnit->pDirInfo);
				pFatherDirInfo	= pUnit->pDirInfo;
			}
			break;
			case	infofile_fileunit_t::TYPE_EOD:
			{
				// ��ǰĿ¼������һ��
				vectDirPtrStack.pop_back();
				// �����µĵ�ǰĿ¼ָ��
				if( vectDirPtrStack.size()>0 )
				{
					pFatherDirInfo	= vectDirPtrStack.getlast();
				}
				else
				{
					pFatherDirInfo	= NULL;
				}
			}
			break;
		}
	}

	return	0;
}
int	whzlib_pck_reader_i::OpenPck(const char *szPckFile)
{
	if( OpenAllFileOnDisk(szPckFile, "rb")<0 )
	{
		return	-1;
	}

	return	OpenPack();
}
int	whzlib_pck_reader_i::OpenPck(const char *szPckFile, whsmpfilecache *pCache)
{
	if( OpenAllFileFromMemOrOnDisk(szPckFile, pCache)<0 )
	{
		return	-1;
	}

	return	OpenPack();
}
int	whzlib_pck_reader_i::OpenPck(whfile *pfileIdx, whfile *pfileDat, whfile *pfileNam, bool bAutoClosePackFile)
{
	m_bAutoClosePackFile	= bAutoClosePackFile;
	m_fileIdx				= pfileIdx;
	m_fileDat				= pfileDat;
	m_fileNam				= pfileNam;
	return	OpenPack();
}
int	whzlib_pck_reader_i::ClosePck()
{
	if( m_fileIdx )
	{
		if( m_bAutoClosePackFile )
		{
			delete	m_fileIdx;
		}
		m_fileIdx	= NULL;
	}
	if( m_fileDat )
	{
		if( m_bAutoClosePackFile )
		{
			delete	m_fileDat;
		}
		m_fileDat	= NULL;
	}
	if( m_fileNam )
	{
		if( m_bAutoClosePackFile )
		{
			delete	m_fileNam;
		}
		m_fileNam	= NULL;
	}

	// ɾ�������ļ���Ϣ�ĸ���Ŀ¼����(��������)
	EXTFILEUNIT_T	*pUnit;
	for(size_t i=0;i<m_finfolist.size();i++)
	{
		pUnit	= m_finfolist.getptr(i);
		// ���ļ�������hash��
		if( pUnit->pDirInfo )
		{
			delete	pUnit->pDirInfo;
			pUnit->pDirInfo	= NULL;
		}
	}
	m_finfolist.clear();

	return	0;
}
whzlib_file_i_pck *	whzlib_pck_reader_i::OpenFile(int nFileID)
{
	if( nFileID<0 || nFileID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	NULL;
	}

	infofile_fileunit_t	*pUnitInFile = &m_finfolist.getptr(nFileID)->infile;
	if( pUnitInFile->type != infofile_fileunit_t::TYPE_FILE )
	{
		return	NULL;
	}

	whzlib_file_i_pck	*file = new whzlib_file_i_pck;
	if( !file )
	{
		assert(0);
		return	NULL;
	}

	// ���ò���
	file->m_info.nFileID		= nFileID;
	file->m_info.nUnitSize		= m_infohdr.hdr.nUnitSize;
	file->m_info.totaldatasize	= pUnitInFile->info.file.totaldatasize;
	file->m_info.pReader		= this;
	// ��ʼ��
	if( file->Init()<0 )
	{
		delete	file;
		return	NULL;
	}

	return	file;
}
whzlib_file_i_pck *	whzlib_pck_reader_i::OpenFile(const char *szFile)
{
	// �ҵ�����Ӧ��FileID��Ȼ���������ķ���
	return	OpenFile(GetPathID(szFile));
}
WHDirInPck *	whzlib_pck_reader_i::OpenDir(int nDirID)
{
	if( nDirID<0 || nDirID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	NULL;
	}

	infofile_fileunit_t	*pUnitInFile = &m_finfolist.getptr(nDirID)->infile;
	if( pUnitInFile->type != infofile_fileunit_t::TYPE_DIR )
	{
		return	NULL;
	}

	WHDirInPck	*dir = new WHDirInPck;
	if( !dir )
	{
		assert(0);
		return	NULL;
	}
	dir->m_info.nDirID	= nDirID+1;
	dir->m_info.pReader	= this;
	dir->Rewind();

	return		dir;
}
WHDirInPck *	whzlib_pck_reader_i::OpenDir(const char *szDir)
{
	// �ҵ�����Ӧ��FileID��Ȼ���������ķ���
	return	OpenDir(GetPathID(szDir));
}

int	whzlib_pck_reader_i::FindEntry(int nDirID, const char *szName)
{
	if( nDirID<0 || nDirID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	-1;
	}

	// ��ñ�dir�Ľṹ
	EXTFILEUNIT_T	*pUnit = m_finfolist.getptr(nDirID);
	if( pUnit->infile.type != infofile_fileunit_t::TYPE_DIR
	||  !pUnit->pDirInfo
	)
	{
		// ֻ��dir�ſ�������ʼ����
		assert(0);
		return	-1;
	}

	// ��hash���в���
	int	nID = -1;
	if( pUnit->pDirInfo->mapName2ID.get(szName, nID) )
	{
		return	nID;
	}

	return	-1;
}
char *	whzlib_pck_reader_i::GetFileNamePtr(int nOff)
{
	return	m_vectnames.getptr(nOff+sizeof(whzlib_filenamesize_t));
}
int	whzlib_pck_reader_i::GetPathID(const char *szPath)
{
	if( szPath[0]==0 )
	{
		// ����ʹ����Ŀ¼
		return	0;
	}

	// ���ļ����ֽ�Ϊ���ɶΣ��ֶηּ�������Ӧƥ���Ŀ¼
	char	szTmpBuf[WH_MAX_PATH];
	// ��Ӧ�ó�����ô�༶Ŀ¼��
	char	*pszDir[WH_MAX_PATHLEVEL];
	int		nNum = WH_MAX_PATHLEVEL;
	if( wh_strexplode(szPath, "/\\", szTmpBuf, pszDir, &nNum)<0 )
	{
		return	-1;
	}
	
	// һ��Ŀ¼�������Ŀ��
	int		nDirID = 0;
	for(int i=0;i<nNum;i++)
	{
		nDirID	= FindEntry(nDirID, pszDir[i]);
		if( nDirID<0 )
		{
			// û���ҵ�
			return	-1;
		}
		infofile_fileunit_t	*pUnitInFile = &m_finfolist.getptr(nDirID)->infile;
		if( pUnitInFile->type != infofile_fileunit_t::TYPE_DIR )
		{
			// ����������һ���ͱ�����Ŀ¼
			if( i<nNum-1 )
			{
				return	-1;
			}
		}
	}

	return	nDirID;
}
int	whzlib_pck_reader_i::GetFileSize(const char *szFile)
{
	int	nID = GetPathID(szFile);
	if( nID<=0 )
	{
		// û�ҵ������Ǹ�Ŀ¼������
		return	-1;
	}
	EXTFILEUNIT_T	*pUnit = GetUnitPtr(nID);
	if( pUnit->infile.type != infofile_fileunit_t::TYPE_FILE )
	{
		// �����ļ�
		return	-1;
	}
	return	pUnit->infile.info.file.totaldatasize;
}
int	whzlib_pck_reader_i::GetDataPart(int nFileID, int nPart, char *pBuf)
{
	if( nFileID<0 || nFileID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	0;
	}

	// �жϺϷ���
	infofile_fileunit_t	*pUnitInFile = &m_finfolist.getptr(nFileID)->infile;
	if( pUnitInFile->type == infofile_fileunit_t::TYPE_DIR )
	{
		// ��ô����Ŀ¼��
		assert(0);
		return	0;
	}
	// �ļ����ֿܷ���
	int	nTotalPart = whzlib_GetFilePartNum(pUnitInFile->info.file.totaldatasize, m_infohdr.hdr.nUnitSize);
	if( nPart<0 || nPart>=nTotalPart )
	{
		// �����˷�Χ(����0��ʾEOF)
		return	0;
	}
	// �õ���ӦnPart����ʼ�ֽ�
	int	nOff;
	m_fileDat->Seek(pUnitInFile->info.file.datapos + sizeof(datafile_file_hdr_t) + nPart*sizeof(int), SEEK_SET);
	m_fileDat->Read(&nOff);
	// ����ö����ݵľ��뵱ǰλ�õľ���ƫ��
	nOff	+= (nTotalPart-nPart-1)*sizeof(int);	// ֮���Լ�1����Ϊ����Offset�Ѿ�������
	m_fileDat->Seek(nOff, SEEK_CUR);
	// ��������ͷ
	datafile_dataunit_hdr_t	dhdr;
	m_fileDat->Read(&dhdr);
	// ��������
	size_t	nSize = m_fileDat->Read(m_buf, dhdr.nSize);
	if( nSize!=dhdr.nSize )
	{
		assert(0);
		return	0;
	}
	// ���������ͽ���
	if( m_szPassword[0] )
	{
		m_des.decrypt((unsigned char *)m_buf, nSize);
	}
	// ��ѹ��ĳߴ�
	size_t	nRawSize = 0;
	// �������ѹ
	switch( dhdr.nMode )
	{
		case	COMPRESSMODE_ZIP:
		{
			nRawSize	= m_infohdr.hdr.nUnitSize;
			if( whzlib_decompress(m_buf, nSize, pBuf, &nRawSize)<0 )
			{
				assert(0);
				return	-1;
			}
		}
		break;
		default:
		{
			// ֱ�ӿ���
			nRawSize	= dhdr.nSize;
			memcpy(pBuf, m_buf, nRawSize);
		}
		break;
	}

	return	nRawSize;
}

////////////////////////////////////////////////////////////////////
// WHDirCarePck
////////////////////////////////////////////////////////////////////
WHDir::ENTRYINFO_T *	WHDirCarePck::Read()
{
	const char	*cszExtList[]	=
	{
		cstDatFileExt,
		cstIdxFileExt,
		cstNamFileExt,
	};
again:
	WHDir::ENTRYINFO_T	*pEntry = WHDir::Read();
	if( !pEntry )
	{
		return	NULL;
	}
	if( !pEntry->bIsDir )
	{
		// �ж��Ƿ������ļ�������
		char	szFullPath[WH_MAX_PATH];
		MakeFullPath(pEntry->szName, szFullPath);
		int		nLen = strlen(szFullPath);
		if( nLen>4 )
		{
			nLen	-= 4;
			// �жϺ�׺�Ƿ��ǿ�ĺ�׺
			int	i;
			for(i=0;i<3;i++)
			{
				if( stricmp(szFullPath+nLen+1, cszExtList[i])==0 )
				{
					// ȡ������.xxx
					szFullPath[nLen]	= 0;
					// �жϰ��Ƿ����
					if( whzlib_ispckexist(szFullPath) )
					{
						switch(i)
						{
							case	0:
							case	1:
								// �����Һ�����ļ�
								goto	again;
							break;
							case	2:
								// ������Ϊ����Ŀ¼��
								pEntry->szName[strlen(pEntry->szName)-4]	= 0;
								pEntry->bIsDir	= true;
								//pEntry->nSize	= 0;
								// ����
								return	pEntry;
							break;
						}
					}
					// ����
					break;
				}
			}
		}
	}

	return	pEntry;
}

////////////////////////////////////////////////////////////////////
// WHDirInPck
////////////////////////////////////////////////////////////////////
whzlib_fileman	*WHDirInPck::m_pFMan	= NULL;
WHDirBase *	WHDirInPck::Create(const char *szDir)
{
	if( !m_pFMan )
	{
		return	NULL;
	}
	// ���m_pFMan����縳ֵ�˾Ϳ���
	return	m_pFMan->OpenDir(szDir);
}

WHDirInPck::WHDirInPck()
: m_nIdx(0), m_nLevel(0)
{
}
WHDirInPck::~WHDirInPck()
{
	Close();
}
int		WHDirInPck::Close()
{
	return	0;
}
WHDirBase::ENTRYINFO_T *	WHDirInPck::Read()
{
	EXTFILEUNIT_T	*pUnit = m_pUnit + m_nIdx;

	m_EntryInfo.ext.pck.nID	= m_nIdx;

	switch( pUnit->infile.type )
	{
		case	infofile_fileunit_t::TYPE_DIR:
		{
			m_EntryInfo.bIsDir	= true;
			strcpy(m_EntryInfo.szName, m_info.pReader->GetFileNamePtr(pUnit->infile.namepos));
			//m_EntryInfo.nSize	= pUnit->infile.info.dir.entrynum;
			// ������һ��ͬ����Ŀ(+2�ǰ�����)
			m_nIdx	+= pUnit->infile.info.dir.totalentrynum+2;
		}
		break;
		case	infofile_fileunit_t::TYPE_FILE:
		{
			m_EntryInfo.bIsDir	= false;
			strcpy(m_EntryInfo.szName, m_info.pReader->GetFileNamePtr(pUnit->infile.namepos));
			//m_EntryInfo.nSize	= pUnit->infile.info.file.totaldatasize;
			m_nIdx	++;
		}
		break;
		case	infofile_fileunit_t::TYPE_EOD:
		{
			// ��ֻ���������ս�β
			return	NULL;
		}
		break;
	}
	return	&m_EntryInfo;
}
void	WHDirInPck::Rewind()
{
	m_pUnit		= m_info.pReader->GetUnitPtr(m_info.nDirID);
	m_nIdx		= 0;
	m_nLevel	= 0;
}

////////////////////////////////////////////////////////////////////
// WHDirInPck4Mod
////////////////////////////////////////////////////////////////////
WHDirInPck4Mod::WHDirInPck4Mod()
: m_nIdx(0), m_nLevel(0)
{
}
int		WHDirInPck4Mod::Close()
{
	return	0;
}
WHDirBase::ENTRYINFO_T *	WHDirInPck4Mod::Read()
{
	infofile_fileunit_t	*pUnit = m_pUnit + m_nIdx;
	switch( pUnit->type )
	{
		case	infofile_fileunit_t::TYPE_DIR:
		{
			m_EntryInfo.bIsDir	= true;
			strcpy(m_EntryInfo.szName, m_info.pReader->GetFileNamePtr(pUnit->namepos));
			//m_EntryInfo.nSize	= pUnit->info.dir.entrynum;
			// ������һ��ͬ����Ŀ(+2�ǰ�����)
			m_nIdx	+= pUnit->info.dir.totalentrynum+2;
		}
		break;
		case	infofile_fileunit_t::TYPE_FILE:
		{
			m_EntryInfo.bIsDir	= false;
			strcpy(m_EntryInfo.szName, m_info.pReader->GetFileNamePtr(pUnit->namepos));
			//m_EntryInfo.nSize	= pUnit->info.file.totaldatasize;
			m_nIdx	++;
		}
		break;
		case	infofile_fileunit_t::TYPE_EOD:
		{
			// ��ֻ���������ս�β
			return	NULL;
		}
		break;
	}
	return	&m_EntryInfo;
}
void	WHDirInPck4Mod::Rewind()
{
	m_pUnit		= m_info.pReader->GetUnitPtr(m_info.nDirID);
	m_nIdx		= 0;
	m_nLevel	= 0;
}

////////////////////////////////////////////////////////////////////
// whzlib_pck_modifier_i
////////////////////////////////////////////////////////////////////
whzlib_pck_modifier_i::whzlib_pck_modifier_i()
: m_bJustCreated(false), m_bModified(false)
, m_fpIdx(NULL), m_fpDat(NULL), m_fpNam(NULL)
{
	m_szPassword[0]	= 0;
}
whzlib_pck_modifier_i::~whzlib_pck_modifier_i()
{
	ClosePck();
}
void	whzlib_pck_modifier_i::SetPassword(const char *szPassword)
{
	if( szPassword && szPassword[0] )
	{
		WH_STRNCPY0(m_szPassword, szPassword);
		WHDES_ConvertToFixed(szPassword, strlen(szPassword), (char *)m_des.m_key, sizeof(m_des.m_key));
		m_des.setmask(m_szPassword, strlen(m_szPassword));
	}
	else
	{
		m_des.clrmask();
	}
}
int	whzlib_pck_modifier_i::Flush()
{
	return	0;
}
int	whzlib_pck_modifier_i::OpenAllFileOnDisk(const char *szPckFile, const char *szMode)
{
	char	szFilePath[WH_MAX_PATH];

	// ���������ļ�
	sprintf(szFilePath, "%s.%s", szPckFile, cstIdxFileExt);
	m_fpIdx	= ::fopen(szFilePath, szMode);
	if( !m_fpIdx )
	{
		return	-1;
	}
	sprintf(szFilePath, "%s.%s", szPckFile, cstDatFileExt);
	m_fpDat	= ::fopen(szFilePath, szMode);
	if( !m_fpDat )
	{
		return	-1;
	}
	sprintf(szFilePath, "%s.%s", szPckFile, cstNamFileExt);
	m_fpNam	= ::fopen(szFilePath, szMode);
	if( !m_fpNam )
	{
		return	-1;
	}

	strcpy(m_szPckFile, szPckFile);

	return	0;
}
int	whzlib_pck_modifier_i::CreatePck(CREATEINFO_T *pInfo)
{
	if( OpenAllFileOnDisk(pInfo->szPckFile, "wb")<0 )
	{
		return	-1;
	}

	// ���list
	m_finfolist.clear();

	m_vectDirStack.clear();

	// ������ֵ
	m_infohdr.hdr.nUnitSize	= pInfo->nUnitSize;
	memset(m_infohdr.junk, 0, sizeof(m_infohdr.junk));

	m_bJustCreated		= true;
	// �����򿪾�ע��Ҫ�޸�
	SetModified();

	return	0;
}
int	whzlib_pck_modifier_i::OpenPck(OPENINFO_T *pInfo)
{
	if( OpenAllFileOnDisk(pInfo->szPckFile, "rb+")<0 )
	{
		return	-1;
	}

	int				rst;

	// ������Ϣͷ
	rst		= fread(&m_infohdr, 1, sizeof(m_infohdr), m_fpIdx);
	if( rst!=sizeof(m_infohdr)
	||  m_infohdr.hdr.nVer != WHZLIB_VER
	)
	{
		// ������ļ�
		return	-1;
	}

	// �����ļ������б��������ļ�����Ŀ¼������
	whvector<int>	vectStack;
	int				nFatherID = -1, nID = 0;
	m_finfolist.clear();
	while( !feof(m_fpIdx) )
	{
		infofile_fileunit_t	*pUnit = m_finfolist.push_back();
		rst	= fread(pUnit, 1, sizeof(*pUnit), m_fpIdx);
		if( rst==0 )
		{
			m_finfolist.pop_back();
			break;
		}
		if( rst!=sizeof(*pUnit) )
		{
			m_finfolist.pop_back();
			// �ļ�����
			assert(0);
			return	-1;
		}
		EXTUNIT_T	*pExt = m_fextlist.push_back();
		pExt->nFatherID	= nFatherID;
		switch( pUnit->type )
		{
			case	infofile_fileunit_t::TYPE_DIR:
				vectStack.push_back(nID);
				nFatherID	= nID;
			break;
			case	infofile_fileunit_t::TYPE_EOD:
				vectStack.pop_back();
				if( vectStack.size() )
				{
					nFatherID	= *vectStack.getlastptr();
				}
				else
				{
					nFatherID	= -1;
				}
			break;
		}
		nID	++;
	}

	m_vectDirStack.clear();

	m_bJustCreated		= false;

	return	0;
}
int	whzlib_pck_modifier_i::ClosePck()
{
	if( m_fpIdx )
	{
		::fclose(m_fpIdx);
		m_fpIdx	= NULL;
	}
	if( m_fpDat )
	{
		::fclose(m_fpDat);
		m_fpDat	= NULL;
	}
	if( m_fpNam )
	{
		::fclose(m_fpNam);
		m_fpNam	= NULL;
	}

	// ���û�д�������д���µ������ļ����滻���ϵ������ļ�
	if( IsModified() )
	{
		ClrModified();
		char	szFilePath[WH_MAX_PATH];
		sprintf(szFilePath, "%s.%s", m_szPckFile, cstIdxFileExt);
		m_fpIdx	= ::fopen(szFilePath, "wb");
		if( m_fpIdx )
		{
			// �ļ�ͷ
			::fwrite(&m_infohdr, 1, sizeof(m_infohdr), m_fpIdx);
			// ������
			::fwrite(
					m_finfolist.getbuf()
					, 1
					, m_finfolist.totalbytes()
					, m_fpIdx
					);
			::fclose(m_fpIdx);
		}
	}

	return	0;
}
int	whzlib_pck_modifier_i::GetPathID(const char *szPath)
{
	if( szPath[0]==0 )
	{
		// ����ʹ����Ŀ¼
		return	0;
	}

	// ���ļ����ֽ�Ϊ���ɶΣ��ֶηּ�������Ӧƥ���Ŀ¼
	char	szTmpBuf[WH_MAX_PATH];
	// ��Ӧ�ó�����ô�༶Ŀ¼��
	char	*pszDir[WH_MAX_PATHLEVEL];
	int		nNum = WH_MAX_PATHLEVEL;
	if( wh_strexplode(szPath, "/\\", szTmpBuf, pszDir, &nNum)<0 )
	{
		return	-1;
	}
	
	// һ��Ŀ¼�������Ŀ��
	int		nDirID = 0;
	for(int i=0;i<nNum;i++)
	{
		nDirID	= FindEntry(nDirID, pszDir[i]);
		if( nDirID<0 )
		{
			// û���ҵ�
			return	-1;
		}
		infofile_fileunit_t	*pUnitInFile = m_finfolist.getptr(nDirID);
		if( pUnitInFile->type != infofile_fileunit_t::TYPE_DIR )
		{
			// ����������һ���ͱ�����Ŀ¼
			if( i<nNum-1 )
			{
				return	-1;
			}
		}
	}

	return	nDirID;
}
int	whzlib_pck_modifier_i::LookForExistPath(const char *szPath, char *szNew)
{
	// Ĭ������
	szNew[0]	= 0;
	if( szPath[0]==0 )
	{
		// ����ʹ����Ŀ¼
		return		0;
	}

	// ���ļ����ֽ�Ϊ���ɶΣ��ֶηּ�������Ӧƥ���Ŀ¼
	char	szTmpBuf[WH_MAX_PATH];
	// ��Ӧ�ó�����ô�༶Ŀ¼��
	char	*pszDir[WH_MAX_PATHLEVEL];
	int		nNum = WH_MAX_PATHLEVEL;
	if( wh_strexplode(szPath, "/\\", szTmpBuf, pszDir, &nNum)<0 )
	{
		// �д���!!!!
		return	-1;
	}

	// һ��Ŀ¼�������Ŀ��
	int		nDirID = 0, nFatherID = 0;
	for(int i=0;i<nNum;i++)
	{
		nDirID	= FindEntry(nDirID, pszDir[i]);
		if( nDirID<0 )
		{
			// ����û���ҵ�����ô����Ϊ���ϼ�Ŀ¼�µ�
			// Ȼ���ʣ�µĲ�����ϳ��µ��ļ���
			wh_strimplode(pszDir+i, nNum-i, "/", szNew);
			return	nFatherID;
		}
		nFatherID	= nDirID;
		infofile_fileunit_t	*pUnitInFile = m_finfolist.getptr(nDirID);
		if( pUnitInFile->type != infofile_fileunit_t::TYPE_DIR )
		{
			// ����������һ���ͱ�����Ŀ¼
			if( i<nNum-1 )
			{
				return	-1;
			}
		}
	}

	// ȫ���ҵ���û�в����ڵĲ���
	szNew[0]	= 0;
	return		nDirID;
}
char *	whzlib_pck_modifier_i::GetFileNamePtr(int nOff)
{
	ReadFileName(nOff, m_szTmpFileName, sizeof(m_szTmpFileName));
	return	m_szTmpFileName;
}
int	whzlib_pck_modifier_i::AppendDir(const char *szDir)
{
	// �������Ŀ
	infofile_fileunit_t	*pUnitInFile = m_finfolist.push_back();
	pUnitInFile->type		= infofile_fileunit_t::TYPE_DIR;
	pUnitInFile->namepos	= ::ftell(m_fpNam);
	pUnitInFile->info.dir.entrynum		= 0;
	pUnitInFile->info.dir.totalentrynum	= 0;
	// �ļ���д���ļ����ļ�
	if( WriteFileName(szDir)<0 )
	{
		return	-1;
	}
	// ����ջ
	m_vectDirStack.push_back(m_finfolist.size()-1);

	return	0;
}
int	whzlib_pck_modifier_i::AppendFile(const char *szFile, whzlib_file *file, int nCompressMode)
{
	assert(m_bJustCreated);

	infofile_fileunit_t	*pUnitInFile;

	// ��Ŀ¼ͬ���ļ��������ļ�����++
	if( m_vectDirStack.size() )
	{
		int	nFather = m_vectDirStack[m_vectDirStack.size()-1];
		pUnitInFile	= m_finfolist.getptr(nFather);
		pUnitInFile->info.dir.entrynum		++;
		pUnitInFile->info.dir.totalentrynum	++;
	}
	// �������Ŀ
	pUnitInFile				= m_finfolist.push_back();
	pUnitInFile->type		= infofile_fileunit_t::TYPE_FILE;
	pUnitInFile->namepos	= ::ftell(m_fpNam);
	pUnitInFile->info.file.datapos			= ::ftell(m_fpDat);
	pUnitInFile->info.file.totaldatasize	= file->FileSize();
	// �ļ�����д�������ļ�
	if( WriteData(pUnitInFile, file, nCompressMode)<0 )
	{
		return	-1;
	}
	// �ļ���д���ļ����ļ�
	if( WriteFileName(szFile)<0 )
	{
		return	-1;
	}

	return	0;
}
int	whzlib_pck_modifier_i::AppendDirEnd()
{
	assert(m_bJustCreated);
	if( m_vectDirStack.size()==0 )
	{
		// ˵���Ǹ�Ŀ¼����
		return	0;
	}
	// д��Ŀ¼��β��־
	infofile_fileunit_t	*pUnitInFile;
	pUnitInFile				= m_finfolist.push_back();
	memset(pUnitInFile, 0, sizeof(*pUnitInFile));
	pUnitInFile->type		= infofile_fileunit_t::TYPE_EOD;
	// �����ľͲ�����д��
	//pUnitInFile->namepos	= 0;
	//pUnitInFile->info.dir.entrynum		= 0;
	//pUnitInFile->info.dir.totalentrynum	= 0;

	// �޸ĸ�Ŀ¼��ͬ���ļ��������ļ���
	if( m_vectDirStack.size()>1 )
	{
		int	nThis, nFather;
		infofile_fileunit_t	*pUnitFather, *pUnitThis;
		nThis		= m_vectDirStack[m_vectDirStack.size()-1];
		nFather		= m_vectDirStack[m_vectDirStack.size()-2];
		pUnitFather	= m_finfolist.getptr(nFather);
		pUnitThis	= m_finfolist.getptr(nThis);
		assert(pUnitFather->type == infofile_fileunit_t::TYPE_DIR);
		pUnitFather->info.dir.entrynum		++;
		pUnitFather->info.dir.totalentrynum	+= 2+pUnitThis->info.dir.totalentrynum;
		// +2��Ҫ������Ŀ¼�Լ�����Ŀ�ͽ�β��־
	}
	// ջ�˳�
	m_vectDirStack.pop_back();
	return	0;
}
int	whzlib_pck_modifier_i::AddDir(const char *szDirInPck)
{
	char	szNew[WH_MAX_PATH];
	int		nID;
	if( (nID=LookForExistPath(szDirInPck, szNew))<0 )
	{
		return	-1;
	}
	if( szNew[0]==0 )
	{
		// ԭ�������ˣ������ټ���
		return	-1;
	}
	// �ֽ����Ŀ¼��һ��һ�����
	char	*pszDir[WH_MAX_PATHLEVEL], szTmp[WH_MAX_PATH];
	int		nNum = WH_MAX_PATHLEVEL;
	if( wh_strexplode(szNew, "/\\", szTmp, pszDir, &nNum)<0 )
	{
		return	-1;
	}
	for(int i=0;i<nNum;i++)
	{
		nID	= AddDirToDir(nID, pszDir[i]);
		if( nID<0 )
		{
			// ��ý����������д���ļ�
			ClrModified();
			assert(0);
			return	-1;
		}
	}
	SetModified();
	return	0;
}
int	whzlib_pck_modifier_i::AddFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
{
	char	szNew[WH_MAX_PATH];
	int		nID;
	if( (nID=LookForExistPath(szFileInPck, szNew))<0 )
	{
		return	-1;
	}
	if( szNew[0]==0 )
	{
		// ԭ�������ˣ������ټ���
		return	-1;
	}
	// �ֽ����Ŀ¼�����Ƿ��б�Ҫ��Ӹ�����Ŀ¼
	char	*pszDir[WH_MAX_PATHLEVEL], szTmp[WH_MAX_PATH];
	int		nNum = WH_MAX_PATHLEVEL;
	int		i;
	if( wh_strexplode(szNew, "/\\", szTmp, pszDir, &nNum)<0 )
	{
		return	-1;
	}
	if( nNum>1 )
	{
		// ��Ҫ��Ӹ�Ŀ¼��һ��һ�����
		for(i=0;i<nNum-1;i++)
		{
			nID	= AddDirToDir(nID, pszDir[i]);
			if( nID<0 )
			{
				// ��ý����������д���ļ�
				ClrModified();
				assert(0);
				return	-1;
			}
		}
	}
	// ����ļ�
	if( AddFileToDir(nID, pszDir[nNum-1], file, nCompressMode)<0 )
	{
		ClrModified();
		assert(0);
		return	-1;
	}

	SetModified();
	return	0;
}
int	whzlib_pck_modifier_i::ReplaceFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
{
	int	nPathID = GetPathID(szFileInPck);
	if( nPathID<0 )
	{
		// ������
		return	-1;
	}
	infofile_fileunit_t	*pUnit = GetUnitPtr(nPathID);
	if(pUnit->type != infofile_fileunit_t::TYPE_FILE )
	{
		// �����ļ�
		return	-1;
	}
	// ��¼�˷ѵĿռ�
	m_infohdr.hdr.nWastedSize	+= pUnit->info.file.totaldatasize;
	//
	::fseek(m_fpDat, 0, SEEK_END);
	pUnit->info.file.datapos		= ::ftell(m_fpDat);
	pUnit->info.file.totaldatasize	= file->FileSize();
	// д������
	if( WriteData(pUnit, file, nCompressMode)<0 )
	{
		return	-1;
	}

	SetModified();
	return	0;
}
int	whzlib_pck_modifier_i::DelPath(const char *szPathInPck)
{
	int	nPathID = GetPathID(szPathInPck);
	if( nPathID<0 )
	{
		// ������
		return	-1;
	}

	infofile_fileunit_t	*pUnit = GetUnitPtr(nPathID);
	switch(pUnit->type)
	{
		case	infofile_fileunit_t::TYPE_DIR:
		{
			if( DelDir(nPathID)<0 )
			{
				ClrModified();
				return	-1;
			}
		}
		break;
		case	infofile_fileunit_t::TYPE_FILE:
		{
			if( DelFile(nPathID)<0 )
			{
				ClrModified();
				return	-1;
			}
		}
		break;
	}

	SetModified();
	return	0;
}

int	whzlib_pck_modifier_i::WriteData(infofile_fileunit_t *pFUnit, whzlib_file *file, int nCompressMode)
{
	int					rst;
	// д���ļ�ͷ
	datafile_file_hdr_t	hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.hdr.nParts		= whzlib_GetFilePartNum(file->FileSize(), m_infohdr.hdr.nUnitSize);
	rst		= ::fwrite(&hdr, 1, sizeof(hdr), m_fpDat);
	if( rst!=sizeof(hdr) )
	{
		assert(0);
		return	-1;
	}

	// ���ڼ���md5
	md5_state_t	md5state;
	md5_init(&md5state);

	// ��¼��������ͷ��λ�ã����ļ�ȫ��д�����д
	int	nPartIdxBeginOff	= ::ftell(m_fpDat);

	// �ж��Ƿ���0�����ļ�
	if( hdr.hdr.nParts > 0 )
	{
		// �ֶ�����(��д��ȥռ�ŵط�)
		whvector<int>	vectPartIdx(hdr.hdr.nParts);
		rst		= ::fwrite(vectPartIdx.getbuf(), 1, sizeof(int)*hdr.hdr.nParts, m_fpDat);
		if( rst!=int(sizeof(int)*hdr.hdr.nParts) )
		{
			assert(0);
			return	-1;
		}

		// ��¼��ʼλ��
		int	nBeginOff = ::ftell(m_fpDat);
		// �����
		int	nCount = 0;
		// �ֶ�д������
		size_t	nUnitSize = m_infohdr.hdr.nUnitSize;
		whvector<char>	vectIn(nUnitSize), vectOut(nUnitSize*2);
		while( !file->IsEOF() )
		{
			char	*pIn = vectIn.getbuf(), *pOut = vectOut.getbuf();
			int		nInSize;
			nInSize	= file->Read(pIn, nUnitSize);
			if( nInSize>0 )
			{
				md5_append(&md5state, (const md5_byte_t *)pIn, nInSize);

				datafile_dataunit_hdr_t	dhdr;
				dhdr.nMode	= nCompressMode;
				switch(nCompressMode)
				{
					case	COMPRESSMODE_ZIP:
						dhdr.nSize	= nUnitSize*2;
						if( whzlib_compress(pIn, nInSize, pOut, &dhdr.nSize)<0 )
						{
							assert(0);
							return	-1;
						}
						if( (int)dhdr.nSize>nInSize )
						{
							// �����粻ѹ��
							pOut		= pIn;
							dhdr.nSize	= nInSize;
							dhdr.nMode	= COMPRESSMODE_NOTHING;
						}
					break;
					default:
						pOut		= pIn;
						dhdr.nSize	= nInSize;
						dhdr.nMode	= COMPRESSMODE_NOTHING;
					break;
				}
				// �������ʼ���λ��
				vectPartIdx[nCount]	= ::ftell(m_fpDat) - nBeginOff;
				// д���ļ�ͷ
				rst	= ::fwrite(&dhdr, 1, sizeof(dhdr), m_fpDat);
				if( rst!=sizeof(dhdr) )
				{
					assert(0);
					return	-1;
				}
				// ���������ͼ���
				if( m_szPassword[0] )
				{
					m_des.encrypt((unsigned char *)pOut, dhdr.nSize);
				}
				// д������
				rst	= ::fwrite(pOut, 1, dhdr.nSize, m_fpDat);
				if( rst!=(int)dhdr.nSize )
				{
					assert(0);
					return	-1;
				}
				//
				nCount	++;
			}
			else
			{
				break;
			}
		}

		// ��д������ͷ
		::fseek(m_fpDat, nPartIdxBeginOff, SEEK_SET);
		rst		= ::fwrite(vectPartIdx.getbuf(), 1, sizeof(int)*hdr.hdr.nParts, m_fpDat);
		if( rst!=int(sizeof(int)*hdr.hdr.nParts) )
		{
			assert(0);
			return	-1;
		}

		// ����seek���ļ�β
		::fseek(m_fpDat, 0, SEEK_END);
	}

	// ��¼md5ֵ
	md5_finish(&md5state, pFUnit->md5);

	// ��¼�ļ�ʱ��
	pFUnit->time	= file->FileTime();

	return	0;
}
int	whzlib_pck_modifier_i::WriteFileName(const char *szFileName)
{
	return	whzlib_WriteFileName(m_fpNam, szFileName);
}
int	whzlib_pck_modifier_i::ReadFileName(int nPos, char *szFileName, int nSize)
{
	return	whzlib_ReadFileName(m_fpNam, nPos, szFileName, nSize);
}
WHDirBase *	whzlib_pck_modifier_i::OpenDir(int nDirID)
{
	if( nDirID<0 || nDirID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	NULL;
	}

	infofile_fileunit_t	*pUnitInFile = m_finfolist.getptr(nDirID);
	if( pUnitInFile->type != infofile_fileunit_t::TYPE_DIR )
	{
		assert(0);
		return	NULL;
	}

	WHDirInPck4Mod	*dir = new WHDirInPck4Mod;
	if( !dir )
	{
		return	NULL;
	}
	dir->m_info.nDirID	= nDirID+1;
	dir->m_info.pReader	= this;
	dir->Rewind();

	return		dir;
}
WHDirBase *	whzlib_pck_modifier_i::OpenDir(const char *szDir)
{
	// �ҵ�����Ӧ��FileID��Ȼ���������ķ���
	return	OpenDir(GetPathID(szDir));
}

int	whzlib_pck_modifier_i::FindEntry(int nDirID, const char *szName)
{
	if( nDirID<0 || nDirID>=(int)m_finfolist.size() )
	{
		// �Ƿ���ID
		return	-1;
	}

	// ��ñ�dir�Ľṹ
	infofile_fileunit_t	*pUnit = GetUnitPtr(nDirID);
	if( pUnit->type != infofile_fileunit_t::TYPE_DIR )
	{
		// ֻ��dir�ſ�������ʼ����
		assert(0);
		return	-1;
	}

	// ʹ������Ϳ���ȫ��ͳһ�Ƿ��Сд���л�����
	whstr4hash	tofind(szName);
	char		szPathName[WH_MAX_PATH];
	pUnit	++;
	nDirID	++;
	while( 1 )
	{
		switch( pUnit->type )
		{
			case	infofile_fileunit_t::TYPE_DIR:
			case	infofile_fileunit_t::TYPE_FILE:
			{
				if( ReadFileName(pUnit->namepos, szPathName, sizeof(szPathName))>0 )
				{
					if( tofind == szPathName )
					{
						// �ҵ���
						return	nDirID;
					}
				}
			}
			break;
		}
		switch( pUnit->type )
		{
			case	infofile_fileunit_t::TYPE_DIR:
			{
				// ��������Ŀ¼���������
				nDirID	+= pUnit->info.dir.totalentrynum + 2;
				pUnit	+= pUnit->info.dir.totalentrynum + 2;
			}
			break;
			case	infofile_fileunit_t::TYPE_FILE:
			{
				// ������һ���ļ�
				nDirID	++;
				pUnit	++;
			}
			break;
			case	infofile_fileunit_t::TYPE_EOD:
			{
				// ֱ���ҵ�����ͽ�����
				goto	End;
			}
			break;
		}
	}

End:
	// û�ҵ�
	return	-1;
}
int	whzlib_pck_modifier_i::AddDirToDir(int nDirID, const char *szName)
{
	// ��ñ�dir�Ľṹ
	// 2004-02-06 ���󰡣�pUnit�ں���ļ���insert������������Ϊresize����realloc������ָ��ʧЧ��
	infofile_fileunit_t	*pUnit = GetUnitPtr(nDirID);
	assert(pUnit->type == infofile_fileunit_t::TYPE_DIR);

	// д������
	::fseek(m_fpNam, 0, SEEK_END);
	infofile_fileunit_t	DirUnit;
	DirUnit.type	= infofile_fileunit_t::TYPE_DIR;
	DirUnit.namepos	= ::ftell(m_fpNam);
	DirUnit.info.dir.entrynum		= 0;
	DirUnit.info.dir.totalentrynum	= 0;
	if( WriteFileName(szName)<0 )
	{
		return	-1;
	}

	EXTUNIT_T	ExtUnit;
	ExtUnit.nFatherID	= nDirID;

	// �ҵ���β������������¼����ʼ�ͽ�β�������ﻹ����
	int	nIPoint = nDirID + pUnit->info.dir.totalentrynum;

	m_finfolist.insertafter(nIPoint, DirUnit);
	memset(&DirUnit, 0, sizeof(DirUnit));
	DirUnit.type	= infofile_fileunit_t::TYPE_EOD;
	m_finfolist.insertafter(nIPoint+1, DirUnit);

	m_fextlist.insertafter(nIPoint, ExtUnit);
	m_fextlist.insertafter(nIPoint, ExtUnit);

	// ��Ŀ¼�ı���entry����1
	// ������Ѿ�ʧЧ��
	// ԭ���������ģ�pUnit->info.dir.entrynum	++;
	GetUnitPtr(nDirID)->info.dir.entrynum	++;
	// �޸ĸ�����Ŀ¼����entry��+2
	AddNumToAllFather(nDirID, 2);

	// �޸Ĳ����֮�������ܵ�Ӱ��ĸ�Ŀ¼����
	// ��������nIPoint+2֮���Ԫ�أ�������ID>nIPoint������Ҫ����Ҫ+2
	AdjustExtUnit(nIPoint+2, nIPoint, 2);

	return	nIPoint+1;
}
int	whzlib_pck_modifier_i::AddFileToDir(int nDirID, const char *szName, whzlib_file *file, int nCompressMode)
{
	// ��ñ�dir�Ľṹ
	infofile_fileunit_t	*pDirUnit = GetUnitPtr(nDirID);
	assert(pDirUnit->type == infofile_fileunit_t::TYPE_DIR);

	::fseek(m_fpNam, 0, SEEK_END);
	::fseek(m_fpDat, 0, SEEK_END);
	infofile_fileunit_t	FileUnit;
	FileUnit.type		= infofile_fileunit_t::TYPE_FILE;
	FileUnit.namepos	= ::ftell(m_fpNam);
	FileUnit.info.file.datapos		= ::ftell(m_fpDat);
	FileUnit.info.file.totaldatasize= file->FileSize();
	// д������
	if( WriteFileName(szName)<0 )
	{
		return	-1;
	}
	// д������
	if( WriteData(&FileUnit, file, nCompressMode)<0 )
	{
		return	-1;
	}

	EXTUNIT_T	ExtUnit;
	ExtUnit.nFatherID	= nDirID;

	// �ҵ���β������һ����¼
	int	nIPoint = nDirID + pDirUnit->info.dir.totalentrynum;
	m_finfolist.insertafter(nIPoint, FileUnit);
	m_fextlist.insertafter(nIPoint, ExtUnit);

	// ��Ŀ¼�ı���entry����1
	GetUnitPtr(nDirID)->info.dir.entrynum	++;
	// �޸ĸ�����Ŀ¼����entry��+1
	AddNumToAllFather(nDirID, 1);

	// �޸Ĳ����֮�������ܵ�Ӱ��ĸ�Ŀ¼����
	// ��������nIPoint+1֮���Ԫ�أ�������ID>nIPoint������Ҫ����Ҫ+1
	AdjustExtUnit(nIPoint+1, nIPoint, 1);

	return	nIPoint+1;
}
int	whzlib_pck_modifier_i::AddNumToAllFather(int nDirID, int nNum)
{
	while(nDirID>=0)
	{
		EXTUNIT_T			*pExt = m_fextlist.getptr(nDirID);
		infofile_fileunit_t	*pUnit = GetUnitPtr(nDirID);
		//pUnit->info.dir.entrynum		+= nNum;
		pUnit->info.dir.totalentrynum	+= nNum;
		nDirID	= pExt->nFatherID;
	}
	return	0;
}
int	whzlib_pck_modifier_i::AdjustExtUnit(int nCheckAfter, int nEffectAfter, int nAdjust)
{
	size_t	i;
	for(i=nCheckAfter;i<m_fextlist.size();i++)
	{
		EXTUNIT_T	*pExt = m_fextlist.getptr(i);
		if( pExt->nFatherID>nEffectAfter )
		{
			pExt->nFatherID	+= nAdjust;
		}
	}
	return	0;
}
int	whzlib_pck_modifier_i::DelDir(int nDirID)
{
	infofile_fileunit_t	*pUnit = GetUnitPtr(nDirID);
	int					nFatherID = m_fextlist.getptr(nDirID)->nFatherID;
	infofile_fileunit_t	*pFather = GetUnitPtr(nFatherID);	// ֻҪ����ɾ��Ŀ¼��һ����

	int	nNum = pUnit->info.dir.totalentrynum+2;
	assert(nDirID+nNum<=int(m_finfolist.size()));

	// ��¼���н�Ҫ��ɾ�����ļ����ȵ��˷ѵ�������
	for(int i=nDirID;i<nDirID+nNum;i++)
	{
		infofile_fileunit_t	*pUnit = GetUnitPtr(i);
		if( pUnit->type == infofile_fileunit_t::TYPE_FILE )
		{
			m_infohdr.hdr.nWastedSize	+= pUnit->info.file.totaldatasize;
		}
	}

	// ɾ����Ŀ¼���������������ļ���Ŀ¼
	m_finfolist.delrange(nDirID, nNum);
	m_fextlist.delrange(nDirID, nNum);

	// ��Ŀ¼�ı���entry��1
	pFather->info.dir.entrynum	--;
	// �޸ĸ�����Ŀ¼����entry����ȥ�ܹ����ٵ�entry��
	AddNumToAllFather(nFatherID, -nNum);

	// �޸Ĳ����֮�������ܵ�Ӱ��ĸ�Ŀ¼����
	// ��������nDirID֮���Ԫ�أ�������ID>nDirID������Ҫ����Ҫ-nNum
	AdjustExtUnit(nDirID, nDirID, -nNum);

	return	0;
}
int	whzlib_pck_modifier_i::DelFile(int nFileID)
{
	infofile_fileunit_t	*pUnit = GetUnitPtr(nFileID);
	int					nFatherID = m_fextlist.getptr(nFileID)->nFatherID;
	infofile_fileunit_t	*pFather = GetUnitPtr(nFatherID);	// ֻҪ����ɾ��Ŀ¼��һ����

	// ��¼�˷ѵĿռ�
	m_infohdr.hdr.nWastedSize	+= pUnit->info.file.totaldatasize;

	// һ���ļ���ɾ��
	m_finfolist.delrange(nFileID, 1);
	m_fextlist.delrange(nFileID, 1);

	// ��Ŀ¼�ı���entry��1
	pFather->info.dir.entrynum	--;
	// �޸ĸ�����Ŀ¼����entry����ȥ�ܹ����ٵ�entry��
	AddNumToAllFather(nFatherID, -1);

	// �޸Ĳ����֮�������ܵ�Ӱ��ĸ�Ŀ¼����
	// ��������nDirID֮���Ԫ�أ�������ID>nDirID������Ҫ����Ҫ-nNum
	AdjustExtUnit(nFileID, nFileID, -1);

	return	0;
}
