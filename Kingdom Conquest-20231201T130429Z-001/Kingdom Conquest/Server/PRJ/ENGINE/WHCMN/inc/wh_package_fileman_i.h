// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman_i.h
// Creator     : Wei Hua (魏华)
// Comment     : 基于包的通用文件系统
// CreationDate: 2006-08-09
// ChangeLOG   : 2007-01-31 给SureOpenPackage和LoadPackage增加了const char *cszExt参数
//             : 2007-02-12 SetPackageOpenMode改为后面set的可以顶掉前面的设置
//             : 2007-03-13 增加了在Debug状态下如果遇到文件名中有大写字符且打不开文件的情况，就报assert。
//             : LoadPackage中先判断了对象是否已经被加载
//             : 2007-06-19 增加了让外界获取m_info的方法，用于主要给蒋黎动态调整文件访问模式
//			   : 2008-06-06	增加一个接口，用于直接读取原始的压缩数据（by 钟文杰）
//			   : 2008-07-18 增加一个接口，用于改变m_bSearchTailFile的值（by 钟文杰）

#ifndef	__WH_PACKAGE_FILEMAN_I_H__
#define __WH_PACKAGE_FILEMAN_I_H__

#include "wh_package_base.h"
#include "wh_package_fileman.h"
#include "whfile.h"

namespace n_whcmn
{

class	wh_package_fileman	: public whfileman
{
	// 为接口实现的
public:
	virtual	~wh_package_fileman();
	virtual	void	SelfDestroy();
	virtual	whfile *		Open(const char *cszFName, OP_MODE_T mode);
	virtual	WHDirBase *		OpenDir(const char *cszDName);
	virtual	int				MakeDir(const char *cszDName);
	virtual	int				SureMakeDirForFile(const char *cszFName);
	virtual	bool			IsPathExist(const char *__path);
	virtual	bool			IsPathDir(const char *__path);
	virtual	int				GetPathInfo(const char *cszPath, PATHINFO_T *pInfo);
	virtual	int				DelFile(const char *cszPath);
	virtual	int				DelDir(const char *cszPath);
	// 自己用的
	// 返回包中文件的原始数据（压缩后的数据），钟文杰添加
	whfile *		OpenRawFile(const char *cszFName, OP_MODE_T mode);

protected:
	whfileman_package_INFO_T	m_info;
	typedef	whhash<whstr4hash, wh_package_base*, whcmnallocationobj, _whstr_hashfunc>		PATH2PACKAGE_T;
	typedef	whhash<whstr4hash, whfileman::OP_MODE_T, whcmnallocationobj, _whstr_hashfunc>	PATH2OPMODE_T;
	PATH2PACKAGE_T				m_mapPath2Package;
	PATH2OPMODE_T				m_mapPath2OPMode;
	// 如果这个为真则说明上个需要打开的文件路径中有包，所以不应该继续通过硬盘路径打开了
	bool						m_bLastPathIsAboutPck;
	// 可以把包当成目录的dir对象
	class	DirKnowPackage	: public WHDirBase
	{
	private:
		whfileman	*m_pFM;
		WHDirBase	*m_pBaseDir;
		bool		m_bAutoDeleteBase;
		bool		m_bSearchTailFile;								// 是否把尾部文件当成包文件
	public:
		DirKnowPackage(whfileman *pFM, WHDirBase *pBaseDir, bool bAutoDeleteBase, bool bSearchTailFile)
			: m_pFM(pFM), m_pBaseDir(pBaseDir), m_bAutoDeleteBase(bAutoDeleteBase), m_bSearchTailFile(bSearchTailFile)
		{
		}
		~DirKnowPackage()
		{
			if( m_bAutoDeleteBase )
			{
				WHSafeSelfDestroy(m_pBaseDir);
			}
		}
		virtual	void	SelfDestroy()	{delete this;}
		virtual	void	Rewind()		{m_pBaseDir->Rewind();}
		virtual const char *	GetDir() const	{return m_pBaseDir->GetDir();}
		virtual	ENTRYINFO_T *	Read();
	};
public:
	inline whfileman_package_INFO_T	&	GetInitInfo()
	{
		return	m_info;
	}

