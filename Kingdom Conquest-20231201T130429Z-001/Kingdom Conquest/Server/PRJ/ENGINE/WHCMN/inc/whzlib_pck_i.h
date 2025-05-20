// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whzlib
// File         : whzlib_pck_i.h
// Creator      : Wei Hua (κ��)
// Comment      : �;�������صĹ���ʵ��
//                �����Ҫ�԰����о�����������罻������ӻ�ɾ���ļ�������Ҫinclude����ļ�
// CreationDate : 2003-09-04

#ifndef	__WHZLIB_PCK_I_H__
#define __WHZLIB_PCK_I_H__

#include "whzlib_i.h"
#include "whzlib_pck.h"
#include "whfile.h"
#include "whdir.h"
#include "whstring.h"

using namespace n_whcmn;

namespace n_whzlib
{

// �ļ���׺
extern const char *cstIdxFileExt;
extern const char *cstDatFileExt;
extern const char *cstNamFileExt;

// ��pck��ص����Ͷ���
// ���ݵ�Ԫ����
typedef	unsigned int	whzlib_unitsize_t;
typedef unsigned char	whzlib_filenamesize_t;
// ���ֽṹ
#pragma pack(1)
// ��Ϣ�ļ�ͷ(������ҪΪ�ļ�������Ϣ���ļ�����)
struct	infofile_hdr_t
{
	static const char *	CSZ_MAGIC;
	struct	hdr_t
	{
		char	szMagic[16];				// �ļ���־
		unsigned int	nVer;				// �汾
		unsigned int	nUnitSize;			// ��������δѹ��ǰ�ĳߴ磬�������һ��һ�㶼һ����Ĭ��Ϊ65536
											// ����ʹ��unsigned int��Ϊ����ý�����һ��whzlib_unitsize_t��С
		unsigned int	nWastedSize;		// �����Ŀռ��ֽ���(һ��������ɾ�����滻�ļ���ɵ�)
		// �����Ѿ���32�ֽ���
	}hdr;
	char	junk[64-sizeof(hdr_t)];			// �������ݣ�����ͷΪһ������(64�ֽ�)
	infofile_hdr_t()
	{
		memset(&hdr, 0, sizeof(hdr));
		strcpy(hdr.szMagic, CSZ_MAGIC);
		hdr.nVer		= WHZLIB_VER;
		hdr.nUnitSize	= 0x10000;
		hdr.nWastedSize	= 0;
	}
	inline bool	IsMagicGood() const
	{
		return	strcmp(CSZ_MAGIC, hdr.szMagic) == 0;
	}
	inline bool	IsVerGood() const
	{
		return	hdr.nVer >= WHZLIB_LOWRVER;
	}
};
// ��Ϣ�ļ��еģ����ļ���Ϣ��Ԫ
struct	infofile_fileunit_t
{
	enum
	{
		TYPE_NOTHING		= 0,			// δ֪����
		TYPE_FILE			= 1,			// �ļ�
		TYPE_DIR			= 2,			// Ŀ¼
		TYPE_EOD			= 3,			// Ŀ¼��β(End Of Dir)
	};
	unsigned char			type;			// ����(�ļ�/Ŀ¼)
	unsigned int			namepos;		// �����������ļ��е�λ��
	time_t					time;			// �ļ�ʱ��
	unsigned char			md5[16];		// �ļ�md5У���
	union
	{
		struct
		{
			unsigned int	datapos;		// �����������ļ��е���ʼλ��
			unsigned int	totaldatasize;	// �ļ��������ݳ���
		}file;
		struct
		{
			unsigned int	entrynum;		// ��Ŀ¼�µı��������
			unsigned int	totalentrynum;	// ��Ŀ¼�µĸ����������(��������Ŀ¼���EOD)
											// ���Ŀ¼��û����Ŀ¼��entrynum==totalentrynum
		}dir;
	}info;
};
// �����ļ������ļ�������ͷ
// �����ͷ��߻����nParts����������ʾ����ĸ���ѹ�����ݵ���ʼλ�ã���һ��Ϊ0��
// �������һ�Σ�ÿ�����ݽ�ѹ��ĳ��ȶ�Ӧ��Ϊinfofile_hdr_t::nUnitSize
// ������Ҫ��ȡĳ��λ�ÿ���Ѹ��ͨ�����ű������ݵľ���λ�ã�������е�seek������ʵ����
// Ψһ��Ҫ���Ƿ����ļ�ʱҪ���ñ�����ڴ档������ʹ��4G���ļ��������64k�ֶΣ�
// ���ű�Ҳ�����ǲ���300k�����ӣ��ÿռ任ʱ������ǿ��Խ��ܵġ�
struct	datafile_file_hdr_t
{
	enum
	{
		PROPERTY_DELETED	= 0x01,			// �Ѿ���ɾ�����ļ�����
	};
	struct	hdr_t
	{
		unsigned char		nProperty;		// ����
		int					nParts;			// �ܹ����ָ������
	}hdr;
	char	junk[16-sizeof(hdr_t)];			// �������ݣ�����ͷΪһ������
};
// �����ļ�������������ͷ
struct	datafile_dataunit_hdr_t
{
	unsigned char			nMode;			// ѹ��ģʽ(�����޷�ѹ�������ݻ��Զ�ʹ�÷�ѹ��ģʽ)
	whzlib_unitsize_t		nSize;			// ����������ݳ���
};
#pragma pack()

// ��չ�������ڴ��е��ļ����ݵ�Ԫ
struct	EXTFILEUNIT_T
{
	struct	DIRINFO
	{
		whhash<whstr4hash, int, whcmnallocationobj, _whstr_hashfunc>	mapName2ID;
											// ��Ŀ¼���ļ�������ڵ�ӳ���ϵ
	};
	infofile_fileunit_t		infile;
	DIRINFO					*pDirInfo;		// �б�Ҫ�Ż��������Ϣ
	EXTFILEUNIT_T()
	: pDirInfo(NULL)
	{
	}
};
struct	EXTUNIT_T
{
	int		nFatherID;						// ��Ŀ¼��ID
};
typedef	whvector<EXTFILEUNIT_T>				FILEEXTINFOLIST_T;
typedef	whvector<infofile_fileunit_t>		FILEINFOLIST_T;
typedef	whvector<EXTUNIT_T>					FILEEXTLIST_T;

class	whzlib_pck_reader_i;
class	whzlib_pck_modifier_i;

// ͨ������ļ���whzlib_file_iʵ��
class	whzlib_file_i_pck	: public whzlib_file_i
{
public:
	struct	INFO_T
	{
		unsigned int	totaldatasize;		// �ļ��������ݳ���
		unsigned int	nUnitSize;			// ����δѹ�����ݵĳ���
		whzlib_pck_reader_i	*pReader;		// pck�ļ����������򿪣���������ݷ���ͨ��������
		int				nFileID;			// ��Reader�е��ļ�ID
	};
	INFO_T				m_info;				// ��Ϣ����
private:
	whvector<char>		m_vectbuf;			// ���ڳ���һ�ν�ѹ�����ݵ�buffer������(ֻ��initʱ����һ�Σ�����ָ����Զ��Ч)
	char				*m_buf;				// ָ��buffer��ָ��
	int					m_bufsize;			// Ŀǰbuffer�����ݵ��ܳ���
	int					m_leftsize;			// Ŀǰbuffer�����ݻ��ɶ��ĳ���
	int					m_curpart;			// ��ǰ�Ĳ���
	int					m_totalpart;		// �ܹ��Ĳ�������
public:
	whzlib_file_i_pck();
	~whzlib_file_i_pck();
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t)	{}
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		GetMD5(unsigned char *MD5);
public:
	// ��ʼ��(��m_info������֮��������)
	int		Init();
	// ����
	int		Release();
};

