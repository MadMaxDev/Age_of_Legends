// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ��ļ����ļ�д��/�������ٹ���
//              : ������д����������һ���߳����
// CreationDate : 2005-02-06
// ChangeLOG    : 2005-03-14 ��ȥ���߳����ϵ�һ���ļ��������޸�Ϊ�߳��ܳ�ʱ��û���޸ĵľ��ļ���Ĭ��12Сʱû���޸ģ���
//              : 2005-03-25 whmemfilewritecache_i::Release_SaverThread()��ԭ����Ȼ����û�йرչ��߳�
//              : 2006-06-06 ������DeleteMatchedFilesInCache�ж�cszFmt�����Ƿ�Ϸ����жϣ�������ܵ��³��Ƿ�����
//              : 2007-04-27 whmemfilewritecache_i::SaveFile��ֻҪд����������д�볤�Ⱦ���Ϊ����

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

	// ��ʼ����������ĳ���
	// ����������ļ����ȵ�4��
	m_vectCmdBuf4Thread.resize(m_info.nFragSize*m_info.nTotalFrag/m_info.nMaxFile * 4);
	m_vectCmdBuf4Main.resize(m_vectCmdBuf4Thread.size());
	m_vectFileBuf4Main.resize(m_vectCmdBuf4Thread.size());

	// ��ʼ����־
	if( m_log.Init(&m_info.logInfo)<0 )
	{
		return	-1;
	}

	m_log.WriteFmtLine("**** whmemfilewritecache BEGIN [%s] ****", wh_gettimestr());

	// ��ʼ�������߼̳У������ڴ棬���ԭ�����ļ��������´洢������������˳���
	if( Init_SHM()<0 )
	{
		return	-2;
	}

	// ���������߳�
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
	// ���Դ򿪹����ڴ�
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
		// û�оʹ���֮
		m_pSHM			= whshm_create(nSHMKey, nSizeNeeded);
		m_log.WriteFmtLine("Init_SHM SHM create ptr:%p!", m_pSHM);
		if( !m_pSHM )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM can not create SHM!");
			return	-1;
		}
		// ��ʼ��ʹ�ù����ڴ���ļ�������
		info.pBuf	= m_pSHM->GetBuf();
		int	rst		= m_fman.Init(&info);
		if( rst<0 )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM m_fman.Init error:%d", rst);
			// ɾ���Ѿ������SHM
			delete	m_pSHM;
			m_pSHM	= NULL;
			whshm_destroy(m_info.nSHMKEY);
			return	-2;
		}
		// ��ʼ��Save��CanKick����
		_ClearAllList();
	}
	else
	{
		m_log.WriteFmtLine("Init_SHM SHM already existed ptr:%p! It will be inherited.", m_pSHM);
		// ��ʼ��ʹ�ù����ڴ���ļ�������
		void	*pBuf	= m_pSHM->GetBuf();
		size_t	nSize	= m_pSHM->GetSize();
		// �жϳߴ��Ƿ���ȷ
		if( nSize != nSizeNeeded )
		{
			m_log.WriteFmtLine("Init_SHM inherited size:%d != needed size:%d", nSize, nSizeNeeded);
			// ֱ���˳��������ɲ���ɾ�����ڴ氡����Ϊ��������������ȷ�ĳ������̳�
			return	-11;
		}
		// �̳������ڴ�
		int	rst		= m_fman.Inherit(pBuf);
		if( rst<0 )
		{
			m_log.WriteFmtLine("ERROR: Init_SHM inherit (err:%d)", rst);
			// �Ͼ��͹����ڴ�Ĺ�ϵ
			delete	m_pSHM;
			m_pSHM	= NULL;
			return	-12;
		}
		m_log.WriteFmtLine("Init_SHM inherit OK. total file:%d total frag:%d", m_fman.GetFileNum(), m_fman.GetFragNum());
		// ��ԭ��û�д��̵��ļ����̣�������̳������ܼ���
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
		// ˵���Ѿ���ʼ������
		return	-1;
	}

	// ��ʼ�����߳̽�����ָ�����
	if( m_queueCmdToThread.Init(m_info.nCmdToThreadBufSize)<0 )
	{
		return	-2;
	}
	if( m_queueCmdFromThread.Init(m_info.nCmdFromThreadBufSize)<0 )
	{
		return	-3;
	}

	// �����߳�
	if( whthread_create(&m_tidSaver, my_SaverThreadProc, this, m_info.nThreadPriority)<0 )
	{
		return	-4;
	}
	m_log.WriteFmtLine("Saver Thread Started.");
	return	0;
}
int	whmemfilewritecache_i::Release()
{
	// ֹͣ�����̡߳��ȴ����սᡣ
	Release_SaverThread();
	// �������ڴ档
	Release_SHM();
	//
	m_log.WriteFmtLine("**** whmemfilewritecache END [%s] ****", wh_gettimestr());
	// �ر�log
	m_log.Release();
	return	0;
}
int	whmemfilewritecache_i::Release_SHM()
{
	// �ͷŹ����ڴ�
	if( m_pSHM )
	{
		// �����ڴ��еĲ����ļ�ȫ�����̣����̳ɹ���ɾ�������ڴ�
		if( _RelesaeOldFilesFromSHM()<0 )
		{
			return	-1;
		}
		// Ϊ���´μ̳е�ʱ�򲻳���
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
		// ֹͣ�߳�
		m_bStopSaver	= true;
		// �ȴ�����(�̻߳�����굱ǰ��һ���洢����Ȼ�����)
		m_log.WriteFmtLine("Wait Thread to end ...");
		if( !whthread_waitend(m_tidSaver, m_info.nWaitThreadEndTime) )
		{
			m_log.WriteFmtLine("Wait ERROR. Force to terminate ...");
			whthread_terminate(m_tidSaver);
		}
		whthread_closetid(m_tidSaver);
		m_log.WriteFmtLine("Thread done.");
		// �ͷųɹ�
		m_tidSaver	= INVALID_TID;
	}
	// �ͷ�ָ�����(������ʲô����Ҳ����ν��)
	m_queueCmdToThread.Release();
	m_queueCmdFromThread.Release();
	m_log.WriteFmtLine("Saver Thread Destroied.");
	return	0;
}
int	whmemfilewritecache_i::_RelesaeOldFilesFromSHM()
{
	// ����SaveList��ԭ��û�д��̵��ļ����̣���һ���Ƴ���������̳������ܼ���
	int	nIdx;
	while( (nIdx=nSaveListHead())!=whalist::INVALIDIDX )
	{
		// ��ȡ�ļ����洢
		_SaveFileByIdx(nIdx, true);
		m_log.WriteFmtLine("_RelesaeOldFilesFromSHM _SaveFileByIdx:%d", nIdx);
		// �Ӷ������Ƴ���������һ��nSaveListHead()�͵õ�������һ����
		SaveList_Remove(nIdx);
	}

	// ����Reset�����ڴ�
	m_fman.Reset();
	// ��ʼ��Save��CanKick����
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
	// ȷ���ļ�Ŀ¼����
	if( whdir_SureMakeDirForFile(cszFileName)<0 )
	{
		// �޷�����Ŀ¼
		return	-1;
	}
	// д���ļ�
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
		// ��������
		file->Read(m_vectFileBuf4Main);
		// �ر��ļ�
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
		// ��Ҳ�Ǻ���ֵ�
		m_log.WriteFmtLine("ERROR: Can not get file by idx:%d !!!", nIdx);
		return	-2;
	}
	return	0;
}

