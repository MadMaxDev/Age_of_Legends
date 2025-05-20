// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdir.h
// Creator      : Wei Hua (魏华)
// Comment      : 目录功能
// CreationDate : 2003-08-30
// ChangeLOG    : 2006-10-23 WHDirSearchActionTmpl:::ActionOnDir改为非纯虚的。

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

// 系统相关的函数
// 创建目录
int	whdir_sysmkdir(const char *szDir);
// 删除文件(这个和whfile_del功能完全一样，只是历史原因造成的两个名字)
int	whdir_sysdelfile(const char *szPath);
// 删除目录
int	whdir_sysdeldir(const char *szPath);

// 获得当前工作路径的全名，如果成功返回szPath，否则返回NULL
char *	whdir_GetCWD(char *szPath, int nSize);
// 设置当前工作路径为
int		whdir_SetCWD(const char *szPath);
// 保证性创建多级目录
int		whdir_MakeDir(const char *szDir);
// 确保目录存在
int		whdir_SureMakeDir(const char *szDir);
// 保证文件需要的目录存在
int		whdir_SureMakeDirForFile(const char *szFile);
// 设置工作目录到exe所在目录
int		whdir_SetCWDToExePath();

// 删除目录
typedef	void (*whdir_cb_DeleteNotify)(const char *szPath);
int		whdir_DeleteDir(const char *szDir, whdir_cb_DeleteNotify pFileNotify=NULL, whdir_cb_DeleteNotify pDirNotify=NULL);

// 拷贝目录
// bDID 表示是否将szSrcDir作为一个子目录拷贝到目标目录下面(DID means DirInDir)
typedef	void (*whdir_cb_CopyNotify)(const char *szDstPath, const char *szSrcPath);
int		whdir_CopyDir(const char *szDstDir, const char *szSrcDir, bool bRecur, bool bDID, whdir_cb_CopyNotify pFileNotify=NULL);

// 得到本可执行文件的全路径名
const char *	whdir_GetExeModuleFileName(char *__exe, int __size);
const char *	whdir_GetExeModuleFileName();
// 得到本可执行文件的全路径名(不包含exe部分)
const char *	whdir_GetExeFullpath(char *__path, int __size);
const char *	whdir_GetExeFullpath();
// 获得本可执行文件名字
const char *	whdir_GetExeFName(char *__path, int __size);
const char *	whdir_GetExeFName();

////////////////////////////////////////////////////////////////////
// 用于海量文件的“序号<->目录”对应方式（比如用于刀剑Online的用户文件存储）
////////////////////////////////////////////////////////////////////
class	WHIDDirMapper
{
public:
	struct	INFO_T
	{
		int			nLevel;									// 分几级
		int			nFileExp;								// 每级的文件数量对10的导数值(也就是说，文件数必须为10的整倍数)，
															// 文件总数为(10^nFileExp)^nLevel
	};
protected:
	char	m_szRoot[WH_MAX_PATH];							// 根目录名的拷贝
	char	m_szFmt[64];									// 产生最终一级目录的sprintf中的格式，如"/%03d"就表示每级
	int		m_nRootLen, m_nLvlLen;							// 上面两个字串最终打印出来的长度
	int		m_nFileNumInLevel;								// 每级的目录数量，即10^nFileExp
	whvector<int>	m_IndexInLevels;						// 临时存放在各个level中的索引值
public:
	WHIDDirMapper();
	~WHIDDirMapper();
	int	Init(INFO_T *pInfo);
	int	Release();
	void	SetRoot(const char *cszRoot);					// 设置根目录
	char *	GetDirByID(int nID, char *szBuf);				// 通过数字ID得到路径全名，放到szBuf中
															// 里面不作范围判断，需要上层调用者保证。对于超界的ID会得到不确定路径名。
															// 如果szBuf为NULL则使用内部的静态变量
};

