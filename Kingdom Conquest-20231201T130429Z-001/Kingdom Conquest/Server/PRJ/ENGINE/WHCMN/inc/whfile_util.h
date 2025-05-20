// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_util.h
// Creator      : Wei Hua (魏华)
// Comment      : 文件辅助功能
// CreationDate : 2004-09-16
//              : 2006-09-30 增加了一些逻辑层需要的功能
//              : 2007-03-09 增加了whsmppasswd
//              : 2007-03-21 给whsmppasswd增加了密码为空就不检测密码的功能
//              : 2007-04-28 增加了bLowerCaseAccount，以保证帐号必须为小写（也即帐号大小写不敏感）

#ifndef	__WHFILE_UTIL_H__
#define	__WHFILE_UTIL_H__

#include "whfile.h"
#include "whhash.h"
#include "whstring.h"
#include "whdataini.h"
#include "whmd5.h"
#include "whrsa.h"
#include <vector>

namespace n_whcmn
{

// 简单文件缓冲区(只读)
class	whsmpfilecache
{
private:
	#pragma pack(1)
	struct	FILEUNIT_T
	{
		char	szFileName[WH_MAX_PATH];					// 文件名
		size_t	nFileSize;									// 文件数据长度
		char	data[1];									// 文件数据
	};
	#pragma pack()
	whhash<whstrptr4hash, FILEUNIT_T *>	m_mapFileName2Unit;	// 文件名对文件对象得映射
public:
	whsmpfilecache();
	~whsmpfilecache();
	int		LoadFile(const char *cszFile, const char *cszFileInMem=NULL);
															// 从硬盘上载入一个文件(如果cszFileInMem非空则在内存中用这个名字)
	int		UnLoadFile(const char *cszFileInMem);			// 把载入的文件释放
	int		UnLoadAllFiles();								// 释放所有文件
	whfile *	OpenFile(const char *cszFile, bool bTryDisk);
															// 从内存中以读方式打开文件(文件对象使用完毕后上层自行删除即可)
															// bTryDisk为真则如果内存中没有就直接打开硬盘上的文件
	bool	HasFileInMem(const char *cszFile);				// 内存中有这个文件
};

// 用多文件组成的大文本队列
class	whlinelistinmultifile
{
protected:
	char	m_szPrefixFmt[WH_MAX_PATH];						// 文件名模式
	char	m_szCurFile[WH_MAX_PATH];						// 当前正在关注的文件
	whvector<char>	m_vectfiledata;							// 存储文件内容
	int		m_nFileIdx;										// 文件序号(0表示还没有开始读、-1表示已经读到了结尾)
public:
	whlinelistinmultifile();
	~whlinelistinmultifile();
	void	SetPrefixFmt(const char *cszFmt);				// 设置公共头
	void	Reset();										// 重新开始读
	int		SaveFile();										// 将文件的剩余部分存盘
	int		GetRemoveLine(char *szLine, int nSize);			// 读出并删除行(返回读到的长度，包括结尾的\n或\r\n)
															// 如果没有东西可读则返回0
};

// 利用文件来做的简单用户名密码校验系统
class	whsmppasswd
{
public:
	struct	INFO_T
	{
		bool	bPassMD5;									// 内存中是否使用MD5串来保存密码
		bool	bLowerCaseAccount;							// 保证帐号名为小写
		char	szPasswordFile[WH_MAX_PATH];				// 用户名密码文件（以":"来分隔用户名和密码），如果为空字串则表示没有内容
		INFO_T()
			: bPassMD5(true)
			, bLowerCaseAccount(true)
		{
			szPasswordFile[0]	= 0;
		}
	};
	// 这个为了可以通过配置文件自动填充
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
private:
	INFO_T		m_info;										// 配置信息
	time_t		m_nLastModifyTime;							// 密码文件上次修改的时间
	size_t		m_nLastSize;								// 密码文件上次的长度
	whhash<whstr4hash, whstr4hash>	m_mapName2Pass;			// 确切的用户名对密码的映射表
	struct		LIKEUNIT_T
	{
		whstrlike	*pLike;									// 用于对象匹配
		whvector<char>	vectPass;							// 存放密码
		LIKEUNIT_T()
			: pLike(NULL)
		{
		}
		LIKEUNIT_T(int nSize)
			: pLike(NULL)
			, vectPass(nSize)
		{
		}
		~LIKEUNIT_T()
		{
			clear();
		}
		void	clear()
		{
			if( pLike )
			{
				delete	pLike;
				pLike	= NULL;
			}
			vectPass.clear();
		}
	};
	whvector<LIKEUNIT_T *>			m_vectLikeUnitPtr;		// 用于模式匹配的对象的数组
public:
	inline int	GetNum() const								// 获得普通的用户密码映射的个数
	{
		return	m_mapName2Pass.size();
	}
	inline int	GetPatternNum() const						// 获得模式匹配的个数
	{
		return	m_vectLikeUnitPtr.size();
	}
public:
	whsmppasswd();
	~whsmppasswd();
	int			Init(INFO_T *pInfo);
	int			Release();
	int			FreeAll();									// 释放所有的映射
	enum
	{
		CHECK_RST_OK					= 0,				// 验证成功
		CHECK_RST_SYSERR				= -1,				// 系统错误（比如文件打开失败）
		CHECK_RST_USER_NOTFOUND			= -2,				// 用户不存在
		CHECK_RST_BAD_PASS				= -3,				// 密码错误
	};
	int			Check(const char *cszUser, const char *cszPass);
															// 对用户名和密码进行验证
	int			CheckGetPass(const char *cszUser, const char *&pcszPass);
															// 根据用户名获得密码（返回和Check的一致，返回CHECK_RST_OK的时候说明获得了密码）
	enum
	{
		UPDATE_RST_OK					= 0,				// 正常（没有更新）
		UPDATE_RST_NEW					= 1,				// 文件有新内容，更新了
		UPDATE_RST_STAT_ERR				= -1,				// 文件stat操作出错
		UPDATE_RST_OPEN_ERR				= -2,				// 文件打开出错
		UPDATE_RST_READ_ERR				= -3,				// 文件读取出错
		UPDATE_RST_ADD_ERR				= -4,				// 添加用户出错（一般都应该是角色名重复了）
	};
	int			Update();									// 检查文件是否更新了，如果更新了就重新初始化一下
	int			AddUser(const char *cszUser, const char *cszPass);
															// 增加一个用户/密码映射
};

class	whvarstrarr_WF	: public whvarstrarr
{
public:
	// 从文件中初始化
	int		initfromfile(const char *cszFileName);
	// 从基础文件对象中初始化
	int		initfromfile(whfile *file);
};

class	whstrmatch_WF	: public whstrmatch
{
public:
	// 从文件中初始化
	int		initfromfile(const char *cszFileName);
	// 从基础文件对象中初始化
	int		initfromfile(whfile *file);
};

// 获得目录中的文件列表
// szExt是“.c|.cpp|.h”这样的字串，如果只有一种后缀就不用写“|”了（象这样：“.c”），如果不需要过滤则直接写NULL或空串即可。
bool	GetDirFileList(whfileman *pMan, const char *szPath, std::vector< std::string > *pList, const char *szExt=NULL);

// 查找指定文件（如果找到就把全路径填写在szBuf中，返回szBuf。否则返回空）
const char *	GetFile(whfileman *pMan, const char *szPath, const char *szFile, char *szBuf);

// 删除某个目录下的在指定时间之前创建的文件
// szExt定义同前面
bool	DelFileBeforeTime(whfileman *pMan, const char *szPath, const char *szExt, time_t nTime);

// 删除目录或文件（如果是目录则递归删除）
bool	DelPath(whfileman *pMan, const char *szPath);

// 一个简单的文件载入的管理器
class	whsmpfilestore
{
public:
	struct	INFO_T
	{
		whfileman		*pFM;										// 底层的文件管理器
		char			szBasePath[WH_MAX_PATH];					// 文件存储的根目录
		int				nLoadRefreshInterval;						// 文件载入刷新间隔（秒）
		int				nUpdateRefreshInterval;						// 文件更新刷新间隔（秒，这个是从外界更新内部文件的间隔）

