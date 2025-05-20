// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfile_base.h
// Creator      : Wei Hua (魏华)
// Comment      : 抽象文件接口定义
//              : 各个模块需要被程序的最顶层设置一个fileman然后使用。之所以不用一个静态的fileman是因为将来程序的组织可能是多个DLL，这样可能导致静态的filename变成多个，导致混乱。
// CreationDate : 2006-06-23 从whfile.h中抽出
// ChangeLOG    : 2006-10-23 把WHDirSearchActionWFM::ActionOnDir改为非纯虚函数，因为的确并不是总需要处理目录的。
//              : 2007-01-24 增加了WHDirSearchActionWFM中停止搜索的变量指针

#ifndef	__WHFILE_BASE_H__
#define	__WHFILE_BASE_H__

#include "./wh_platform.h"
#ifdef	WIN32
#define WH_MAX_PATH			MAX_PATH
#endif
#ifdef	__GNUC__
#include <stdlib.h>
#define WH_MAX_PATH			260
#define	S_IREAD				S_IRUSR
#define	S_IWRITE			S_IWUSR
#endif

// 最多的目录级数
#define WH_MAX_PATHLEVEL	64

#include <stdio.h>
#include <time.h>
#include "./whvector.h"
#include "./whhash.h"
#include <map>
#include <string>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 通用文件基类(目前还不支持多线程访问一个file对象)
////////////////////////////////////////////////////////////////////
class	whfile
{
protected:
	// 防止被直接new
	whfile()			{}
public:
	// 不过为了简单应该可以直接删除(省去了Close方法，不过最好不要在一个DLL内创建了让另外一个DLL去删除)
	virtual ~whfile()	{}
	// 如果用delete不好使的时候可以用这个来删除（比如跨DLL的删除调用）
	virtual	void	SelfDestroy()							= 0;
	// 获得文件总长度(这个在估计文件可能被分成多少块读入时很有用)
	// 应该不会出现超过2G的文件（包括打包文件）
	virtual int		FileSize() const						= 0;
	// 获得文件时间（就是最后一次修改时间）
	virtual time_t	FileTime() const						= 0;
	// 设置文件修改时间（即写打开的时间。如果这样设置了即便文件被写修改了，那么在关闭的时候文件时间也是设定的时间）
	// 如果t==0，则自动取当前的系统时间
	virtual void	SetFileTime(time_t t=0)					= 0;
	// 返回实际得到的字节数，0表示没有数据了，<0表示出错了
	virtual int		Read(void *pBuf, int nSize)				= 0;
	// 移动文件指针
	// SEEK的参数和fseek一致，提供反向seek
	// 返回0表示成功，<0表示出错了
	// nOrigin使用和fseek参数一样的SEEK_SET、SEEK_CUR和SET_END
	// 在写文件的时候，Seek应该可以扩大文件
	virtual int		Seek(int nOffset, int nOrigin)			= 0;
	// 判断是否到了文件尾
	virtual bool	IsEOF()									= 0;
	// 获得当前文件指针位置
	virtual int		Tell()									= 0;
	// 写入数据(和标准文件操作一样，Write返回写入的字节数)
	virtual int		Write(const void *pBuf, int nSize)		= 0;
	// 缓冲到硬盘（返回和fflush的结果一致）
	virtual int		Flush()									= 0;

	// 重复写入一段数据
	virtual int		DupWrite(const void *pBuf, int nSize, int nDupNum);

	// 获得文件的md5 (如果是zip中的文件就直接返回，否则默认需要重新全部文件计算，所以外界最好不要经常调用)
	// MD5应该是16字节的缓冲区
	// 注意，这个函数可能会改变文件指针的位置!!!!
	virtual int		GetMD5(unsigned char *MD5);

	// 获得一行，返回行的字节数，行包含结束符"\n"或"\r\n"，文件里是什么就是什么
	// 如果一行的长度超过了nSize，则只读到缓冲这么大（最后会填上一个0），该行剩下保持未读状态
	int	ReadLine(char *pBuf, int nSize);

