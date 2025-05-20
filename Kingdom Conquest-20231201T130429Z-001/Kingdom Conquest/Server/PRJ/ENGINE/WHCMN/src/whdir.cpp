// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdir.cpp
// Creator      : Wei Hua (κ��)
// Comment      : Ŀ¼����
// CreationDate : 2003-08-30

#include "../inc/wh_platform.h"
#include "../inc/whdir.h"
#include "../inc/whfile.h"
#include "../inc/whstring.h"
#include <stdio.h>
#include <assert.h>
#ifdef	WIN32
#include <direct.h>
#endif
#ifdef	__GNUC__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// ����ʵ��
////////////////////////////////////////////////////////////////////
// ��õ�ǰ����·����ȫ����
char *	whdir_GetCWD(char *szPath, int nSize)
{
	return	getcwd(szPath, nSize);
}
// ���õ�ǰ����·��Ϊ
int		whdir_SetCWD(const char *szPath)
{
	if( !szPath || szPath[0]==0 )
	{
		return	-1;
	}
	return	chdir(szPath)==0 ? 0 : -1;
}
int		whdir_sysmkdir(const char *szDir)
{
#ifdef	WIN32
	if( mkdir(szDir)<0 )
	{
		return	-1;
	}
#endif
#ifdef	__GNUC__
	if( mkdir(szDir, 0755)<0 )
	{
		return	-1;
	}
#endif
	return	0;
}
int		whdir_sysdelfile(const char *szPath)
{
	if( unlink(szPath)<0 )
	{
		return	-1;
	}
	return	0;
}
int		whdir_sysdeldir(const char *szPath)
{
	if( rmdir(szPath)<0 )
	{
		return	-1;
	}
	return	0;
}
int		whdir_MakeDir(const char *szDir)
{
	// ����·����ʼ��һ���ִ�ջ
	// ���¶�࣬�������ı�����srcstack��ʱ����ĳЩ����������ʼ�������ͻ�Ƿ�����������070228�������ע�ͻ��Ǿ��ú���ְ����ѵ��Ǻ�ĳ��VC��ϵͳ���������ˣ���
	whstrstack	whsrcstack('/'), whnewstatck(0);
	if( whsrcstack.reinitfrom(szDir, "/\\")<0 )
	{
		return	-1;
	}

	// һ��һ���ж��ǲ�Ŀ¼�Ǵ��ڵ�
	do
	{
		// �жϸü�Ŀ¼�Ƿ����
		if( whfile_ispathexisted(whsrcstack.getwholestr()) )
		{
			break;
		}
		// �����ڣ���pop���ü�Ŀ¼��������Ҫ�����Ķ�ջ
		whnewstatck.push(whsrcstack.pop());
	}while(whsrcstack.size()>0);

	// ���δ�������Ŀ¼(������м���Ŀ¼�����ڣ���ֱ�ӷ��ؼ���)
	while( whnewstatck.size()>0 )
	{
		whsrcstack.push(whnewstatck.pop());
		const char *szNewDir = whsrcstack.getwholestr();
		if( szNewDir[0]==0 )
		{
			// ����ʹ����"/xxx/xxx�ĸ�ʽ"
			continue;
		}
		if( whdir_sysmkdir(szNewDir)<0 )
		{
			// ����Ŀ¼����
			return	-2;
		}
	}

	return	0;
}
int		whdir_SureMakeDir(const char *szDir)
{
	if( !whfile_ispathexisted(szDir) )
	{
		return	whdir_MakeDir(szDir);
	}
	return	0;
}
int		whdir_SureMakeDirForFile(const char *szFile)
{
	char	szPath[WH_MAX_PATH];
	if( whfile_getfilepath(szFile, szPath) )
	{
		return	whdir_SureMakeDir(szPath);
	}
	return	0;
}
int		whdir_SetCWDToExePath()
{
	char	szPath[WH_MAX_PATH];
	return	whdir_SetCWD(whdir_GetExeFullpath(szPath, sizeof(szPath)));
}

