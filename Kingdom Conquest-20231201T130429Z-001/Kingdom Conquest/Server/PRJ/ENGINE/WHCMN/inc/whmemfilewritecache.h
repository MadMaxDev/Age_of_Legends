// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.h
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ��ļ����ļ�д��/�������ٹ���
//              : ������д����������һ���߳����
//              : ������ҪԤ���㹻��Ŀռ䣬��Ҫ��֤�ļ����ﵽ����ʱ���ļ�ռ���ܿռ䲻�ܴﵽ����!!!! ���Ƿǳ���Ҫ�ġ�
//              ��Ϊ��ͬһ���ļ����ظ����У���һ������ȫ·����д�ļ�������ȫ��ʹ��"/"
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
		whlogwriter::INFO_T	logInfo;			// ��־��Ϣ
		int		nSHMKEY;						// �����ڴ�ļ�ֵ
		int		nFragSize;						// ÿ���ļ���ĳߴ�
		int		nTotalFrag;						// �ܿ���(����nFragSize�����������ļ����ܿռ���)
		int		nMaxFile;						// �����ļ�����
		int		nCmdToThreadBufSize;			// ���߳��ύ��������еĳ���(���Ҫ��һЩ����Ϊ�ļ����ݻ�ֱ�ӷ������������)
		int		nCmdFromThreadBufSize;			// ���̷߳��ص�������еĳ���
		int		nMaxFileToSavePerSec;			// ÿ�����洢���ļ�����
		int		nTimeToStay;					// �ļ������cache�д��ڶ೤ʱ��(��)
		int		nWaitThreadEndTime;				// �ȴ��߳̽�����ʱ��
		int		nThreadPriority;				// �̼߳���
		int		nMaxCmdInQueue;					// ���̵߳Ķ��������������ɵ�ָ�����
		bool	bCacheReadFile;					// �Ƿ񻺳�ֻ�������ļ�
		INFO_T()
		: nSHMKEY(2005)
		, nFragSize(1000)
		, nTotalFrag(5000)
		, nMaxFile(100)
		, nCmdToThreadBufSize(50000*20)			// ƽʱ�ڻ������ڵ��ļ�������Ӧ��̫��
		, nCmdFromThreadBufSize(100000)			// ���߳�Ӧ�ò��᷵�غܶ�ָ���
		, nMaxFileToSavePerSec(100)				// ÿ��100���ļ���
		, nTimeToStay(12*3600)					// Ĭ�ϵ�12��Сʱ��
		, nWaitThreadEndTime(30000)				// ��30���㹻�˰�
		, nThreadPriority(-1)					// Saver�߳�ʹ��Ĭ�ϼ���
		, nMaxCmdInQueue(20)
		, bCacheReadFile(false)
		{
			logInfo.nProp	= whlogwriter::INFO_T::PROP_SOHU_DEFAULT;
			strcpy(logInfo.szLogFilePrefix, "LOG/mmfwc");
												// mmfwc means "Mem File Write Cache"
		}
	};
	// ͳ����Ϣ
	struct	STATINFO_T
	{
		int	nSaveListLen;						// ׼�����̵Ķ�����Ԫ�صĸ���
		int	nCmdNumInQueue;						// ��ǰ�̶߳����е�ָ�������
		int	nCurFragNum;						// ��ǰռ�õ��ļ�������
		int	nCurFileNum;						// ��ǰ���ļ�����
		// ������������ֵ
		int	nSaveListLenMax;
		int	nCmdNumInQueueMax;
		int	nCurFragMax;
		int	nCurFileMax;
	};
public:
	// ��ʼ��(���������ڴ沿�֣����������洢�߳�)
	virtual int	Init(INFO_T *pInfo)				= 0;
	// �ս�(��������ڴ��л����ļ�����̵���Ӧ��Ŀ¼�£��������ʧ�������ڴ治ɾ��)
	// ��ɾ������ǰ�����ֶ�����Release
	virtual int	Release()						= 0;
	// �߼�����(������һ�������ʴ��̵ȵ�)
	virtual int	Tick()							= 0;
	// �ϲ���������һ���ļ�
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
			// �����Ļ���ߴ粻����ֻ�ܶ������޵ĳ���
			vectDataTmp.resize(*pnSize);
		}
		*pnSize	= vectDataTmp.size();
		memcpy(pData, vectDataTmp.getbuf(), *pnSize);
		return	0;
	}
	// �ϲ������洢һ���ļ�
	virtual int	SaveFile(const char *cszFileName, const void *pData, int nSize)
												= 0;
	inline int	SaveFile(const char *cszFileName, whvector<char> &vectData)
	{
		return	SaveFile(cszFileName, vectData.getbuf(), vectData.size());
	}
	// �Զ�д��ʽ���ļ�(����ļ�ԭ���������򴴽�֮)
	// �޸���֮���Լ�ͨ��deleteɾ���ļ��������
	virtual whfile *	OpenFileRW(const char *cszFileName)
												= 0;
	// ɾ���ļ�(ע���ļ����򿪵�ʱ��Ҫɾ������OpenFileRW���ļ�ʹ�����֮ǰ����ɾ����)
	virtual int	DeleteFile(const char *cszFileName)
												= 0;
	// ɾ��Ŀ¼(����Ŀ¼��ʵ�ʵ��ļ�ɾ���ڴ��е��ļ������������ò�Ҫ�ֶ�ɾ��Ŀ¼)
	// ����ע�⣬���ɾ����֧�ִμ�Ŀ¼��
	virtual int	DeleteDir(const char *cszDir)	= 0;

	// ���ڵ���
	// ����д��ʱ��sleep��ô��ʱ��(����ģ��Ӳ��æ�����)
	virtual void	SetSaveSleep(int nSleep)	= 0;

	// ���ͳ����Ϣ
	virtual int		GetStatInfo(STATINFO_T *pInfo)
												= 0;
	// ��cache���ļ����ݵ������ı��ļ�
	virtual int		ExportCacheToText(const char *cszTextFile)
												= 0;
	// ɾ��cacheƥ����ļ�����ɾ�������ϵģ�
	// ����ɾ�����ļ�����
	virtual int		DeleteMatchedFilesInCache(const char *cszFmt)
												= 0;
	// ����ֹͣ������Cache������ļ�(�������ļ���Ȼ����)
	virtual void	SetNotExpand(bool bSet)		= 0;
	// ���NotExpand���趨ֵ
	virtual bool	GetNotExpand() const		= 0;
	// ��������ļ�
	virtual int		Clear()						= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEMFILEWRITECACHE_H__