	// 重新seek到头
	inline int	Rewind()
	{
		return	Seek(0, SEEK_SET);
	}
	// 单纯变长尺寸的读写
	// 读出
	int	ReadVSize();
	// 写入
	int	WriteVSize(int nSize);
	// 变长数据读写
	// 读出变长数据(返回读到的数据部分的长度)
	int	ReadVData(void *pData, int nMaxSize);
	// 写入变长数据（返回的是数据部分的长度，如果结果<=0则说明有错误）
	int	WriteVData(const void *pData, int nSize);
	// 变长字串读写（字串长度可以是1~0x3FFFFFFF）
	// 读出变长字串(返回读到的长度)（注：会自动在结尾加0）
	int	ReadVStr(char *szStr, int nMaxSize);
	// 写入变长字串(返回写入的长度)
	// 如果nMaxSize>0则说明需要保证szStr的长度不小于nMaxSize
	int	WriteVStr(const char *szStr, int nMaxSize=-1);
	// 针对特殊类型的读写
	template<typename _Ty>
		inline int	Read(_Ty *pBuf)
	{
		return	Read(pBuf, sizeof(*pBuf));
	}
	template<typename _Ty>
		inline int	Write(const _Ty *pBuf)
	{
		return	Write(pBuf, sizeof(*pBuf));
	}
public:
	// 获得系统文件指针(如果有的话。这个主要给刘岩用)
	virtual FILE *	GetFILE() const
	{
		return	NULL;
	}
};

////////////////////////////////////////////////////////////////////
// 旧的通用目录对象基类（为了和老的whzlib兼容所以保留）
////////////////////////////////////////////////////////////////////
enum
{
	WHDIR_MAX_NAME	= WH_MAX_PATH,
};
class	WHDirBase
{
public:
	struct	ENTRYINFO_T
	{
		bool	bIsDir;
		char	szName[WHDIR_MAX_NAME];			// 一段名字应该不会超过这个值
		union
		{
			struct
			{
				int		nID;					// 文件序号
			}pck;								// 只在打包文件中有效的信息
		}ext;
	};
	WHDirBase	*m_pNext;						// 如果一个目录可能由多个部分组成，则这里指向下一个
public:
	WHDirBase() : m_pNext(NULL)					{}
	virtual ~WHDirBase()						{}
	virtual	void	SelfDestroy()				= 0;
	// 获得基础dir
	virtual const char *	GetDir() const		= 0;
	// 读入一个条目(返回NULL表示没有可读的了)
	// ENTRYINFO_T无需释放，是一个内部变量，上层读了也可以修改其中的内容，但是要注意它只有一份!!!!
	virtual ENTRYINFO_T *	Read()				= 0;
	// 从头开始读
	virtual void			Rewind()			= 0;
};
class	WHDirChain	: public WHDirBase
{
protected:
	WHDirBase	*m_pHeadDir;
	whvector<ENTRYINFO_T>	m_vectEntry;
	int			m_nCurIdx;
public:
	explicit WHDirChain(WHDirBase *pDir)
		: m_pHeadDir(NULL)
		, m_nCurIdx(0)
	{
		m_vectEntry.reserve(64);
		SetHeadDir(pDir);
	}
	void	SetHeadDir(WHDirBase *pDir)
	{
		std::map< std::string, ENTRYINFO_T >	mapEntry;
		// 这时候就读一次
		m_pHeadDir	= pDir;
		m_pNext		= m_pHeadDir;
		while( m_pNext )
		{
			ENTRYINFO_T	*pRst	= m_pNext->Read();
			if( !pRst )
			{
				m_pNext	= m_pNext->m_pNext;
			}
			else
			{
				mapEntry.insert(std::make_pair(pRst->szName, *pRst));
			}
		}
		for(std::map< std::string, ENTRYINFO_T >::iterator it=mapEntry.begin(); it!=mapEntry.end(); ++it)
		{
			m_vectEntry.push_back((*it).second);
		}
		m_nCurIdx	= 0;
	}
	virtual ~WHDirChain()
	{
		// 每个都删除一下
		m_pNext		= m_pHeadDir;
		while( m_pNext )
		{
			WHDirBase	*pTmp	= m_pNext;
			m_pNext	= m_pNext->m_pNext;
			pTmp->SelfDestroy();
		}
		m_pHeadDir	= NULL;
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	// 获得基础dir
	virtual const char *	GetDir() const
	{
		return	m_pHeadDir->GetDir();
	}
	// 读入一个条目(返回NULL表示没有可读的了)
	// ENTRYINFO_T无需释放，是一个内部变量，上层读了也可以修改其中的内容，但是要注意它只有一份!!!!
	virtual ENTRYINFO_T *	Read()
	{
		if( m_nCurIdx>=(int)m_vectEntry.size() )
		{
			return	NULL;
		}
		return	m_vectEntry.getptr(m_nCurIdx++);
	}
	// 从头开始读
	virtual void			Rewind()
	{
		m_nCurIdx	= 0;
	}
};

////////////////////////////////////////////////////////////////////
// 通用文件系统基类（whfile、WHDirBase的创建应该是通过这个对象）
////////////////////////////////////////////////////////////////////
class	whfileman
{
public:
	virtual	~whfileman()		{};
	// 如果用delete不好使的时候可以用这个来删除（比如跨DLL的删除调用）
	virtual	void	SelfDestroy()				= 0;
	// 目前打开模式只提供二进制方式
	enum OP_MODE_T
	{
		OP_MODE_UNDEFINED		= 0,			// 未定义的模式
		OP_MODE_BIN_READONLY	= 1,			// 只读
		OP_MODE_BIN_CREATE		= 2,			// 读写（如果不存在就创建）
		OP_MODE_BIN_READWRITE	= 3,			// 读写（如果不存在就出错）
		OP_MODE_BIN_RAWREAD		= 4,			// 只读，完全把底层数据读出来，不进行解密或者解压（主要用于数据出错调试）
	};
	// 下面的文件名和目录名前后不能有空格，传入前应该trim一下；而且目录名最后不能有'/'或'\'
	virtual	whfile *		Open(const char *cszFName, OP_MODE_T mode)	= 0;
	virtual	WHDirBase *		OpenDir(const char *cszDName)				= 0;	// 如果是打开当前目录一定要用"."，不能什么都不写
	virtual	int				MakeDir(const char *cszDName)				= 0;	// 如果目录存在就直接返回，否则创建目录
	virtual	int				SureMakeDirForFile(const char *cszFName)	= 0;	// 保证文件需要的目录存在
	virtual	bool			IsPathExist(const char *__path)				= 0;	// 判断路径是否存在
	virtual	bool			IsPathDir(const char *__path)				= 0;	// 判断路径是否是目录
	struct	PATHINFO_T
	{
		enum
		{
			TYPE_DIR	= 0x01,										// 是目录
			TYPE_SYS	= 0x02,										// 是系统文件
			TYPE_HIDE	= 0x04,										// 是隐藏文件
			TYPE_RDOL	= 0x08,										// 是只读文件
		};
		unsigned char	nType;										// 节点类型
		time_t			nMTime;										// 修改时间
		union	UN_T
		{
			struct	FILE_T
			{
				size_t	nFSize;										// 文件长度
			}			file;
		}				un;
	};
	virtual	int				GetPathInfo(const char *cszPath, PATHINFO_T *pInfo)
																	= 0;
																	// 获得路径代表的文件或目录的信息
	virtual int				DelFile(const char *cszPath)			= 0;	
																	// 删除文件
	virtual int				DelDir(const char *cszPath)				= 0;	
																	// 删除空目录