int	whmemfilewritecache_i::Tick()
{
	// tickʱ����
	m_itv.tick();

	// ��־
	m_log.Tick();

	// ��һ������ʵʩ��������(���ʹ���ָ���Saver�߳�)
	Tick_RealSave();

	return	0;
}
int	whmemfilewritecache_i::Tick_RealSave()
{
	if( (int)m_queueCmdToThread.GetUnitNum()>=m_info.nMaxCmdInQueue )
	{
		// �����Ѿ������ˣ�����
		return	0;
	}

	// ��ñ�tick�����Ҫ�洢�����ļ�
	int	nNum = m_generRealSave.MakeDecision(m_itv.getfloatInterval());

	// ��SaveList������ȡһ����һ��
	int		nIdx;
	CMD_T	*pCmd	= (CMD_T *)m_vectCmdBuf4Main.getbuf();
	pCmd->nCmd		= CMD_SAVE;
	while( (nNum-- > 0) && (nIdx=nSaveListHead())!=whalist::INVALIDIDX )
	{
		// 2005-04-04�ӣ���Ϊ�п���һ�μ����ָ���nMaxCmdInQueue������
		if( (int)m_queueCmdToThread.GetUnitNum()>=m_info.nMaxCmdInQueue )
		{
			// �����Ѿ������ˣ�����
			return	0;
		}
		// ��ȡ�ļ�
		whmemfile	*file	= m_fman.OpenByIdx(nIdx, whmemfileman::MODE_READ, NULL);
		if( file )
		{
			whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
			assert(pFName);
			// ��������
			file->Read(m_vectFileBuf4Main);
			// �ر��ļ�
			m_fman.Close(file);
			// ���Լ���ָ�����
			size_t	nSize	= _WriteFileNameAndDataToBuffer(pCmd->data, pFName->szFName, m_vectFileBuf4Main.getbuf(), m_vectFileBuf4Main.size());
			nSize			+= wh_offsetof(CMD_T, data);
			if( m_queueCmdToThread.In(pCmd, nSize)<0 )
			{
				// �������ʧ��(Ӧ���������߳�̫�����������������)��ֱ�����������������ϲ��Ĺ���
				// ������̱߳Ƚ�æ��ʱ���ǿ��ܳ��ֵģ����Բ��ü�¼��־
				return	0;
			}
		}
		else
		{
			m_log.WriteFmtLine("ERROR: Tick_RealSave: Can not open fidx:%d", nIdx);
			// �����ܴ򲻿���
			assert(0);
		}
		// �Ӵ��̶������Ƴ�
		SaveList_Remove(nIdx);
		// ������߳�����
		CanKickList_AddToTail(nIdx);
	}

	return	0;
}
int	whmemfilewritecache_i::KickOldFiles()
{
	// ���Ŀǰ���ɴ������ļ���
	int	nFileNumLeft	= m_fman.GetFileNumLeft();
	// �߳�CanKick���������ϵ��Ǹ�
	int	nHead	= nCanKickListHead();
	if( nHead == whalist::INVALIDIDX )
	{
		if( nFileNumLeft>0 )
		{
			// ���еط����ǾͲ�������
			return	0;
		}
		// û�еط�����û�п��ߵģ���ְ���һ�����õ�̫����
		m_log.WriteFmtLine("ERROR: KickOldFiles: no old file to kick!!! and no available file slot!!!");
		return	-1;
	}
	// ������ò�ɾ���ˣ�������ɾ��һ��
	if( nFileNumLeft <= 0 )
	{
		int	rst	= DeleteFileByIdx(nHead);
		if( rst<0 )
		{
			return	-100 + rst;
		}
	}
	// ���Ŵ��ڴ���ɾ�����ļ��������ļ�ʱ�䳬��TimeToStay���ļ�
	time_t	nNow	= wh_time();
	while( (nHead=nCanKickListHead()) != whalist::INVALIDIDX )
	{
		// ����ļ��ṹ���Ƚ��ļ�ʱ��
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
			// �����û����Ҫkick����
			break;
		}
	}
	return	0;
}