// �����Ŀ¼ʱ���԰Ѱ�����Ŀ¼��
class	WHDirCarePck	: public n_whcmn::WHDir
{
public:
	// ��������.idx��.nam�ļ������ԣ�����.dat����
	// ��Ȼ��Ҫ�ж������ļ��Ƿ�ͬʱ����
	ENTRYINFO_T *	Read();
};

// ͨ������ļ���WHDirBaseʵ��
// ��reader�õ�
class	WHDirInPck		: public n_whcmn::WHDirBase
{
public:
	static whzlib_fileman	*m_pFMan;		// ���д��Open�Ϳ���ͨ�������
	struct	INFO_T
	{
		whzlib_pck_reader_i	*pReader;		// pck�ļ����������򿪣���������ݷ���ͨ��������
		int				nDirID;				// Ŀ¼�е�һ���ļ���Reader�е��ļ�ID
		INFO_T() : pReader(NULL), nDirID(-1)
		{}
	};
	INFO_T				m_info;				// ��Ϣ����
private:
	n_whcmn::WHDirBase::ENTRYINFO_T	m_EntryInfo;
											// ��������緵��һ�����
	EXTFILEUNIT_T		*m_pUnit;			// ��һ��Unit
	int					m_nIdx;				// ��ǰ����������Ŀ������
	int					m_nLevel;			// Ŀǰ�������ļ�����0������EOD��˵��������
public:
	WHDirInPck();
	~WHDirInPck();
	void			SelfDestroy()	{delete this;}
	const char *	GetDir() const	{assert(0); return NULL;}	// ���ֻ��Ϊ�˼��ݾɴ�����ϵģ���Ӧ�ñ�����
	int				Close();
	ENTRYINFO_T *	Read();
	void			Rewind();
	static WHDirBase *	Create(const char *szDir);
};
// ��modifier�õ�
class	WHDirInPck4Mod	: public n_whcmn::WHDirBase
{
public:
	struct	INFO_T
	{
		whzlib_pck_modifier_i	*pReader;	// pck�ļ����������򿪣���������ݷ���ͨ��������
		int				nDirID;				// Ŀ¼�е�һ���ļ���Reader�е��ļ�ID
	};
	INFO_T				m_info;				// ��Ϣ����
private:
	n_whcmn::WHDirBase::ENTRYINFO_T	m_EntryInfo;
											// ��������緵��һ�����
	infofile_fileunit_t	*m_pUnit;			// ��һ��Unit
	int					m_nIdx;				// ��ǰ����������Ŀ������
	int					m_nLevel;			// Ŀǰ�������ļ�����0������EOD��˵��������
public:
	WHDirInPck4Mod();
	void			SelfDestroy()	{delete this;}
	const char *	GetDir() const	{assert(0); return NULL;}	// ���ֻ��Ϊ�˼��ݾɴ�����ϵģ���Ӧ�ñ�����
	int				Close();
	ENTRYINFO_T *	Read();
	void			Rewind();
};


