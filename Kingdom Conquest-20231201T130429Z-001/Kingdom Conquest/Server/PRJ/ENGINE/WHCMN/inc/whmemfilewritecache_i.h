// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache_i.h
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ��ļ����ļ�д��/�������ٹ���
//              : ������д����������һ���߳����
//              : �κ�һ���ļ�ֻҪ������Cache�У���Ҫô��SaveListҪô��CanKickList
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
// Ϊ�ӿ�ʵ�ֵķ���
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
	// ���ڵ���
	// ����д��ʱ��sleep��ô��ʱ��(����ģ��Ӳ��æ�����)
	virtual void	SetSaveSleep(int nSleep)
	{
		m_nSaveSleep	= nSleep;
	}
	virtual int		GetStatInfo(STATINFO_T *pInfo);

	virtual int		ExportCacheToText(const char *cszTextFile);

	virtual int		DeleteMatchedFilesInCache(const char *cszFmt);

	// ����ֹͣ������Cache������ļ�(�������ļ���Ȼ����)
	virtual void	SetNotExpand(bool bSet)
	{
		m_bNotExpand	= bSet;
	}
	virtual bool	GetNotExpand() const
	{
		return	m_bNotExpand;
	}
	virtual int		Clear();
// �Լ��õĳ�Ա�ͽṹ����
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
				// ��Ȼû��load���������ǿ��������ļ������ڣ�����Ϊ0
				// !!!!�ǵò�Ҫreturn��!!!!
			}
			SetVector(m_vectFileData, false);
			return	0;
		}
		int	CloseFile()
		{
			return	m_pHost->SaveFile(m_szFileName, m_vectFileData);
		}
	};
	// �������Ǹ��̵߳Ĵ���ָ���������ļ��������ݲ��ֵĳ�����������������
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
		CMD_LOG			= 0,			// ��¼һ����־
										// ʹ��CMD_T
										// ���ݲ���Ϊ��־�ִ�����0��β
		CMD_SAVE		= 1,			// �洢�ļ�������
										// ʹ��CMD_T
										// ���ݲ���Ϊ�䳤����+�䳤����
	};
	struct	CMD_T
	{
		cmd_t		nCmd;				// ָ��CMD_XXX
		char		data[1];			// ���ݲ���
	};
	#pragma pack()
private:
	// ������Ϣ
	INFO_T				m_info;
	// �����̵߳�tid
	whtid_t				m_tidSaver;
	// �߳��Ƿ�Ӧ��ֹͣ
	bool				m_bStopSaver;
	// �ڴ��ļ�������
	whmemfilemanWithName		m_fman;
	// ��־��¼��
	whlogwriter			m_log;
	// �����ڴ�
	whshm				*m_pSHM;
	// ���������̷߳���ָ��Ķ���
	whsmpqueueWL		m_queueCmdToThread;
	// �߳���ʹ�õ�ָ�����(���ĳ�����ƽ���ļ����ȵ�4������)
	whvector<char>		m_vectCmdBuf4Thread;
	// ���߳��߳���ʹ�õ�ָ�����(���ĳ���ͬ��)
	whvector<char>		m_vectCmdBuf4Main;
	// ���߳��߳���ʹ�õ���ʱ�洢�ļ����ݵĻ�����(���ĳ���ͬ��)
	whvector<char>		m_vectFileBuf4Main;
	// �߳�����������ָ��Ķ���(���緢��д�ļ�ʧ�ܵ���־)
	whsmpqueueWL		m_queueCmdFromThread;
	// ����ÿ��tick������Ҫ�洢���ļ�����
	whgener				m_generSave;
	// 
	whinterval			m_itv;			// ���ڻ��ÿ��tick��ʱ����
	whgener				m_generRealSave;// ��ʵ��������
	// ���ڵ���
	int					m_nSaveSleep;
	// �Ƿ�ֹͣ��չcache�������ɵ��ļ���Ȼ����
	bool				m_bNotExpand;
	// ����ͳ���ڴ����ռ��
	int					m_nSaveListLenMax, m_nCmdNumInQueueMax;
	int					m_nCurFragMax, m_nCurFileMax;
