// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于内存文件的文件写盘/读盘提速功能
//              : 真正的写盘启用另外一个线程完成
// CreationDate : 2005-02-06
// ChangeLOG    : 2005-03-14 过去是踢出最老的一个文件，现在修改为踢出很长时间没有修改的旧文件（默认12小时没有修改）。
//              : 2005-03-25 whmemfilewritecache_i::Release_SaverThread()中原来竟然从来没有关闭过线程
//              : 2006-06-06 增加了DeleteMatchedFilesInCache中对cszFmt参数是否合法的判断，否则可能导致出非法错误
//              : 2007-04-27 whmemfilewritecache_i::SaveFile中只要写入结果不等于写入长度就认为出错

#include "../inc/whmemfilewritecache_i.h"
#include "../inc/whdir.h"
#include "../inc/whshm.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whmemfilewritecache_i
////////////////////////////////////////////////////////////////////
whmemfilewritecache_i::whmemfilewritecache_i()
: m_tidSaver(INVALID_TID)
, m_bStopSaver(false)
, m_pSHM(NULL)
, m_nSaveSleep(0)
, m_bNotExpand(false)
, m_nSaveListLenMax(0), m_nCmdNumInQueueMax(0)
, m_nCurFragMax(0), m_nCurFileMax(0)
{
}
whmemfilewritecache_i::~whmemfilewritecache_i()
{
	Release();
}
int	whmemfilewritecache_i::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));

	// 初始化几个缓冲的长度
	// 它们是最大文件长度的4倍
	m_vectCmdBuf4Thread.resize(m_info.nFragSize*m_info.nTotalFrag/m_info.nMaxFile * 4);
	m_vectCmdBuf4Main.resize(m_vectCmdBuf4Thread.size());
	m_vectFileBuf4Main.resize(m_vectCmdBuf4Thread.size());

	// 初始化日志
	if( m_log.Init(&m_info.logInfo)<0 )
	{
		return	-1;
	}

	m_log.WriteFmtLine("**** whmemfilewritecache BEGIN [%s] ****", wh_gettimestr());

	// 初始化（或者继承）共享内存，如果原来有文件则尝试重新存储，如果出错则退出。
	if( Init_SHM()<0 )
	{
		return	-2;
	}

	// 创建存盘线程
	if( Init_SaverThread()<0 )
	{
		return	-3;
	}

	m_itv.reset();
	m_generRealSave.SetGenSpeed((float)m_info.nMaxFileToSavePerSec);
	m_nSaveListLenMax	= 0;
	m_nCmdNumInQueueMax	= 0;
	m_nCurFragMax		= 0;
	m_nCurFragMax		= 0;

	return	0;
}
int	whmemfilewritecache_i::Init_SHM()
{
	// 尝试打开共享内存
	int		nSHMKey		= m_info.nSHMKEY;
	whmemfilemanWithName::INFO_T	info;
	info.pBuf			= NULL;
	info.nMaxFile		= m_info.nMaxFile;
	info.nFragSize		= m_info.nFragSize;
	info.nTotalFrag		= m_info.nTotalFrag;
	size_t	nSizeNeeded	= whmemfilemanWithName::CalcMemSizeNeeded(&info);
	assert(!m_pSHM);
	m_pSHM				= whshm_open(nSHMKey, NULL);
	if( !m_pSHM )
	{
		// 没有就创建之
		m_pSHM			= whshm_create(nSHMKey, nSizeNeeded);
		m_log.WriteFmtLine("Init_SHM SHM create ptr:%p!", m_pSHM);
		if( !m_pSHM )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM can not create SHM!");
			return	-1;
		}
		// 初始化使用共享内存的文件管理器
		info.pBuf	= m_pSHM->GetBuf();
		int	rst		= m_fman.Init(&info);
		if( rst<0 )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM m_fman.Init error:%d", rst);
			// 删除已经申请的SHM
			delete	m_pSHM;
			m_pSHM	= NULL;
			whshm_destroy(m_info.nSHMKEY);
			return	-2;
		}
		// 初始化Save和CanKick队列
		_ClearAllList();
	}
	else
	{
		m_log.WriteFmtLine("Init_SHM SHM already existed ptr:%p! It will be inherited.", m_pSHM);
		// 初始化使用共享内存的文件管理器
		void	*pBuf	= m_pSHM->GetBuf();
		size_t	nSize	= m_pSHM->GetSize();
		// 判断尺寸是否正确
		if( nSize != nSizeNeeded )
		{
			m_log.WriteFmtLine("Init_SHM inherited size:%d != needed size:%d", nSize, nSizeNeeded);
			// 直接退出，不过可不能删除旧内存啊，因为可以用其他的正确的程序来继承
			return	-11;
		}
		// 继承已有内存
		int	rst		= m_fman.Inherit(pBuf);
		if( rst<0 )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM inherit (err:%d)", rst);
			// 断绝和共享内存的关系
			delete	m_pSHM;
			m_pSHM	= NULL;
			return	-12;
		}
		m_log.WriteFmtLine("Init_SHM inherit OK. total file:%d total frag:%d", m_fman.GetFileNum(), m_fman.GetFragNum());
		// 把原来没有存盘的文件存盘，如果存盘出错则不能继续
		if( _RelesaeOldFilesFromSHM()<0 )
		{
			return	-13;
		}
	}

	return	0;
}
static void * my_SaverThreadProc(void *pHost)
{
	return	((whmemfilewritecache_i*)pHost)->SaverThreadProc();
}
int	whmemfilewritecache_i::Init_SaverThread()
{
	if( m_tidSaver != INVALID_TID )
	{
		// 说明已经初始化过了
		return	-1;
	}

	// 初始化和线程交互的指令队列
	if( m_queueCmdToThread.Init(m_info.nCmdToThreadBufSize)<0 )
	{
		return	-2;
	}
	if( m_queueCmdFromThread.Init(m_info.nCmdFromThreadBufSize)<0 )
	{
		return	-3;
	}

	// 启动线程
	if( whthread_create(&m_tidSaver, my_SaverThreadProc, this, m_info.nThreadPriority)<0 )
	{
		return	-4;
	}
	m_log.WriteFmtLine("Saver Thread Started.");
	return	0;
}
int	whmemfilewritecache_i::Release()
{
	// 停止存盘线程。等待其终结。
	Release_SaverThread();
	// 清理共享内存。
	Release_SHM();
	//
	m_log.WriteFmtLine("**** whmemfilewritecache END [%s] ****", wh_gettimestr());
	// 关闭log
	m_log.Release();
	return	0;
}
int	whmemfilewritecache_i::Release_SHM()
{
	// 释放共享内存
	if( m_pSHM )
	{
		// 所有内存中的残留文件全部存盘，存盘成功则删除共享内存
		if( _RelesaeOldFilesFromSHM()<0 )
		{
			return	-1;
		}
		// 为了下次继承的时候不出错
		m_fman.MarkClean();
		//
		m_log.WriteFmtLine("Old File Released.");

		delete	m_pSHM;
		m_pSHM	= NULL;
		whshm_destroy(m_info.nSHMKEY);
		m_log.WriteFmtLine("SHM Destroied.");
	}
	return	0;
}
int	whmemfilewritecache_i::Release_SaverThread()
{
	if( m_tidSaver!=INVALID_TID )
	{
		// 停止线程
		m_bStopSaver	= true;
		// 等待结束(线程会进行完当前的一个存储操作然后结束)
		m_log.WriteFmtLine("Wait Thread to end ...");
		if( !whthread_waitend(m_tidSaver, m_info.nWaitThreadEndTime) )
		{
			m_log.WriteFmtLine("Wait ERROR. Force to terminate ...");
			whthread_terminate(m_tidSaver);
		}
		whthread_closetid(m_tidSaver);
		m_log.WriteFmtLine("Thread done.");
		// 释放成功
		m_tidSaver	= INVALID_TID;
	}
	// 释放指令队列(里面有什么垃圾也无所谓了)
	m_queueCmdToThread.Release();
	m_queueCmdFromThread.Release();
	m_log.WriteFmtLine("Saver Thread Destroied.");
	return	0;
}
int	whmemfilewritecache_i::_RelesaeOldFilesFromSHM()
{
	// 根据SaveList把原来没有存盘的文件存盘，存一个移除，如果存盘出错则不能继续
	int	nIdx;
	while( (nIdx=nSaveListHead())!=whalist::INVALIDIDX )
	{
		// 读取文件并存储
		_SaveFileByIdx(nIdx, true);
		m_log.WriteFmtLine("_RelesaeOldFilesFromSHM _SaveFileByIdx:%d", nIdx);
		// 从队列中移除（这样下一次nSaveListHead()就得到的是下一个）
		SaveList_Remove(nIdx);
	}

	// 重新Reset共享内存
	m_fman.Reset();
	// 初始化Save和CanKick队列
	_ClearAllList();

	return	0;
}
int	whmemfilewritecache_i::_ClearAllList()
{
	for(int i=0;i<MEMHDR_IDX_TOTAL;i++)
	{
		_nMemInfoUnit(i)	= whalist::INVALIDIDX;
	}
	return	0;
}
int	whmemfilewritecache_i::_SaveFileToDisk(const char *cszFileName, const void *pData, size_t nSize)
{
	// 确保文件目录存在
	if( whdir_SureMakeDirForFile(cszFileName)<0 )
	{
		// 无法创建目录
		return	-1;
	}
	// 写入文件
	if( whfile_safereplace(cszFileName, pData, nSize, true)<0 )
	{
		return	-2;
	}
	return	0;
}
int	whmemfilewritecache_i::_SaveFileByIdx(int nIdx, bool bLogIt)
{
	whmemfile	*file	= m_fman.OpenByIdx(nIdx, whmemfileman::MODE_READ, NULL);
	if( file )
	{
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		assert(pFName);
		// 读出数据
		file->Read(m_vectFileBuf4Main);
		// 关闭文件
		m_fman.Close(file);
		if( _SaveFileToDisk(pFName->szFName, m_vectFileBuf4Main.getbuf(), m_vectFileBuf4Main.size())<0 )
		{
			m_log.WriteFmtLine("ERROR: _SaveFileByIdx: Can NOT save %s ! fsize:%d", pFName->szFName, m_vectFileBuf4Main.size());
			return	-1;
		}
		if( bLogIt )
		{
			m_log.WriteFmtLine("_SaveFileByIdx: %s [fsize:%d] saved to disk.", pFName->szFName, m_vectFileBuf4Main.size());
		}
	}
	else
	{
		// 这也是很奇怪的
		m_log.WriteFmtLine("ERROR: Can not get file by idx:%d !!!", nIdx);
		return	-2;
	}
	return	0;
}

