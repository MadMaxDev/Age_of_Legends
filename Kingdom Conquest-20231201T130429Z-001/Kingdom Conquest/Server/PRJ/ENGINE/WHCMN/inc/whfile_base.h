// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfile_base.h
// Creator      : Wei Hua (κ��)
// Comment      : �����ļ��ӿڶ���
//              : ����ģ����Ҫ��������������һ��filemanȻ��ʹ�á�֮���Բ���һ����̬��fileman����Ϊ�����������֯�����Ƕ��DLL���������ܵ��¾�̬��filename��ɶ�������»��ҡ�
// CreationDate : 2006-06-23 ��whfile.h�г��
// ChangeLOG    : 2006-10-23 ��WHDirSearchActionWFM::ActionOnDir��Ϊ�Ǵ��麯������Ϊ��ȷ����������Ҫ����Ŀ¼�ġ�
//              : 2007-01-24 ������WHDirSearchActionWFM��ֹͣ�����ı���ָ��

#ifndef	__WHFILE_BASE_H__
#define	__WHFILE_BASE_H__

#include "./wh_platform.h"
#ifdef	WIN32
#define WH_MAX_PATH			MAX_PATH
#endif
#ifdef	__GNUC__
#include <stdlib.h>
#define WH_MAX_PATH			260
#define	S_IREAD				S_IRUSR
#define	S_IWRITE			S_IWUSR
#endif

// ����Ŀ¼����
#define WH_MAX_PATHLEVEL	64

#include <stdio.h>
#include <time.h>
#include "./whvector.h"
#include "./whhash.h"
#include <map>
#include <string>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// ͨ���ļ�����(Ŀǰ����֧�ֶ��̷߳���һ��file����)
////////////////////////////////////////////////////////////////////
class	whfile
{
protected:
	// ��ֹ��ֱ��new
	whfile()			{}
public:
	// ����Ϊ�˼�Ӧ�ÿ���ֱ��ɾ��(ʡȥ��Close������������ò�Ҫ��һ��DLL�ڴ�����������һ��DLLȥɾ��)
	virtual ~whfile()	{}
	// �����delete����ʹ��ʱ������������ɾ���������DLL��ɾ�����ã�
	virtual	void	SelfDestroy()							= 0;
	// ����ļ��ܳ���(����ڹ����ļ����ܱ��ֳɶ��ٿ����ʱ������)
	// Ӧ�ò�����ֳ���2G���ļ�����������ļ���
	virtual int		FileSize() const						= 0;
	// ����ļ�ʱ�䣨�������һ���޸�ʱ�䣩
	virtual time_t	FileTime() const						= 0;
	// �����ļ��޸�ʱ�䣨��д�򿪵�ʱ�䡣������������˼����ļ���д�޸��ˣ���ô�ڹرյ�ʱ���ļ�ʱ��Ҳ���趨��ʱ�䣩
	// ���t==0�����Զ�ȡ��ǰ��ϵͳʱ��
	virtual void	SetFileTime(time_t t=0)					= 0;
	// ����ʵ�ʵõ����ֽ�����0��ʾû�������ˣ�<0��ʾ������
	virtual int		Read(void *pBuf, int nSize)				= 0;
	// �ƶ��ļ�ָ��
	// SEEK�Ĳ�����fseekһ�£��ṩ����seek
	// ����0��ʾ�ɹ���<0��ʾ������
	// nOriginʹ�ú�fseek����һ����SEEK_SET��SEEK_CUR��SET_END
	// ��д�ļ���ʱ��SeekӦ�ÿ��������ļ�
	virtual int		Seek(int nOffset, int nOrigin)			= 0;
	// �ж��Ƿ����ļ�β
	virtual bool	IsEOF()									= 0;
	// ��õ�ǰ�ļ�ָ��λ��
	virtual int		Tell()									= 0;
	// д������(�ͱ�׼�ļ�����һ����Write����д����ֽ���)
	virtual int		Write(const void *pBuf, int nSize)		= 0;
	// ���嵽Ӳ�̣����غ�fflush�Ľ��һ�£�
	virtual int		Flush()									= 0;