int	whmemfilewritecache_i::LoadFile(const char *cszFileName, whvector<char> &vectData)
{
	// �ȼ���ڴ����Ƿ��Ѿ�����
	whmemfilemanWithName::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( !pFInfo )
	{
		// û�����Ӳ������
		if( whfile_readfile(cszFileName, vectData)<0 )
		{
			// �ļ������ڻ��߳���
			return	-1;
		}
		if( !m_info.bCacheReadFile )
		{
			// ����ֻ�������ļ������洢cache���У�������Ҫ������Ӳ������Ļ��棩
			return	0;
		}
		// ��ֻ���Ļ��廹û��ʵ���ء�
		assert(0);	// �Ժ���ʵ�ֺ����
		// �߳����ϵ��ļ�
		// �����ļ�
		// �������ɹ��������Ӧ����(����CanKick����)
	}
	else
	{
		// ���ڴ��ļ�����
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
	// �ȼ���ڴ����Ƿ��Ѿ�����
	whmemfilemanWithName::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( !pFInfo )
	{
		if( m_bNotExpand )
		{
			// ԭ���ļ������ڣ��Ҳ�ϣ������cache����ֱ�Ӵ���
			_SaveFileToDisk(cszFileName, pData, nSize);
			return	0;
		}
		// �߳����ϵ��ļ�
		KickOldFiles();
	}
	else
	{
		// ���ļ����ڡ��Ƚ�һ�º;��ļ��Ƿ���ͬ�������ͬ����Ҫ������
		whmemfile	*file	= m_fman.OpenByIdx(pFInfo->nIdx, whmemfileman::MODE_READ, NULL);
		if( !file )
		{
			// ����Ӧ���ǲ����ܵ�
			assert(0);
			return	-1;
		}
		// �Ƚ��ļ�������
		int	rst	= file->CmpWithData(pData, nSize);
		// �ر��ļ�
		m_fman.Close(file);
		if( rst==0 )
		{
			// �ļ�������һ�£����������
			return	0;
		}
	}
	// д���ļ�����
	whmemfile	*file		= m_fman.Open(cszFileName, whmemfileman::MODE_CREATE);
	int	rst=0;
	if( !file || (rst=file->Write(pData, nSize))!=(int)nSize )
	{
		// ��¼��־
		m_log.WriteFmtLine("ERROR: SaveFile Can Not write to file:%p!!! fsize:%d rst:%d", file, nSize, rst);
		// ��Ϊֱ��дӲ��
		_SaveFileToDisk(cszFileName, pData, nSize);
		// �ر��ļ������ش���
		if( file )	m_fman.Close(file);
		// 2005-03-08 ��Ϊ����0����Ϊ����Ӧ�û���Ҫ��������
		return	0;
	}
	pFInfo	= file->GetInfo();
	m_fman.Close(file);

	// ����洢����(���ԭ�����ڴ洢�����еĻ�)
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(pFInfo->nIdx);
	assert(pFName);
	switch( pFName->nInWhatList )
	{
		case	WHATLIST_SAVE:
			// �Ѿ��ڴ洢�������ˣ������κβ���
		break;
		case	WHATLIST_CANKICK:
			// ��CanKick����������
			CanKickList_Remove(pFInfo->nIdx);
			// Ȼ���������default�Ĳ���
		default:
			// ˵��ԭ�������κζ�����
			// ����洢����
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
	// ��ɾ���ڴ��еģ�����еĻ�
	whmemfileman::FILEINFO_T	*pFInfo	= m_fman.GetFileInfo(cszFileName);
	if( pFInfo )
	{
		if( DeleteFileByIdx(pFInfo->nIdx)<0 )
		{
			return	-1;
		}
	}
	// Ȼ��ɾ��Ӳ���ϵ���Ӧ�ļ�
	if( whdir_sysdelfile(cszFileName)<0 )
	{
		return	-2;
	}
	return	0;
}
int	whmemfilewritecache_i::DeleteFileByIdx(int nIdx)
{
	// �ȴӵ�ǰ��ص��б�(Save��CanKick)��ɾ��(���������Ĵ������б��еĻ�)
	if( _List_Remove(nIdx)<0 )
	{
		return	-1;
	}
	// Ȼ���fman��ɾ��
	if( m_fman.DeleteByIdx(nIdx)<0 )
	{
		return	-2;
	}
	return	0;
}
int	whmemfilewritecache_i::DeleteDir(const char *cszDir)
{
	// ���Ӳ���ϵ��ļ����õ��б�ͨ��DeleteFileɾ��
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
			// ���������·������Ҫ�ٵõ�һ��
			// ɾ���ڴ��е��ļ���ɾ�������ϵ��ļ�
			if( m_pHost->DeleteFile(GetFullPath(szFile))<0 )
			{
				return	-1;
			}
			return	0;
		}
		int		ActionOnDir(const char *szDir)
		{
			// ������Ϊ��û��ʼ����������ļ�
			return	0;
		}
		int		LeaveDir(const char *szDir)
		{
			// ������ȫ·��
			// ɾ����Dir
			// ������ɾ��DoSearch�ĸ�Ŀ¼
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
	// ������Ϣ
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
	// ͳ��SaveList����
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

	// ��������ļ�д�ļ���Ϣ
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
		// ��������
		return	-1;
	}
	// ��������ļ�д�ļ���Ϣ
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
	// ������ɾ������
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
	// Ӧ���ȵȴ��߳�ָ�����û��Ҫ�����洢���ļ���
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
			// �������
			return	-1;
		}
	}
	return	_RelesaeOldFilesFromSHM();
}

