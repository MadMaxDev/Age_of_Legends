// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace	: n_whzlib
// File			: whzlib_i.cpp
// Creator		: Wei Hua (κ��)
// Comment		: whzlib���ڲ�ʵ��
// CreationDate	: 2003-09-04
// ChangeLOG	:
// 2004-02-20	: ������whzlib_filemodifyman_i::PutFileToDisk���ļ�ʱ����޸�
// 2005-01-14	: �޸���single compress��pIn��pOut���������´�ѹ����ʱ��û�лָ���ȥ�Ĵ���
//              : ��ʱ������˼�zlib���޸�In��buf��������������

#include "../inc/whzlib_pck_i.h"
#include "../inc/whfile.h"
#include "../inc/whtime.h"
#include <sys/stat.h>

namespace n_whzlib
{

const char *sc_file_hdr_t::CSZ_MAGIC	= "whz_single";

int		whzlib_single_compress_i(whzlib_file *file, const char *cszDstFile, const char *cszPasswd)
{
	// ���ļ� to д
	FILE	*fp = fopen(cszDstFile, "wb");
	if( !fp )
	{
		return	-1;
	}

	// ��֤���ļ�ͷ��ʼ������Ϊ���ڷ���Ҳ�Ǵ�file����ļ��ܳ����������ֻ�����ļ�һ���ֵ����
	file->Seek(0, SEEK_SET);

	// д���ļ�ͷ����ռ��λ�ã���Ȼ���û�����ݲ��־Ͳ����ˣ�
	int				rst;
	sc_file_hdr_t	schdr;
	schdr.hdr.time		= file->FileTime();
	schdr.hdr.nFileSize	= file->FileSize();
	schdr.hdr.nParts	= whzlib_GetFilePartNum(schdr.hdr.nFileSize, schdr.hdr.nUnitSize);
	rst				= ::fwrite(&schdr, 1, sizeof(schdr), fp);
	if( rst!=sizeof(schdr) )
	{
		fclose(fp);
		return	-1;
	}

	// ���ڼ���
	WHSimpleDES	des;
	bool		bUseDes = false;
	if( cszPasswd && cszPasswd[0] )
	{
		bUseDes		= true;
		WHDES_ConvertToFixed(cszPasswd, strlen(cszPasswd), (char *)des.m_key, sizeof(des.m_key));
		des.setmask(cszPasswd, strlen(cszPasswd));
	}

	// ���ڼ���md5
	md5_state_t	md5state;
	md5_init(&md5state);

	size_t	nUnitSize = schdr.hdr.nUnitSize;
	whvector<char>	vectIn(nUnitSize), vectOut(nUnitSize*2);

	// �ж��Ƿ���0�����ļ�
	if( schdr.hdr.nParts > 0 )
	{
		int	nDPartSize = sizeof(int)*schdr.hdr.nParts;

		// �ֶ�����(��д��ȥռ�ŵط�)
		whvector<int>	vectPartIdx(schdr.hdr.nParts);
		rst		= ::fwrite(vectPartIdx.getbuf(), 1, nDPartSize, fp);
		if( rst != nDPartSize )
		{
			fclose(fp);
			return	-1;
		}
		// ��¼��ʼλ��
		int		nBeginOff = ::ftell(fp);
		// �����
		int		nCount = 0;
		// �ֶ�д������

		while( !file->IsEOF() )
		{
			char	*pIn = vectIn.getbuf(), *pOut = vectOut.getbuf();

			int		nInSize;
			nInSize	= file->Read(pIn, nUnitSize);
			if( nInSize>0 )
			{
				md5_append(&md5state, (const md5_byte_t *)pIn, nInSize);

				sc_dataunit_hdr_t	dhdr;
				dhdr.nMode	= COMPRESSMODE_ZIP;
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
				// �������ʼ���λ��
				vectPartIdx[nCount]	= ::ftell(fp) - nBeginOff;
				// д���ļ�ͷ
				rst	= ::fwrite(&dhdr, 1, sizeof(dhdr), fp);
				if( rst!=sizeof(dhdr) )
				{
					assert(0);
					return	-1;
				}
				// ���������ͼ���
				if( bUseDes )
				{
					des.encrypt((unsigned char *)pOut, dhdr.nSize);
				}
				// д������
				rst	= ::fwrite(pOut, 1, dhdr.nSize, fp);
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
				// �ļ�����
				break;
			}
		}

		// ��¼md5ֵ
		md5_finish(&md5state, schdr.hdr.md5);

		// ��д���ĵĲ���
		::fseek(fp, 0, SEEK_SET);
		// ��д�ļ�ͷ(��Ϊmd5������)
		rst		= ::fwrite(&schdr, 1, sizeof(schdr), fp);
		if( rst!=sizeof(schdr) )
		{
			assert(0);
			return	-1;
		}
		// ��д�ֶ�����
		rst		= ::fwrite(vectPartIdx.getbuf(), 1, nDPartSize, fp);
		if( rst!=nDPartSize )
		{
			assert(0);
			return	-1;
		}
	}
	else
	{
		md5_finish(&md5state, schdr.hdr.md5);
		// ��д���ĵĲ���
		::fseek(fp, 0, SEEK_SET);
		// ��д�ļ�ͷ(��Ϊmd5������)
		rst		= ::fwrite(&schdr, 1, sizeof(schdr), fp);
		if( rst!=sizeof(schdr) )
		{
			assert(0);
			return	-1;
		}
	}

	fclose(fp);
	return	0;
}
bool	whzlib_issinglecompressed_i(whzlib_file *srcfile, sc_file_hdr_t *hdr)
{
	// �ص��ļ���ʼ
	srcfile->Seek(0, SEEK_SET);

	// �����ļ�ͷ
	int		rst;
	rst		= srcfile->Read(hdr, sizeof(*hdr));
	if( rst != sizeof(*hdr) )
	{
		return	false;
	}

	// �ж��Ƿ��ǺϷ���ͷ
	if( !hdr->IsMagicGood()
	||  !hdr->IsVerGood()
	)
	{
		return	false;
	}

	return	true;
}

}		// EOF namespace n_whzlib