// ����WHDirSearchAction��ɾ���ļ�
int		whdir_DeleteDir(const char *szDir, whdir_cb_DeleteNotify pFileNotify, whdir_cb_DeleteNotify pDirNotify)
{
	class	MyDel	: public WHDirSearchAction
	{
	public:
		whdir_cb_DeleteNotify	m_pFileNotify;
		whdir_cb_DeleteNotify	m_pDirNotify;
		MyDel()
		: m_pFileNotify(NULL), m_pDirNotify(NULL)
		{
		}
	private:
		int		ActionOnFile(const char *szFile)
		{
			// ���������·������Ҫ�ٵõ�һ��
			// �޸��ļ�����
			whfile_makefilewritable(GetFullPath(szFile));
			// ɾ��
			int	rst = whdir_sysdelfile(GetLastFullPath());
			if( m_pFileNotify )
			{
				(*m_pFileNotify)(GetLastFullPath());
			}
			return	rst;
		}
		int		LeaveDir(const char *szDir)
		{
			// ������ȫ·��
			if( m_pDirNotify )
			{
				(*m_pDirNotify)(szDir);
			}
			whfile_makefilewritable(szDir);
			return	whdir_sysdeldir(szDir);
		}
	};
	MyDel	mydel;
	mydel.m_pFileNotify	= pFileNotify;
	mydel.m_pDirNotify	= pDirNotify;
	return	mydel.DoSearch(szDir, true);
}

int		whdir_CopyDir(const char *szDstDir, const char *szSrcDir, bool bRecur, bool bDID, whdir_cb_CopyNotify pFileNotify)
{
	class	MyCopy	: public WHDirSearchAction
	{
	public:
		const char			*m_pszDstDir;
		whdir_cb_CopyNotify	m_pFileNotify;
		MyCopy()
		: m_pszDstDir(NULL)
		, m_pFileNotify(NULL)
		{
		}
	private:
		int		ActionOnFile(const char *szFile)
		{
			// Ŀ��Ŀ¼
			char	szDstDir[WH_MAX_PATH];
			sprintf(szDstDir, "%s%s", m_pszDstDir, GetCurRelPath());
			// ��֤Ŀ��Ŀ¼����
			if( !whfile_ispathexisted(szDstDir) )
			{
				if( whdir_MakeDir(szDstDir)<0 )
				{
					// ����Ŀ¼����
					return	-1;
				}
			}
			// Ŀ���ļ�ȫ·��
			char	szDstFile[WH_MAX_PATH];
			sprintf(szDstFile, "%s/%s", szDstDir, szFile);
			// Դ�ļ���ȫ·��
			// �����ļ�
			if( whfile_cpy(szDstFile, GetFullPath(szFile), true, false)<0 )
			{
				return	-1;
			}
			if( m_pFileNotify )
			{
				(*m_pFileNotify)(szDstFile, GetLastFullPath());
			}
			return	0;
		}
		int		ActionOnDir(const char *szDir)
		{
			// ��ʵ�������ﴴ��Ŀ¼��
			return	0;
		}
	};
	MyCopy	mycopy;
	char	szDID[WH_MAX_PATH];
	if( bDID )
	{
		char	szLastDirName[WH_MAX_PATH];
		if( !whfile_getfilename(szSrcDir, szLastDirName) )
		{
			return	-1;
		}
		sprintf(szDID, "%s/%s", szDstDir, szLastDirName);
		mycopy.m_pszDstDir	= szDID;
	}
	else
	{
		mycopy.m_pszDstDir	= szDstDir;
	}
	mycopy.m_pFileNotify	= pFileNotify;
	return	mycopy.DoSearch(szSrcDir, bRecur);
}

const char *	whdir_GetExeModuleFileName(char *__exe, int __size)
{
	__exe[0]	= 0;
#ifdef	WIN32
	GetModuleFileName(NULL, __exe, __size);
#endif
#ifdef	__GNUC__
	int	rst	= readlink("/proc/self/exe", __exe, __size-1);
	if( rst<0 )
	{
		return NULL;
	}
	__exe[rst] = 0;
#endif
	return	__exe;
}
const char *	whdir_GetExeModuleFileName()
{
	static char	szExe[WH_MAX_PATH];
	return	whdir_GetExeModuleFileName(szExe, sizeof(szExe)-1);
}
const char *	whdir_GetExeFullpath(char *__path, int __size)
{
	whfile_getfilepath(whdir_GetExeModuleFileName(__path, __size), __path);
	return	__path;
}
const char *	whdir_GetExeFullpath()
{
	static char	szExe[WH_MAX_PATH];
	return	whdir_GetExeFullpath(szExe, sizeof(szExe)-1);
}
const char *	whdir_GetExeFName(char *__path, int __size)
{
	return	whfile_getfilename(whdir_GetExeModuleFileName(__path, __size), __path);
}
const char *	whdir_GetExeFName()
{
	static char	szExe[WH_MAX_PATH];
	return	whdir_GetExeFName(szExe, sizeof(szExe)-1);
}

