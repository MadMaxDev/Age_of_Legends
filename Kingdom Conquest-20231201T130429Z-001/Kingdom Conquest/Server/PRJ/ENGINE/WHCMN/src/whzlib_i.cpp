// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace	: n_whzlib
// File			: whzlib_i.cpp
// Creator		: Wei Hua (魏华)
// Comment		: whzlib的内部实现
// CreationDate	: 2003-09-04
// ChangeLOG	:
// 2004-02-20	: 增加了whzlib_filemodifyman_i::PutFileToDisk对文件时间的修改
// 2005-01-14	: 修改了single compress中pIn和pOut交换后在下次压缩的时候没有恢复回去的错误
//              : 当时还误解人家zlib会修改In的buf，唉，惭愧惭愧

#include "../inc/whzlib_pck_i.h"
#include "../inc/whfile.h"
#include "../inc/whtime.h"
#include <sys/stat.h>

namespace n_whzlib
{

const char *sc_file_hdr_t::CSZ_MAGIC	= "whz_single";

int		whzlib_single_compress_i(whzlib_file *file, const char *cszDstFile, const char *cszPasswd)
{
	// 打开文件 to 写
	FILE	*fp = fopen(cszDstFile, "wb");
	if( !fp )
	{
		return	-1;
	}

	// 保证从文件头开始读，因为现在反正也是从file获得文件总长，不会存在只保存文件一部分的情况
	file->Seek(0, SEEK_SET);

	// 写入文件头（先占个位置，当然如果没有数据部分就不用了）
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

	// 用于加密
	WHSimpleDES	des;
	bool		bUseDes = false;
	if( cszPasswd && cszPasswd[0] )
	{
		bUseDes		= true;
		WHDES_ConvertToFixed(cszPasswd, strlen(cszPasswd), (char *)des.m_key, sizeof(des.m_key));
		des.setmask(cszPasswd, strlen(cszPasswd));
	}

	// 用于计算md5
	md5_state_t	md5state;
	md5_init(&md5state);

	size_t	nUnitSize = schdr.hdr.nUnitSize;
	whvector<char>	vectIn(nUnitSize), vectOut(nUnitSize*2);

	// 判断是否是0长度文件
	if( schdr.hdr.nParts > 0 )
	{
		int	nDPartSize = sizeof(int)*schdr.hdr.nParts;

		// 分段索引(先写进去占着地方)
		whvector<int>	vectPartIdx(schdr.hdr.nParts);
		rst		= ::fwrite(vectPartIdx.getbuf(), 1, nDPartSize, fp);
		if( rst != nDPartSize )
		{
			fclose(fp);
			return	-1;
		}
		// 记录起始位置
		int		nBeginOff = ::ftell(fp);
		// 块计数
		int		nCount = 0;
		// 分段写入数据

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
					// 还不如不压呢
					pOut		= pIn;
					dhdr.nSize	= nInSize;
					dhdr.nMode	= COMPRESSMODE_NOTHING;
				}
				// 本块的起始相对位置
				vectPartIdx[nCount]	= ::ftell(fp) - nBeginOff;
				// 写入文件头
				rst	= ::fwrite(&dhdr, 1, sizeof(dhdr), fp);
				if( rst!=sizeof(dhdr) )
				{
					assert(0);
					return	-1;
				}
				// 如果有密码就加密
				if( bUseDes )
				{
					des.encrypt((unsigned char *)pOut, dhdr.nSize);
				}
				// 写入数据
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
				// 文件结束
				break;
			}
		}

		// 记录md5值
		md5_finish(&md5state, schdr.hdr.md5);

		// 重写更改的部分
		::fseek(fp, 0, SEEK_SET);
		// 重写文件头(因为md5更新了)
		rst		= ::fwrite(&schdr, 1, sizeof(schdr), fp);
		if( rst!=sizeof(schdr) )
		{
			assert(0);
			return	-1;
		}
		// 重写分段索引
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
		// 重写更改的部分
		::fseek(fp, 0, SEEK_SET);
		// 重写文件头(因为md5更新了)
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
	// 重到文件开始
	srcfile->Seek(0, SEEK_SET);

	// 读入文件头
	int		rst;
	rst		= srcfile->Read(hdr, sizeof(*hdr));
	if( rst != sizeof(*hdr) )
	{
		return	false;
	}

	// 判断是否是合法的头
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
// 管理器whzlib_fileman_i实现
////////////////////////////////////////////////////////////////////
whzlib_fileman_i::whzlib_fileman_i()
: m_nSearchMode(SEARCHMODE_1PCK2FILE)
{
	m_szPassword[0]	= 0;
}
whzlib_fileman_i::~whzlib_fileman_i()
{
	// 关闭所有的包文件
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
	// 如果不是读方式就直接从硬盘上打开
	if( strcmp(szMode, "rb")!=0 )
	{
		return	static_cast<whzlib_file_i *>(whzlib_OpenCmnFile(szFileName, szMode));
	}
	// 否则可以先检查内存，然后再从硬盘上读出
	return	static_cast<whzlib_file_i *>(m_fcache.OpenFile(szFileName, true));
}
whzlib_file_i *	whzlib_fileman_i::OpenFromDisk(const char *szFileName)
{
	return	OpenFromDisk(szFileName, "rb");
}
whzlib_file_i *	whzlib_fileman_i::OpenFromPackage(const char *szFileName)
{
	// 看看路径最多存在到哪个级别
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
		// 打开相应文件
		return	pReader->OpenFile(szFile);
	}

	return	NULL;
}
whzlib_pck_reader_i	*	whzlib_fileman_i::CheckAndLoadReader(const char *szPath)
{
	// 看看包文件是否已经打开
	whzlib_pck_reader_i	*pReader = NULL;
	if( !m_mapName2Pck.get(szPath, pReader) )
	{
		// 没有打开就打开包
		pReader	= new whzlib_pck_reader_i;
		if( pReader )
		{
			// 里面会优先从内存cache中打开
			if( pReader->OpenPck(szPath, &m_fcache) < 0 )
			{
				delete	pReader;
				pReader	= NULL;
			}
			else
			{
				// 加入映射
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
	// 将pck相关的三个文件都载入
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
	// 将pck相关的三个文件都释放
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
		// 只要不是读方式就按磁盘文件方式打开文件
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
	// 删除该对象
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
	// 用于虑掉后面的斜杠
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
	// 看看路径最多存在到哪个级别
	char	szPath[WH_MAX_PATH], szDir[WH_MAX_PATH];
	if( !_LookForPackageByfile(szDirName, szPath, szDir, true) )
	{
		return	NULL;
	}

	whzlib_pck_reader_i	*pReader = CheckAndLoadReader(szPath);
	if( pReader )
	{
		// 打开相应文件
		return	pReader->OpenDir(szDir);
	}

	return		NULL;
}
int	whzlib_fileman_i::GetFileSizeFromPackage(const char *szFileName)
{
	// 看看路径最多存在到哪个级别
	char	szPath[WH_MAX_PATH], szFile[WH_MAX_PATH];
	if( !_LookForPackageByfile(szFileName, szPath, szFile, false) )
	{
		return	-1;
	}
	whzlib_pck_reader_i	*pReader = CheckAndLoadReader(szPath);
	if( pReader )
	{
		// 找到对应记录并获得长度
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
							// 找到了包文件
							szPath[i]	= 0;
							if( i==len-1 )
							{
								// 说明就是根
								szFile[0]	= 0;
							}
							else
							{
								// 相对文件名
								strcpy(szFile, szFileName+i+1);
							}
							return	true;
						}
					}
				}
			break;
		}
	}
	// Kao，没有存在的啊
	return	false;
}
bool	whzlib_fileman_i::_IsFileExistInMemOrOnDisk(const char *szPath)
{
	if( m_fcache.HasFileInMem(szPath) )
	{
		return	true;
	}
	// 然后判断是否在硬盘上
	return	whfile_ispathexisted(szPath);
}

