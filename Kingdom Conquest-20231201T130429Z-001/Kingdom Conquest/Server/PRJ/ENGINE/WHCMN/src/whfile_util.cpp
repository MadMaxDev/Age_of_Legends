// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_util.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 文件辅助功能
// CreationDate : 2004-09-16

#include "../inc/whfile_util.h"
#include "../inc/whdir.h"
#include "../inc/whtime.h"
#include <sys/stat.h>

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whsmpfilecache
////////////////////////////////////////////////////////////////////
whsmpfilecache::whsmpfilecache()
{
}
whsmpfilecache::~whsmpfilecache()
{
	UnLoadAllFiles();
}
int		whsmpfilecache::UnLoadAllFiles()
{
	// 没有文件可以清除
	if( m_mapFileName2Unit.size()==0 )
	{
		return	0;
	}

	// 释放所有文件内存
	whvector<FILEUNIT_T*>	vect;
	m_mapFileName2Unit.exportvaltovector(vect);
	for(size_t i=0;i<vect.size();i++)
	{
		delete	vect[i];
	}
	// hash表清空
	m_mapFileName2Unit.clear();

	return	0;
}
int		whsmpfilecache::LoadFile(const char *cszFile, const char *cszFileInMem)
{
	// 判断内存文件是否存在
	if( cszFileInMem==NULL )
	{
		cszFileInMem	= cszFile;
	}
	if( m_mapFileName2Unit.has(cszFileInMem) )
	{
		// 文件已经存在
		return	-1;
	}

	// 获得文件长度(如果长度为0是不可以的)
	int	nSize	= whfile_getfilesize(cszFile);
	if( nSize<=0 )
	{
		return	-1;
	}

	// 申请内存
	FILEUNIT_T	*pUnit	= (FILEUNIT_T *) new char[nSize + wh_offsetof(FILEUNIT_T, data)];
	assert(pUnit);
	WH_STRNCPY0(pUnit->szFileName, cszFileInMem);
	pUnit->nFileSize	= nSize;
	// 读入文件(前面应该已经保证了文件的可读性了)
	whfile_readfile(cszFile, pUnit->data, (size_t *)&nSize);

	// 放入hash表
	m_mapFileName2Unit.put(pUnit->szFileName, pUnit);

	return	0;
}
int		whsmpfilecache::UnLoadFile(const char *cszFileInMem)
{
	// 判断文件是否存在
	FILEUNIT_T	*pUnit;
	if( !m_mapFileName2Unit.geterase(cszFileInMem, pUnit) )
	{
		// 文件不存在
		return	-1;
	}
	
	delete	pUnit;

	return	0;
}
whfile *	whsmpfilecache::OpenFile(const char *cszFile, bool bTryDisk)
{
	// 判断文件是否存在
	FILEUNIT_T	*pUnit;
	if( m_mapFileName2Unit.get(cszFile, pUnit) )
	{
		return	whfile_OpenMemBufFile(pUnit->data, pUnit->nFileSize);
	}
	// 内存中没有文件
	// 如果不用从硬盘上载入，就直接返回NULL了
	if( !bTryDisk )
	{
		return	NULL;
	}

	// 从硬盘上打开
	return	whfile_OpenCmnFile(cszFile, "rb");
}
bool	whsmpfilecache::HasFileInMem(const char *cszFile)
{
	return	m_mapFileName2Unit.has(cszFile);
}