// Thread����
void *	whmemfilewritecache_i::SaverThreadProc()
{
	while( !m_bStopSaver )
	{
		while( !m_bStopSaver )
		{
			size_t	nSize	= m_queueCmdToThread.PeekSize();
			if( nSize==0 )
			{
				// û��ָ��Ҫ��
				break;
			}
			m_vectCmdBuf4Thread.resize(nSize);
			CMD_T	*pCmd	= (CMD_T *)m_vectCmdBuf4Thread.getbuf();
			// ���Ӧ����һ���ɹ���
			int	rst	= m_queueCmdToThread.Out(pCmd, &nSize);
			assert(rst==0);
			switch( pCmd->nCmd )
			{
				case	CMD_SAVE:
					_SaverThreadProc_SAVE(pCmd, nSize);
				break;
			}
		}
		// ��ϵͳ��Ϣһ��
		wh_sleep(10);
	}
	return	NULL;
}
int	whmemfilewritecache_i::_SaverThreadProc_SAVE(CMD_T *pCmd, size_t nSize)
{
	// ���ڲ���д�̺��������
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
		// �޷���������
		return	-1;
	}
	if( _SaveFileToDisk(pcszFName, pData, nDSize)<2 )
	{
		// �޷�д������
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
	// ���������ֽṹ
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		return	-1;
	}
	if( pFName->nInWhatList == nWhatList )
	{
		// �Ѿ����б����ˣ���Ҫ�ظ����(�ϲ���ܻ�����������ж��Ƿ��Ѿ��ڶ�����)
		return	-2;
	}
	// ����prevָ��ԭ����β
	pFName->nPrevIdx	= nTail;
	pFName->nNextIdx	= whalist::INVALIDIDX;
	if( nTail == whalist::INVALIDIDX )
	{
		// ��һ��
		nHead			= nIdx;
	}
	else
	{
		// ���tail�Ķ���
		whmemfilemanWithName::FILENAME_T	*pTailFName	= m_fman.GetFileNameByIdx(nTail);
		pTailFName->nNextIdx	= nIdx;
	}
	// Tailָ�������������
	nTail	= nIdx;
	// ���Ϊ���б�����
	pFName->nInWhatList	= nWhatList;

	return	0;
}
int	whmemfilewritecache_i::_List_Remove(int &nHead, int &nTail, int nIdx, int nWhatList)
{
	// ���������ֽṹ
	whmemfilemanWithName::FILENAME_T	*pFName	= m_fman.GetFileNameByIdx(nIdx);
	if( !pFName )
	{
		return	-1;
	}
	if( pFName->nInWhatList<0 )
	{
		// �Ѿ������б�����
		return	-2;
	}
	// ��ǰ��Ļ���һ��
	// ǰ
	if( pFName->nPrevIdx == whalist::INVALIDIDX )
	{
		nHead					= pFName->nNextIdx;
	}
	else
	{
		whmemfilemanWithName::FILENAME_T	*pPrevFName	= m_fman.GetFileNameByIdx(pFName->nPrevIdx);
		pPrevFName->nNextIdx	= pFName->nNextIdx;
	}
	// ��
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