// ѹ���ļ���ȡ��(��Ϊ��粻��Ҫ�����ԾͲ����ϲ��ṩ�ӿ��ˣ���_i��Ϊ���ڱ��㿴����ͳһ)
class	whzlib_pck_reader_i
{
private:
	n_whcmn::WHSimpleDES	m_des;
public:
	whzlib_pck_reader_i();
	~whzlib_pck_reader_i();
private:
	// �����ļ�ͷ
	infofile_hdr_t		m_infohdr;
	// ���룬����еĻ�(���ִ���ʾû������)
	char				m_szPassword[PCKPASSWORD_MAXLEN];
	// ����ļ�������(����Ĵ����ʽ������Ŀ¼�ṹ���ļ�����˳���ɲ���˳�����)
	FILEEXTINFOLIST_T	m_finfolist;
	// ��������ļ���ָ��
	whfile	*m_fileIdx, *m_fileDat, *m_fileNam;
	// �Ƿ��Զ�ɾ���ļ����
	bool	m_bAutoClosePackFile;
	// �Ƿ��Сд����
	bool	m_bIC;
	// ���ڶ���һ�����ݵ�Ԫ
	whvector<char>		m_vectbuf;
	char	*m_buf;
	// ��������ļ���(ֱ�Ӷ����ļ������ݵ��������)
	whvector<char>		m_vectnames;
public:
	// �����Ƿ��Сд����(�����Ӱ�쵽����ʹ�õ�whstr4hash�ĵط�)
	// inline static void	SetIC(bool bIC)
	// {
	// 	n_whcmn::whstr4hash::m_bIC	= bIC;
	// }	2006-07-12 ȡ��������ã��ļ�����windows��Ҳǿ�ƴ�Сд����
	// ���ý�ѹ����
	void	SetPassword(const char *szPassword);