int	whmemfilewritecache_i::Tick()
{
	// tick时间间隔
	m_itv.tick();

	// 日志
	m_log.Tick();

	// 以一定速率实施真正存盘(发送存盘指令给Saver线程)
	Tick_RealSave();

	return	0;
}
int	whmemfilewritecache_i::Tick_RealSave()
{
	if( (int)m_queueCmdToThread.GetUnitNum()>=m_info.nMaxCmdInQueue )
	{
		// 队列已经很满了，返回
		return	0;
	}

	// 获得本tick最多需要存储多少文件
	int	nNum = m_generRealSave.MakeDecision(m_itv.getfloatInterval());

	// 从SaveList队列中取一个发一个
	int		nIdx;
	CMD_T	*pCmd	= (CMD_T *)m_vectCmdBuf4Main.getbuf();
	pCmd->nCmd		= CMD_SAVE;
	while( (nNum-- > 0) && (nIdx=nSaveListHead())!=whalist::INVALIDIDX )
	{
		// 2005-04-04加，因为有可能一次加入的指令超过nMaxCmdInQueue的限制
		if( (int)m_queueCmdToThread.GetUnitNum()>=m_info.nMaxCmdInQueue )
		{
			// 队列已经很满了，返回
			return	0;
		}
		// 读取文件
		whmemfile	*file	= m_fman.OpenByIdx(nIdx, whmemfileman::MODE_READ, NULL);
		if( file )
		{
			whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
			assert(pFName);
			// 读出数据
			file->Read(m_vectFileBuf4Main);
			// 关闭文件
			m_fman.Close(file);
			// 尝试加入指令队列
			size_t	nSize	= _WriteFileNameAndDataToBuffer(pCmd->data, pFName->szFName, m_vectFileBuf4Main.getbuf(), m_vectFileBuf4Main.size());
			nSize			+= wh_offsetof(CMD_T, data);
			if( m_queueCmdToThread.In(pCmd, nSize)<0 )
			{
				// 如果加入失败(应该是由于线程太慢导致命令缓冲区满了)则直接跳出函数，继续上层别的工作
				// 这个在线程比较忙的时候是可能出现的，所以不用记录日志
				return	0;
			}
		}
		else
		{
			m_log.WriteFmtLine("ERROR: Tick_RealSave: Can not open fidx:%d", nIdx);
			// 不可能打不开的
			assert(0);
		}
		// 从存盘队列中移除
		SaveList_Remove(nIdx);
		// 加入可踢出队列
		CanKickList_AddToTail(nIdx);
	}

	return	0;
}
int	whmemfilewritecache_i::KickOldFiles()
{
	// 获得目前还可创建的文件数
	int	nFileNumLeft	= m_fman.GetFileNumLeft();
	// 踢出CanKick队列中最老的那个
	int	nHead	= nCanKickListHead();
	if( nHead == whalist::INVALIDIDX )
	{
		if( nFileNumLeft>0 )
		{
			// 还有地方，那就不用踢了
			return	0;
		}
		// 没有地方，又没有可踢的，奇怪啊，一定是用的太多了
		m_log.WriteFmtLine("ERROR: KickOldFiles: no old file to kick!!! and no available file slot!!!");
		return	-1;
	}
	// 如果不得不删除了，则至少删除一个
	if( nFileNumLeft <= 0 )
	{
		int	rst	= DeleteFileByIdx(nHead);
		if( rst<0 )
		{
			return	-100 + rst;
		}
	}
	// 接着从内存中删除该文件和所有文件时间超过TimeToStay的文件
	time_t	nNow	= wh_time();
	while( (nHead=nCanKickListHead()) != whalist::INVALIDIDX )
	{
		// 获得文件结构，比较文件时间
		whmemfileman::FILEINFO_T	*pInfo	= m_fman.GetFileInfoByIdx(nHead);
		assert( pInfo != NULL );
		if( (int(nNow-pInfo->nLastModify)) >= m_info.nTimeToStay )
		{
			int	rst	= DeleteFileByIdx(nHead);
			if( rst<0 )
			{
				return	-200 + rst;
			}
		}
		else
		{
			// 后面就没有需要kick的啦
			break;
		}
	}
	return	0;
}