////////////////////////////////////////////////////////////////////
// whlinelistinmultifile
////////////////////////////////////////////////////////////////////
whlinelistinmultifile::whlinelistinmultifile()
{
	m_szPrefixFmt[0]	= 0;
	m_szCurFile[0]		= 0;
	m_vectfiledata.reserve(65536);
	m_nFileIdx			= 0;
}
whlinelistinmultifile::~whlinelistinmultifile()
{
}
void	whlinelistinmultifile::SetPrefixFmt(const char *cszFmt)
{
	WH_STRNCPY0(m_szPrefixFmt, cszFmt);
}
void	whlinelistinmultifile::Reset()
{
	m_nFileIdx			= 0;
	m_szCurFile[0]		= 0;
}
int		whlinelistinmultifile::SaveFile()
{
	if( m_szCurFile[0] == 0 )
	{
		return	-1;
	}
	int	nSize	= m_vectfiledata.size()-m_vectfiledata.f_tell();
	if( nSize==0 )
	{
		whfile_del(m_szCurFile);
	}
	else
	{
		if( whfile_writefile(m_szCurFile, m_vectfiledata.getptr(m_vectfiledata.f_tell()), nSize)<0 )
		{
			return	-2;
		}
	}
	return		0;
}
int		whlinelistinmultifile::GetRemoveLine(char *szLine, int nSize)
{
	// 如果已经没有东西可读了就直接返回
	if( m_nFileIdx<0 )
	{
		return	0;
	}
	// 如果还没有载入文件则通过索引文件开始读
	if( m_nFileIdx==0 )
	{
		sprintf(m_szCurFile, m_szPrefixFmt, 0);
		FILE	*file	= fopen(m_szCurFile, "rt");
		if( !file )
		{
			// 文件不存在，则不开始读
			return	0;
		}
		fscanf(file, "%d", &m_nFileIdx);
		fclose(file);
		if( m_nFileIdx<=0 )
		{
			// 表示索引文件已经表示没有东西可读了
			m_nFileIdx	= -1;
			return	0;
		}
		goto	newfile;
	}

readline:
	// 看看当前的vector中是否有可读的东西
	int	rst;
	rst	= m_vectfiledata.f_readline(szLine, nSize);
	if( rst>0 )
	{
		return	rst;
	}

	// 读到当前文件结尾了
	// 删除当前文件
	whfile_del(m_szCurFile);

	// 看下一个文件是否存在
	m_nFileIdx	++;

	// 记录下个文件的序号
	FILE	*file;
	char	szFile[WH_MAX_PATH];
	sprintf(szFile, m_szPrefixFmt, 0);
	file	= fopen(szFile, "wt");
	fprintf(file, "%d", m_nFileIdx);
	fclose(file);

newfile:
	sprintf(m_szCurFile, m_szPrefixFmt, m_nFileIdx);
	if( whfile_readfile(m_szCurFile, m_vectfiledata)<0 )
	{
		// 表示已经没有东西可读了
		// 清零表示如果下次新加入了文件则还可以继续读
		m_nFileIdx	= 0;
		return	0;
	}

	goto	readline;
}