////////////////////////////////////////////////////////////////////
// 目录内容读取类
////////////////////////////////////////////////////////////////////
// 普通文件系统上的实现
class	WHDir	: public WHDirBase
{
protected:
	ENTRYINFO_T	m_EntryInfo;
	// Open打开的Dir的全名
	char		m_szDir[WHDIR_MAX_NAME];
#ifdef __GNUC__
	DIR		*m_dp;
#endif
#ifdef WIN32
	HANDLE	m_hFind;
#endif
protected:
	// 将文件名放在打开的目录后面构成文件全名
	char *	MakeFullPath(const char *cszFile, char *szBuf);
public:
	WHDir();
	// 这个里面会调用Close
	~WHDir();
	virtual void	SelfDestroy()	{delete this;}
	virtual const char *	GetDir() const
	{
		return	m_szDir;
	}
	// 打开一个目录(szDir最后不能有"/"或"\")
	int		Open(const char *szDir);
	// 关闭已经打开的目录
	int		Close();
	// 读入一个条目(返回NULL表示没有可读的了)
	ENTRYINFO_T *	Read();
	void			Rewind();
	// 创建一个自己
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
// 目录浏览动作类
////////////////////////////////////////////////////////////////////
template<class _DirRD>
class WHDirSearchActionTmpl
{
public:
	WHDirSearchActionTmpl();
	virtual ~WHDirSearchActionTmpl()	{}
	// szPath最后不能有"/"或"\"
	int		DoSearch(const char *szPath, bool bRecur);
	int		StopSearch();
	// 设置文件后缀过滤
	// cszFilters的取值like: "c cpp h"（不同的后缀之间用空格分隔）
	// 如果cszFilters为NULL或空字串则表示清除取消所有过滤器
	// 另外还要注意，后缀匹配是大小写敏感的，这个是为了将来linux不出错!!!!
	int		SetExtFilter(const char *cszFilters);
protected:
	// 一次搜索动作，所有的搜索由一系列递归的搜索动作组成
	int		DoOneDir(const char *szDir, bool bRecur);
	// 获得全路径名
	char *	GetFullPath(const char *szPath, char *szFullPath);
	// 使用完这个函数后可以用GetLastFullPath得到同样的返回
	inline char *	GetFullPath(const char *szPath)
	{
		return	GetFullPath(szPath, m_szFullPath);
	}
	inline const char *	GetLastFullPath() const
	{
		return	m_szFullPath;
	}
	// 获得当前目录相对起始目录的文件名(前面会带最后不带"/"或"\"，可以直接加到别的路径后面)
	inline const char *	GetCurRelPath() const
	{
		return	m_szCurPath + m_nRootPathLen;
	}
	// 在使用了GetFullPath(const char *szPath)之后可以用这个得到相对路径
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
	// 初始路径的字符长度
	int		m_nRootPathLen;
	// 存放当前正在工作的路径
	char	m_szCurPath[WHDIR_MAX_NAME];
	// 用于临时存放全路径名
	char	m_szFullPath[WHDIR_MAX_NAME];
	// 当前的目录深度
	int		m_nPathLevel;
protected:
	// 在ActionOnDir中设置，如果设为真，则不用继续进入这个目录
	bool	m_bShouldNotGoInThisDir;
private:
	// 这两个函数传入的是最后一段路径名，即相对父目录的路径名，且最后不会有"/"或"\"
	// 函数内部可以通过GetFullPath()得到全路径名
	// 这两个函数的任何一个返回-1，整个搜索都会中止
	// 对文件的操作
	virtual int		ActionOnFile(const char *szFile)	= 0;
	// 对目录的操作(这个是在打开该目录之前做的)
	virtual int		ActionOnDir(const char *szDir)		{return 0;}
	// 一个目录搜索完毕
	// 下面两个函数传入的是全路径，且最后不会有"/"或"\"
	// 进入目录
	virtual int		EnterDir(const char *szDir)			{return 0;}
	// 从目录中退出（此时目录已经关闭，如果要删除该可以在这里）
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

	// 后面只有最后有return了
	m_nPathLevel		++;

	// 如果不写typename，在linux下会出错
	typename _DirRD::ENTRYINFO_T	*pEntry;

	while( !m_bStop && (pEntry=pDir->Read())!=NULL )
	{
		if( pEntry->bIsDir )
		{
			if( strcmp(".", pEntry->szName)==0
			||  strcmp("..", pEntry->szName)==0
			)
			{
				// 这两个什么也不用做
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
						// 用局部变量szFullPath是为了避免递归调用时影响后续结果
						char	szFullPath[WH_MAX_PATH];
						rst		= DoOneDir(GetFullPath(pEntry->szName, szFullPath), bRecur);
						// 恢复上级的CurPath
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
	// 注意LeaveDir的实现里可能会用到m_nPathLevel，所以m_nPathLevel--和LeaveDir的顺序不能变了!!!!
	m_nPathLevel		--;

	// pDir->Close();
	delete	pDir;	// 如果确保析构中调用了，就不用Close了。Close好像还在别的地方用过。
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

// 最普通的就用WHDir了
typedef	WHDirSearchActionTmpl<WHDir>	WHDirSearchAction;

}		// EOF namespace n_whcmn

#endif	// EOF __WHDIR_H__
