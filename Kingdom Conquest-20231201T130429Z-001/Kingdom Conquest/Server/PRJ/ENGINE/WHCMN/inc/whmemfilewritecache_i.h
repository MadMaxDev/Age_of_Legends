// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache_i.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于内存文件的文件写盘/读盘提速功能
//              : 真正的写盘启用另外一个线程完成
//              : 任何一个文件只要存在于Cache中，则要么在SaveList要么在CanKickList
// CreationDate : 2005-02-05
// ChangeLOG    : 

#ifndef	__WHMEMFILEWRITECACHE_I_H__
#define	__WHMEMFILEWRITECACHE_I_H__

#include "whmemfilewritecache.h"
#include "../inc/whmemfile.h"
#include "../inc/whthread.h"
#include "../inc/whqueue.h"
#include "../inc/whshm.h"
#include "../inc/whgener.h"

namespace n_whcmn
{

class	whmemfilewritecache_i	: public whmemfilewritecache
{
// 为接口实现的方法
public:
	whmemfilewritecache_i();
	virtual ~whmemfilewritecache_i();
	virtual int	Init(INFO_T *pInfo);
	virtual int	Release();
	virtual int	Tick();
	virtual int	LoadFile(const char *cszFileName, whvector<char> &vectData);
	virtual int	SaveFile(const char *cszFileName, const void *pData, int nSize);
	virtual whfile *	OpenFileRW(const char *cszFileName);
	virtual int	DeleteFile(const char *cszFileName);
	virtual int	DeleteDir(const char *cszDir);
	inline int	SaveFile(const char *cszFileName, whvector<char> &vectData)
	{
		return	SaveFile(cszFileName, vectData.getbuf(), vectData.size());
	}
	// 用于调试
	// 设置写盘时多sleep这么长时间(用于模拟硬盘忙的情况)
	virtual void	SetSaveSleep(int nSleep)
	{
		m_nSaveSleep	= nSleep;
	}
	virtual int		GetStatInfo(STATINFO_T *pInfo);

	virtual int		ExportCacheToText(const char *cszTextFile);

	virtual int		DeleteMatchedFilesInCache(const char *cszFmt);

	// 设置停止继续向Cache中添加文件(不过旧文件依然存在)
	virtual void	SetNotExpand(bool bSet)
	{
		m_bNotExpand	= bSet;
	}
	virtual bool	GetNotExpand() const
	{
		return	m_bNotExpand;
	}
	virtual int		Clear();
// 自己用的成员和结构定义
private:
	class	autoRWFile	: public	whfile_i_byvector
	{
	private:
		whmemfilewritecache_i	*m_pHost;
		char					m_szFileName[WH_MAX_PATH];
		whvector<char>			m_vectFileData;
	public:
		autoRWFile(whmemfilewritecache_i *pHost, const char *pszFileName)
		: m_pHost(pHost)
		{
			strcpy(m_szFileName, pszFileName);
		}
		~autoRWFile()
		{
			CloseFile();
		}
		int	LoadFile()
		{
			m_vectFileData.clear();
			if( m_pHost->LoadFile(m_szFileName, m_vectFileData)<0 )
			{
				// 虽然没有load上来，但是可以算作文件不存在，长度为0
				// !!!!记得不要return啊!!!!
			}
			SetVector(m_vectFileData, false);
			return	0;
		}
		int	CloseFile()
		{
			return	m_pHost->SaveFile(m_szFileName, m_vectFileData);
		}
	};
	// 这两个是给线程的存盘指令数据中文件名和数据部分的长度描述变量的类型
	typedef	short		namelen_t;
	typedef	int		buflen_t;
	enum
	{
		MEMHDR_IDX_SAVELIST_HEAD		= 0,
		MEMHDR_IDX_SAVELIST_TAIL		= 1,
		MEMHDR_IDX_CANKICKLIST_HEAD		= 2,
		MEMHDR_IDX_CANKICKLIST_TAIL		= 3,