namespace
{
	struct	DIR_STAT_T_notify	: public DIR_STAT_T::notify
	{
		int		nDCount;
		int		nFCount;
		char	szCurDir[WH_MAX_PATH];
		char	szCurFile[WH_MAX_PATH];

		DIR_STAT_T_notify()
			: nDCount(0), nFCount(0)
		{
			szCurDir[0]		= 0;
			szCurFile[0]	= 0;
		}
		void	Print()
		{
			printf("\rTotalDir:%d File:%d CurFile:%s", nDCount, nFCount, szCurFile);
		}
		virtual	void		SelfDestroy()
		{
			delete	this;
		}
		virtual	int			cb_DStat_File(const char *cszFName)
		{
			nFCount++;
			strcpy(szCurFile, cszFName);
			Print();
			return	0;
		}
		virtual	int			cb_DStat_Dir(const char *cszDName)
		{
			nDCount++;
			strcpy(szCurDir, cszDName);
			Print();
			return	0;
		}
	};
}
DIR_STAT_T::notify *	DIR_STAT_T::notify::CreateCmnConsole()
{
	return	new DIR_STAT_T_notify;
}

DIR_STAT_T::DIR_STAT_T()
: nTotalFile(0)
, nTotalFileByte(0)
, nTotalDir(0)
, pNotify(NULL)
, pbShouldStop(NULL)
{
}
void	DIR_STAT_T::reset()
{
	nTotalFile		= 0;
	nTotalFileByte	= 0;
	nTotalDir		= 0;
}

int	WHDirSearchGetStatistics(whfileman *pFM, const char *cszDir, bool bQuick, DIR_STAT_T *pStat)
{
	// ���srcĿ¼��һһ��Ӧ��dstĿ¼��
	class	MyDirSearch	: public WHDirSearchActionWFM
	{
	public:
		DIR_STAT_T	*m_pStat;
		bool		m_bQuick;
	public:
		MyDirSearch(whfileman *pFM, DIR_STAT_T *pStat, bool bQuick)
			: WHDirSearchActionWFM(pFM)
			, m_pStat(pStat)
			, m_bQuick(bQuick)
		{
		}
	private:
		virtual int		ActionOnFile(const char *szFile)
		{
			if( m_pStat->pbShouldStop && (*m_pStat->pbShouldStop) )
			{
				// user canceled
				return	-1;
			}
			int	nFSize	= 0;
			if( m_bQuick )
			{
				whfileman::PATHINFO_T	pathinfo;
				int	rst	= m_pFM->GetPathInfo(GetFullPath(szFile), &pathinfo);
				if( rst<0 )
				{
					return	-10;
				}
				nFSize	= pathinfo.un.file.nFSize;
			}
			else
			{
				whfile	*fp	= m_pFM->Open(GetFullPath(szFile), whfileman::OP_MODE_BIN_READONLY);
				if( fp )
				{
					nFSize	= fp->FileSize();
					WHSafeSelfDestroy(fp);
					if( nFSize<0 )
					{
						return	-20;
					}
				}
			}
			m_pStat->nTotalFile		++;
			m_pStat->nTotalFileByte	+= nFSize;
			if( m_pStat->pNotify )
			{
				m_pStat->pNotify->cb_DStat_File(szFile);
			}
			return	0;
		}
		virtual int		ActionOnDir(const char *szDir)
		{
			if( m_pStat->pbShouldStop && (*m_pStat->pbShouldStop) )
			{
				// user canceled
				return	-1;
			}
			m_pStat->nTotalDir		++;
			if( m_pStat->pNotify )
			{
				m_pStat->pNotify->cb_DStat_Dir(szDir);
			}
			return	0;
		}
	};

	pStat->reset();
	MyDirSearch	mds(pFM, pStat, bQuick);
	return	mds.DoSearch(cszDir, true);
}

}