// �Լ��õķ���
public:
	// �����̵߳���������
	void *	SaverThreadProc();
private:
	int		Init_SHM();
	int		Init_SaverThread();
	int		Release_SHM();
	int		Release_SaverThread();
	// �����Init�̳��ڴ��Լ�Release�洢�ļ�ʱ�����õ�
	int		_RelesaeOldFilesFromSHM();
	// ������ж���(Save��CanKick��)
	int		_ClearAllList();
	// �洢һ���ļ�
	// �����Saver�߳����е�ʱ�����̺߳�Saver�����õ��á���Saver�߳�ֹͣ��ʱ����������̵߳�������ļ���
	int		_SaveFileToDisk(const char *cszFileName, const void *pData, size_t nSize);
	// ���Ӧ��ֻ�������̵߳���
	int		_SaveFileByIdx(int nIdx, bool bLogIt);
	// SaverThreadProcʹ�õ��ӹ���
	int		_SaverThreadProc_SAVE(CMD_T *pCmd, size_t nSize);
	// ����������д�������ܴ����������������ϲ㱣֤��������һ�����ܶ����ģ����ܷ���ͷ�����Ȳ��ֱ�ʵ�ʳ�������
	// �ļ��������ݴ��뻺�岢�����ܳ���
	size_t	_WriteFileNameAndDataToBuffer(void *pBuf, const char *cszFName, const void *pData, size_t nDSize);
	// �ӻ�����ȡ���ļ��������ݲ���ͷָ��
	int		_ReadFileNameAndDataFromBuffer(const void *pBuf, const char *&pcszFName, const void *&pData, size_t &nDSize);
	// ÿ��Tick�ڶ��ļ����������̲���
	int		Tick_RealSave();
	// �߳��������ɵ��ļ�
	int		KickOldFiles();
	// ����idxɾ���ļ�
	int		DeleteFileByIdx(int nIdx);

	// �����ڴ�ͷ����ĸ�������
	inline int &	_nMemInfoUnit(int nIdx)
	{
		return	m_fman.GetMemInfo()->nExtInfo[nIdx];
	}
	// ���̶��е�ͷβ
	// ÿ���ļ���Saveָ��󶼻����������У��ɹ������߳�ָ����к�����������ɾ��
	inline int &	nSaveListHead()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_SAVELIST_HEAD);
	}
	inline int &	nSaveListTail()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_SAVELIST_TAIL);
	}
	// ���߳����е�ͷβ
	// ÿ���ļ��ڳɹ�Save�󶼻����������У�����Save֮ǰ������ɾ��
	inline int &	nCanKickListHead()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_CANKICKLIST_HEAD);
	}
	inline int &	nCanKickListTail()
	{
		return	_nMemInfoUnit(MEMHDR_IDX_CANKICKLIST_TAIL);
	}

	// �����������Add/Remove���ڲ�ʵ��
	// �������β(�����֤�����Ĳ��ܼ������롣����Ϊ�п����Ǽ̳����ĵ��ڴ棬�����ϲ�ÿ�η���֮��Ҫclearһ��)
	int	_List_AddToTail(int &nHead, int &nTail, int nIdx, int nWhatList);
	// �Ӷ�����ɾ��һ��(�����֤��ɾ�����Ĳ��ܼ���ɾ��)
	int	_List_Remove(int &nHead, int &nTail, int nIdx, int nWhatList);
	// ���Լ�ĳ�����͵ĵĶ������Ƴ�
	inline int	_List_Remove(int nIdx, int nWhatList)
	{
		if( nWhatList<0 )
		{
			// �����κζ�����
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
	// ���ش����б���ļ�����
	int		_GetSaveListCount();
	// ����һ���ļ����ı��ļ�
	int		_ExportOneFileToText(FILE *fp, int nIdx);
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEMFILEWRITECACHE_I_H__
