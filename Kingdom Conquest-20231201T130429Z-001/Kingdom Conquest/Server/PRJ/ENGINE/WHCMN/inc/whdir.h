// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdir.h
// Creator      : Wei Hua (κ��)
// Comment      : Ŀ¼����
// CreationDate : 2003-08-30
// ChangeLOG    : 2006-10-23 WHDirSearchActionTmpl:::ActionOnDir��Ϊ�Ǵ���ġ�

#ifndef	__WHDIR_H__
#define __WHDIR_H__

#include "../inc/wh_platform.h"

#ifdef __GNUC__
#include <dirent.h>
#include <sys/types.h>
#endif

#include "whfile.h"
#include "whvector.h"

namespace n_whcmn
{

// ϵͳ��صĺ���
// ����Ŀ¼
int	whdir_sysmkdir(const char *szDir);
// ɾ���ļ�(�����whfile_del������ȫһ����ֻ����ʷԭ����ɵ���������)
int	whdir_sysdelfile(const char *szPath);
// ɾ��Ŀ¼
int	whdir_sysdeldir(const char *szPath);

// ��õ�ǰ����·����ȫ��������ɹ�����szPath�����򷵻�NULL
char *	whdir_GetCWD(char *szPath, int nSize);
// ���õ�ǰ����·��Ϊ
int		whdir_SetCWD(const char *szPath);
// ��֤�Դ����༶Ŀ¼
int		whdir_MakeDir(const char *szDir);
// ȷ��Ŀ¼����
int		whdir_SureMakeDir(const char *szDir);
// ��֤�ļ���Ҫ��Ŀ¼����
int		whdir_SureMakeDirForFile(const char *szFile);
// ���ù���Ŀ¼��exe����Ŀ¼
int		whdir_SetCWDToExePath();

// ɾ��Ŀ¼
typedef	void (*whdir_cb_DeleteNotify)(const char *szPath);
int		whdir_DeleteDir(const char *szDir, whdir_cb_DeleteNotify pFileNotify=NULL, whdir_cb_DeleteNotify pDirNotify=NULL);

// ����Ŀ¼
// bDID ��ʾ�Ƿ�szSrcDir��Ϊһ����Ŀ¼������Ŀ��Ŀ¼����(DID means DirInDir)
typedef	void (*whdir_cb_CopyNotify)(const char *szDstPath, const char *szSrcPath);
int		whdir_CopyDir(const char *szDstDir, const char *szSrcDir, bool bRecur, bool bDID, whdir_cb_CopyNotify pFileNotify=NULL);

// �õ�����ִ���ļ���ȫ·����
const char *	whdir_GetExeModuleFileName(char *__exe, int __size);
const char *	whdir_GetExeModuleFileName();
// �õ�����ִ���ļ���ȫ·����(������exe����)
const char *	whdir_GetExeFullpath(char *__path, int __size);
const char *	whdir_GetExeFullpath();
// ��ñ���ִ���ļ�����
const char *	whdir_GetExeFName(char *__path, int __size);
const char *	whdir_GetExeFName();

////////////////////////////////////////////////////////////////////
// ���ں����ļ��ġ����<->Ŀ¼����Ӧ��ʽ���������ڵ���Online���û��ļ��洢��
////////////////////////////////////////////////////////////////////
class	WHIDDirMapper
{
public:
	struct	INFO_T
	{
		int			nLevel;									// �ּ���
		int			nFileExp;								// ÿ�����ļ�������10�ĵ���ֵ(Ҳ����˵���ļ�������Ϊ10��������)��
															// �ļ�����Ϊ(10^nFileExp)^nLevel
	};
protected:
	char	m_szRoot[WH_MAX_PATH];							// ��Ŀ¼���Ŀ���
	char	m_szFmt[64];									// ��������һ��Ŀ¼��sprintf�еĸ�ʽ����"/%03d"�ͱ�ʾÿ��
	int		m_nRootLen, m_nLvlLen;							// ���������ִ����մ�ӡ�����ĳ���
	int		m_nFileNumInLevel;								// ÿ����Ŀ¼��������10^nFileExp
	whvector<int>	m_IndexInLevels;						// ��ʱ����ڸ���level�е�����ֵ
public:
	WHIDDirMapper();
	~WHIDDirMapper();
	int	Init(INFO_T *pInfo);
	int	Release();
	void	SetRoot(const char *cszRoot);					// ���ø�Ŀ¼
	char *	GetDirByID(int nID, char *szBuf);				// ͨ������ID�õ�·��ȫ�����ŵ�szBuf��
															// ���治����Χ�жϣ���Ҫ�ϲ�����߱�֤�����ڳ����ID��õ���ȷ��·������
															// ���szBufΪNULL��ʹ���ڲ��ľ�̬����
};

////////////////////////////////////////////////////////////////////
// Ŀ¼���ݶ�ȡ��
////////////////////////////////////////////////////////////////////
// ��ͨ�ļ�ϵͳ�ϵ�ʵ��
class	WHDir	: public WHDirBase
{
protected:
	ENTRYINFO_T	m_EntryInfo;
	// Open�򿪵�Dir��ȫ��
	char		m_szDir[WHDIR_MAX_NAME];
#ifdef __GNUC__
	DIR		*m_dp;
#endif
#ifdef WIN32
	HANDLE	m_hFind;
#endif
protected:
	// ���ļ������ڴ򿪵�Ŀ¼���湹���ļ�ȫ��
	char *	MakeFullPath(const char *cszFile, char *szBuf);
public:
	WHDir();
	// �����������Close
	~WHDir();
	virtual void	SelfDestroy()	{delete this;}
	virtual const char *	GetDir() const
	{
		return	m_szDir;
	}
	// ��һ��Ŀ¼(szDir�������"/"��"\")
	int		Open(const char *szDir);
	// �ر��Ѿ��򿪵�Ŀ¼
	int		Close();
	// ����һ����Ŀ(����NULL��ʾû�пɶ�����)
	ENTRYINFO_T *	Read();
	void			Rewind();
	// ����һ���Լ�
	static WHDir *	Create(const char *szDir)
	{
		WHDir	*pDir = new WHDir;
		if( pDir->Open(szDir)<0 )
		{
			delete	pDir;
			return	NULL;
		}
		return	pDir;
	}
};

////////////////////////////////////////////////////////////////////
// Ŀ¼���������
////////////////////////////////////////////////////////////////////
template<class _DirRD>
class WHDirSearchActionTmpl
{
public:
	WHDirSearchActionTmpl();
	virtual ~WHDirSearchActionTmpl()	{}
	// szPath�������"/"��"\"
	int		DoSearch(const char *szPath, bool bRecur);
	int		StopSearch();
	// �����ļ���׺����
	// cszFilters��ȡֵlike: "c cpp h"����ͬ�ĺ�׺֮���ÿո�ָ���
	// ���cszFiltersΪNULL����ִ����ʾ���ȡ�����й�����
	// ���⻹Ҫע�⣬��׺ƥ���Ǵ�Сд���еģ������Ϊ�˽���linux������!!!!
	int		SetExtFilter(const char *cszFilters);
protected:
	// һ���������������е�������һϵ�еݹ�������������
	int		DoOneDir(const char *szDir, bool bRecur);
	// ���ȫ·����
	char *	GetFullPath(const char *szPath, char *szFullPath);
	// ʹ������������������GetLastFullPath�õ�ͬ���ķ���
	inline char *	GetFullPath(const char *szPath)
	{
		return	GetFullPath(szPath, m_szFullPath);
	}
	inline const char *	GetLastFullPath() const
	{
		return	m_szFullPath;
	}
	// ��õ�ǰĿ¼�����ʼĿ¼���ļ���(ǰ������󲻴�"/"��"\"������ֱ�Ӽӵ����·������)
	inline const char *	GetCurRelPath() const
	{
		return	m_szCurPath + m_nRootPathLen;
	}
	// ��ʹ����GetFullPath(const char *szPath)֮�����������õ����·��
	inline const char *	GetFullRelPath() const
	{
		return	m_szFullPath + m_nRootPathLen;
	}
	inline int	GetPathLevel() const
	{
		return	m_nPathLevel;
	}
private:
	bool	m_bStop;
	// ��ʼ·�����ַ�����
	int		m_nRootPathLen;
	// ��ŵ�ǰ���ڹ�����·��
	char	m_szCurPath[WHDIR_MAX_NAME];
	// ������ʱ���ȫ·����
	char	m_szFullPath[WHDIR_MAX_NAME];
	// ��ǰ��Ŀ¼���
	int		m_nPathLevel;
protected:
	// ��ActionOnDir�����ã������Ϊ�棬���ü����������Ŀ¼
	bool	m_bShouldNotGoInThisDir;
private:
	// ��������������������һ��·����������Ը�Ŀ¼��·����������󲻻���"/"��"\"
	// �����ڲ�����ͨ��GetFullPath()�õ�ȫ·����
	// �������������κ�һ������-1����������������ֹ
	// ���ļ��Ĳ���
	virtual int		ActionOnFile(const char *szFile)	= 0;
	// ��Ŀ¼�Ĳ���(������ڴ򿪸�Ŀ¼֮ǰ����)
	virtual int		ActionOnDir(const char *szDir)		{return 0;}
	// һ��Ŀ¼�������
	// �������������������ȫ·��������󲻻���"/"��"\"
	// ����Ŀ¼
	virtual int		EnterDir(const char *szDir)			{return 0;}
	// ��Ŀ¼���˳�����ʱĿ¼�Ѿ��رգ����Ҫɾ���ÿ��������
	virtual int		LeaveDir(const char *szDir)			{return 0;}
};

template<class _DirRD>
WHDirSearchActionTmpl<_DirRD>::WHDirSearchActionTmpl()
: m_bStop(false), m_nRootPathLen(0), m_nPathLevel(0)
, m_bShouldNotGoInThisDir(false)
{
}
template<class _DirRD>
int		WHDirSearchActionTmpl<_DirRD>::DoSearch(const char *szPath, bool bRecur)
{
	m_bStop			= false;
	m_nRootPathLen	= strlen(szPath);
	m_nPathLevel		= 0;
	return	DoOneDir(szPath, bRecur);
}
template<class _DirRD>
int		WHDirSearchActionTmpl<_DirRD>::StopSearch()
{
	m_bStop	= true;
	return	0;
}
template<class _DirRD>
int		WHDirSearchActionTmpl<_DirRD>::DoOneDir(const char *szDir, bool bRecur)
{
	int		rst = 0;

	strcpy(m_szCurPath, szDir);

	if( EnterDir(szDir)<0 )
	{
		return	-1;
	}

	WHDirBase	*pDir	= _DirRD::Create(szDir);
	if( !pDir )
	{
		return	-1;
	}

	// ����ֻ�������return��
	m_nPathLevel		++;

	// �����дtypename����linux�»����
	typename _DirRD::ENTRYINFO_T	*pEntry;

	while( !m_bStop && (pEntry=pDir->Read())!=NULL )
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
				if( ActionOnDir(pEntry->szName)<0 )
				{
					rst		= -1;
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
			if( ActionOnFile(pEntry->szName)<0 )
			{
				rst		= -1;
				goto	End;
			}
		}
	}

End:
	// ע��LeaveDir��ʵ������ܻ��õ�m_nPathLevel������m_nPathLevel--��LeaveDir��˳���ܱ���!!!!
	m_nPathLevel		--;

	// pDir->Close();
	delete	pDir;	// ���ȷ�������е����ˣ��Ͳ���Close�ˡ�Close�����ڱ�ĵط��ù���
	if( rst==0 && LeaveDir(szDir)<0 )
	{
		return	-1;
	}

	return	rst;
}
template<class _DirRD>
char *	WHDirSearchActionTmpl<_DirRD>::GetFullPath(const char *szPath, char *szFullPath)
{
	sprintf(szFullPath, "%s/%s", m_szCurPath, szPath);
	return	szFullPath;
}

// ����ͨ�ľ���WHDir��
typedef	WHDirSearchActionTmpl<WHDir>	WHDirSearchAction;

}		// EOF namespace n_whcmn

#endif	// EOF __WHDIR_H__