	// �ظ�д��һ������
	virtual int		DupWrite(const void *pBuf, int nSize, int nDupNum);

	// ����ļ���md5 (�����zip�е��ļ���ֱ�ӷ��أ�����Ĭ����Ҫ����ȫ���ļ����㣬���������ò�Ҫ��������)
	// MD5Ӧ����16�ֽڵĻ�����
	// ע�⣬����������ܻ�ı��ļ�ָ���λ��!!!!
	virtual int		GetMD5(unsigned char *MD5);

	// ���һ�У������е��ֽ������а���������"\n"��"\r\n"���ļ�����ʲô����ʲô
	// ���һ�еĳ��ȳ�����nSize����ֻ����������ô����������һ��0��������ʣ�±���δ��״̬
	int	ReadLine(char *pBuf, int nSize);

	// ����seek��ͷ
	inline int	Rewind()
	{
		return	Seek(0, SEEK_SET);
	}
	// �����䳤�ߴ�Ķ�д
	// ����
	int	ReadVSize();
	// д��
	int	WriteVSize(int nSize);
	// �䳤���ݶ�д
	// �����䳤����(���ض��������ݲ��ֵĳ���)
	int	ReadVData(void *pData, int nMaxSize);
	// д��䳤���ݣ����ص������ݲ��ֵĳ��ȣ�������<=0��˵���д���
	int	WriteVData(const void *pData, int nSize);
	// �䳤�ִ���д���ִ����ȿ�����1~0x3FFFFFFF��
	// �����䳤�ִ�(���ض����ĳ���)��ע�����Զ��ڽ�β��0��
	int	ReadVStr(char *szStr, int nMaxSize);
	// д��䳤�ִ�(����д��ĳ���)
	// ���nMaxSize>0��˵����Ҫ��֤szStr�ĳ��Ȳ�С��nMaxSize
	int	WriteVStr(const char *szStr, int nMaxSize=-1);
	// ����������͵Ķ�д
	template<typename _Ty>
		inline int	Read(_Ty *pBuf)
	{
		return	Read(pBuf, sizeof(*pBuf));
	}
	template<typename _Ty>
		inline int	Write(const _Ty *pBuf)
	{
		return	Write(pBuf, sizeof(*pBuf));
	}
public:
	// ���ϵͳ�ļ�ָ��(����еĻ��������Ҫ��������)
	virtual FILE *	GetFILE() const
	{
		return	NULL;
	}
};