int	whmemfilewritecache_i::LoadFile(const char *cszFileName, whvector<char> &vectData)
{
	// 先检查内存中是否已经存在
	whmemfilemanWithName::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( !pFInfo )
	{
		// 没有则从硬盘载入
		if( whfile_readfile(cszFileName, vectData)<0 )
		{
			// 文件不存在或者出错
			return	-1;
		}
		if( !m_info.bCacheReadFile )
		{
			// 对于只读出的文件，不存储cache在中（这样主要是利用硬盘自身的缓存）
			return	0;
		}
		// 对只读的缓冲还没有实现呢。
		assert(0);	// 以后再实现后面的
		// 踢出最老的文件
		// 载入文件
		// 如果载入成功则进行相应关联(让如CanKick队列)
	}
	else
	{
		// 从内存文件载入
		whmemfile	*file	= m_fman.OpenByIdx(pFInfo->nIdx, whmemfileman::MODE_READ, NULL);
		if( !file )
		{
			assert(0);
			return	-2;
		}
		file->ReadToVector(vectData);
		m_fman.Close(file);
	}
	return	0;
}
int	whmemfilewritecache_i::SaveFile(const char *cszFileName, const void *pData, int nSize)
{
	// 先检查内存中是否已经存在
	whmemfilemanWithName::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( !pFInfo )
	{
		if( m_bNotExpand )
		{
			// 原来文件不存在，且不希望扩充cache，则直接存盘
			_SaveFileToDisk(cszFileName, pData, nSize);
			return	0;
		}
		// 踢出最老的文件
		KickOldFiles();
	}
	else
	{
		// 就文件存在。比较一下和旧文件是否相同，如果相同则不需要存盘了
		whmemfile	*file	= m_fman.OpenByIdx(pFInfo->nIdx, whmemfileman::MODE_READ, NULL);
		if( !file )
		{
			// 整个应该是不可能的
			assert(0);
			return	-1;
		}
		// 比较文件和数据
		int	rst	= file->CmpWithData(pData, nSize);
		// 关闭文件
		m_fman.Close(file);
		if( rst==0 )
		{
			// 文件和数据一致，无需存盘了
			return	0;
		}
	}
	// 写入文件内容
	whmemfile	*file		= m_fman.Open(cszFileName, whmemfileman::MODE_CREATE);
	int	rst=0;
	if( !file || (rst=file->Write(pData, nSize))!=(int)nSize )
	{
		// 记录日志
		m_log.WriteFmtLine("ERROR: SaveFile Can Not write to file:%p!!! fsize:%d rst:%d", file, nSize, rst);
		// 改为直接写硬盘
		_SaveFileToDisk(cszFileName, pData, nSize);
		// 关闭文件，返回错误
		if( file )	m_fman.Close(file);
		// 2005-03-08 改为返回0，因为程序应该还需要正常运行
		return	0;
	}
	pFInfo	= file->GetInfo();
	m_fman.Close(file);

	// 加入存储队列(如果原来不在存储队列中的话)
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(pFInfo->nIdx);
	assert(pFName);
	switch( pFName->nInWhatList )
	{
		case	WHATLIST_SAVE:
			// 已经在存储队列中了，不作任何操作
		break;
		case	WHATLIST_CANKICK:
			// 从CanKick队列中跳出
			CanKickList_Remove(pFInfo->nIdx);
			// 然后继续下面default的操作
		default:
			// 说明原来不在任何队列中
			// 加入存储队列
			SaveList_AddToTail(pFInfo->nIdx);
		break;
	}
	return	0;
}
whfile *	whmemfilewritecache_i::OpenFileRW(const char *cszFileName)
{
	autoRWFile	*file	= new autoRWFile(this, cszFileName);
	if( file->LoadFile()<0 )
	{
		delete	file;
		return	NULL;
	}
	return	file;
}
int	whmemfilewritecache_i::DeleteFile(const char *cszFileName)
{
	// 先删除内存中的，如果有的话
	whmemfileman::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( pFInfo )
	{
		if( DeleteFileByIdx(pFInfo->nIdx)<0 )
		{
			return	-1;
		}
	}
	// 然后删除硬盘上的相应文件
	if( whdir_sysdelfile(cszFileName)<0 )
	{
		return	-2;
	}
	return	0;
}
int	whmemfilewritecache_i::DeleteFileByIdx(int nIdx)
{
	// 先从当前相关的列表(Save或CanKick)中删除(如果它们真的存在于列表中的话)
	if( _List_Remove(nIdx)<0 )
	{
		return	-1;
	}
	// 然后从fman中删除
	if( m_fman.DeleteByIdx(nIdx)<0 )
	{
		return	-2;
	}
	return	0;
}
int	whmemfilewritecache_i::DeleteDir(const char *cszDir)
{
	// 浏览硬盘上的文件，得到列表，通过DeleteFile删除
	class	MyDel	: public WHDirSearchAction
	{
	public:
		whmemfilewritecache_i	*m_pHost;
		MyDel(whmemfilewritecache_i *pHost)
		: m_pHost(pHost)
		{
		}
	private:
		int		ActionOnFile(const char *szFile)
		{
			// 这里是相对路径，需要再得到一次
			// 删除内存中的文件并删除磁盘上的文件
			if( m_pHost->DeleteFile(GetFullPath(szFile))<0 )
			{
				return	-1;
			}
			return	0;
		}
		int		ActionOnDir(const char *szDir)
		{
			// 这里因为还没开始处理里面的文件
			return	0;
		}
		int		LeaveDir(const char *szDir)
		{
			// 这里是全路径
			// 删除该Dir
			// 在最后会删除DoSearch的根目录
			whfile_makefilewritable(szDir);
			return	whdir_sysdeldir(szDir);
		}
	};
	MyDel	mydel(this);
	if( mydel.DoSearch(cszDir, true)<0 )
	{
		return	-1;
	}
	return	0;
}
int	whmemfilewritecache_i::GetStatInfo(STATINFO_T *pInfo)
{
	pInfo->nSaveListLen		= _GetSaveListCount();
	// 其他信息
	pInfo->nCmdNumInQueue	= m_queueCmdToThread.GetUnitNum();
	pInfo->nCurFragNum		= m_fman.GetFragNum();
	pInfo->nCurFileNum		= m_fman.GetFileNum();

	if( m_nSaveListLenMax<pInfo->nSaveListLen )
	{
		m_nSaveListLenMax	= pInfo->nSaveListLen;
	}
	if( m_nCmdNumInQueueMax<pInfo->nCmdNumInQueue )
	{
		m_nCmdNumInQueueMax	= pInfo->nCmdNumInQueue;
	}
	if( m_nCurFragMax<pInfo->nCurFragNum )
	{
		m_nCurFragMax	= pInfo->nCurFragNum;
	}
	if( m_nCurFileMax<pInfo->nCurFileNum )
	{
		m_nCurFileMax	= pInfo->nCurFileNum;
	}
	pInfo->nSaveListLenMax		= m_nSaveListLenMax;
	pInfo->nCmdNumInQueueMax	= m_nCmdNumInQueueMax;
	pInfo->nCurFragMax			= m_nCurFragMax;
	pInfo->nCurFileMax			= m_nCurFileMax;

	return	0;
}
int		whmemfilewritecache_i::_GetSaveListCount()
{
	// 统计SaveList长度
	int	nIdx	= nSaveListHead();
	int	nCount	= 0;
	while( nIdx!=whalist::INVALIDIDX )
	{
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		if( !pFName )
		{
			break;
		}
		nCount	++;
		nIdx	= pFName->nNextIdx;
	}
	return	nCount;
}
int		whmemfilewritecache_i::ExportCacheToText(const char *cszTextFile)
{
	FILE	*fp	= fopen(cszTextFile, "at");
	if( !fp )
	{
		return	-1;
	}

	fprintf(fp, "**** Export BEGIN [%s] ****%s", wh_gettimestr(), WHLINEEND);

	// 浏览所有文件写文件信息
	int	nIdx, nCount;
	// SaveList
	fprintf(fp, "**** SaveList BEGIN ***%s", WHLINEEND);
	nIdx	= nSaveListHead();
	nCount	= 0;
	while( nIdx != whalist::INVALIDIDX )
	{
		_ExportOneFileToText(fp, nIdx);
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		if( !pFName )
		{
			break;
		}
		nCount	++;
		nIdx	= pFName->nNextIdx;
	}
	fprintf(fp, "**** SaveList END [Total:%d]***%s", nCount, WHLINEEND);
	// CanKickList
	fprintf(fp, "**** CanKickList BEGIN ***%s", WHLINEEND);
	nIdx	= nCanKickListHead();
	nCount	= 0;
	while( nIdx != whalist::INVALIDIDX )
	{
		_ExportOneFileToText(fp, nIdx);
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		if( !pFName )
		{
			break;
		}
		nCount	++;
		nIdx	= pFName->nNextIdx;
	}
	fprintf(fp, "**** CanKickList END [Total:%d]***%s", nCount, WHLINEEND);

	fprintf(fp, "**** Export END [%s]****%s", wh_gettimestr(), WHLINEEND);

	fclose(fp);
	return	0;
}
int		whmemfilewritecache_i::DeleteMatchedFilesInCache(const char *cszFmt)
{
	if( !cszFmt || cszFmt[0]==0 )
	{
		// 参数错误
		return	-1;
	}
	// 浏览所有文件写文件信息
	int				nIdx;
	whvector<int>	vect;
	whstrlike		*psl	= whstrlike::Create();
	// SaveList
	nIdx	= nSaveListHead();
	psl->SetPattern(cszFmt);
	while( nIdx != whalist::INVALIDIDX )
	{
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		if( !pFName )
		{
			break;
		}
		if( psl->IsLike(pFName->szFName) )
		{
			vect.push_back(nIdx);
		}
		nIdx	= pFName->nNextIdx;
	}
	// CanKickList
	nIdx	= nCanKickListHead();
	while( nIdx != whalist::INVALIDIDX )
	{
		whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
		if( !pFName )
		{
			break;
		}
		if( psl->IsLike(pFName->szFName) )
		{
			vect.push_back(nIdx);
		}
		nIdx	= pFName->nNextIdx;
	}
	// 真正的删除工作
	for(int i=0;i<(int)vect.size();i++)
	{
		DeleteFileByIdx(vect[i]);
	}

	return	vect.size();
}
int		whmemfilewritecache_i::_ExportOneFileToText(FILE *fp, int nIdx)
{
	whmemfilemanWithName::FILEINFO_T	*pFInfo	= m_fman.GetFileInfoByIdx(nIdx);
	if( !pFInfo )
	{
		return	-1;
	}
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		return	-2;
	}
	fprintf(fp, "[idx]:%d [NAME]:%s, [SIZE]:%u, [MODIFY]:%s [CRC]:%s%s"
		, pFInfo->nIdx
		, pFName->szFName
		, pFInfo->nSize
		, wh_gettimestr(pFInfo->nLastModify)
		, wh_hex2str((const unsigned char *)&pFInfo->nCRC32, sizeof(pFInfo->nCRC32))
		, WHLINEEND
		);
	return	0;
}
int		whmemfilewritecache_i::Clear()
{
	// 应该先等待线程指令缓冲中没有要继续存储的文件了
	if( m_queueCmdToThread.GetUnitNum()>0 )
	{
		int	i;
		for(i=0;i<10;i++)
		{
			wh_sleep(5);
			if( m_queueCmdToThread.GetUnitNum()==0 )
			{
				break;
			}
		}
		if( i>=10 )
		{
			// 不能清除
			return	-1;
		}
	}
	return	_RelesaeOldFilesFromSHM();
}