using namespace n_whcmn;
using namespace n_whzlib;

////////////////////////////////////////////////////////////////////
// ������whzlib_fileman_iʵ��
////////////////////////////////////////////////////////////////////
whzlib_fileman_i::whzlib_fileman_i()
: m_nSearchMode(SEARCHMODE_1PCK2FILE)
{
	m_szPassword[0]	= 0;
}
whzlib_fileman_i::~whzlib_fileman_i()
{
	// �ر����еİ��ļ�
	if( m_mapName2Pck.size()>0 )
	{
		whvector<whzlib_pck_reader_i *>	vectVal;
		m_mapName2Pck.exportvaltovector(vectVal);
		for(size_t i=0;i<vectVal.size();i++)
		{
			delete	vectVal[i];
		}
		m_mapName2Pck.clear();
	}
}
void	whzlib_fileman_i::SetSearchMode(int nMode)
{
	m_nSearchMode	= nMode;
}
int		whzlib_fileman_i::GetSearchMode() const
{
	return	m_nSearchMode;
}
void	whzlib_fileman_i::SetPassword(const char *szPassword)
{
	if( szPassword )
	{
		WH_STRNCPY0(m_szPassword, szPassword);
	}
	else
	{
		m_szPassword[0]	= 0;
	}
}
whzlib_file_i *	whzlib_fileman_i::OpenFromDisk(const char *szFileName, const char *szMode)
{
	// ������Ƕ���ʽ��ֱ�Ӵ�Ӳ���ϴ�
	if( strcmp(szMode, "rb")!=0 )
	{
		return	static_cast<whzlib_file_i *>(whzlib_OpenCmnFile(szFileName, szMode));
	}
	// ��������ȼ���ڴ棬Ȼ���ٴ�Ӳ���϶���
	return	static_cast<whzlib_file_i *>(m_fcache.OpenFile(szFileName, true));
}
whzlib_file_i *	whzlib_fileman_i::OpenFromDisk(const char *szFileName)
{
	return	OpenFromDisk(szFileName, "rb");
}
whzlib_file_i *	whzlib_fileman_i::OpenFromPackage(const char *szFileName)
{
	// ����·�������ڵ��ĸ�����
	char	szPath[WH_MAX_PATH], szFile[WH_MAX_PATH];
	if( !_LookForPackageByfile(szFileName, szPath, szFile, false) )
	{
		return	NULL;
	}

	whzlib_pck_reader_i	*pReader = CheckAndLoadReader(szPath);
	if( pReader )
	{
		if( m_szPassword[0] )
		{
			pReader->SetPassword(m_szPassword);
		}
		// ����Ӧ�ļ�
		return	pReader->OpenFile(szFile);
	}

	return	NULL;
}
whzlib_pck_reader_i	*	whzlib_fileman_i::CheckAndLoadReader(const char *szPath)
{
	// �������ļ��Ƿ��Ѿ���
	whzlib_pck_reader_i	*pReader = NULL;
	if( !m_mapName2Pck.get(szPath, pReader) )
	{
		// û�д򿪾ʹ򿪰�
		pReader	= new whzlib_pck_reader_i;
		if( pReader )
		{
			// ��������ȴ��ڴ�cache�д�
			if( pReader->OpenPck(szPath, &m_fcache) < 0 )
			{
				delete	pReader;
				pReader	= NULL;
			}
			else
			{
				// ����ӳ��
				bool bval = m_mapName2Pck.put(szPath, pReader);
				assert(bval);
			}
		}
		else
		{
			assert(0);
		}
	}

	return	pReader;
}