		INFO_T()
			: pFM(NULL)
			, nLoadRefreshInterval(30)								// 30秒检查刷一次应该不会太快吧
			, nUpdateRefreshInterval(100)							// 外界因为需要从服务器传来，所以这个请求不易过快
		{
			szBasePath[0]	= 0;
		}
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	struct	FILE_T
	{
		int				nFileID;									// 文件名
		unsigned char	MD5[WHMD5LEN];
		whvector<char>	vectfiledata;								// 存储文件内容
		time_t			nLastFileModifyTime;						// 上次载入时，文件的更改时间（这个用来表明文件是否需要重新载入）
		time_t			nLastLoadTime;								// 上次载入的时间（从磁盘载入）
		time_t			nLastUpdateTime;							// 上次用外界数据更新的时间（外界更新到硬盘）
		int				anExt[4];									// 附加数据
		FILE_T()
			: nFileID(0)
			, nLastFileModifyTime(0)
			, nLastLoadTime(0)
			, nLastUpdateTime(0)
		{
			WHMEMSET0(MD5);
			WHMEMSET0(anExt);
		}
	};
private:
	INFO_T				m_info;
	whhash<int, FILE_T *>	m_mapFileID2Obj;						// 文件ID到文件对象的映射
	char				m_szFNBuf[WH_MAX_PATH];						// 用于临时存放生成的文件名
public:
	whsmpfilestore();
	~whsmpfilestore();
	int		Init(INFO_T *pInfo);
	int		Release();
	// SureLoadFile获得的文件对象不用释放，内部自己会释放
	FILE_T *	SureLoadFile(int nID, bool bMustLoad=false);		// 确保文件被加载（bMustLoad为真表示如果文件存在且从来没有被载入过就必须重新加载一次，如果文件不存在就创建一个空文件）
																	// 客户端应该是需要bMustLoad为真的，因为还需要设置一些标记（比如表示这个文件已经在向服务器的请求过程中）
	FILE_T *	GetFileInfo(int nID);								// 仅仅是获得
	FILE_T *	UpdateFile(int nID, void *pData, int nDSize);		// 更新文件内容（同时也会更新MD5）
	void	clear();												// 清空m_mapFileID2Obj的内容
	bool	CheckIfFileNeedUpdate(FILE_T *pF);						// 判断文件是否需要更新（即上次更新的时间到现在是否过长了）
private:
	const char *	MakeFileName(int nID, char *pszBuf=NULL);
	int		LoadFile(int nID, FILE_T *pF, bool bMustLoad=false);	// 载入文件（只在文件没有改变的时候）。如果bMustLoad为真则表示如果文件不存在就创建一个空文件
};


////////////////////////////////////////////////////////////////////
// 从密文件中获取授权信息
////////////////////////////////////////////////////////////////////
int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, const char *szAuthFile, whvector<char> &vectAuth);
int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth);
int		GetAuthInfo(char *pszRSAPriKeyFileData, int nRSAPriKeyFileSize, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth);
int		GetAuthInfo(char *pszFileData, int nFileSize, WHRSA *pDec, whvector<char> &vectAuth);

}		// EOF namespace n_whcmn

#endif	// __WHFILE_UTIL_H__