////////////////////////////////////////////////////////////////////
// ��ʵ��
////////////////////////////////////////////////////////////////////
using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// WHIDDirMapper
////////////////////////////////////////////////////////////////////
WHIDDirMapper::WHIDDirMapper()
{
	m_szRoot[0]			= 0;
	m_szFmt[0]			= 0;
	m_nFileNumInLevel	= 0;
	m_nRootLen			= 0;
	m_nLvlLen			= 0;
}
WHIDDirMapper::~WHIDDirMapper()
{
	Release();
}
int	WHIDDirMapper::Init(INFO_T *pInfo)
{
	Release();

	m_szRoot[0]			= 0;
	m_nRootLen			= 0;
	m_nFileNumInLevel	= 1;
	m_nLvlLen			= pInfo->nFileExp;
	sprintf(m_szFmt, "/%%0%dd", m_nLvlLen);
	for(int i=0;i<m_nLvlLen;i++)
	{
		m_nFileNumInLevel*= 10;
	}
	m_IndexInLevels.resize(pInfo->nLevel);
	return	0;
}
int	WHIDDirMapper::Release()
{
	return	0;
}
void	WHIDDirMapper::SetRoot(const char *cszRoot)
{
	strcpy(m_szRoot, cszRoot);
	m_nRootLen			= strlen(m_szRoot);
}
char *	WHIDDirMapper::GetDirByID(int nID, char *szBuf)
{
	static char	szTmpBuf[WH_MAX_PATH];
	if( !szBuf )
	{
		szBuf	= szTmpBuf;
	}

	memset(m_IndexInLevels.begin(), 0, m_IndexInLevels.totalbytes());
	int		i = m_IndexInLevels.size()-1;
	while(i>=0 && nID>0)
	{
		m_IndexInLevels[i]	=  nID % m_nFileNumInLevel;
		nID					/= m_nFileNumInLevel;
		i					--;
	}
	int		len = m_nRootLen;
	memcpy(szBuf, m_szRoot, len);
	szBuf[len]	= 0;
	for(i=0;i<(int)m_IndexInLevels.size();i++)
	{
		sprintf(szBuf+len, m_szFmt, m_IndexInLevels[i]);
		len	+= m_nLvlLen+1;
	}
	return	szBuf;
}

////////////////////////////////////////////////////////////////////
// WHDir
////////////////////////////////////////////////////////////////////
WHDir::WHDir()
: 
#ifdef __GNUC__
	m_dp(NULL)
#endif
#ifdef WIN32
	m_hFind(INVALID_HANDLE_VALUE)
#endif
{
}
WHDir::~WHDir()
{
	Close();
}
int		WHDir::Open(const char *szDir)
{
	Close();
	strcpy(m_szDir, szDir);

#ifdef __GNUC__
	m_dp	= opendir(szDir);
	if( !m_dp )
	{
		return	-1;
	}
#endif
#ifdef WIN32
	// �����ж�һ�¸�Ŀ¼�Ƿ���ڲ�����Dir
	// 2004-03-15 ���жϸ�Ŀ¼�������繲����ʱ������Ϊ��Ŀ¼��stat����-1��
	// 2006-08-11 �����޸���whfile_ispathdir�������ϲ�Ӧ���ܹ�ȷ�����������Ŀ¼����������Ͳ����ж�����ж��ˡ�
#endif

	return	0;
}
int		WHDir::Close()
{
#ifdef __GNUC__
	if( m_dp )
	{
		closedir(m_dp);
		m_dp	= NULL;
	}
#endif
#ifdef WIN32
	if( m_hFind!=INVALID_HANDLE_VALUE )
	{
		FindClose(m_hFind);
		m_hFind	= INVALID_HANDLE_VALUE;
	}
#endif

	return	0;
}
WHDir::ENTRYINFO_T *	WHDir::Read()
{
	ENTRYINFO_T	*pEntryInfo = NULL;

	memset(&m_EntryInfo, 0, sizeof(m_EntryInfo));

#ifdef __GNUC__
	struct dirent   *pdirent;
	pdirent	= readdir(m_dp);
	if( pdirent )
	{
		pEntryInfo	= &m_EntryInfo;
		assert(pdirent->d_reclen < sizeof(pEntryInfo->szName)-1 );
		strcpy(m_EntryInfo.szName, pdirent->d_name);
		pEntryInfo->bIsDir	= (pdirent->d_type & DT_DIR) != 0;
	}
#endif
#ifdef WIN32
	WIN32_FIND_DATA	wfd;
	if( m_hFind==INVALID_HANDLE_VALUE )
	{
		char	szFindName[WHDIR_MAX_NAME];
		sprintf(szFindName, "%s/*.*", m_szDir);
		m_hFind	= FindFirstFile(szFindName, &wfd);
		if( m_hFind!=INVALID_HANDLE_VALUE )
		{
			pEntryInfo		= &m_EntryInfo;
		}
	}
	else
	{
		if( FindNextFile(m_hFind, &wfd) )
		{
			pEntryInfo		= &m_EntryInfo;
		}
		DWORD dwe = GetLastError();
	}
	if( pEntryInfo )
	{
		strcpy(pEntryInfo->szName, wfd.cFileName);
		pEntryInfo->bIsDir	= (FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes) != 0;
		//pEntryInfo->nSize	= wfd.nFileSizeLow;	// �����ϲ�Ӧ�ô���DWORD�ĳ���
	}
#endif

	return	pEntryInfo;
}
void	WHDir::Rewind()
{
#ifdef __GNUC__
	rewinddir(m_dp);
#endif
#ifdef WIN32
	// �����Ϳ�����������
	if( m_hFind!=INVALID_HANDLE_VALUE )
	{
		FindClose(m_hFind);
		m_hFind	= INVALID_HANDLE_VALUE;
	}
#endif
}
char *	WHDir::MakeFullPath(const char *cszFile, char *szBuf)
{
	sprintf(szBuf, "%s/%s", m_szDir, cszFile);
	return	szBuf;
}

