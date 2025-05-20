// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfile.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵��ڴ��ļ�����
//                ���ڴ��е�һ���Ԥ������õ����򹹳�
//                �����򽫱�����Ϊ�������֣�
//                1. ��Ϣͷ��MEMINFO_T�������¼��ȫ�ֵ���Ϣ
//                2. �ļ���Ϣ����������whalist�����һ���ڴ棬�洢�����ļ���Ϣ
//                3. �ļ�������������whalist�����һ���ڴ棬�洢�����ļ�����Ϣ
//                ǿ��ע�⣬������ļ�������Ҫɾ�������������û���жϵġ������������
// CreationDate : 2004-01-18
// ChangeLOG    : 2004-11-07 �޸���FILEINFO_T��nProperty�ķ�Χ����
//              : 2005-02-10 ȡ����whmemfile�е�MODE_XXX����

#ifndef	__WHMEMFILE_H__
#define	__WHMEMFILE_H__

#include "whhash.h"
#include "whlist.h"
#include "whtime.h"
#include "whbits.h"
#include "whfile_base.h"

namespace	n_whcmn
{

class	whmemfile;
class	whmemfileman;

// �ڴ��ļ�������
class	whmemfileman
{
	friend	class whmemfile;
public:
	enum
	{
		MODE_READ		= 0,			// ֻ��
		MODE_CREATE		= 1,			// �����ļ�(ԭ���о���գ��ȿɶ��ֿ�д)
		MODE_READWRITE	= 2,			// ��д��(�����ԭ���ģ�ԭ��û�оͳ����ȿɶ��ֿ�д)
	};