	// 判断打开模式否是只读的
	static bool	IsReadOnlyMode(OP_MODE_T nOPMode)
	{
		return	(nOPMode == OP_MODE_BIN_READONLY)
			||  (nOPMode == OP_MODE_BIN_RAWREAD)
			;
	}
};
// 创建一个普通文件系统的文件管理器
whfileman *	whfileman_Cmn_Create();

struct	EXTINFO_UNIT_T
{
	const char	*cszBegin;
	int			len;
};

////////////////////////////////////////////////////////////////////
// 下面跟目录相关的内容实现在whdir.cpp中
////////////////////////////////////////////////////////////////////
// 利用whfileman实现的目录浏览功能（WFM是WithFileMan的意思）
class	WHDirSearchActionWFM
{
public:
	WHDirSearchActionWFM(whfileman *pFM)
		: m_pFM(pFM)
		, m_pbStop(NULL), m_nRootPathLen(0), m_nPathLevel(0)
		, m_bShouldNotGoInThisDir(false)
	{
	}
	virtual ~WHDirSearchActionWFM()	{}
	virtual void	SelfDestroy()	{delete this;}
	// szPath最后不能有"/"或"\"
	int		DoSearch(const char *szPath, bool bRecur);
	inline void	SetStopSearchVar(bool *pbStop)
	{
		m_pbStop	= pbStop;
	}
	inline void	SetFileMan(whfileman *pFM)
	{
		m_pFM	= pFM;
	}
	// 设置满足条件就读取的过滤
	void	SetFilter(const char *cszFilter);
	// 设置满足条件就不读取的过滤
	void	SetBadFilter(const char *cszFilter);
	// 添加一个不读取的文件(注意，文件是大小写敏感的)
	void	AddSkipFile(const char *cszFile);
protected:
	// 一次搜索动作，所有的搜索由一系列递归的搜索动作组成
	int		DoOneDir(const char *szDir, bool bRecur);
	// 获得全路径名
	const char *	GetFullPath(const char *szPath, char *szFullPath);
	// 使用完这个函数后可以用GetLastFullPath得到同样的返回
	inline const char *	GetFullPath(const char *szPath)
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
protected:
	// 文件管理器
	whfileman	*m_pFM;
	// 是否停止浏览（指向表示停止浏览的变量）
	bool	*m_pbStop;
	// 初始路径的字符长度
	int		m_nRootPathLen;
	// 存放当前正在工作的路径
	char	m_szCurPath[WHDIR_MAX_NAME];
	// 用于临时存放全路径名
	char	m_szFullPath[WHDIR_MAX_NAME];
	// 当前的目录深度
	int		m_nPathLevel;
	// 在ActionOnDir中设置，如果设为真，则不用继续进入这个目录
	bool	m_bShouldNotGoInThisDir;
	// 后缀匹配模式
	// 要的
	whvector<EXTINFO_UNIT_T>	m_vectGoodFilter;
	// 不要的
	whvector<EXTINFO_UNIT_T>	m_vectBadFilter;
	// 用于跳过的文件列表
	whstrinvectorhashset		m_setFileToSkip;
private:
	// 这两个函数传入的是最后一段路径名，即相对父目录的路径名，且最后不会有"/"或"\"
	// 函数内部可以通过GetFullPath()得到全路径名
	// 这两个函数的任何一个返回-1，整个搜索都会中止
	// 对文件的操作
	virtual int		ActionOnFile(const char *szFile)	= 0;
	// 对目录的操作(这个是在打开该目录之前做的)
	virtual int		ActionOnDir(const char *szDir)		{return	0;}
	// 一个目录搜索完毕
	// 下面两个函数传入的是（全路径），且最后不会有"/"或"\"
	// 进入目录
	virtual int		EnterDir(const char *szDir)			{return 0;}
	// 从目录中退出（此时目录已经关闭，如果要删除该可以在这里）
	virtual int		LeaveDir(const char *szDir)			{return 0;}
};

// 统计一个目录中的文件数量、总字节数、目录数量等信息
struct	DIR_STAT_T
{
	int	nTotalFile;
	int	nTotalFileByte;
	int	nTotalDir;
	struct	notify			// 对外通知接口
	{
		virtual	~notify()	{};
		virtual	void		SelfDestroy()							= 0;
		virtual	int			cb_DStat_File(const char *cszFName)		= 0;
		virtual	int			cb_DStat_Dir(const char *cszDName)		= 0;
		virtual	int			cb_DStat_Over()							{return 0;}
		static notify *		CreateCmnConsole();
	};
	notify	*pNotify;
	bool	*pbShouldStop;

	DIR_STAT_T();
	void	reset();		// 用于重新开始统计
};
// 如果bQuick为false则真正打开所有文件来获取信息，否则之用文件系统的GetPathInfo函数来获取信息
int	WHDirSearchGetStatistics(whfileman *pFM, const char *cszDir, bool bQuick, DIR_STAT_T *pStat);

// 临时把路径字串变成统一的对象
class	whPathStrTransformer
{
protected:
	const char		*m_pcszStr;
	whvector<char>	m_vectStr;
public:
	whPathStrTransformer(const char *szStr)
		: m_pcszStr(NULL)
	{
		SetStr(szStr);
	}
	inline const char * GetPtr() const
	{
		return	m_pcszStr;
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
protected:
	// 设置字串
	void	SetStr(const char *szStr);
};
#define WHPATHSTRTRANSFORMER	(const char *)whPathStrTransformer

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILE_BASE_H__