// Thread部分
void *	whmemfilewritecache_i::SaverThreadProc()
{
	while( !m_bStopSaver )
	{
		while( !m_bStopSaver )
		{
			size_t	nSize	= m_queueCmdToThread.PeekSize();
			if( nSize==0 )
			{
				// 没有指令要读
				break;
			}
			m_vectCmdBuf4Thread.resize(nSize);
			CMD_T	*pCmd	= (CMD_T *)m_vectCmdBuf4Thread.getbuf();
			// 这个应该是一定成功的
			int	rst	= m_queueCmdToThread.Out(pCmd, &nSize);
			assert(rst==0);
			switch( pCmd->nCmd )
			{
				case	CMD_SAVE:
					_SaverThreadProc_SAVE(pCmd, nSize);
				break;
			}
		}
		// 让系统休息一下
		wh_sleep(10);
	}
	return	NULL;
}
int	whmemfilewritecache_i::_SaverThreadProc_SAVE(CMD_T *pCmd, size_t nSize)
{
	// 用于测试写盘很慢的情况
	if( m_nSaveSleep>0 )
	{
		wh_sleep(m_nSaveSleep);
	}
	const char	*pcszFName;
	const void	*pData;
	size_t		nDSize;
	int			rst;
	if( (rst=_ReadFileNameAndDataFromBuffer(pCmd->data, pcszFName, pData, nDSize))<0 )
	{
		// 无法读出数据
		return	-1;
	}
	if( _SaveFileToDisk(pcszFName, pData, nDSize)<2 )
	{
		// 无法写入数据
		return	-2;
	}
	return	0;
}
size_t	whmemfilewritecache_i::_WriteFileNameAndDataToBuffer(void *pBuf, const char *cszFName, const void *pData, size_t nDSize)
{
	size_t	nLen	= whvldata_write(cszFName, strlen(cszFName)+1, (namelen_t *)pBuf);
	pBuf			= wh_getoffsetaddr(pBuf, nLen);
	nLen			+= whvldata_write(pData, (int)nDSize, (buflen_t *)pBuf);
	return	nLen;
}
int	whmemfilewritecache_i::_ReadFileNameAndDataFromBuffer(const void *pBuf, const char *&pcszFName, const void *&pData, size_t &nDSize)
{
	int		nLen	= whvldata_read_ref((namelen_t *)pBuf, (const void *&)pcszFName);
	pBuf			= wh_getoffsetaddr(pBuf, nLen+sizeof(namelen_t));
	nDSize			= whvldata_read_ref((buflen_t *)pBuf, (const void *&)pData);
	return	0;
}
int	whmemfilewritecache_i::_List_AddToTail(int &nHead, int &nTail, int nIdx, int nWhatList)
{
	// 获得这个名字结构
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		return	-1;
	}
	if( pFName->nInWhatList == nWhatList )
	{
		// 已经在列表内了，不要重复添加(上层可能会利用这个来判断是否已经在队列中)
		return	-2;
	}
	// 它的prev指向原来的尾
	pFName->nPrevIdx	= nTail;
	pFName->nNextIdx	= whalist::INVALIDIDX;
	if( nTail == whalist::INVALIDIDX )
	{
		// 第一个
		nHead			= nIdx;
	}
	else
	{
		// 获得tail的对象
		whmemfilemanWithName::FILENAME_T	*pTailFName	= m_fman.GetFileNameByIdx(nTail);
		pTailFName->nNextIdx	= nIdx;
	}
	// Tail指向现在这个对象
	nTail	= nIdx;
	// 标记为在列表内了
	pFName->nInWhatList	= nWhatList;

	return	0;
}
int	whmemfilewritecache_i::_List_Remove(int &nHead, int &nTail, int nIdx, int nWhatList)
{
	// 获得这个名字结构
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		return	-1;
	}
	if( pFName->nInWhatList<0 )
	{
		// 已经不在列表内了
		return	-2;
	}
	// 他前后的互连一下
	// 前
	if( pFName->nPrevIdx == whalist::INVALIDIDX )
	{
		nHead					= pFName->nNextIdx;
	}
	else
	{
		whmemfilemanWithName::FILENAME_T	*pPrevFName	= m_fman.GetFileNameByIdx(pFName->nPrevIdx);
		pPrevFName->nNextIdx	= pFName->nNextIdx;
	}
	// 后
	if( pFName->nNextIdx == whalist::INVALIDIDX )
	{
		nTail					= pFName->nPrevIdx;
	}
	else
	{
		whmemfilemanWithName::FILENAME_T	*pNextFName	= m_fman.GetFileNameByIdx(pFName->nNextIdx);
		pNextFName->nPrevIdx	= pFName->nPrevIdx;
	}
	pFName->nInWhatList			= -1;
	return	0;
}