	#pragma pack(1)						// BEGIN OF pack	{
	struct	INFO_T						// ��ʼ����Ϣ
	{
		void		*pBuf;				// ��Ҫ��ʹ�õ��ڴ���
		size_t		nMaxFile;			// ���������ɵ��ļ���
		size_t		nFragSize;			// �ļ���ĳߴ�
		size_t		nTotalFrag;			// �����ļ�������
										// nFragSize * nTotalFrag�������ļ����ֵ���������ɵĴ�С (��һ�㶼���˷ѵ������)
	};
	struct	MEMINFO_T					// pBuf���ᱻת��Ϊ���ͷ
	{
		// ����������
		size_t		offFileSect;		// �ļ���Ϣ���Ŀ�ͷƫ����
		size_t		offFragSect;		// �ļ������Ŀ�ͷƫ����
		// 2005-02-18 ԭ�������ĳ�����64��int��512�ֽڣ����������������Ϣ
		// ����ΪnExtInfo���Ժ��Լ���Ӧ��������Ӧ�ĵ�Ԫ(ע��һ��Ӧ�����±겻�ظ�����)
		enum
		{
			EXTIDX_PROPERTY		= 63,	// ���һ���������Լ���;�����һЩ�ڲ�����
		};
		enum
		{
			PROPERTY_MARKCLEAN	= 1,	// (�ڽ�����ʱ�����)�����ڽ����Ƿ���ʱ�������Ϳ��Բ��̳���
		};
		int			nExtInfo[64];		// �������洢
	};
	struct	FILEINFO_T					// �ļ���Ϣ��
	{
		enum
		{
			PROPERTY_WR		= 0x0001,	// ��д�룬û�йر�(��������ڳ����������¼̳к����Ҫ���飬���ܷ���)
			PROPERTY_DEL	= 0x0002,	// ��Ǳ�ɾ��
		};
		int				nIdx;			// �ļ�����������m_alFileInfo�е�����
		int				nName;			// �ļ�����Ϊ�˼򵥾�ȡ����
		unsigned int	nProperty;		// ��������(�ο������PROPERTY_XXX����16λλwhmemfilemanʹ�ã���16λΪӦ�ó������ж���)
		int				nHeadIdx;		// �ļ���ʼ�������
		int				nTailIdx;		// �ļ���β�������
		size_t			nSize;			// �ļ�����
		unsigned		nCRC32;			// CRCУ��(���һ������д�ļ��رպ�д��ģ�����ļ�û�������رգ�����ҪУ��)
		time_t			nLastModify;	// ���һ�θ����ļ���ʱ��(������ļ��ر�ʱ�ż�¼)
		unsigned char	MD5[16];		// �ϴε�MD5ֵ(����������ڹر�ʱ�Զ���ģ�һ�㶼���㣬������Ҫ��)
		unsigned char	tmpMD5[16];		// ��ʱ�Ĵ��MD5ֵ
		void *			aExtPtr[4];		// ����ָ��(�ϲ��������ã����ڱ�����ļ�����صĶ���)
										// !!!! ע�⣺����ļ�ԭ�����ڣ���create����Ӱ��aExtPtr������!!!!
		void	clear()					// ���ļ�״̬���Ϊ��ʼ״̬
		{
			// nIdx��nName����Ҫ�ϲ��ڷ���֮����д�ģ����ܱ�ģ�����Ҳ�����������
			nProperty	= 0;
			nHeadIdx	= whalist::INVALIDIDX;
			nTailIdx	= whalist::INVALIDIDX;
			nSize		= 0;
			nCRC32		= 0;
			nLastModify	= 0;
			memset(MD5, 0, sizeof(MD5));
			memset(tmpMD5, 0, sizeof(tmpMD5));
			// memset(aExtPtr, 0, sizeof(aExtPtr)); !!!!�������գ�ֻ���ϲ��ֶ����(ֻ���ڴ��޵��е�ʱ����һ�Σ��Ժ�ͱ���)��
		}
	};
	// ͳ��
	struct	STATISTIC_T
	{
		int				nFileNum;		// ���е��ļ�����
		int				nFileMax;		// �ܹ����Է�����ļ�����
		int				nFragNum;		// �ļ�������
		int				nFragMax;		// �ܹ����Է�����ļ�������
	};
	#pragma pack()						// BEGIN OF pack	}

protected:
	INFO_T				m_info;
	MEMINFO_T			*m_pMemHdr;		// �ϲ�ϵͳ�ṩ���ڴ濪ʼ��
	whhash<int, int>	m_mapFileName2Idx;
										// �ļ�����FILEINFO_T��Ԫ��ŵĶ�Ӧ��ϵ
	whalist				m_alFileInfo;	// ���ڴ������FILEINFO_T��Ԫ
	whalist				m_alFragInfo;	// ���ڴ����ļ���
public:
	// ͨ�������ļ���֪���ļ���Ϣ������Ҫ�ĳߴ�
	// ͨ��Ƭ�ߴ����Ƭ��֪��������Ҫ�ĳߴ�
	// Ȼ�����MEMINFO_T�ߴ緵���ܵ���Ҫ�ĳߴ�
	// pInfo�е�pBuf����Ҫ�ܣ����������ȥ����(ֱ�������ڴ���ߴӹ����ڴ����붼����)
	static size_t	CalcMemSizeNeeded(INFO_T *pInfo);
	inline size_t	CalcMemSizeNeeded()
	{
		return	CalcMemSizeNeeded(&m_info);
	}
public:
	whmemfileman();
	virtual ~whmemfileman();
	// ��ʼ��(��Ϊ���涼���Զ�������ϲ������ڴ棬���Բ���Ҫ��Ӧrelease��)
	int		Init(INFO_T *pInfo);
	// ������ж������ָ���ʲôҲû�е�״̬
	inline void	Reset()
	{
		Init(&m_info);
	}
	// ��������ļ�
	// ע�⣬�������ֻ����Init����Inherit֮�����
	inline void	MarkClean()
	{
		whbit_ulong_set((unsigned int *)&m_pMemHdr->nExtInfo[MEMINFO_T::EXTIDX_PROPERTY], MEMINFO_T::PROPERTY_MARKCLEAN);
	}
	inline bool	IsMarkedClean() const
	{
		return	whbit_ulong_chk((unsigned int *)&m_pMemHdr->nExtInfo[MEMINFO_T::EXTIDX_PROPERTY], MEMINFO_T::PROPERTY_MARKCLEAN);
	}
	// ��һ�����е��ڴ�̳й���(Init��Inheritֻ��Ҫ����һ����������ɳ�ʼ������)
	int		Inherit(void *pBuf);
	// �ж�һ���ļ��Ƿ����
	bool	IsFileExist(int nFileName) const;
	// �����ļ�����
	// ����<0��ʾ�ļ�������
	int		GetFileSize(int nFileName);
	// ��ȡ�ļ���Ϣ�ṹ
	FILEINFO_T *	GetFileInfo(int nFileName);
	inline FILEINFO_T *	GetFileInfoByIdx(int nIdx)
	{
		return	(FILEINFO_T *)m_alFileInfo.GetDataUnitPtr(nIdx);
	}
	// ��һ���ļ�(whmemfile�رձ���ͨ��Close������ֱ��ɾ��)
	whmemfile *	Open(int nFileName, int nMode=MODE_READ);
	// OpenByIdx��pNameֻ��nIdx��������nModeΪд��ʽ��ʱ�����Ҫ
	inline whmemfile *	OpenByIdx(int nIdx, int nMode, void *pName)
	{
		return	OpenFile(GetFileInfoByIdx(nIdx), nMode, pName);
	}
	// �ر�һ���ļ�(�رպ�whmemfile����ʧЧ)
	// ʹ��Close��ԭ������Ϊ����д�򿪵��ļ���Ҫ�ڹر�ʱ����ļ�״̬
	// ����ע�⣬�Ҳ����Զ��ͷ�û�йرյ��ļ�
	// ���pFileΪNULL��������ڲ��������κβ���
	int		Close(whmemfile *pFile);
	// ɾ��һ���ļ�
	int		Delete(int nFileName);
	inline int		DeleteByIdx(int nIdx)
	{
		return	DeleteFile(GetFileInfoByIdx(nIdx));
	}
	// ���ɾ��������ļ�
	int		MarkDelAndClean(int nFileName);
	// ����ͳ����Ϣ
	void	GetStatistic(STATISTIC_T *pStat) const;
	// ��ô��ڵ��ļ����б�
	void	GetFileNameList(whvector<int> &vectList) const;
	// ��ô��ڵ��ļ�����
	inline int		GetFileNum() const
	{
		return	m_alFileInfo.GetHdr()->nCurNum;
	}
	// ��û����Դ������ļ�����
	inline int		GetFileNumLeft() const
	{
		return	m_alFileInfo.GetHdr()->nMaxNum - m_alFileInfo.GetHdr()->nCurNum;
	}
	// ����ļ������
	inline int		GetFragNum() const
	{
		return	m_alFragInfo.GetHdr()->nCurNum;
	}
	// ����ڴ�ͷ
	inline MEMINFO_T *	GetMemInfo()
	{
		return	m_pMemHdr;
	}
protected:
	// �����ļ���CRC
	unsigned	CalcCRC32(FILEINFO_T *pFileInfo) const;
	// ����ļ�����
	int			EmptyFile(FILEINFO_T *pFileInfo);
	// ɾ���ļ�
	int			DeleteFile(FILEINFO_T *pFileInfo);
	// ���ļ�(pName�Ǹ�����Ϣ������nFileName)
	whmemfile *	OpenFile(FILEINFO_T *pFileInfo, int nMode, void *pName);
private:
	// Init�ĸ��Ӳ���
	virtual int	Init_Ext()
	{
		return	0;
	}
	// Inherit�ĸ��Ӳ���
	virtual int	Inherit_Ext()
	{
		return	0;
	}
	// ��Inheritһ���ļ��ɹ��󣬸���Ҫ��������
	virtual int	Inherit_AfterGood(FILEINFO_T *pFInfo, int nIdx)
	{
		m_mapFileName2Idx.put(pFInfo->nName, nIdx);
		return	0;
	}
	// �������ֶ��ļ���Ϣ��Ԫ��ӳ��
	virtual int	Open_SetNameMap(void *pName, FILEINFO_T *pFInfo)
	{
		pFInfo->nName	= (int)pName;
		m_mapFileName2Idx.put(pFInfo->nName, pFInfo->nIdx);
		return	0;
	}
	// ��DeleteFile����ɾ����������֮ǰ��������
	virtual int	DeleteFile_BeforeRealDel(FILEINFO_T *pFInfo)
	{
		m_mapFileName2Idx.erase(pFInfo->nName);
		return	0;
	}
};

// �ڴ��ļ������൱��FILE
class	whmemfile
{
	friend	class whmemfileman;
protected:
	whmemfileman				*m_pMan;
	whmemfileman::FILEINFO_T	*m_pFileInfo;
										// �ڹ������ж�Ӧ�Ķ���ָ�룬��ʱ��ɾ����ͨ�������
	int				m_nOpenMode;		// �ļ��Ĵ�ģʽwhmemfileman::MODE_XXX
	int				m_nCurOffset;		// ��ǰ�ļ�ָ���λ��
	int				m_nCurBlockIdx;		// ��ǰ�ļ��������
public:
	whmemfile();
protected:
	~whmemfile();						// �����Ͳ�����ֱ��ɾ��
public:
	typedef	whmemfileman::FILEINFO_T	FILEINFO_T;
	// �������ݣ����ض����ĳ���
	size_t	Read(void *pBuf, size_t nSize);
	// д�����ݣ�����д��ĳ���
	size_t	Write(const void *pBuf, size_t nSize);
	// Seek�Ĳ�����fseek��ȫ��ͬ
	int		Seek(int nOffset, int nOrigin);
	// ��ʣ�µ��ļ����ݶ���һ��vector
	size_t	ReadToVector(whvector<char> &vect);
	// �����ݱȽ�
	int		CmpWithData(const void *pData, int nSize);
	// �ж��ļ��Ƿ��˽�β
	inline bool	IsEOF() const
	{
		return	m_nCurOffset == (int)m_pFileInfo->nSize;
		//return	m_nCurBlockIdx == whalist::INVALIDIDX 
		//	||	m_nCurOffset == (int)m_pFileInfo->nSize
		//	;
	}
	// ����ļ���Ϣͷ
	inline FILEINFO_T *	GetInfo() const
	{
		return	m_pFileInfo;
	}
	// ����ļ�����
	inline size_t	GetFileSize() const
	{
		return	m_pFileInfo->nSize;
	}
	// ��õ�ǰ�ļ�ָ���λ��
	inline size_t	Tell() const
	{
		return	m_nCurOffset;
	}
	// �ر��ļ�(�ڲ�ʵ�����ǵ���m_pMan->Close��������ֻ��Ϊ�˷���)
	// ע�⣬Close֮��thisָ���ʧЧ��!!
	inline int	Close()
	{
		return	m_pMan->Close(this);
	}
	// �����֪���͵ļ򵥶�д
	template<typename _Ty>
	inline size_t	Read(_Ty *pData)
	{
		return	Read(pData, sizeof(*pData));
	}
	template<typename _Ty>
	inline size_t	Write(_Ty *pData)
	{
		return	Write(pData, sizeof(*pData));
	}
	// ���whvector�Ķ�д��һ�ζ�����
	size_t	Read(whvector<char> &vectData)
	{
		int	nSize	= GetFileSize() - Tell();
		if( nSize>=0 )
		{
			vectData.resize(nSize);
			return	Read(vectData.getbuf(), nSize);
		}
		return	0;
	}
	inline size_t	Write(whvector<char> &vectData)
	{
		return	Write(vectData.getbuf(), vectData.size());
	}
};

// �����ֵ��ļ�������
// !!!!ע�⣺��Ҫʹ��whmemfileman�еķ���
class	whmemfilemanWithName	: public whmemfileman
{
public:
	#pragma pack(1)
	struct	FILENAME_T
	{
		char	szFName[WH_MAX_PATH];			// �ļ���
		// �����Ա���Ǹ��ϲ�ʹ�õġ�whmemfileman�ڲ���ʹ����Щ��
		int		nInWhatList;					// �����ĸ�˫��������(-1��ʾû��)
		int		nPrevIdx, nNextIdx;				// �����������ϲ�����ʵ��˫������
		int		nExt[8];						// �����������
		void	clear()							// ���ļ�״̬���Ϊ��ʼ״̬
		{
			szFName[0]	= 0;
			nInWhatList	= -1;
			nPrevIdx	= whalist::INVALIDIDX;
			nNextIdx	= whalist::INVALIDIDX;
			memset(nExt, 0, sizeof(nExt));
		}
	};
	#pragma pack()
protected:
	whalist		m_alFileName;					// ���ڴ�Ÿ����ļ���������m_alFileInfo��Ԫ������һ�¡��������ͳһ�ж����������������IDӦ��Ҳһ�¡�
	whhash<whstrptr4hash, int>	m_mapFileNameStr2Idx;
												// �ļ����ִ���FILEINFO_T��Ԫ��ŵĶ�Ӧ��ϵ
public:
	// ������Ҫ�ĳߴ�
	// �ʼ��whmemfileman��Ҫ�ģ�������whmemfilemanWithName��Ҫ��
	// ��FILENAME_T����
	static size_t	CalcMemSizeNeeded(INFO_T *pInfo);
	inline size_t	CalcMemSizeNeeded()
	{
		return	whmemfilemanWithName::CalcMemSizeNeeded(&m_info);
	}
public:
	// ��һ���ļ�(whmemfile�رձ���ͨ��Close������ֱ��ɾ��)
	whmemfile *	Open(const char *cszFileName, int nMode=MODE_READ);
	// ɾ��һ���ļ�
	int		Delete(const char * cszFileName);
	// �ļ��Ƿ����
	bool	IsFileExist(const char * cszFileName) const;
	// ��ȡ�ļ���Ϣ�ṹ
	FILEINFO_T *	GetFileInfo(const char * cszFileName);
	inline FILENAME_T *	GetFileNameByIdx(int nIdx)
	{
		return	(FILENAME_T *)m_alFileName.GetDataUnitPtr(nIdx);
	}
	// ��ô��ڵ��ļ��������б�
	void	GetFileIdxList(whvector<int> &vectList) const;
private:
	virtual int	Init_Ext();
	virtual int	Inherit_Ext();
	virtual int	Inherit_AfterGood(FILEINFO_T *pFInfo, int nIdx);
	virtual int	Open_SetNameMap(void *pName, FILEINFO_T *pFInfo);
	virtual int	DeleteFile_BeforeRealDel(FILEINFO_T *pFInfo);
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEMFILE_H__