////////////////////////////////////////////////////////////////////
// �ɵ�ͨ��Ŀ¼������ࣨΪ�˺��ϵ�whzlib�������Ա�����
////////////////////////////////////////////////////////////////////
enum
{
	WHDIR_MAX_NAME	= WH_MAX_PATH,
};
class	WHDirBase
{
public:
	struct	ENTRYINFO_T
	{
		bool	bIsDir;
		char	szName[WHDIR_MAX_NAME];			// һ������Ӧ�ò��ᳬ�����ֵ
		union
		{
			struct
			{
				int		nID;					// �ļ����
			}pck;								// ֻ�ڴ���ļ�����Ч����Ϣ
		}ext;
	};
	WHDirBase	*m_pNext;						// ���һ��Ŀ¼�����ɶ��������ɣ�������ָ����һ��
public:
	WHDirBase() : m_pNext(NULL)					{}
	virtual ~WHDirBase()						{}
	virtual	void	SelfDestroy()				= 0;
	// ��û���dir
	virtual const char *	GetDir() const		= 0;
	// ����һ����Ŀ(����NULL��ʾû�пɶ�����)
	// ENTRYINFO_T�����ͷţ���һ���ڲ��������ϲ����Ҳ�����޸����е����ݣ�����Ҫע����ֻ��һ��!!!!
	virtual ENTRYINFO_T *	Read()				= 0;
	// ��ͷ��ʼ��
	virtual void			Rewind()			= 0;
};
class	WHDirChain	: public WHDirBase
{
protected:
	WHDirBase	*m_pHeadDir;
	whvector<ENTRYINFO_T>	m_vectEntry;
	int			m_nCurIdx;
public:
	explicit WHDirChain(WHDirBase *pDir)
		: m_pHeadDir(NULL)
		, m_nCurIdx(0)
	{
		m_vectEntry.reserve(64);
		SetHeadDir(pDir);
	}
	void	SetHeadDir(WHDirBase *pDir)
	{
		std::map< std::string, ENTRYINFO_T >	mapEntry;
		// ��ʱ��Ͷ�һ��
		m_pHeadDir	= pDir;
		m_pNext		= m_pHeadDir;
		while( m_pNext )
		{
			ENTRYINFO_T	*pRst	= m_pNext->Read();
			if( !pRst )
			{
				m_pNext	= m_pNext->m_pNext;
			}
			else
			{
				mapEntry.insert(std::make_pair(pRst->szName, *pRst));
			}
		}
		for(std::map< std::string, ENTRYINFO_T >::iterator it=mapEntry.begin(); it!=mapEntry.end(); ++it)
		{
			m_vectEntry.push_back((*it).second);
		}
		m_nCurIdx	= 0;
	}
	virtual ~WHDirChain()
	{
		// ÿ����ɾ��һ��
		m_pNext		= m_pHeadDir;
		while( m_pNext )
		{
			WHDirBase	*pTmp	= m_pNext;
			m_pNext	= m_pNext->m_pNext;
			pTmp->SelfDestroy();
		}
		m_pHeadDir	= NULL;
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	// ��û���dir
	virtual const char *	GetDir() const
	{
		return	m_pHeadDir->GetDir();
	}
	// ����һ����Ŀ(����NULL��ʾû�пɶ�����)
	// ENTRYINFO_T�����ͷţ���һ���ڲ��������ϲ����Ҳ�����޸����е����ݣ�����Ҫע����ֻ��һ��!!!!
	virtual ENTRYINFO_T *	Read()
	{
		if( m_nCurIdx>=(int)m_vectEntry.size() )
		{
			return	NULL;
		}
		return	m_vectEntry.getptr(m_nCurIdx++);
	}
	// ��ͷ��ʼ��
	virtual void			Rewind()
	{
		m_nCurIdx	= 0;
	}
};

////////////////////////////////////////////////////////////////////
// ͨ���ļ�ϵͳ���ࣨwhfile��WHDirBase�Ĵ���Ӧ����ͨ���������
////////////////////////////////////////////////////////////////////
class	whfileman
{
public:
	virtual	~whfileman()		{};
	// �����delete����ʹ��ʱ������������ɾ���������DLL��ɾ�����ã�
	virtual	void	SelfDestroy()				= 0;
	// Ŀǰ��ģʽֻ�ṩ�����Ʒ�ʽ
	enum OP_MODE_T
	{
		OP_MODE_UNDEFINED		= 0,			// δ�����ģʽ
		OP_MODE_BIN_READONLY	= 1,			// ֻ��
		OP_MODE_BIN_CREATE		= 2,			// ��д����������ھʹ�����
		OP_MODE_BIN_READWRITE	= 3,			// ��д����������ھͳ���
		OP_MODE_BIN_RAWREAD		= 4,			// ֻ������ȫ�ѵײ����ݶ������������н��ܻ��߽�ѹ����Ҫ�������ݳ�����ԣ�
	};
	// ������ļ�����Ŀ¼��ǰ�����пո񣬴���ǰӦ��trimһ�£�����Ŀ¼���������'/'��'\'
	virtual	whfile *		Open(const char *cszFName, OP_MODE_T mode)	= 0;
	virtual	WHDirBase *		OpenDir(const char *cszDName)				= 0;	// ����Ǵ򿪵�ǰĿ¼һ��Ҫ��"."������ʲô����д
	virtual	int				MakeDir(const char *cszDName)				= 0;	// ���Ŀ¼���ھ�ֱ�ӷ��أ����򴴽�Ŀ¼
	virtual	int				SureMakeDirForFile(const char *cszFName)	= 0;	// ��֤�ļ���Ҫ��Ŀ¼����
	virtual	bool			IsPathExist(const char *__path)				= 0;	// �ж�·���Ƿ����
	virtual	bool			IsPathDir(const char *__path)				= 0;	// �ж�·���Ƿ���Ŀ¼
	struct	PATHINFO_T
	{
		enum
		{
			TYPE_DIR	= 0x01,										// ��Ŀ¼
			TYPE_SYS	= 0x02,										// ��ϵͳ�ļ�
			TYPE_HIDE	= 0x04,										// �������ļ�
			TYPE_RDOL	= 0x08,										// ��ֻ���ļ�
		};
		unsigned char	nType;										// �ڵ�����
		time_t			nMTime;										// �޸�ʱ��
		union	UN_T
		{
			struct	FILE_T
			{
				size_t	nFSize;										// �ļ�����
			}			file;
		}				un;
	};
	virtual	int				GetPathInfo(const char *cszPath, PATHINFO_T *pInfo)
																	= 0;
																	// ���·��������ļ���Ŀ¼����Ϣ
	virtual int				DelFile(const char *cszPath)			= 0;	
																	// ɾ���ļ�
	virtual int				DelDir(const char *cszPath)				= 0;	
																	// ɾ����Ŀ¼