int		whzlib_fileman_i::LoadPckToMem(const char *szPckName)
{
	// ��pck��ص������ļ�������
	PCKFILENAMES_T	fs;
	whzlib_getpckfilenames(szPckName, &fs);
	if( LoadFileToMem(fs.szIdx)<0
	||  LoadFileToMem(fs.szDat)<0
	||  LoadFileToMem(fs.szNam)<0
	)
	{
		return	-1;
	}
	return		0;
}
int		whzlib_fileman_i::UnLoadPckFromMem(const char *szPckName)
{
	// ��pck��ص������ļ����ͷ�
	PCKFILENAMES_T	fs;
	whzlib_getpckfilenames(szPckName, &fs);
	if( UnLoadFileFromMem(fs.szIdx)<0
	||  UnLoadFileFromMem(fs.szDat)<0
	||  UnLoadFileFromMem(fs.szNam)<0
	)
	{
		return	-1;
	}
	return		0;
}
int		whzlib_fileman_i::LoadFileToMem(const char *szFileName)
{
	return		m_fcache.LoadFile(szFileName);
}
int		whzlib_fileman_i::UnLoadFileFromMem(const char *szFileName)
{
	return		m_fcache.UnLoadFile(szFileName);
}
whzlib_file *	whzlib_fileman_i::Open(const char *szFileName, const char *szMode)
{
	whzlib_file_i	*file = NULL;

	if( szMode && strcmp(szMode, "rb")!=0 )
	{
		// ֻҪ���Ƕ���ʽ�Ͱ������ļ���ʽ���ļ�
		file	= OpenFromDisk(szFileName, szMode);
	}
	else
	{
		switch(m_nSearchMode)
		{
			case	SEARCHMODE_1PCK2FILE:
			{
				if( (file=OpenFromPackage(szFileName))==NULL )
				{
					file	= OpenFromDisk(szFileName);
				}
			}
			break;
			case	SEARCHMODE_1FILE2PCK:
			{
				if( (file=OpenFromDisk(szFileName))==NULL )
				{
					file	= OpenFromPackage(szFileName);
				}
			}
			break;
			case	SEARCHMODE_PCKONLY:
			{
				file	= OpenFromPackage(szFileName);
			}
			break;
			case	SEARCHMODE_FILEONLY:
			{
				file	= OpenFromDisk(szFileName);
			}
			break;
		}
	}

	return	file;
}
bool	whzlib_fileman_i::Close(whzlib_file *file)
{
	// ɾ���ö���
	delete	file;
	return	true;
}
int		whzlib_fileman_i::GetFileSize(const char *szFileName)
{
	int	nSize = 0;
	switch(m_nSearchMode)
	{
		case	SEARCHMODE_1PCK2FILE:
		{
			if( (nSize=GetFileSizeFromPackage(szFileName))<0 )
			{
				nSize	= whfile_getfilesize(szFileName);
			}
		}
		break;
		case	SEARCHMODE_1FILE2PCK:
		{
			if( (nSize=whfile_getfilesize(szFileName))<0 )
			{
				nSize	= GetFileSizeFromPackage(szFileName);
			}
		}
		break;
		case	SEARCHMODE_PCKONLY:
		{
			nSize	= GetFileSizeFromPackage(szFileName);
		}
		break;
		case	SEARCHMODE_FILEONLY:
		{
			nSize	= whfile_getfilesize(szFileName);
		}
		break;
	}

	return	nSize;
}
bool	whzlib_fileman_i::IsFileExist(const char *szFileName)
{
	bool	bRst =false;
	switch(m_nSearchMode)
	{
		case	SEARCHMODE_1PCK2FILE:
		{
			if( !(bRst=IsFileExistFromPackage(szFileName)) )
			{
				bRst	= whfile_ispathexisted(szFileName);
			}
		}
		break;
		case	SEARCHMODE_1FILE2PCK:
		{
			if( !(bRst=whfile_ispathexisted(szFileName)) )
			{
				bRst	= IsFileExistFromPackage(szFileName);
			}
		}
		break;
		case	SEARCHMODE_PCKONLY:
		{
			bRst	= IsFileExistFromPackage(szFileName);
		}
		break;
		case	SEARCHMODE_FILEONLY:
		{
			bRst	= whfile_ispathexisted(szFileName);
		}
		break;
	}

	return	bRst;
}
WHDirBase *	whzlib_fileman_i::OpenDir(const char *szDirName)
{
	// �����ǵ������б��
	char	szTmpDirName[WH_MAX_PATH];
	WH_STRNCPY0(szTmpDirName, szDirName);
	wh_strkickendslash(szTmpDirName);
	szDirName	= szTmpDirName;

	WHDirBase	*dir = NULL;
	switch(m_nSearchMode)
	{
		case	SEARCHMODE_1PCK2FILE:
		{
			if( (dir=OpenDirFromPackage(szDirName))==NULL )
			{
				dir	= OpenDirFromDisk(szDirName);
			}
		}
		break;
		case	SEARCHMODE_1FILE2PCK:
		{
			if( (dir=OpenDirFromDisk(szDirName))==NULL )
			{
				dir	= OpenDirFromPackage(szDirName);
			}
		}
		break;
		case	SEARCHMODE_PCKONLY:
		{
			dir	= OpenDirFromPackage(szDirName);
		}
		break;
		case	SEARCHMODE_FILEONLY:
		{
			dir	= OpenDirFromDisk(szDirName);
		}
		break;
	}

	return	dir;
}
WHDirBase *	whzlib_fileman_i::OpenDirFromDisk(const char *szDirName)
{
	WHDirCarePck	*dir = new WHDirCarePck;
	if( dir )
	{
		if( dir->Open(szDirName)<0 )
		{
			delete	dir;
			dir	= NULL;
		}
	}
	return	dir;
}
WHDirBase *	whzlib_fileman_i::OpenDirFromPackage(const char *szDirName)
{
	// ����·�������ڵ��ĸ�����
	char	szPath[WH_MAX_PATH], szDir[WH_MAX_PATH];
	if( !_LookForPackageByfile(szDirName, szPath, szDir, true) )
	{
		return	NULL;
	}

	whzlib_pck_reader_i	*pReader = CheckAndLoadReader(szPath);
	if( pReader )
	{
		// ����Ӧ�ļ�
		return	pReader->OpenDir(szDir);
	}

	return		NULL;
}
int	whzlib_fileman_i::GetFileSizeFromPackage(const char *szFileName)
{
	// ����·�������ڵ��ĸ�����
	char	szPath[WH_MAX_PATH], szFile[WH_MAX_PATH];
	if( !_LookForPackageByfile(szFileName, szPath, szFile, false) )
	{
		return	-1;
	}
	whzlib_pck_reader_i	*pReader = CheckAndLoadReader(szPath);
	if( pReader )
	{
		// �ҵ���Ӧ��¼����ó���
		return	pReader->GetFileSize(szFile);
	}

	return	-1;
}
bool	whzlib_fileman_i::IsFileExistFromPackage(const char *szFileName)
{
	return	GetFileSizeFromPackage(szFileName)>=0;
}
bool	whzlib_fileman_i::_LookForPackageByfile(const char *szFileName, char *szPath, char *szFile, bool bIsDir)
{
	int		i;
	int		len;
	strcpy(szPath, szFileName);
	len		= strlen(szPath);
	if( bIsDir )
	{
		szPath[len]	= '/';
		len			++;
		szPath[len]	= 0;
	}

	for(i=len-1;i>0;i--)
	{
		switch(szPath[i])
		{
			case	'/':
			case	'\\':
				sprintf(szPath+i, ".%s", cstIdxFileExt);
				if( _IsFileExistInMemOrOnDisk(szPath) )
				{
					sprintf(szPath+i, ".%s", cstNamFileExt);
					if( _IsFileExistInMemOrOnDisk(szPath) )
					{
						sprintf(szPath+i, ".%s", cstDatFileExt);
						if( _IsFileExistInMemOrOnDisk(szPath) )
						{
							// �ҵ��˰��ļ�
							szPath[i]	= 0;
							if( i==len-1 )
							{
								// ˵�����Ǹ�
								szFile[0]	= 0;
							}
							else
							{
								// ����ļ���
								strcpy(szFile, szFileName+i+1);
							}
							return	true;
						}
					}
				}
			break;
		}
	}
	// Kao��û�д��ڵİ�
	return	false;
}
bool	whzlib_fileman_i::_IsFileExistInMemOrOnDisk(const char *szPath)
{
	if( m_fcache.HasFileInMem(szPath) )
	{
		return	true;
	}
	// Ȼ���ж��Ƿ���Ӳ����
	return	whfile_ispathexisted(szPath);
}