namespace n_whcmn
{

bool	GetDirFileList(whfileman *pMan, const char *szPath, std::vector< std::string > *pList, const char *szExt)
{
	class	MySA	: public WHDirSearchActionWFM
	{
	public:
		std::vector< std::string >	*m_pList;
	public:
		MySA(whfileman *pMan, std::vector< std::string > *pList)
			: WHDirSearchActionWFM(pMan)
			, m_pList(pList)
		{
		}
	private:
		//virtual int		ActionOnDir(const char *szFile)
		//{
		//	printf("Act on dir:%s%s", GetFullPath(szFile), WHLINEEND);
		//	return	0;
		//}
		virtual int		ActionOnFile(const char *szFile)
		{
			// 如果文件重复了也不会重复添加的
			//m_setFName.insert(GetFullPath(szFile));
			m_pList->push_back(GetFullPath(szFile));
			return	0;
		}
	};

	MySA	mysa(pMan, pList);
	mysa.SetFilter(szExt);
	if( mysa.DoSearch(szPath, true)<0 )
	{
		return	false;
	}

	return	true;
}
const char *	GetFile(whfileman *pMan, const char *szPath, const char *szFile, char *szBuf)
{
	szBuf[0]	= 0;
	class	MySA	: public WHDirSearchActionWFM
	{
	public:
		const char	*m_pszFile;
		char		*m_pszBuf;
		bool		m_bStop;
	public:
		MySA(whfileman *pMan, const char *szFile, char *szBuf)
			: WHDirSearchActionWFM(pMan)
			, m_pszFile(szFile)
			, m_pszBuf(szBuf)
			, m_bStop(false)
		{
			SetStopSearchVar(&m_bStop);
		}
	private:
		virtual int		ActionOnFile(const char *szFile)
		{
			if( strcmp(szFile, m_pszFile)==0 )
			{
				strcpy(m_pszBuf, GetFullPath(szFile));
				m_bStop	= true;
			}
			return	0;
		}
	};

	MySA	mysa(pMan, szFile, szBuf);
	if( mysa.DoSearch(szPath, true)<0 )
	{
		return	NULL;
	}

	if( szBuf[0] )
		return	szBuf;
	else
		return	NULL;
}

bool	DelFileBeforeTime(whfileman *pMan, const char *szPath, const char *szExt, time_t nTime)
{
	class	MySA	: public WHDirSearchActionWFM
	{
	private:
		time_t	m_nTime;
	public:
		MySA(whfileman *pMan, time_t nTime)
			: WHDirSearchActionWFM(pMan)
			, m_nTime(nTime)
		{
		}
	private:
		virtual int		ActionOnFile(const char *szFile)
		{
			whfileman::PATHINFO_T	info;
			if( WHDirSearchActionWFM::m_pFM->GetPathInfo(GetFullPath(szFile), &info)==0 )
			{
				if( info.nMTime<m_nTime )
				{
					WHDirSearchActionWFM::m_pFM->DelFile( GetLastFullPath() );
				}
			}
			return	0;
		}
	};

	MySA	mysa(pMan, nTime);
	mysa.SetFilter(szExt);
	if( mysa.DoSearch(szPath, true)<0 )
	{
		return	false;
	}

	return	true;
}
bool	DelPath(whfileman *pMan, const char *szPath)
{
	if( !pMan->IsPathDir(szPath) )
	{
		// 直接删除文件
		return	pMan->DelFile(szPath)==0;
	}
	else
	{
		class	MySA	: public WHDirSearchActionWFM
		{
		public:
			MySA(whfileman *pMan)
				: WHDirSearchActionWFM(pMan)
			{
			}
		private:
			virtual int		LeaveDir(const char *szDir)
			{
				return	WHDirSearchActionWFM::m_pFM->DelDir( szDir );
			}
			virtual int		ActionOnFile(const char *szFile)
			{
				return	WHDirSearchActionWFM::m_pFM->DelFile( GetFullPath(szFile) );
			}
		};

		MySA	mysa(pMan);
		if( mysa.DoSearch(szPath, true)<0 )
		{
			return	false;
		}

		return	true;
	}
}

int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, const char *szAuthFile, whvector<char> &vectAuth)
{
	if( szRSAPriKeyFile[0]==0 || szRSAPass[0]==0 || szAuthFile[0]==0 )
	{
		return	-1;
	}
	// 判断一下是否有文件，并且可以打开，读出数据
	// auth文件
	whvector<char>		vectFile;
	if( whfile_readfile(szAuthFile, vectFile)<0 )
	{
		return	-2;
	}
	return	GetAuthInfo(szRSAPriKeyFile, szRSAPass, vectFile.getbuf(), vectFile.size(), vectAuth);
}
int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth)
{
	// key文件
	whvector<char>	vectRSA;
	if( whfile_crc_readfile(szRSAPriKeyFile, vectRSA)!=WHFILE_CRC_READ_OK )
	{
		return	-11;
	}
	return	GetAuthInfo(vectRSA.getbuf(), vectRSA.size(), szRSAPass, pszFileData, nFileSize, vectAuth);
}
int		GetAuthInfo(char *pszRSAPriKeyFileData, int nRSAPriKeyFileSize, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth)
{
	// 解码对象
	whsafeSDptr<WHRSA>	pDec(WHRSA::CreateDecoder());
	// 解开的缓冲
	vectAuth.reserve(256);
	vectAuth.clear();
	if( pDec->Init(szRSAPass, strlen(szRSAPass), pszRSAPriKeyFileData, nRSAPriKeyFileSize)<0 )
	{
		return	-21;
	}

	return	GetAuthInfo(pszFileData, nFileSize, pDec, vectAuth);
}
int		GetAuthInfo(char *pszFileData, int nFileSize, WHRSA *pDec, whvector<char> &vectAuth)
{
	vectAuth.reserve(256);
	vectAuth.clear();
	whfile_i_mem	fSrc(pszFileData, nFileSize);
	while(!fSrc.IsEOF())
	{
		const int	nMaxLen	= 128;	// 加密之后的串一般都这么长
		char		srcBuf[nMaxLen];
		int	nSrc	= fSrc.Read(srcBuf, nMaxLen);
		if( nSrc<=0 )
		{
			break;
		}
		char		dstBuf[nMaxLen];
		int	rst		= pDec->Process(srcBuf, nSrc, dstBuf, sizeof(dstBuf));
		if( rst<0 )
		{
			// 出错了
			return	-31;
		}
		vectAuth.pushn_back(dstBuf, rst);
	}

	return	0;
}

}

////////////////////////////////////////////////////////////////////
// whsmppasswd
////////////////////////////////////////////////////////////////////