// 寻找szFileName是否可能在包文件中，如果是则将文件名分解为包和相对包的文件名
// 包名后面没有"/"结尾
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
							// 找到了包文件
							szPath[i]	= 0;
							if( i==len-1 )
							{
								// 说明就是根
								szFile[0]	= 0;
							}
							else
							{
								// 相对文件名
								strcpy(szFile, szFileName+i+1);
							}
							return	true;
						}
					}
				}
			break;
		}
	}
	// Kao，没有存在的啊
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
					// 相对文件名
					strcpy(szFile, szFileName+prev+1);
					return	true;
				}
				prev	= i;
			break;
		}
	}
	// Kao，没有存在的啊
	if( prev == 0)
	{
		return	false;
	}

	szPath[prev]	= 0;
	// 相对文件名
	strcpy(szFile, szFileName+prev+1);

	return	true;
}
////////////////////////////////////////////////////////////////////
// 包更新管理器whzlib_filemodifyman_i实现
////////////////////////////////////////////////////////////////////
//
whzlib_filemodifyman_i::whzlib_filemodifyman_i()
{
	m_szPassword[0]	= 0;
}
whzlib_filemodifyman_i::~whzlib_filemodifyman_i()
{
	// 关闭所有的包文件
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

	// 先判断目标文件位置是否在包中
	char	szPath[WH_MAX_PATH], szFileInPack[WH_MAX_PATH];
	if( !LookForPackageByfile(szFileName, szPath, szFileInPack, false) )
	{
		if( bShouldInPck )
		{
			// 文件应该在包里
			if( (m_nProperty & PROPERTY_AUTOCREATEPCK)
			&&  LookForLongestDirByfile(szFileName, szPath, szFileInPack)
			)
			{
				// 再后面自动创建包
				bForceCreate	= true;
			}
			else
			{
				return	-1;
			}
		}
		else
		{
			// 在磁盘上，直接拷贝数据即可
			return	PutFileToDisk(szFileName, file);
		}
	}
	// 应该在包中，或者自动创建
	pModifier = CheckAndLoadModifier(szPath, bForceCreate);

	if( !pModifier )
	{
		// 出错了
		return	-1;
	}
	if( m_szPassword[0] )
	{
		pModifier->SetPassword(m_szPassword);
	}

	if( file->FileSize()==0 )
	{
		// 删除对应文件
		pModifier->DelPath(szFileInPack);
	}
	else
	{
		// 先按照替换做
		if( pModifier->ReplaceFile(szFileInPack, file, nCompressMode)<0 )
		{
			// 再按照添加文件做
			if( pModifier->AddFile(szFileInPack, file, nCompressMode)<0 )
			{
				// 那就是错了
				return	-1;
			}
		}
	}

	// 成功了
	return	0;
}
int		whzlib_filemodifyman_i::DelFile(const char *szFileName)
{
	whzlib_pck_modifier_i	*pModifier = NULL;

	// 先判断目标文件位置是否在包中
	char	szPath[WH_MAX_PATH], szFileInPack[WH_MAX_PATH];
	if( !LookForPackageByfile(szFileName, szPath, szFileInPack, false) )
	{
		// 直接删除硬盘上的文件
		if( whdir_sysdelfile(szFileName)<0 )
		{
			return	-1;
		}
		return	0;
	}

	// 应该在包中，或者自动创建
	pModifier = CheckAndLoadModifier(szPath, false);
	if( !pModifier )
	{
		// 无法加载包
		return	-2;
	}
	if( m_szPassword[0] )
	{
		pModifier->SetPassword(m_szPassword);
	}
	// 删除对应文件
	if( pModifier->DelPath(szFileInPack)<0 )
	{
		return	-3;
	}

	return	0;
}
int		whzlib_filemodifyman_i::DelDir(const char *szDir)
{
	assert(0);	// 没有实现
	return	0;
}
int		whzlib_filemodifyman_i::FlushAll()
{
	// 调用所有modifier的flush
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
	// 看看包文件是否已经打开
	whzlib_pck_modifier_i	*pModifier = NULL;
	if( !m_mapName2Pck.get(szPath, pModifier) )
	{
		// 没有打开就打开包
		pModifier	= new whzlib_pck_modifier_i;
		if( pModifier )
		{
			whzlib_pck_modifier_i::OPENINFO_T		info;
			info.szPckFile	= szPath;
			if( pModifier->OpenPck(&info) < 0 )
			{
				if( !bForceCreate )
				{
					// 直接删除之
					delete		pModifier;
					pModifier	= NULL;
				}
				else
				{
					// 原来不存在就创建之
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
						// 创建一个空文件
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
				// 加入映射
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
	// 保证目录存在
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
		// 如果打不开则尝试改变文件属性
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
	// 默认file已经seek到开头了，如果不在开头则认为使用者就是想从中间写
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

	// 修改文件时间为原来的时间
	time_t	FT = file->FileTime();
	if( FT>0 )
	{
		whfile_setmtime(szFileName, FT);
	}

	return	0;
}

////////////////////////////////////////////////////////////////////
// 单一文件压缩 single compress
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
		// 文件内容不是单压缩格式
		return	-1;
	}

	// 接手流
	m_srcfile	= srcfile;

	// 初始化
	m_vectbuf.resize(m_schdr.hdr.nUnitSize);
	m_buf		= m_vectbuf.getbuf();
	m_vectsrcbuf.resize(m_schdr.hdr.nUnitSize);
	m_srcbuf	= m_vectsrcbuf.getbuf();

	// 读入第一块数据(第一块数据是一般会有的)
	m_curpart	= 0;
	m_totalpart	= whzlib_GetFilePartNum(m_schdr.hdr.nFileSize, m_schdr.hdr.nUnitSize);
	m_bufsize	= GetDataPart(m_curpart, m_buf);
	m_leftsize	= m_bufsize;
	if( m_bufsize==0 )
	{
		// 文件长度为0，当前块默认变成超出块
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
	// 判断当前内存中的尺寸是否够用
	if( nSize<=m_leftsize ) 
	{
		memcpy(pcBuf, m_buf+(m_bufsize-m_leftsize), nSize);
		m_leftsize		-= nSize;
		nReadSize		+= nSize;			// 因为可能是again到这里的
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
			// 到了文件结尾(没有下一块了)
			m_bufsize	= 0;
			m_leftsize	= 0;
			goto		End;
		}
		// 向reader索要下一块数据
		m_bufsize		= GetDataPart(m_curpart, m_buf);
		m_leftsize		= m_bufsize;
		if( m_bufsize<=0 )
		{
			assert(0);
			return	-1;
		}
		// 接着来
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
	// 超界判断
	if( newpos<0 )
	{
		newpos		= 0;
		curpart		= 0;
		curidx		= 0;
	}
	else if( newpos>=FileSize() )
	{
		// 超过就是EOF了
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
		// 超出了范围(返回0表示EOF)
		return	0;
	}

	// 得到相应nPart的起始字节
	int	nOff;
	m_srcfile->Seek( sizeof(m_schdr) + nPart*sizeof(int), SEEK_SET );
	m_srcfile->Read(&nOff, sizeof(nOff));
	// 计算该段数据的距离当前位置的具体偏移
	nOff	+= (m_totalpart-nPart-1)*sizeof(int);	// 之所以减1是因为本个Offset已经读过了
	m_srcfile->Seek(nOff, SEEK_CUR);

	// 读入数据头
	sc_dataunit_hdr_t	dhdr;
	m_srcfile->Read(&dhdr, sizeof(dhdr));
	// 读入数据部分
	size_t	nSize = m_srcfile->Read(m_srcbuf, dhdr.nSize);
	if( nSize!=dhdr.nSize )
	{
		assert(0);
		return	0;
	}

	// 如果有密码就解密
	if( m_szPassword[0] )
	{
		m_des.decrypt((unsigned char *)m_srcbuf, nSize);
	}

	// 解压后的尺寸
	size_t	nRawSize = 0;
	// 看情况解压
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
			// 直接拷贝
			nRawSize	= dhdr.nSize;
			memcpy(pBuf, m_srcbuf, nRawSize);
		}
		break;
	}

	return	nRawSize;
}