	// ��һ����
	int	OpenPck(const char *szPckFile);
	// ��һ����������ڴ�cache���о����ȴ��ڴ��е��ļ�
	int	OpenPck(const char *szPckFile, whsmpfilecache *pCache);
	// �Ƚϵͼ��ĸ����ļ�����򿪰�
	int	OpenPck(whfile *pfileIdx, whfile *pfileDat, whfile *pfileNam, bool bAutoClosePackFile);
	// �رհ�
	int	ClosePck();

	// ����nFileID��Ӧ��whzlib_file_i_pck����(���������ɾ������)
	whzlib_file_i_pck *	OpenFile(int nFileID);
	// ע�⣺�������������е�szFile��������ڰ�������·������ǰ��û��"/"��"\"
	// �����ļ�������whzlib_file_i_pck����
	whzlib_file_i_pck *	OpenFile(const char *szFile);

	// ����Ŀ¼����
	WHDirInPck *		OpenDir(int nDirID);
	WHDirInPck *		OpenDir(const char *szDir);

	// �����ļ�������ļ���Ŀ¼ID
	int	GetPathID(const char *szPath);
	// �����ļ�������ļ����ȣ�����-1��ʾ�ļ�������
	int	GetFileSize(const char *szFile);
	// ��ȡһ���ļ�����ĵ�N������(���峤���㹻)
	// ����ʵ�����ݳ���
	// ����0��ʾ�Ѿ�û�������������ˣ����߱�ʾ����
	int	GetDataPart(int nFileID, int nPart, char *pBuf);
	// �������ָ��
	char *	GetFileNamePtr(int nOff);
	// ���ָ����ŵ��ļ���ڵ�ָ��
	inline EXTFILEUNIT_T *	GetUnitPtr(int nFileID) const
	{
		return	m_finfolist.getptr(nFileID);
	}
private:
	// ��һ��ģʽ�����е�����ļ�
	int	OpenAllFileOnDisk(const char *szPckFile, const char *szMode);
	// ��cache��Ӳ���ϴ���������ļ�
	int	OpenAllFileFromMemOrOnDisk(const char *szPckFile, whsmpfilecache *pCache);
	// �������ļ��Ѿ��򿪲������úø�����Ҫ������ǰ���£��򿪰�
	int	OpenPack();
	// ��һ��Ŀ¼��Ԫ��ʼ�ұ����µ�һ��ƥ��������
	// �ɹ�������ֵΪ�ҵ�����ţ�ʧ�ܣ�-1
	int	FindEntry(int nDirID, const char *szName);
};

// ���޸Ķ���
class	whzlib_pck_modifier_i	: public whzlib_pck_modifier
{
private:
	n_whcmn::WHSimpleDES	m_des;
public:
	whzlib_pck_modifier_i();
	~whzlib_pck_modifier_i();
public:
	int	AppendDir(const char *szDir);
	int	AppendFile(const char *szFile, whzlib_file *file, int nCompressMode);
	int	AppendDirEnd();
	int	AddDir(const char *szDirInPck);
	int	AddFile(const char *szFileInPck, whzlib_file *file, int nCompressMode);
	int	ReplaceFile(const char *szFileInPck, whzlib_file *file, int nCompressMode);
	int	DelPath(const char *szPathInPck);

	// ����Ŀ¼����
	WHDirBase *	OpenDir(int nDirID);
	WHDirBase *	OpenDir(const char *szDir);