WHDATAPROP_MAP_BEGIN_AT_ROOT(whsmppasswd::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp(bool, bPassMD5, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bLowerCaseAccount, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPasswordFile, 0)
WHDATAPROP_MAP_END()

whsmppasswd::whsmppasswd()
: m_nLastModifyTime(0), m_nLastSize(0)
{
}
whsmppasswd::~whsmppasswd()
{
}
int			whsmppasswd::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	Update();	// 比如文件不存在，则也是会出现错误的，所以这个错误忽略吧
	return	0;
}
int			whsmppasswd::Release()
{
	FreeAll();
	m_nLastModifyTime	= 0;
	m_nLastSize			= 0;
	return	0;
}
int			whsmppasswd::FreeAll()
{
	m_mapName2Pass.clear();
	for(int i=0;i<(int)m_vectLikeUnitPtr.size();i++)
	{
		LIKEUNIT_T	*&p	= m_vectLikeUnitPtr[i];
		assert( p!=NULL );
		{
			delete	p;
			p		= NULL;
		}
	}
	m_vectLikeUnitPtr.clear();
	return	0;
}
int			whsmppasswd::Check(const char *cszUser, const char *cszPass)
{
	const char *pcszPass	= NULL;
	int	rst	= CheckGetPass(cszUser, pcszPass);
	if( rst == CHECK_RST_OK )
	{
		// 判断密码是否匹配（如果原来没设置密码则说明不用检查密码）
		if( pcszPass[0]==0 || strcmp(cszPass, pcszPass)==0 )
		{
			return	CHECK_RST_OK;
		}
		return	CHECK_RST_BAD_PASS;
	}
	return	rst;
}
int			whsmppasswd::CheckGetPass(const char *cszUser, const char *&pcszPass)
{
	// 先检查是否是固定匹配
	whstr4hash	*pStr	= NULL;
	if( m_mapName2Pass.get(cszUser, &pStr) )
	{
		pcszPass	= pStr->GetPtr();
		return	CHECK_RST_OK;
	}
	// 再检查是否模式匹配
	for(int i=0;i<(int)m_vectLikeUnitPtr.size();i++)
	{
		LIKEUNIT_T	*&p	= m_vectLikeUnitPtr[i];
		assert( p!=NULL );
		if( p->pLike->IsLike(cszUser) )
		{
			pcszPass	= p->vectPass.getbuf();
			return	CHECK_RST_OK;
		}
	}
	// 没有找到匹配
	return	CHECK_RST_USER_NOTFOUND;
}
int			whsmppasswd::Update()
{
	// 文件不存在也可以认为是需要禁用这个功能
	if( m_info.szPasswordFile[0]==0 || !whfile_ispathexisted(m_info.szPasswordFile) )
	{
		FreeAll();
		return	UPDATE_RST_OK;
	}
	// 检查文件大小是否改变（如果这个改变了就一定是变了）
	struct stat	st;
	int			rst;
	rst			= stat(m_info.szPasswordFile, &st);
	if( rst!=0 || (st.st_mode&S_IFDIR)!=0 )
	{
		// 文件有问题了
		return	UPDATE_RST_STAT_ERR;
	}
	// 检查文件长度或时间是否改变
	if( st.st_size == (int)m_nLastSize )
	{
		if( st.st_mtime == m_nLastModifyTime )
		{
			// 不需要更新
			return	UPDATE_RST_OK;
		}
	}
	whsafeptr_FILE	fp(fopen(m_info.szPasswordFile, "rt"));
	if( !fp )
	{
		// 打开文件失败
		return	UPDATE_RST_OPEN_ERR;
	}
	// 说明文件改变，清除原来的内容，重新载入所有内容
	// 把这个放在打开文件后面，因为不知道为什么有时候文件不能打开（因为在上次读入文件时添加最后一条记录出错，立即返回，没有关闭文件，结果每次这样的操作之后文件都没有关闭，这样在一定程度之后，文件就不能打开了（估计是文件句柄个数超界了））
	FreeAll();
	//
	while( !feof(fp) )
	{
		char	buf[1024]	= "";
		if( fgets(buf, sizeof(buf)-1, fp)==NULL )
		{
			break;
		}
		char	szUser[256]	= "";
		char	szPass[256]	= "";
		wh_strsplit("ss", buf, ":",	szUser, szPass);
		if( szUser[0]=='/' && szUser[1]=='/' )	// 注释行
		{
			continue;
		}
		if( AddUser(szUser, szPass)<0 )
		{
			// 一般都应该是用户名重复了
			return	UPDATE_RST_ADD_ERR;
		}
	}
	// 修改对应最新文件的长度和时间
	m_nLastModifyTime	= st.st_mtime;
	m_nLastSize			= st.st_size;
	return	UPDATE_RST_NEW;
}
int			whsmppasswd::AddUser(const char *cszUser, const char *cszPass)
{
	char	szUser[128]	= "";
	if( m_info.bLowerCaseAccount )
	{
		wh_strlwr(cszUser, szUser);
		cszUser	= szUser;
	}
	// 先判断cszUser的样式
	if( strchr(cszUser, whstrlike::DFTSEP) )
	{
		// 如果是匹配模板，则加入likelist
		LIKEUNIT_T	*&pUnit	= *m_vectLikeUnitPtr.push_back();
		pUnit				= new LIKEUNIT_T;
		assert(pUnit);
		pUnit->pLike		= whstrlike::Create();
		assert(pUnit->pLike);
		pUnit->pLike->SetPattern(cszUser);
		if( m_info.bPassMD5 )
		{
			if( cszPass[0] )
			{
				char	szMD5[WHMD5LEN*2+1];
				whmd5str(cszPass, strlen(cszPass), szMD5);
				pUnit->vectPass.SetString(szMD5);
			}
			else
			{
				pUnit->vectPass.SetString("");
			}
		}
		else
		{
			pUnit->vectPass.SetString(cszPass);
		}
	}
	else
	{
		// 否则加入hash表
		if( m_info.bPassMD5 )
		{
			char	szMD5[WHMD5LEN*2+1];
			whmd5str(cszPass, strlen(cszPass), szMD5);
			if( !m_mapName2Pass.put(cszUser, szMD5) )
			{
				return	-1;
			}
		}
		else
		{
			if( !m_mapName2Pass.put(cszUser, cszPass) )
			{
				return	-2;
			}
		}
	}
	return	0;
}