	inline void setSearchTailFile(bool bSearchTailFile)
	{
		m_info.bSearchTailFile = bSearchTailFile;
	}
public:
	wh_package_fileman(whfileman_package_INFO_T *pInfo);
	int		SureOpenPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode=whfileman::OP_MODE_UNDEFINED);
																	// 确保打开（如果原来没有就创建）包
																	// 其中cszPckPath是不带包后缀的；
																	// nOPMode如果是OP_MODE_UNDEFINED则使用默认的打开方式，
																	// 如果不是则使用这种方式打开（比如在默认都是只读的情况下用写方式打开某些包）
																	// 注意如果是OP_MODE_CREATE则会永远创建新的空包
	wh_package_base *	LoadPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode);
																	// 以固定模式载入指定包文件，并做好名字映射
																	// 其中cszPckPath是不带包后缀的
	int		UnloadPackage(const char *cszPckPath);
	int		Close();												// 在析构中会调用这个
	int		SetPackageOpenMode(const char *cszPCKName, whfileman::OP_MODE_T nOPMode);
																	// 设置某个特殊包的打开模式（如果设置相同的名字，后面的会把前面的抵消掉）
	int		FlushAllPackage();										// 把所有的包flush一下
private:
	// 只在硬盘上查找打开
	whfile *		Open_On_Disk(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		OpenDir_On_Disk_Only(const char *cszDName);
	WHDirBase *		OpenDir_On_Disk(const char *cszDName);
	int				CreateDir_On_Disk(const char *cszDName);
	int				GetPathInfo_On_Disk(const char *cszPath, PATHINFO_T *pInfo);
	int				DelFile_On_Disk(const char *cszPath);
	int				DelDir_On_Disk(const char *cszPath);
	// 只在包中查找打开
	whfile *		Open_In_Pck(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		OpenDir_In_Pck(const char *cszDName);
	// 返回包中文件的原始数据（压缩后的数据），钟文杰添加
	whfile *		OpenRaw_In_Pck(const char *cszFName, OP_MODE_T mode);
	enum
	{
		CREATEDIR_IN_PCK_RST_OK					= 0,
		CREATEDIR_IN_PCK_RST_ERR_NOPCK			= -1,				// 没有包
		CREATEDIR_IN_PCK_RST_ERR_OTHER			= -10,				// 其他错误（会加上这个）
	};
	int				CreateDir_In_Pck(const char *cszDName);
	int				GetPathInfo_In_Pck(const char *cszPath, PATHINFO_T *pInfo);
	int				DelFile_In_Pck(const char *cszPath);
	int				DelDir_In_Pck(const char *cszPath);
	// 查找路径上的包（如果找到，*pnOffset中的是未完成部分相对包的路径）
	// bIsFile表示cszPath表示的是一个文件而不是路径，这样的话可以先弹出最后的文件名
	wh_package_base *	FindPackageOnPath(const char *cszPath, bool bIsFile, int *pnOffset);
	// Open和OpenDir的内部实现部分
	whfile *		_Open(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		_OpenDir(const char *cszDName);
public:
	static const char	CSZPCKEXT[];								// 默认的包文件后缀
	static const int	CNPCKEXTLEN;								// 默认的包文件后缀长度
	static const char	*CSZPCKEXT_All[];							// 所有可能的包文件后缀
	static const int	CNPCKEXT_DFT_IDX;							// 默认后缀在CSZPCKEXT_All中的序号
	static const char *	IsPathPackage(whfileman *pFM, const char *cszPath);	// 判断一个路径是否是包（如果是包，则返回后缀）
};

class	wh_package_fileman_Easy	: public wh_package_fileman
{
private:
	ICryptFactory	*m_pICryptFactory;
public:
	wh_package_fileman_Easy(whfileman_package_INFO_T *pInfo, ICryptFactory *pICryptFactory)
		: wh_package_fileman(pInfo)
		, m_pICryptFactory(pICryptFactory)
	{
	}
	virtual	~wh_package_fileman_Easy()
	{
		// 抢先调用父类的Close
		wh_package_fileman::Close();
		// 然后释放自己的info内的对象
		WHSafeSelfDestroy(m_info.pWHCompress);
		WHSafeSelfDestroy(m_info.pEncryptor);
		WHSafeSelfDestroy(m_info.pDecryptor);
		WHSafeSelfDestroy(m_pICryptFactory);
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_FILEMAN_I_H__