	// ���ý�ѹ����(���������CreatePck��OpenPck֮�����)
	void	SetPassword(const char *szPassword);
	// ���ڴ�����flush��Ӳ����
	int		Flush();
// �Լ��õ�
public:
	// ��ʼ��(����Ϊ�˱����ļ����ܴ򿪵�״��)
	int	CreatePck(CREATEINFO_T *pInfo);
	int	OpenPck(OPENINFO_T *pInfo);
	int	ClosePck();
	// �����ļ�������ļ���Ŀ¼ID
	int	GetPathID(const char *szPath);
	// Ѱ��·���Ѿ����ڵĲ��֣����ش��ڲ��ֵ�ID����szNew�з��ز����ڵĲ���
	int	LookForExistPath(const char *szPath, char *szNew);
	// �������ָ��(������Ҫ����ʹ��)
	char *	GetFileNamePtr(int nOff);
public:
	inline void	SetModified()
	{
		m_bModified	= true;
	}
	inline void	ClrModified()
	{
		m_bModified	= false;
	}
	inline bool	IsModified() const
	{
		return	m_bModified;
	}
	// ���ָ����ŵ��ļ���ڵ�ָ��
	inline infofile_fileunit_t *	GetUnitPtr(int nPathID) const
	{
		if( nPathID<0 || nPathID>=int(m_finfolist.size()) )
		{
			return	NULL;
		}
		return	m_finfolist.getptr(nPathID);
	}
private:
	// �Ƿ��Ǹմ������ļ�
	bool	m_bJustCreated;
	// �ļ��Ƿ��޸ģ��Ƿ���Ҫ��д�����ļ�
	bool	m_bModified;
	// �ļ���
	char	m_szPckFile[WH_MAX_PATH];
	char	m_szTmpFileName[WH_MAX_PATH];
	// ��������ļ���ָ��
	FILE	*m_fpIdx, *m_fpDat, *m_fpNam;
	// �����ļ�ͷ
	infofile_hdr_t		m_infohdr;
	// ���룬����еĻ�(���ִ���ʾû������)
	char				m_szPassword[PCKPASSWORD_MAXLEN];
	// ����ļ�������(����Ĵ����ʽ������Ŀ¼�ṹ���ļ�����˳���ɲ���˳�����)
	FILEINFOLIST_T		m_finfolist;
	// ������Ϣ�б����ļ������б�һһ��Ӧ
	FILEEXTLIST_T		m_fextlist;
	// ר��Ϊappend���ĵ�ǰĿ¼ջ
	whvector<int>		m_vectDirStack;
private:
	// ��������ص������ļ�
	int	OpenAllFileOnDisk(const char *szPckFile, const char *szMode);
	// ����ļ����ݵ������ļ�(ͬʱ����infofile_fileunit_t�е�md5���ļ�ʱ��)
	int	WriteData(infofile_fileunit_t *pFUnit, whzlib_file *file, int nCompressMode);
	// ����ļ������ļ����ļ�
	int	WriteFileName(const char *szFileName);
	// ���ض�λ�ö�ȡ�ļ����������ļ���ȫ������������'\0'��
	// ����-1��ʾû��
	int	ReadFileName(int nPos, char *szFileName, int nSize);
	// ��һ��Ŀ¼��Ԫ��ʼ�ұ����µ�һ��ƥ��������
	// �ɹ�������ֵΪ�ҵ�����ţ�ʧ�ܣ�-1
	int	FindEntry(int nDirID, const char *szName);
	// ��һ��Ŀ¼�����һ����Ŀ¼(�����֤nDirID���Դ���)��������Ŀ¼��ID
	int	AddDirToDir(int nDirID, const char *szName);
	// ��һ��Ŀ¼�����һ���ļ����������ļ���ID
	int	AddFileToDir(int nDirID, const char *szName, whzlib_file *file, int nCompressMode);
	// Ϊ���и�Ŀ¼����������������(Ҳ����nDirID��һ��)
	int	AddNumToAllFather(int nDirID, int nNum);
	// ��������nCheckAfter֮���Ԫ�أ�������ID>nEffectAfter������Ҫ����Ҫ+nAdjust
	int	AdjustExtUnit(int nCheckAfter, int nEffectAfter, int nAdjust);
	// ɾ��Ŀ¼
	int	DelDir(int nDirID);
	// ɾ���ļ�
	int	DelFile(int nFileID);
};

// ���ض�λ�ö�ȡ�ļ����������ļ���ȫ������������'\0'��
int	whzlib_GetFilePartNum(int nTotalSize, int nUnitSize);
// ��д�ļ���
int	whzlib_ReadFileName(FILE *fp, int nPos, char *szFileName, int nSize);
int	whzlib_WriteFileName(FILE *fp, const char *szFileName);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_PCK_I_H__