// Ѱ��szFileName�Ƿ�����ڰ��ļ��У���������ļ����ֽ�Ϊ������԰����ļ���
// ��������û��"/"��β
bool	LookForPackageByfile(const char *szFileName, char *szPath, char *szFile, bool bIsDir)
{
	int		i;
	int		len;
	strcpy(szPath, szFileName);
	len		= strlen(szPath);
	if( bIsDir )
	{
		szPath[len]	= '/';
		len			++;
		szPath[len]	= 0;
	}

	for(i=len-1;i>0;i--)
	{
		switch(szPath[i])
		{
			case	'/':
			case	'\\':
				sprintf(szPath+i, ".%s", cstIdxFileExt);
				if( whfile_ispathexisted(szPath) )
				{
					sprintf(szPath+i, ".%s", cstNamFileExt);
					if( whfile_ispathexisted(szPath) )
					{
						sprintf(szPath+i, ".%s", cstDatFileExt);
						if( whfile_ispathexisted(szPath) )
						{
							// �ҵ��˰��ļ�
							szPath[i]	= 0;
							if( i==len-1 )
							{
								// ˵�����Ǹ�
								szFile[0]	= 0;
							}
							else
							{
								// ����ļ���
								strcpy(szFile, szFileName+i+1);
							}
							return	true;
						}
					}
				}
			break;
		}
	}
	// Kao��û�д��ڵİ�
	return	false;
}
bool	LookForLongestDirByfile(const char *szFileName, char *szPath, char *szFile)
{
	int		i, prev=0;
	int		len;
	strcpy(szPath, szFileName);
	len		= strlen(szPath);

	for(i=len-1;i>0;i--)
	{
		switch(szPath[i])
		{
			case	'/':
			case	'\\':
				szPath[i]	= 0;
				if( whfile_ispathexisted(szPath) && prev>0 )
				{
					szPath[i]		= '/';
					szPath[prev]	= 0;
					// ����ļ���
					strcpy(szFile, szFileName+prev+1);
					return	true;
				}
				prev	= i;
			break;
		}
	}
	// Kao��û�д��ڵİ�
	if( prev == 0)
	{
		return	false;
	}

	szPath[prev]	= 0;
	// ����ļ���
	strcpy(szFile, szFileName+prev+1);

	return	true;
}
////////////////////////////////////////////////////////////////////
// �����¹�����whzlib_filemodifyman_iʵ��
////////////////////////////////////////////////////////////////////
//
whzlib_filemodifyman_i::whzlib_filemodifyman_i()
{
	m_szPassword[0]	= 0;
}
whzlib_filemodifyman_i::~whzlib_filemodifyman_i()
{
	// �ر����еİ��ļ�
	if( m_mapName2Pck.size()>0 )
	{
		whvector<whzlib_pck_modifier_i *>	vectVal;
		m_mapName2Pck.exportvaltovector(vectVal);
		for(size_t i=0;i<vectVal.size();i++)
		{
			delete	vectVal[i];
		}
		m_mapName2Pck.clear();
	}
}
void	whzlib_filemodifyman_i::SetPassword(const char *szPassword)
{
	if( szPassword )
	{
		WH_STRNCPY0(m_szPassword, szPassword);
	}
	else
	{
		m_szPassword[0]	= 0;
	}
}
int		whzlib_filemodifyman_i::PutFile(const char *szFileName, whzlib_file *file, bool bShouldInPck, int nCompressMode)
{
	whzlib_pck_modifier_i	*pModifier = NULL;
	bool	bForceCreate = false;

	// ���ж�Ŀ���ļ�λ���Ƿ��ڰ���
	char	szPath[WH_MAX_PATH], szFileInPack[WH_MAX_PATH];
	if( !LookForPackageByfile(szFileName, szPath, szFileInPack, false) )
	{
		if( bShouldInPck )
		{
			// �ļ�Ӧ���ڰ���
			if( (m_nProperty & PROPERTY_AUTOCREATEPCK)
			&&  LookForLongestDirByfile(szFileName, szPath, szFileInPack)
			)
			{
				// �ٺ����Զ�������
				bForceCreate	= true;
			}
			else
			{
				return	-1;
			}
		}
		else
		{
			// �ڴ����ϣ�ֱ�ӿ������ݼ���
			return	PutFileToDisk(szFileName, file);
		}
	}
	// Ӧ���ڰ��У������Զ�����
	pModifier = CheckAndLoadModifier(szPath, bForceCreate);

	if( !pModifier )
	{
		// ������
		return	-1;
	}
	if( m_szPassword[0] )
	{
		pModifier->SetPassword(m_szPassword);
	}

	if( file->FileSize()==0 )
	{
		// ɾ����Ӧ�ļ�
		pModifier->DelPath(szFileInPack);
	}
	else
	{
		// �Ȱ����滻��
		if( pModifier->ReplaceFile(szFileInPack, file, nCompressMode)<0 )
		{
			// �ٰ�������ļ���
			if( pModifier->AddFile(szFileInPack, file, nCompressMode)<0 )
			{
				// �Ǿ��Ǵ���
				return	-1;
			}
		}
	}

	// �ɹ���
	return	0;
}
int		whzlib_filemodifyman_i::DelFile(const char *szFileName)
{
	whzlib_pck_modifier_i	*pModifier = NULL;

	// ���ж�Ŀ���ļ�λ���Ƿ��ڰ���
	char	szPath[WH_MAX_PATH], szFileInPack[WH_MAX_PATH];
	if( !LookForPackageByfile(szFileName, szPath, szFileInPack, false) )
	{
		// ֱ��ɾ��Ӳ���ϵ��ļ�
		if( whdir_sysdelfile(szFileName)<0 )
		{
			return	-1;
		}
		return	0;
	}

	// Ӧ���ڰ��У������Զ�����
	pModifier = CheckAndLoadModifier(szPath, false);
	if( !pModifier )
	{
		// �޷����ذ�
		return	-2;
	}
	if( m_szPassword[0] )
	{
		pModifier->SetPassword(m_szPassword);
	}
	// ɾ����Ӧ�ļ�
	if( pModifier->DelPath(szFileInPack)<0 )
	{
		return	-3;
	}

	return	0;
}
int		whzlib_filemodifyman_i::DelDir(const char *szDir)
{
	assert(0);	// û��ʵ��
	return	0;
}
int		whzlib_filemodifyman_i::FlushAll()
{
	// ��������modifier��flush
	whvector<whzlib_pck_modifier_i *>	vect;
	m_mapName2Pck.exportvaltovector(vect);
	for(size_t i=0;i<vect.size();i++)
	{
		vect[i]->Flush();
	}
	return	0;
}
whzlib_pck_modifier_i *	whzlib_filemodifyman_i::CheckAndLoadModifier(const char *szPath, bool bForceCreate)
{
	// �������ļ��Ƿ��Ѿ���
	whzlib_pck_modifier_i	*pModifier = NULL;
	if( !m_mapName2Pck.get(szPath, pModifier) )
	{
		// û�д򿪾ʹ򿪰�
		pModifier	= new whzlib_pck_modifier_i;
		if( pModifier )
		{
			whzlib_pck_modifier_i::OPENINFO_T		info;
			info.szPckFile	= szPath;
			if( pModifier->OpenPck(&info) < 0 )
			{
				if( !bForceCreate )
				{
					// ֱ��ɾ��֮
					delete		pModifier;
					pModifier	= NULL;
				}
				else
				{
					// ԭ�������ھʹ���֮
					whzlib_pck_modifier_i::CREATEINFO_T	cinfo;
					cinfo.szPckFile	= szPath;
					cinfo.nUnitSize	= m_nUnitSize;
					if( pModifier->CreatePck(&cinfo)<0 )
					{
						delete		pModifier;
						pModifier	= NULL;
					}
					else
					{
						// ����һ�����ļ�
						pModifier->AppendDir("");
						pModifier->AppendDirEnd();
						pModifier->ClosePck();
						if( pModifier->OpenPck(&info) < 0 )
						{
							delete		pModifier;
							pModifier	= NULL;
						}
					}
				}
			}
			if( pModifier )
			{
				// ����ӳ��
				m_mapName2Pck.put(szPath, pModifier);
			}
		}
		else
		{
			assert(0);
		}
	}

	return	pModifier;
}
int		whzlib_filemodifyman_i::PutFileToDisk(const char *szFileName, whzlib_file *file)
{
	// ��֤Ŀ¼����
	char	szPath[WH_MAX_PATH];
	if( whfile_getfilepath(szFileName, szPath) )
	{
		if( !whfile_ispathexisted(szPath) )
		{
			whdir_MakeDir(szPath);
		}
	}
	FILE	*fp;
	fp		= ::fopen(szFileName, "wb");
	if( !fp )
	{
		// ����򲻿����Ըı��ļ�����
		if( whfile_makefilewritable(szFileName) )
		{
			fp	= ::fopen(szFileName, "wb");
			if( !fp )
			{
				return	-1;
			}
		}
		else
		{
			return	-1;
		}
	}

	char	szBuf[4096];
	int		nSize;
	// Ĭ��file�Ѿ�seek����ͷ�ˣ�������ڿ�ͷ����Ϊʹ���߾�������м�д
	while( !file->IsEOF() )
	{
		nSize	= file->Read(szBuf, sizeof(szBuf));
		if( nSize<=0 )
		{
			break;
		}
		::fwrite(szBuf, 1, nSize, fp);
	}

	fclose(fp);

	// �޸��ļ�ʱ��Ϊԭ����ʱ��
	time_t	FT = file->FileTime();
	if( FT>0 )
	{
		whfile_setmtime(szFileName, FT);
	}

	return	0;
}