// whvarstrarr_WF
int		whvarstrarr_WF::initfromfile(const char *cszFileName)
{
	whfile	*file = whfile_OpenCmnFile(cszFileName, "r");
	if( !file )
	{
		return	-1;
	}

	initfromfile(file);

	whfile_CloseCmnFile(file);

	return		0;
}
int		whvarstrarr_WF::initfromfile(whfile *file)
{
	char	szBuf[1024];
	int		nTotalSize = 0;
	int		nTotalLine = 0;
	// 先统计
	// 一行一行读入
	while(!file->IsEOF())
	{
		if( file->ReadLine(szBuf, sizeof(szBuf)-1) )
		{
			wh_strtrim(szBuf);
			if( szBuf[0] )
			{
				nTotalSize	+= strlen(szBuf)+1;
				nTotalLine	++;
			}
		}
	}
	reserve(nTotalSize, nTotalLine);
	// 重新读入，并填入
	file->Rewind();
	while(!file->IsEOF())
	{
		if( file->ReadLine(szBuf, sizeof(szBuf)-1) )
		{
			wh_strtrim(szBuf);
			if( szBuf[0] )
			{
				if( szBuf[0]=='/'
				&&  szBuf[1]=='/'
				)
				{
					// 注释行
				}
				else
				{
					add(szBuf);
				}
			}
		}
	}

	return	0;
}

// whstrmatch_WF
int		whstrmatch_WF::initfromfile(const char *cszFileName)
{
	whfile	*file = whfile_OpenCmnFile(cszFileName, "r");
	if( !file )
	{
		return	-1;
	}

	initfromfile(file);

	whfile_CloseCmnFile(file);

	return		0;
}
int		whstrmatch_WF::initfromfile(whfile *file)
{
	char	szBuf[1024];

	// 重新读入，并填入
	while(!file->IsEOF())
	{
		if( file->ReadLine(szBuf, sizeof(szBuf)-1) )
		{
			wh_strtrim(szBuf);
			if( szBuf[0] )
			{
				if( szBuf[0]=='/'
				&&  szBuf[1]=='/'
				)
				{
					// 注释行
				}
				else
				{
					addstr(szBuf, 0);
				}
			}
		}
	}

	return	0;
}

