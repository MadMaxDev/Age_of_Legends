// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_util.h
// Creator     : Wei Hua (魏华)
// Comment     : 包的相关操作
// CreationDate: 2006-08-14
//             : 2007-01-24 增加了CP_INFO_T中的文件后缀。
//             : 增加了对于整个目录的单文件压缩和解压功能。
//             : 2007-02-01 增加了cb_MSGOUT回调函数
//             : 2007-08-08 增加了apszSkipFile参数用于跳过特定的文件
//             : 2007-08-09 增加了pszBadFileFilter参数用于跳过和通配符匹配的文件
//			   : 2008-08-22	为手动更新包增加版本验证机制，by 钟文杰

#ifndef	__WH_PACKAGE_UTIL_H__
#define	__WH_PACKAGE_UTIL_H__

#include "whfile_base.h"
#include "whcmn_def.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "wh_package_def.h"

namespace n_whcmn
{

// 工具集合（接口）
struct	wh_package_util
{
	// 一些公用定义
	enum
	{
		PROP_SKIPSYSTEMFILE		= 0x01,								// 跳过系统文件不管
		PROP_SKIPRDONLYFILE		= 0x02,								// 跳过只读文件不管
		PROP_KEEPFILETIME		= 0x04,								// 保留旧文件时间
	};
	enum
	{
		GETSRCDIRSTATMODE_NO	= 0x00,								// 不获取目录信息
		GETSRCDIRSTATMODE_QUICK	= 0x01,								// 快速获取
		GETSRCDIRSTATMODE_SLOW	= 0x02,								// 慢速获取（需要真的打开文件）
	};
	// notify负责下面打包和解包的通知（显示）工作
	struct	notify
	{
	public:
		virtual	~notify()		{}
		virtual	void	SelfDestroy()								= 0;
		// 下面各个函数的返回
		enum
		{
			RST_OK_CONTINUE				= 0,						// 成功，可以继续查找下面的
			RST_ERR_ABORT				= -1,						// 有错误，停止后面的工作
		};
		virtual	int		cb_DIR_BEGIN(const char *cszDirName)		= 0;
		virtual	int		cb_DIR_END(const char *cszDirName)			= 0;
		virtual	int		cb_FILE_BEGIN(const char *cszFrom, const char *cszTo, int nFileSize)
																	= 0;
		virtual	int		cb_FILE_END()								= 0;
		virtual	int		cb_FILE_PROCESS(int nCurPos)				= 0;
																	// nCurPos是当前拷贝的字节数
		virtual	int		cb_FILE_SKIP(const char *cszFName)			= 0;
																	// 跳过某个文件
		enum
		{
			ERRCODE_OK					= 0,						// 没错:)
			ERRCODE_F_OPEN				= -1,						// 打开文件错误
			ERRCODE_F_CREATE			= -2,						// 创建文件错误
			ERRCODE_F_READ				= -3,						// 读取文件错误
			ERRCODE_F_WRITE				= -4,						// 写入文件错误
			ERRCODE_COUNTTOTAL_ERR		= -5,						// 统计总尺寸过程出错
			ERRCODE_NO_DISKSPACE		= -100,						// 写入文件错误
			ERRCODE_OTHER				= -1000,					// 其他未知错误，具体见错误字串
		};
		virtual	int		cb_ERROR(int nErrCode, const char *cszErrString)
																	= 0;
																	// 处理错误
		virtual int		cb_MSGOUT(int nMsgCode, const char *cszMsg)	= 0;
																	// 输出消息
		virtual int		cb_SRCDIRSTAT(const DIR_STAT_T *pInfo)		{return 0;}
																	// 获取源目录的统计信息
	public:
		static notify	*CreateCmnConsole();						// 创建一个普通的基于Console的通知器（使用者也可以自行派生notify）
	};
	// 下面两个拷贝公共的参数结构
	struct	CP_INFO_T
	{
		whfileman		*pFM;										// 底层的文件管理器（包文件的操作需要通过它进行）
		const char		*cszSrc;									// 源目录
		const char		*cszDst;									// 目标目录
		int				nCutFileTail;								// 如果非0则把目标文件结尾删除这么多字节
		const char		*cszAppendExt;								// 如果非空则在目标文件后面加上这个后缀
		const char		*szPass;
		int				nPassLen;
		int				nBlockSize;									// 基本文件块的大小（越大对于小文件浪费越大，越小对于大文件浪费越大）
		// 这个可必须要填写的，否则没法创建包文件
		WHCompress					*pWHCompress;					// 压缩器
		ICryptFactory::ICryptor		*pEncryptor;					// 加密器
		ICryptFactory::ICryptor		*pDecryptor;					// 解密器（注意：加密器和解密器可不能一样）
		notify			*pNotify;									// 用于对外显示的对象
		DIR_STAT_T::notify			*pStatNotify;					// 统计目录时的对外显示对象
		int				nNotifyStepBytes;							// 每拷贝这么多字节就通知一下
		unsigned char	nProp;										// 和dircp::INFO_T中的PROP_XXX定义一致
		unsigned char	nGetSrcDirStatMode;							// 是否需要获得源目录的统计信息
		bool			*pbShouldStop;								// 外界如果设置了这个内容为真则拷贝过程需要停止
		const char		**apszSkipFile;								// 跳过文件的列表
		const char		*pszBadFileFilter;							// 需要跳过文件的通配符。是".h|.cpp"这样的格式
		const char		*pValidityStr;								// 验证字串