////////////////////////////////////////////////////////////////////
// ��һ�ļ�ѹ�� single compress
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// whzlib_file_i_sc
////////////////////////////////////////////////////////////////////
whzlib_file_i_sc::whzlib_file_i_sc()
: m_srcfile(NULL)
{
	m_szPassword[0]	= 0;
}
whzlib_file_i_sc::~whzlib_file_i_sc()
{
}
int		whzlib_file_i_sc::InitFrom(whzlib_file *srcfile)
{
	if( !whzlib_issinglecompressed_i(srcfile, &m_schdr) )
	{
		// �ļ����ݲ��ǵ�ѹ����ʽ
		return	-1;
	}

	// ������
	m_srcfile	= srcfile;

	// ��ʼ��
	m_vectbuf.resize(m_schdr.hdr.nUnitSize);
	m_buf		= m_vectbuf.getbuf();
	m_vectsrcbuf.resize(m_schdr.hdr.nUnitSize);
	m_srcbuf	= m_vectsrcbuf.getbuf();

	// �����һ������(��һ��������һ����е�)
	m_curpart	= 0;
	m_totalpart	= whzlib_GetFilePartNum(m_schdr.hdr.nFileSize, m_schdr.hdr.nUnitSize);
	m_bufsize	= GetDataPart(m_curpart, m_buf);
	m_leftsize	= m_bufsize;
	if( m_bufsize==0 )
	{
		// �ļ�����Ϊ0����ǰ��Ĭ�ϱ�ɳ�����
		m_curpart	= 1;
	}

	return	0;
}
void	whzlib_file_i_sc::SetPassword(const char *szPassword)
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
int		whzlib_file_i_sc::FileSize() const
{
	return	m_schdr.hdr.nFileSize;
}
time_t	whzlib_file_i_sc::FileTime() const
{
	return	m_schdr.hdr.time;
}
void	whzlib_file_i_sc::SetFileTime(time_t t)
{
	if( t==0 )
	{
		t	= wh_time();
	}
	m_schdr.hdr.time	= t;
}
int		whzlib_file_i_sc::Read(void *pBuf, int nSize)
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
		m_bufsize		= GetDataPart(m_curpart, m_buf);
		m_leftsize		= m_bufsize;
		if( m_bufsize<=0 )
		{
			assert(0);
			return	-1;
		}
		// ������
		goto			Again;
	}