	// �жϴ�ģʽ����ֻ����
	static bool	IsReadOnlyMode(OP_MODE_T nOPMode)
	{
		return	(nOPMode == OP_MODE_BIN_READONLY)
			||  (nOPMode == OP_MODE_BIN_RAWREAD)
			;
	}
};
// ����һ����ͨ�ļ�ϵͳ���ļ�������
whfileman *	whfileman_Cmn_Create();

struct	EXTINFO_UNIT_T
{
	const char	*cszBegin;
	int			len;
};

////////////////////////////////////////////////////////////////////
// �����Ŀ¼��ص�����ʵ����whdir.cpp��
////////////////////////////////////////////////////////////////////
// ����whfilemanʵ�ֵ�Ŀ¼������ܣ�WFM��WithFileMan����˼��
class	WHDirSearchActionWFM
{
public:
	WHDirSearchActionWFM(whfileman *pFM)
		: m_pFM(pFM)
		, m_pbStop(NULL), m_nRootPathLen(0), m_nPathLevel(0)
		, m_bShouldNotGoInThisDir(false)
	{
	}
	virtual ~WHDirSearchActionWFM()	{}
	virtual void	SelfDestroy()	{delete this;}
	// szPath�������"/"��"\"
	int		DoSearch(const char *szPath, bool bRecur);
	inline void	SetStopSearchVar(bool *pbStop)
	{
		m_pbStop	= pbStop;
	}
	inline void	SetFileMan(whfileman *pFM)
	{
		m_pFM	= pFM;
	}
	// �������������Ͷ�ȡ�Ĺ���
	void	SetFilter(const char *cszFilter);
	// �������������Ͳ���ȡ�Ĺ���
	void	SetBadFilter(const char *cszFilter);
	// ���һ������ȡ���ļ�(ע�⣬�ļ��Ǵ�Сд���е�)
	void	AddSkipFile(const char *cszFile);
protected:
	// һ���������������е�������һϵ�еݹ�������������
	int		DoOneDir(const char *szDir, bool bRecur);
	// ���ȫ·����
	const char *	GetFullPath(const char *szPath, char *szFullPath);
	// ʹ������������������GetLastFullPath�õ�ͬ���ķ���
	inline const char *	GetFullPath(const char *szPath)
	{
		return	GetFullPath(szPath, m_szFullPath);
	}
	inline const char *	GetLastFullPath() const
	{
		return	m_szFullPath;
	}
	// ��õ�ǰĿ¼�����ʼĿ¼���ļ���(ǰ������󲻴�"/"��"\"������ֱ�Ӽӵ����·������)
	inline const char *	GetCurRelPath() const
	{
		return	m_szCurPath + m_nRootPathLen;
	}
	// ��ʹ����GetFullPath(const char *szPath)֮�����������õ����·��
	inline const char *	GetFullRelPath() const
	{
		return	m_szFullPath + m_nRootPathLen;
	}
	inline int	GetPathLevel() const
	{
		return	m_nPathLevel;
	}
protected:
	// �ļ�������
	whfileman	*m_pFM;
	// �Ƿ�ֹͣ�����ָ���ʾֹͣ����ı�����
	bool	*m_pbStop;
	// ��ʼ·�����ַ�����
	int		m_nRootPathLen;
	// ��ŵ�ǰ���ڹ�����·��
	char	m_szCurPath[WHDIR_MAX_NAME];
	// ������ʱ���ȫ·����
	char	m_szFullPath[WHDIR_MAX_NAME];
	// ��ǰ��Ŀ¼���
	int		m_nPathLevel;
	// ��ActionOnDir�����ã������Ϊ�棬���ü����������Ŀ¼
	bool	m_bShouldNotGoInThisDir;
	// ��׺ƥ��ģʽ
	// Ҫ��
	whvector<EXTINFO_UNIT_T>	m_vectGoodFilter;
	// ��Ҫ��
	whvector<EXTINFO_UNIT_T>	m_vectBadFilter;
	// �����������ļ��б�
	whstrinvectorhashset		m_setFileToSkip;
private:
	// ��������������������һ��·����������Ը�Ŀ¼��·����������󲻻���"/"��"\"
	// �����ڲ�����ͨ��GetFullPath()�õ�ȫ·����
	// �������������κ�һ������-1����������������ֹ
	// ���ļ��Ĳ���
	virtual int		ActionOnFile(const char *szFile)	= 0;
	// ��Ŀ¼�Ĳ���(������ڴ򿪸�Ŀ¼֮ǰ����)
	virtual int		ActionOnDir(const char *szDir)		{return	0;}
	// һ��Ŀ¼�������
	// ������������������ǣ�ȫ·����������󲻻���"/"��"\"
	// ����Ŀ¼
	virtual int		EnterDir(const char *szDir)			{return 0;}
	// ��Ŀ¼���˳�����ʱĿ¼�Ѿ��رգ����Ҫɾ���ÿ��������
	virtual int		LeaveDir(const char *szDir)			{return 0;}
};

// ͳ��һ��Ŀ¼�е��ļ����������ֽ�����Ŀ¼��������Ϣ
struct	DIR_STAT_T
{
	int	nTotalFile;
	int	nTotalFileByte;
	int	nTotalDir;
	struct	notify			// ����֪ͨ�ӿ�
	{
		virtual	~notify()	{};
		virtual	void		SelfDestroy()							= 0;
		virtual	int			cb_DStat_File(const char *cszFName)		= 0;
		virtual	int			cb_DStat_Dir(const char *cszDName)		= 0;
		virtual	int			cb_DStat_Over()							{return 0;}
		static notify *		CreateCmnConsole();
	};
	notify	*pNotify;
	bool	*pbShouldStop;

	DIR_STAT_T();
	void	reset();		// �������¿�ʼͳ��
};
// ���bQuickΪfalse�������������ļ�����ȡ��Ϣ������֮���ļ�ϵͳ��GetPathInfo��������ȡ��Ϣ
int	WHDirSearchGetStatistics(whfileman *pFM, const char *cszDir, bool bQuick, DIR_STAT_T *pStat);

// ��ʱ��·���ִ����ͳһ�Ķ���
class	whPathStrTransformer
{
protected:
	const char		*m_pcszStr;
	whvector<char>	m_vectStr;
public:
	whPathStrTransformer(const char *szStr)
		: m_pcszStr(NULL)
	{
		SetStr(szStr);
	}
	inline const char * GetPtr() const
	{
		return	m_pcszStr;
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
protected:
	// �����ִ�
	void	SetStr(const char *szStr);
};
#define WHPATHSTRTRANSFORMER	(const char *)whPathStrTransformer

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILE_BASE_H__