		CP_INFO_T()
		{
			WHMEMSET0THIS();
			nNotifyStepBytes	= 100000;
			nBlockSize			= WHPACKAGE_DFT_BLOCKSIZE;
			nGetSrcDirStatMode	= GETSRCDIRSTATMODE_QUICK;
		}
	};
	// 目录打包
	struct	dir2package
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~dir2package()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
																	// 进行目录打包工作
	
		static dir2package *	Create();							// 创建实例
	};
	// 解包到目录
	struct	package2dir
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~package2dir()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
																	// 进行目录解包工作

		static package2dir *	Create();							// 创建实例
	};
	// 目录的单文件压缩
	struct	sf_compress
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~sf_compress()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
		// 进行目录解包工作

		static sf_compress *	Create();							// 创建实例
	};
	// 目录的单文件解压
	struct	sf_decompress
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~sf_decompress()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
		// 进行目录解包工作

		static sf_decompress *	Create();							// 创建实例
	};
	// 用一个目录更新另外一个目录（这个之所以和拷贝不同是因为可能有一些脚本要执行）
	struct	update
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~update()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
		// 进行目录打包工作

		static update *			Create();							// 创建实例
	};
	// 在两个文件系统间拷贝目录及文件（这个就是上面几个应用的底层实现）
	struct	dircp
	{
		struct	INFO_T
		{
			whfileman		*pFMSrc;								// 文件来自这个系统
			whfileman		*pFMDst;								// 文件拷贝到这个系统
			const char		*cszSrcDir;								// 源目录
			const char		*cszDstDir;								// 目标目录
			int				nCutFileTail;							// 如果非0则把目标文件结尾删除这么多字节
			const char		*cszAppendExt;							// 如果非空则在目标文件后面加上这个后缀
			notify			*pNotify;								// 给上层的通知器
			DIR_STAT_T::notify		*pStatNotify;					// 统计目录时的对外显示对象
			int				nNotifyStepBytes;						// 每拷贝这么多字节就通知一下
			unsigned char	nProp;									// 一些设置
			unsigned char	nGetSrcDirStatMode;						// 是否需要获得源目录的统计信息
			bool			*pbShouldStop;
			const char		**apszSkipFile;							// 跳过文件的列表
			const char		*pszBadFileFilter;						// 需要跳过文件的通配符。是".h|.cpp"这样的格式
			INFO_T()
			{
				WHMEMSET0THIS();
				nNotifyStepBytes	= 100000;
				nGetSrcDirStatMode	= GETSRCDIRSTATMODE_QUICK;
			}
			void	getcmninfofrom(CP_INFO_T *pInfo)
			{
				cszDstDir			= pInfo->cszDst;
				nCutFileTail		= pInfo->nCutFileTail;
				cszAppendExt		= pInfo->cszAppendExt;
				pNotify				= pInfo->pNotify;
				pStatNotify			= pInfo->pStatNotify;
				nNotifyStepBytes	= pInfo->nNotifyStepBytes;
				nProp				= pInfo->nProp;
				nGetSrcDirStatMode	= pInfo->nGetSrcDirStatMode;
				pbShouldStop		= pInfo->pbShouldStop;
				apszSkipFile		= pInfo->apszSkipFile;
				pszBadFileFilter	= pInfo->pszBadFileFilter;
			}
		};
		virtual	~dircp()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// 干活
		static dircp *		Create();								// 创建实例
	};
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_UTIL_H__