////////////////////////////////////////////////////////////////////
// WHDirSearchActionWFM
////////////////////////////////////////////////////////////////////
int		WHDirSearchActionWFM::DoSearch(const char *szPath, bool bRecur)
{
	if( m_pbStop )
	{
		*m_pbStop	= false;
	}
	m_nRootPathLen	= strlen(szPath);
	m_nPathLevel	= 0;
	return	DoOneDir(szPath, bRecur);
}
void	WHDirSearchActionWFM::SetFilter(const char *cszFilter)
{
	m_vectGoodFilter.clear();
	whfile_MakeExtFilter(cszFilter, m_vectGoodFilter);
}
void	WHDirSearchActionWFM::SetBadFilter(const char *cszFilter)
{
	m_vectBadFilter.clear();
	whfile_MakeExtFilter(cszFilter, m_vectBadFilter);
}
void	WHDirSearchActionWFM::AddSkipFile(const char *cszFile)
{
	m_setFileToSkip.Add(cszFile);
}
int		WHDirSearchActionWFM::DoOneDir(const char *szDir, bool bRecur)
{
	int		rst = 0;

	strcpy(m_szCurPath, szDir);

	if( EnterDir(szDir)<0 )
	{
		return	-1;
	}

	WHDirBase	*pDir	= m_pFM->OpenDir(szDir);
	if( !pDir )
	{
		return	-2;
	}

	// ����ֻ�������return��
	m_nPathLevel		++;

	WHDirBase::ENTRYINFO_T	*pEntry;

	while( (m_pbStop==NULL || !(*m_pbStop)) && (pEntry=pDir->Read())!=NULL )
	{
		if( pEntry->bIsDir )
		{
			if( strcmp(".", pEntry->szName)==0
			||  strcmp("..", pEntry->szName)==0
			)
			{
				// ������ʲôҲ������
			}
			else
			{
				int	actrst	= ActionOnDir(pEntry->szName);
				if( actrst<0 )
				{
					rst		= -100 + actrst;
					goto	End;
				}
				else
				{
					if( bRecur && !m_bShouldNotGoInThisDir )
					{
						// �þֲ�����szFullPath��Ϊ�˱���ݹ����ʱӰ��������
						char	szFullPath[WH_MAX_PATH];
						rst		= DoOneDir(GetFullPath(pEntry->szName, szFullPath), bRecur);
						// �ָ��ϼ���CurPath
						strcpy(m_szCurPath, szDir);
						if( rst<0 )
						{
							goto	End;
						}
					}
					m_bShouldNotGoInThisDir	= false;
				}
			}
		}
		else
		{
			// ���Ƿ��Ǳ�ƥ���
			if( whfile_ExtIsMatch(pEntry->szName, m_vectGoodFilter) )
			{
				if( !m_setFileToSkip.Has(pEntry->szName) )
				{
					if( m_vectBadFilter.size()==0 || !whfile_ExtIsMatch(pEntry->szName, m_vectBadFilter) )
					{
						int	actrst	= ActionOnFile(pEntry->szName);
						if( actrst<0 )
						{
							rst		= -1000 + actrst;
							goto	End;
						}
					}
				}
			}
		}
	}

End:
	// ע��LeaveDir��ʵ������ܻ��õ�m_nPathLevel������m_nPathLevel--��LeaveDir��˳���ܱ���!!!!
	m_nPathLevel	--;

	// pDir->Close();
	delete	pDir;	// ���ȷ�������е����ˣ��Ͳ���Close�ˡ�Close�����ڱ�ĵط��ù���
	if( rst==0 && LeaveDir(szDir)<0 )
	{
		return		-3;
	}

	return			rst;
}
const char *	WHDirSearchActionWFM::GetFullPath(const char *szPath, char *szFullPath)
{
	sprintf(szFullPath, "%s/%s", m_szCurPath, szPath);
	return	szFullPath;
}