		MEMHDR_IDX_TOTAL				= 4,
	};
	enum
	{
		WHATLIST_SAVE					= 0,
		WHATLIST_CANKICK				= 1,
	};
	#pragma pack(1)
	typedef	int		cmd_t;
	enum
	{
		CMD_LOG			= 0,			// 记录一条日志
										// 使用CMD_T
										// 数据部分为日志字串，有0结尾
		CMD_SAVE		= 1,			// 存储文件的请求
										// 使用CMD_T
										// 数据部分为变长名字+变长数据
	};
	struct	CMD_T
	{
		cmd_t		nCmd;				// 指令CMD_XXX
		char		data[1];			// 数据部分
	};
	#pragma pack()
private:
	// 配置信息
	INFO_T				m_info;
	// 存盘线程的tid
	whtid_t				m_tidSaver;
	// 线程是否应该停止
	bool				m_bStopSaver;
	// 内存文件管理器
	whmemfilemanWithName		m_fman;
	// 日志记录器
	whlogwriter			m_log;
	// 共享内存
	whshm				*m_pSHM;
	// 主对象向线程发送指令的队列
	whsmpqueueWL		m_queueCmdToThread;
	// 线程中使用的指令缓冲区(它的长度是平均文件长度的4倍即可)
	whvector<char>		m_vectCmdBuf4Thread;
	// 主线程线程中使用的指令缓冲区(它的长度同上)
	whvector<char>		m_vectCmdBuf4Main;
	// 主线程线程中使用的临时存储文件内容的缓冲区(它的长度同上)
	whvector<char>		m_vectFileBuf4Main;
	// 线程向主对象发送指令的队列(比如发送写文件失败的日志)
	whsmpqueueWL		m_queueCmdFromThread;
	// 用于每个tick生成需要存储的文件个数
	whgener				m_generSave;
	// 
	whinterval			m_itv;			// 用于获得每个tick的时间间隔
	whgener				m_generRealSave;// 真实存盘速率
	// 用于调试
	int					m_nSaveSleep;
	// 是否停止扩展cache，不过旧的文件依然存在
	bool				m_bNotExpand;
	// 用于统计内存最大占用
	int					m_nSaveListLenMax, m_nCmdNumInQueueMax;
	int					m_nCurFragMax, m_nCurFileMax;
// 自己用的方法
public:
	// 存盘线程的真正部分
	void *	SaverThreadProc();
private:
	int		Init_SHM();
	int		Init_SaverThread();
	int		Release_SHM();
	int		Release_SaverThread();
	// 这个在Init继承内存以及Release存储文件时都会用到
	int		_RelesaeOldFilesFromSHM();
	// 清除所有队列(Save、CanKick等)
	int		_ClearAllList();
	// 存储一个文件
	// 这个在Saver线程运行的时候主线程和Saver都可用调用。在Saver线程停止的时候可以由主线程调用清除文件。
	int		_SaveFileToDisk(const char *cszFileName, const void *pData, size_t nSize);
	// 这个应该只能由主线程调用
	int		_SaveFileByIdx(int nIdx, bool bLogIt);
	// SaverThreadProc使用的子功能
	int		_SaverThreadProc_SAVE(CMD_T *pCmd, size_t nSize);
	// 下面两个读写函数不能处理错误情况，所以上层保证缓冲数据一定是能读出的，不能发生头部长度部分比实际长的现象。
	// 文件名和数据存入缓冲并返回总长度
	size_t	_WriteFileNameAndDataToBuffer(void *pBuf, const char *cszFName, const void *pData, size_t nDSize);
	// 从缓冲中取出文件名和数据部分头指针
	int		_ReadFileNameAndDataFromBuffer(const void *pBuf, const char *&pcszFName, const void *&pData, size_t &nDSize);
	// 每个Tick内对文件的真正存盘部分
	int		Tick_RealSave();
	// 踢出最老若干的文件
	int		KickOldFiles();
	// 根据idx删除文件
	int		DeleteFileByIdx(int nIdx);

	// 共享内存头里面的附加数据
	inline int &	_nMemInfoUnit(int nIdx)
	{
		return	m_fman.GetMemInfo()->nExtInfo[nIdx];
	}
	// 存盘队列的头尾
	// 每个文件在Save指令后都会放入这个队列，成功加入线程指令队列后从这个队列中删除
	inline int &	nSaveListHead()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_SAVELIST_HEAD);
	}
	inline int &	nSaveListTail()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_SAVELIST_TAIL);
	}
	// 可踢除队列的头尾
	// 每个文件在成功Save后都会放入这个队列，放入Save之前从这里删除
	inline int &	nCanKickListHead()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_CANKICKLIST_HEAD);
	}
	inline int &	nCanKickListTail()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_CANKICKLIST_TAIL);
	}

	// 这是下面各个Add/Remove的内部实现
	// 加入队列尾(这个保证加入表的不能继续加入。但因为有可能是继承来的的内存，所以上层每次分配之后要clear一下)
	int	_List_AddToTail(int &nHead, int &nTail, int nIdx, int nWhatList);
	// 从队列中删除一项(这个保证了删除过的不能继续删除)
	int	_List_Remove(int &nHead, int &nTail, int nIdx, int nWhatList);
	// 从自己某种类型的的队列中移除
	inline int	_List_Remove(int nIdx, int nWhatList)
	{
		if( nWhatList<0 )
		{
			// 不在任何队列中
			return	0;
		}
		int	nListIdx	= nWhatList*2;
		return	_List_Remove(_nMemInfoUnit(nListIdx), _nMemInfoUnit(nListIdx+1), nIdx, nWhatList);
	}
	inline int	_List_Remove(int nIdx)
	{
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		assert(pFName);
		return	_List_Remove(nIdx, pFName->nInWhatList);
	}
	//
	inline int	SaveList_AddToTail(int nIdx)
	{
		return	_List_AddToTail(nSaveListHead(), nSaveListTail(), nIdx, WHATLIST_SAVE);
	}
	inline int	SaveList_Remove(int nIdx)
	{
		return	_List_Remove(nSaveListHead(), nSaveListTail(), nIdx, WHATLIST_SAVE);
	}
	inline int	CanKickList_AddToTail(int nIdx)
	{
		return	_List_AddToTail(nCanKickListHead(), nCanKickListTail(), nIdx, WHATLIST_CANKICK);
	}
	inline int	CanKickList_Remove(int nIdx)
	{
		return	_List_Remove(nCanKickListHead(), nCanKickListTail(), nIdx, WHATLIST_CANKICK);
	}
	// 返回存盘列表的文件个数
	int		_GetSaveListCount();
	// 导出一个文件到文本文件
	int		_ExportOneFileToText(FILE *fp, int nIdx);
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEMFILEWRITECACHE_I_H__
