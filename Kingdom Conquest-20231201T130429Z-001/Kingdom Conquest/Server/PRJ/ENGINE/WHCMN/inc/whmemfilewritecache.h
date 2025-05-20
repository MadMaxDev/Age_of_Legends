// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于内存文件的文件写盘/读盘提速功能
//              : 真正的写盘启用另外一个线程完成
//              : 我们需要预留足够大的空间，需要保证文件数达到上限时，文件占用总空间不能达到上限!!!! 这是非常重要的。
//              ：为了同一个文件不重复命中，请一定按照全路径名写文件，并且全部使用"/"
// CreationDate : 2005-02-05
// ChangeLOG    : 

#ifndef	__WHMEMFILEWRITECACHE_H__
#define	__WHMEMFILEWRITECACHE_H__

#include "whlog.h"

namespace n_whcmn
{

class	whmemfilewritecache
{
public:
	static whmemfilewritecache *	Create();
	virtual ~whmemfilewritecache()	{}
public:
	struct	INFO_T
	{
		whlogwriter::INFO_T	logInfo;			// 日志信息
		int		nSHMKEY;						// 共享内存的键值
		int		nFragSize;						// 每个文件块的尺寸
		int		nTotalFrag;						// 总块数(它和nFragSize决定了所有文件的总空间数)
		int		nMaxFile;						// 最多的文件数量
		int		nCmdToThreadBufSize;			// 向线程提交的命令队列的长度(这个要大一些，因为文件内容回直接放入这个队列中)
		int		nCmdFromThreadBufSize;			// 从线程返回的命令队列的长度
		int		nMaxFileToSavePerSec;			// 每秒最多存储的文件个数
		int		nTimeToStay;					// 文件最多在cache中存在多长时间(秒)
		int		nWaitThreadEndTime;				// 等待线程结束的时间
		int		nThreadPriority;				// 线程级别
		int		nMaxCmdInQueue;					// 给线程的队列中最多可以容纳的指令个数
		bool	bCacheReadFile;					// 是否缓冲只读过的文件
		INFO_T()
		: nSHMKEY(2005)
		, nFragSize(1000)
		, nTotalFrag(5000)
		, nMaxFile(100)
		, nCmdToThreadBufSize(50000*20)			// 平时在缓冲区内的文件数量不应该太多
		, nCmdFromThreadBufSize(100000)			// 从线程应该不会返回很多指令的
		, nMaxFileToSavePerSec(100)				// 每秒100个文件啦
		, nTimeToStay(12*3600)					// 默认等12个小时吧
		, nWaitThreadEndTime(30000)				// 等30秒足够了吧
		, nThreadPriority(-1)					// Saver线程使用默认级别
		, nMaxCmdInQueue(20)
		, bCacheReadFile(false)
		{
			logInfo.nProp	= whlogwriter::INFO_T::PROP_SOHU_DEFAULT;
			strcpy(logInfo.szLogFilePrefix, "LOG/mmfwc");
												// mmfwc means "Mem File Write Cache"
		}
	};
	// 统计信息
	struct	STATINFO_T
	{
		int	nSaveListLen;						// 准备存盘的队列中元素的个数
		int	nCmdNumInQueue;						// 当前线程队列中的指令的总数
		int	nCurFragNum;						// 当前占用的文件块数量
		int	nCurFileNum;						// 当前的文件总数
		// 上面各个的最大值
		int	nSaveListLenMax;
		int	nCmdNumInQueueMax;
		int	nCurFragMax;
		int	nCurFileMax;
	};
public:
	// 初始化(包括共享内存部分，包括启动存储线程)
	virtual int	Init(INFO_T *pInfo)				= 0;
	// 终结(如果共享内存中还有文件则存盘到相应的目录下，如果存盘失败则共享内存不删除)
	// 在删除对象前必须手动调用Release
	virtual int	Release()						= 0;
	// 逻辑运行(比如以一定的速率存盘等等)
	virtual int	Tick()							= 0;
	// 上层期望载入一个文件
	virtual int	LoadFile(const char *cszFileName, whvector<char> &vectData)
												= 0;
	int	LoadFile(const char *cszFileName, whvector<char> &vectDataTmp, void *pData, size_t *pnSize)
	{
		int	rst	= LoadFile(cszFileName, vectDataTmp);
		if( rst<0 )
		{
			return	-1;
		}
		if( *pnSize<vectDataTmp.size() )
		{
			// 外界给的缓冲尺寸不够，只能读出有限的长度
			vectDataTmp.resize(*pnSize);
		}
		*pnSize	= vectDataTmp.size();
		memcpy(pData, vectDataTmp.getbuf(), *pnSize);
		return	0;
	}
	// 上层期望存储一个文件
	virtual int	SaveFile(const char *cszFileName, const void *pData, int nSize)
												= 0;
	inline int	SaveFile(const char *cszFileName, whvector<char> &vectData)
	{
		return	SaveFile(cszFileName, vectData.getbuf(), vectData.size());
	}
	// 以读写方式打开文件(如果文件原来不存在则创建之)
	// 修改完之后自己通过delete删除文件对象存盘
	virtual whfile *	OpenFileRW(const char *cszFileName)
												= 0;
	// 删除文件(注意文件被打开的时候不要删除，即OpenFileRW打开文件使用完毕之前不能删除它)
	virtual int	DeleteFile(const char *cszFileName)
												= 0;
	// 删除目录(根据目录中实际的文件删除内存中的文件，所以外界最好不要手动删除目录)
	// 不过注意，这个删除不支持次级目录的
	virtual int	DeleteDir(const char *cszDir)	= 0;

	// 用于调试
	// 设置写盘时多sleep这么长时间(用于模拟硬盘忙的情况)
	virtual void	SetSaveSleep(int nSleep)	= 0;

	// 获得统计信息
	virtual int		GetStatInfo(STATINFO_T *pInfo)
												= 0;
	// 把cache的文件内容导出到文本文件
	virtual int		ExportCacheToText(const char *cszTextFile)
												= 0;
	// 删除cache匹配的文件（不删除磁盘上的）
	// 返回删除的文件总数
	virtual int		DeleteMatchedFilesInCache(const char *cszFmt)
												= 0;
	// 设置停止继续向Cache中添加文件(不过旧文件依然存在)
	virtual void	SetNotExpand(bool bSet)		= 0;
	// 获得NotExpand的设定值
	virtual bool	GetNotExpand() const		= 0;
	// 清除所有文件
	virtual int		Clear()						= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEMFILEWRITECACHE_H__
