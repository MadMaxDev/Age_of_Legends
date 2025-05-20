// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_util.h
// Creator      : Wei Hua (κ��)
// Comment      : �ļ���������
// CreationDate : 2004-09-16
//              : 2006-09-30 ������һЩ�߼�����Ҫ�Ĺ���
//              : 2007-03-09 ������whsmppasswd
//              : 2007-03-21 ��whsmppasswd����������Ϊ�վͲ��������Ĺ���
//              : 2007-04-28 ������bLowerCaseAccount���Ա�֤�ʺű���ΪСд��Ҳ���ʺŴ�Сд�����У�

#ifndef	__WHFILE_UTIL_H__
#define	__WHFILE_UTIL_H__

#include "whfile.h"
#include "whhash.h"
#include "whstring.h"
#include "whdataini.h"
#include "whmd5.h"
#include "whrsa.h"
#include <vector>

namespace n_whcmn
{

// ���ļ�������(ֻ��)
class	whsmpfilecache
{
private:
	#pragma pack(1)
	struct	FILEUNIT_T
	{
		char	szFileName[WH_MAX_PATH];					// �ļ���
		size_t	nFileSize;									// �ļ����ݳ���
		char	data[1];									// �ļ�����
	};
	#pragma pack()
	whhash<whstrptr4hash, FILEUNIT_T *>	m_mapFileName2Unit;	// �ļ������ļ������ӳ��
public:
	whsmpfilecache();
	~whsmpfilecache();
	int		LoadFile(const char *cszFile, const char *cszFileInMem=NULL);
															// ��Ӳ��������һ���ļ�(���cszFileInMem�ǿ������ڴ������������)
	int		UnLoadFile(const char *cszFileInMem);			// ��������ļ��ͷ�
	int		UnLoadAllFiles();								// �ͷ������ļ�
	whfile *	OpenFile(const char *cszFile, bool bTryDisk);
															// ���ڴ����Զ���ʽ���ļ�(�ļ�����ʹ����Ϻ��ϲ�����ɾ������)
															// bTryDiskΪ��������ڴ���û�о�ֱ�Ӵ�Ӳ���ϵ��ļ�
	bool	HasFileInMem(const char *cszFile);				// �ڴ���������ļ�
};

// �ö��ļ���ɵĴ��ı�����
class	whlinelistinmultifile
{
protected:
	char	m_szPrefixFmt[WH_MAX_PATH];						// �ļ���ģʽ
	char	m_szCurFile[WH_MAX_PATH];						// ��ǰ���ڹ�ע���ļ�
	whvector<char>	m_vectfiledata;							// �洢�ļ�����
	int		m_nFileIdx;										// �ļ����(0��ʾ��û�п�ʼ����-1��ʾ�Ѿ������˽�β)
public:
	whlinelistinmultifile();
	~whlinelistinmultifile();
	void	SetPrefixFmt(const char *cszFmt);				// ���ù���ͷ
	void	Reset();										// ���¿�ʼ��
	int		SaveFile();										// ���ļ���ʣ�ಿ�ִ���
	int		GetRemoveLine(char *szLine, int nSize);			// ������ɾ����(���ض����ĳ��ȣ�������β��\n��\r\n)
															// ���û�ж����ɶ��򷵻�0
};

// �����ļ������ļ��û�������У��ϵͳ
class	whsmppasswd
{
public:
	struct	INFO_T
	{
		bool	bPassMD5;									// �ڴ����Ƿ�ʹ��MD5������������
		bool	bLowerCaseAccount;							// ��֤�ʺ���ΪСд
		char	szPasswordFile[WH_MAX_PATH];				// �û��������ļ�����":"���ָ��û��������룩�����Ϊ���ִ����ʾû������
		INFO_T()
			: bPassMD5(true)
			, bLowerCaseAccount(true)
		{
			szPasswordFile[0]	= 0;
		}
	};
	// ���Ϊ�˿���ͨ�������ļ��Զ����
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
private:
	INFO_T		m_info;										// ������Ϣ
	time_t		m_nLastModifyTime;							// �����ļ��ϴ��޸ĵ�ʱ��
	size_t		m_nLastSize;								// �����ļ��ϴεĳ���
	whhash<whstr4hash, whstr4hash>	m_mapName2Pass;			// ȷ�е��û����������ӳ���
	struct		LIKEUNIT_T
	{
		whstrlike	*pLike;									// ���ڶ���ƥ��
		whvector<char>	vectPass;							// �������
		LIKEUNIT_T()
			: pLike(NULL)
		{
		}
		LIKEUNIT_T(int nSize)
			: pLike(NULL)
			, vectPass(nSize)
		{
		}
		~LIKEUNIT_T()
		{
			clear();
		}
		void	clear()
		{
			if( pLike )
			{
				delete	pLike;
				pLike	= NULL;
			}
			vectPass.clear();
		}
	};
	whvector<LIKEUNIT_T *>			m_vectLikeUnitPtr;		// ����ģʽƥ��Ķ��������
public:
	inline int	GetNum() const								// �����ͨ���û�����ӳ��ĸ���
	{
		return	m_mapName2Pass.size();
	}
	inline int	GetPatternNum() const						// ���ģʽƥ��ĸ���
	{
		return	m_vectLikeUnitPtr.size();
	}
public:
	whsmppasswd();
	~whsmppasswd();
	int			Init(INFO_T *pInfo);
	int			Release();
	int			FreeAll();									// �ͷ����е�ӳ��
	enum
	{
		CHECK_RST_OK					= 0,				// ��֤�ɹ�
		CHECK_RST_SYSERR				= -1,				// ϵͳ���󣨱����ļ���ʧ�ܣ�
		CHECK_RST_USER_NOTFOUND			= -2,				// �û�������
		CHECK_RST_BAD_PASS				= -3,				// �������
	};
	int			Check(const char *cszUser, const char *cszPass);
															// ���û��������������֤
	int			CheckGetPass(const char *cszUser, const char *&pcszPass);
															// �����û���������루���غ�Check��һ�£�����CHECK_RST_OK��ʱ��˵����������룩
	enum
	{
		UPDATE_RST_OK					= 0,				// ������û�и��£�
		UPDATE_RST_NEW					= 1,				// �ļ��������ݣ�������
		UPDATE_RST_STAT_ERR				= -1,				// �ļ�stat��������
		UPDATE_RST_OPEN_ERR				= -2,				// �ļ��򿪳���
		UPDATE_RST_READ_ERR				= -3,				// �ļ���ȡ����
		UPDATE_RST_ADD_ERR				= -4,				// ����û�����һ�㶼Ӧ���ǽ�ɫ���ظ��ˣ�
	};
	int			Update();									// ����ļ��Ƿ�����ˣ���������˾����³�ʼ��һ��
	int			AddUser(const char *cszUser, const char *cszPass);
															// ����һ���û�/����ӳ��
};

class	whvarstrarr_WF	: public whvarstrarr
{
public:
	// ���ļ��г�ʼ��
	int		initfromfile(const char *cszFileName);
	// �ӻ����ļ������г�ʼ��
	int		initfromfile(whfile *file);
};

class	whstrmatch_WF	: public whstrmatch
{
public:
	// ���ļ��г�ʼ��
	int		initfromfile(const char *cszFileName);
	// �ӻ����ļ������г�ʼ��
	int		initfromfile(whfile *file);
};

// ���Ŀ¼�е��ļ��б�
// szExt�ǡ�.c|.cpp|.h���������ִ������ֻ��һ�ֺ�׺�Ͳ���д��|���ˣ�����������.c�������������Ҫ������ֱ��дNULL��մ����ɡ�
bool	GetDirFileList(whfileman *pMan, const char *szPath, std::vector< std::string > *pList, const char *szExt=NULL);

// ����ָ���ļ�������ҵ��Ͱ�ȫ·����д��szBuf�У�����szBuf�����򷵻ؿգ�
const char *	GetFile(whfileman *pMan, const char *szPath, const char *szFile, char *szBuf);

// ɾ��ĳ��Ŀ¼�µ���ָ��ʱ��֮ǰ�������ļ�
// szExt����ͬǰ��
bool	DelFileBeforeTime(whfileman *pMan, const char *szPath, const char *szExt, time_t nTime);

// ɾ��Ŀ¼���ļ��������Ŀ¼��ݹ�ɾ����
bool	DelPath(whfileman *pMan, const char *szPath);

// һ���򵥵��ļ�����Ĺ�����
class	whsmpfilestore
{
public:
	struct	INFO_T
	{
		whfileman		*pFM;										// �ײ���ļ�������
		char			szBasePath[WH_MAX_PATH];					// �ļ��洢�ĸ�Ŀ¼
		int				nLoadRefreshInterval;						// �ļ�����ˢ�¼�����룩
		int				nUpdateRefreshInterval;						// �ļ�����ˢ�¼�����룬����Ǵ��������ڲ��ļ��ļ����

		INFO_T()
			: pFM(NULL)
			, nLoadRefreshInterval(30)								// 30����ˢһ��Ӧ�ò���̫���
			, nUpdateRefreshInterval(100)							// �����Ϊ��Ҫ�ӷ�����������������������׹���
		{
			szBasePath[0]	= 0;
		}
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	struct	FILE_T
	{
		int				nFileID;									// �ļ���
		unsigned char	MD5[WHMD5LEN];
		whvector<char>	vectfiledata;								// �洢�ļ�����
		time_t			nLastFileModifyTime;						// �ϴ�����ʱ���ļ��ĸ���ʱ�䣨������������ļ��Ƿ���Ҫ�������룩
		time_t			nLastLoadTime;								// �ϴ������ʱ�䣨�Ӵ������룩
		time_t			nLastUpdateTime;							// �ϴ���������ݸ��µ�ʱ�䣨�����µ�Ӳ�̣�
		int				anExt[4];									// ��������
		FILE_T()
			: nFileID(0)
			, nLastFileModifyTime(0)
			, nLastLoadTime(0)
			, nLastUpdateTime(0)
		{
			WHMEMSET0(MD5);
			WHMEMSET0(anExt);
		}
	};
private:
	INFO_T				m_info;
	whhash<int, FILE_T *>	m_mapFileID2Obj;						// �ļ�ID���ļ������ӳ��
	char				m_szFNBuf[WH_MAX_PATH];						// ������ʱ������ɵ��ļ���
public:
	whsmpfilestore();
	~whsmpfilestore();
	int		Init(INFO_T *pInfo);
	int		Release();
	// SureLoadFile��õ��ļ��������ͷţ��ڲ��Լ����ͷ�
	FILE_T *	SureLoadFile(int nID, bool bMustLoad=false);		// ȷ���ļ������أ�bMustLoadΪ���ʾ����ļ������Ҵ���û�б�������ͱ������¼���һ�Σ�����ļ������ھʹ���һ�����ļ���
																	// �ͻ���Ӧ������ҪbMustLoadΪ��ģ���Ϊ����Ҫ����һЩ��ǣ������ʾ����ļ��Ѿ��������������������У�
	FILE_T *	GetFileInfo(int nID);								// �����ǻ��
	FILE_T *	UpdateFile(int nID, void *pData, int nDSize);		// �����ļ����ݣ�ͬʱҲ�����MD5��
	void	clear();												// ���m_mapFileID2Obj������
	bool	CheckIfFileNeedUpdate(FILE_T *pF);						// �ж��ļ��Ƿ���Ҫ���£����ϴθ��µ�ʱ�䵽�����Ƿ�����ˣ�
private:
	const char *	MakeFileName(int nID, char *pszBuf=NULL);
	int		LoadFile(int nID, FILE_T *pF, bool bMustLoad=false);	// �����ļ���ֻ���ļ�û�иı��ʱ�򣩡����bMustLoadΪ�����ʾ����ļ������ھʹ���һ�����ļ�
};


////////////////////////////////////////////////////////////////////
// �����ļ��л�ȡ��Ȩ��Ϣ
////////////////////////////////////////////////////////////////////
int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, const char *szAuthFile, whvector<char> &vectAuth);
int		GetAuthInfo(const char *szRSAPriKeyFile, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth);
int		GetAuthInfo(char *pszRSAPriKeyFileData, int nRSAPriKeyFileSize, const char *szRSAPass, char *pszFileData, int nFileSize, whvector<char> &vectAuth);
int		GetAuthInfo(char *pszFileData, int nFileSize, WHRSA *pDec, whvector<char> &vectAuth);

}		// EOF namespace n_whcmn

#endif	// __WHFILE_UTIL_H__