////////////////////////////////////////////////////////////////////
// whsmpfilestore
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(whsmpfilestore::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szBasePath, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nLoadRefreshInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nUpdateRefreshInterval, 0)
WHDATAPROP_MAP_END()
whsmpfilestore::whsmpfilestore()
{
}
whsmpfilestore::~whsmpfilestore()
{
	clear();
}
int		whsmpfilestore::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	// 保证目标目录存在
	if( m_info.pFM->MakeDir(m_info.szBasePath)<0 )
	{
		return	-1;
	}
	return	0;
}
int		whsmpfilestore::Release()
{
	clear();
	return	0;
}
whsmpfilestore::FILE_T *	whsmpfilestore::SureLoadFile(int nID, bool bMustLoad)
{
	// 看看文件是否在内存中
	time_t	nNow	= wh_time();
	FILE_T	*pF		= NULL;
	if( !m_mapFileID2Obj.get(nID, pF) )
	{
		pF	= new FILE_T;
		pF->nFileID			= nID;
		pF->nLastUpdateTime	= nNow - 3600;	// 这样可以保证，最开始可以进行一次update的尝试
		// 没有载入过，尝试打开文件并载入
		int	rst		= LoadFile(nID, pF, bMustLoad);
		if( rst<0 )
		{
			delete	pF;
			return	NULL;
		}
		m_mapFileID2Obj.put(nID, pF);
	}
	else
	{
		// 判断一下上次是什么时候载入的，如果很长时间了，就重新载入一下（LoadFile会判断文件时间是否改变再载入）
		if( bMustLoad
		|| ((nNow-pF->nLastLoadTime) > m_info.nLoadRefreshInterval)
		)
		{
			int	rst	= LoadFile(nID, pF, bMustLoad);
			if( rst<0 )
			{
				return	NULL;
			}
		}
	}
	return	pF;
}
whsmpfilestore::FILE_T *	whsmpfilestore::GetFileInfo(int nID)
{
	FILE_T	*pF		= NULL;
	if( m_mapFileID2Obj.get(nID, pF) )
	{
		return	pF;
	}
	return	NULL;
}
whsmpfilestore::FILE_T *	whsmpfilestore::UpdateFile(int nID, void *pData, int nDSize)
{
	time_t	nNow	= wh_time();
	whfile	*f	= m_info.pFM->Open(MakeFileName(nID), whfileman::OP_MODE_BIN_CREATE);
	if( f==NULL )
	{
		return	NULL;
	}
	int	rst	= f->Write(pData, nDSize);
	if( rst!=nDSize )
	{
		WHSafeSelfDestroy(f);
		return	NULL;
	}
	// 关闭文件
	WHSafeSelfDestroy(f);
	FILE_T	*pF	= SureLoadFile(nID, true);
	if( pF )
	{
		pF->nLastUpdateTime	= nNow;
	}
	return	pF;
}
void	whsmpfilestore::clear()
{
	if( m_mapFileID2Obj.size()==0 )
	{
		return;
	}
	// 浏览map，把每个对象都释放了
	for(whhash<int, FILE_T *>::kv_iterator it=m_mapFileID2Obj.begin(); it!=m_mapFileID2Obj.end(); ++it)
	{
		FILE_T	*pF	= it.getvalue();
		delete	pF;
	}
	m_mapFileID2Obj.clear();
}
bool	whsmpfilestore::CheckIfFileNeedUpdate(FILE_T *pF)
{
	if( (wh_time()-pF->nLastUpdateTime) > m_info.nUpdateRefreshInterval )
	{
		return	true;
	}
	return		false;
}
const char *	whsmpfilestore::MakeFileName(int nID, char *pszBuf)
{
	if( pszBuf==NULL )
	{
		pszBuf	= m_szFNBuf;
	}
	sprintf(pszBuf, "%s/%d", m_info.szBasePath, nID);
	return	pszBuf;
}
int		whsmpfilestore::LoadFile(int nID, FILE_T *pF, bool bMustLoad)
{
	time_t	nNow	= wh_time();
	const char *	pszFN	= MakeFileName(nID);
	whfile	*f	= m_info.pFM->Open(pszFN, whfileman::OP_MODE_BIN_READONLY);
	if( f==NULL )
	{
		if( !bMustLoad )
		{
			return	-1;
		}
		f		= m_info.pFM->Open(pszFN, whfileman::OP_MODE_BIN_CREATE);
		if( f==NULL )
		{
			return	-2;
		}
	}
	if( bMustLoad
	||  f->FileTime() != pF->nLastFileModifyTime )
	{
		pF->nLastLoadTime		= nNow;
		pF->nLastFileModifyTime	= f->FileTime();
		pF->vectfiledata.resize(f->FileSize());
		f->Read(pF->vectfiledata.getbuf(), pF->vectfiledata.size());
		whmd5(pF->vectfiledata.getbuf(), pF->vectfiledata.size(), pF->MD5);
	}
	// 关闭文件
	WHSafeSelfDestroy(f);
	return	0;
}