End:
	return	nReadSize;
}
int		whzlib_file_i_sc::Seek(int nOffset, int nOrigin)
{
	int	curpos = m_curpart * m_schdr.hdr.nUnitSize + m_bufsize - m_leftsize;
	if( IsEOF() )
	{
		curpos	= FileSize();
	}
	int	newpos	= curpos;
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
		curpart		= newpos / m_schdr.hdr.nUnitSize;
		curidx		= newpos % m_schdr.hdr.nUnitSize;
	}
	if( curpart!=m_curpart )
	{
		m_curpart	= curpart;
		m_bufsize	= GetDataPart(m_curpart, m_buf);
	}
	m_leftsize	= m_bufsize - curidx;

	return	0;
}
bool	whzlib_file_i_sc::IsEOF()
{
	if( m_curpart>=m_totalpart )
	{
		return	true;
	}
	return		false;
}
int		whzlib_file_i_sc::Tell()
{
	return	m_curpart * m_schdr.hdr.nUnitSize + m_bufsize-m_leftsize;
}
int		whzlib_file_i_sc::GetMD5(unsigned char *MD5)
{
	memcpy(MD5, m_schdr.hdr.md5, sizeof(m_schdr.hdr.md5));
	return	0;
}
int		whzlib_file_i_sc::GetDataPart(int nPart, char *pBuf)
{
	if( nPart<0 || nPart>=m_totalpart )
	{
		// �����˷�Χ(����0��ʾEOF)
		return	0;
	}

	// �õ���ӦnPart����ʼ�ֽ�
	int	nOff;
	m_srcfile->Seek( sizeof(m_schdr) + nPart*sizeof(int), SEEK_SET );
	m_srcfile->Read(&nOff, sizeof(nOff));
	// ����ö����ݵľ��뵱ǰλ�õľ���ƫ��
	nOff	+= (m_totalpart-nPart-1)*sizeof(int);	// ֮���Լ�1����Ϊ����Offset�Ѿ�������
	m_srcfile->Seek(nOff, SEEK_CUR);

	// ��������ͷ
	sc_dataunit_hdr_t	dhdr;
	m_srcfile->Read(&dhdr, sizeof(dhdr));
	// �������ݲ���
	size_t	nSize = m_srcfile->Read(m_srcbuf, dhdr.nSize);
	if( nSize!=dhdr.nSize )
	{
		assert(0);
		return	0;
	}

	// ���������ͽ���
	if( m_szPassword[0] )
	{
		m_des.decrypt((unsigned char *)m_srcbuf, nSize);
	}

	// ��ѹ��ĳߴ�
	size_t	nRawSize = 0;
	// �������ѹ
	switch( dhdr.nMode )
	{
		case	COMPRESSMODE_ZIP:
		{
			nRawSize	= m_schdr.hdr.nUnitSize;
			if( whzlib_decompress(m_srcbuf, nSize, pBuf, &nRawSize)<0 )
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
			memcpy(pBuf, m_srcbuf, nRawSize);
		}
		break;
	}

	return	nRawSize;
}
