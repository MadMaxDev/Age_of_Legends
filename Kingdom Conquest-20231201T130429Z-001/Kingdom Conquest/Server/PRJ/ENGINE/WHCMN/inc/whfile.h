// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfile.h
// Creator      : Wei Hua (κ��)
// Comment      : �ļ�����
// CreationDate : 2003-06-13
// ChangeLOG    : 2005-05-28 ȡ����iostreamͷ�ļ�����Ϊ����afx.hһ��ᵼ�º���ֵ�����
//                2005-10-12 ��whlistinfile�������ļ��𻵵Ĳ��ȴ�ʩ�����沢��վ��ļ��������������ļ�У���롣
//                2006-02-17 ��linux�µ�WH_MAX_PATHҲ�ĳ�260����windows����һ����
//                           ��whfile_i_mem������GetCurBufPtr��GetSizeLeft����������������Read��Write��ģ�庯����
//                2006-06-12 whfile_cpy�ķ��������˶��塣������whfile_cpy_calcmd5��whfile_cpy_cmp������

#ifndef	__WHFILE_H__
#define	__WHFILE_H__

#include "whfile_base.h"
#include "whcmn_def.h"
#include "whlist.h"
#include "whmd5.h"
//#include <iostream>

// ���windows�Ķ���
#if defined( WIN32 )
// ��access�õ�
#ifndef	F_OK
#define	F_OK	00			// existance
#endif
#ifndef	W_OK
#define	W_OK	02
#endif
#ifndef	R_OK
#define	R_OK	04
#endif
#endif

namespace n_whcmn
{

// �Ƿ��Ǿ���·��
bool	whfile_isabspath(const char *__path);

// �ж�·���Ƿ���Ŀ¼
bool	whfile_ispathdir(const char *__path);
// �ж�·���Ƿ����
bool	whfile_ispathexisted(const char *__path);
// �ж��ļ��Ķ�д����(ע�⣬д������windows�»����ܵ���ϵͳ�ļ��������ļ���Ӱ��)
bool	whfile_ispathreadable(const char *__path);
bool	whfile_ispathwritable(const char *__path);
// �Ƿ���ϵͳ�ļ�(���Ӧ��ֻ��windows������)
bool	whfile_ispathsysfile(const char *__path);
// ��ÿ��ܳ���4G���ļ�����
whint64	whfile_getfilebigsize(const char *__path);
// ����ļ�����(��Ϊ�ϵĳ������кܶ�Ӧ�õ�����ĵط������Ի��Ǽ�������)
inline int		whfile_getfilesize(const char *__path)
{
	return	(int)whfile_getfilebigsize(__path);
}

// ֧��4G���ϵ�seek
int		whfseek(FILE *fp, whint64 nOffset, int nOrigin);

// �����ļ�/Ŀ¼��д(�����Ƿ����óɹ�)
bool	whfile_makefilewritable(const char *__path);

// ����ļ���·��(��β����û��"/"��"\"������о�����������wh_strkickendslash����)
// ����false��ʾû��·�����������һ���ļ���
// __file��__path������ͬһ��ָ��
bool	whfile_getfilepath(const char *__file, char *__path = 0);
// ����ļ���
// __file��__fname������ͬһ��ָ��
const char *	whfile_getfilename(const char *__file, char *__fname);
// ���·�����ļ���(ȫ·����buffer�ᱻ�ı�)
// __fpath���û��'/'��'\'��β
// �磺c:\winnt\system32\something.dll�ᱻ�ֽ�Ϊ��c:\winnt\system32��something.dll
bool	whfile_splitfilenameandpath(char *__fullfilepath, const char **__fpath, const char **__fname);
// ��һ�����·����ϵ�һ����׼·����
const char *	whfile_makerealpath(const char *__basepath, const char *__reltivepath, char *__realpath);

// ɾ���ļ�(�����whdir_sysdelfile������ȫһ����ֻ����ʷԭ����ɵ���������)
int		whfile_del(const char *__file);
// �ļ�����
int		whfile_ren(const char *__oldfile, const char *__newfile);
// �ļ�����
// __cptime��ʾ�Ƿ�Ҫ�������ļ���ʱ��;��ļ�һ��
// __nofileok��ʾû��Դ�ļ��򿽱��ɹ�(�յ���)
// ����
enum
{
	WHFILE_CPY_RST_OK		= 0,
	WHFILE_CPY_RST_ONSELF	= -1,		// �Լ��������Լ�
	WHFILE_CPY_RST_SRCERR	= -2,		// Դ�ļ����󣨶�д��
	WHFILE_CPY_RST_DSTERR	= -3,		// Ŀ���ļ����󣨶�д��
	WHFILE_CPY_RST_SRC_O_ERR= -4,		// Դ�ļ����󣨴򿪣�
	WHFILE_CPY_RST_DST_C_ERR= -5,		// Ŀ���ļ����󣨴�����
};
int		whfile_cpy(const char *__dstfile, const char *__srcfile, bool __cptime=false, bool __nofileok=true);
// �����ļ�������MD5��
// ����ͬ��WHFILE_CPY_RST_XXX
struct	WHFILE_CPY_T
{
	const char	*dstfile;
	const char	*srcfile;
	bool		cptime;
	bool		nofileok;
	unsigned char	MD5[16];
	size_t		cbstep;					// ÿ����������ô���ֽھ͵���һ�»ص����������Ϊ0���ʾ���õ��ûص���
	void		*cbparam;				// �ص��Ĳ���
	void (*cb_progress)(size_t copied, void *cbparam);
										// �ص�����copied���Ѿ��������ֽ�����param�ǻش��Ĳ���
	WHFILE_CPY_T()
		: dstfile(NULL), srcfile(NULL)
		, cptime(false)
		, nofileok(true)
		, cbstep(0)
		, cbparam(NULL)
		, cb_progress(NULL)
	{
	}
};
int		whfile_cpy_calcmd5(WHFILE_CPY_T *pParam);
// �����ļ����Աȣ�����������ʹ���ڲ��Ļص�������
// ����ͬ��WHFILE_CPY_RST_XXX
int		whfile_cpy_cmp(WHFILE_CPY_T *pParam);

// �ļ�ʱ�����(��ֻ����modification time)
// ����
int		whfile_setmtime(const char *__file, time_t t);
// ��ȡ(����ļ������ڷ���0��Ӧ��û�б������᷵��0�ģ�)
time_t	whfile_getmtime(const char *__file);

// ���ļ����ݶ���һ��whvector��whvector�ĳ��Ⱦ��Ƕ������ļ�����
int		whfile_readfile(const char *__file, whvector<char> &__vectbuf);
// ���ļ����ݶ����ڴ档*__size��ԭ����__data����󳤶ȣ�*__size���ض��������ݳ���
int		whfile_readfile(const char *__file, void *__data, size_t *__size);

// ���ļ����ݺ����ݱȽ�(����ļ�����򲻴��ڱȽϽ��Ϊ-1)
// ����0��ʾ����һ��
int		whfile_cmpdata(const char *__file, const void *__data, int __size);
int		whfile_cmpdata(whfile *__file, const void *__data, int __size);
// �ļ����ļ��Ƚ�
// ����0��ʾ����һ��
// -1��ʾ�ļ���д����
// 1��ʾ�ļ����ݲ�ͬ
// 2��ʾ�ļ����Ȳ�ͬ
int		whfile_cmp(whfile *__file1, whfile *__file2);
int		whfile_cmp(const char *__file1, const char *__file2);

// ͬlinux�µ�touch
// __filetime==0��ʾnow
int		whfile_touch(const char *__file, time_t __filetime=0);

// ֱ�Ӱ�һ���ڴ�д���ļ�
// �ɹ�������д�볤�ȣ����򷵻�<0
int		whfile_writefile(const char *__file, const void *__data, size_t __size);
// ��ȫ���ļ��滻
// __readagain��ʾ�Ƿ���Ҫ�������ٶ�һ���Ա�֤ȷʵ����
int		whfile_safereplace(const char *__file, const void *__data, size_t __size, bool __readagain);

// ����ָ�����ȵ��ļ�������Ϊ������ĳ����ֵ��
int		whfile_createfile_fill(const char *__file, unsigned char __pad, size_t __size);
// ��ָ�����ļ�ָ����д������__size��byte
int		whfile_file_fill(whfile *__fp, unsigned char __pad, size_t __size);
// ����ָ�����ȵ��ļ�(һֱû����:(���������Ͳ���)
//int		whfile_createfile(const char *__file, size_t __size);
// �ж��ļ�����׺�Ƿ���Ϲ����ִ���Ҫ��
// cszFilter��".h|.cpp"�����ĸ�ʽ
bool	whfile_ExtIsMatch(const char *cszFileName, const char *cszFilter);
bool	whfile_MakeExtFilter(const char *cszFilter, whvector<EXTINFO_UNIT_T> &vectFilter);
bool	whfile_ExtIsMatch(const char *cszFileName, whvector<EXTINFO_UNIT_T> &vectFilter);

// ������У��λ���ļ���д��CRCͷ��16�ֽڣ�
// ���ؽ��
enum
{
	WHFILE_CRC_READ_OK			= 0,			// ��ȷ����
	WHFILE_CRC_READ_NOTEXST		= -1,			// �ļ�������
	WHFILE_CRC_READ_ERRFILE		= -2,			// �ļ��޷����룬Ӳ����
	WHFILE_CRC_READ_ERRHDR		= -3,			// �ļ�ͷ����
	WHFILE_CRC_READ_ERRCRC		= -4,			// �ļ�CRCУ�����
};
int		whfile_crc_readfile(const char *__file, void *__data, size_t *__size);
int		whfile_crc_readfile(whfile *__file, void *__data, size_t *__size);
int		whfile_crc_readfile(const char *__file, whvector<char> &vectData);
int		whfile_crc_readfile(whfile *__file, whvector<char> &vectData);
// __writetmp��ʾ�Ƿ���Ҫ��дһ����ʱ�ļ�
int		whfile_crc_writefile(const char *__file, const void *__data, size_t __size, bool __writetmp);

int		whfile_crc_readfromvector(whvector<char> &vectFile, void *__data, size_t *__size);
int		whfile_crc_readfromvector(whvector<char> &vectFile, whvector<char> &vectData);
int		whfile_crc_writetovector(whvector<char> &vectFile, const void *__data, size_t __size);

// �ļ�MD5�ļ���
// ���һ���ļ���md5
unsigned char *	whfilemd5(const char *szFile, unsigned char *szDst);
unsigned char *	whfilemd5(whfile *fp, unsigned char *szDst);
// ���һ���ļ���md5�ִ�
char *	whfilemd5str(const char *szFile, char *szDst);
// �ú�����ľ�̬�ִ�����
char *	whfilemd5str(const char *szFile);


////////////////////////////////////////////////////////////////////
// ��������������ʵ��
////////////////////////////////////////////////////////////////////
class	whfile_i		: public whfile
{
public:
	virtual int		Write(const void *pBuf, int nSize)
	{
		// Ĭ�ϲ�ʵ��д�ķ���
		return	-1;
	}
	virtual int		Flush()
	{
		// Ĭ��Ҳû��flush
		return	0;
	}
};

////////////////////////////////////////////////////////////////////
// ��׼�ļ���ʵ��
////////////////////////////////////////////////////////////////////
class	whfile_i_std	: public whfile_i
{
private:
	FILE	*m_fp;
	int		m_nSize;
	char	m_szFileName[WH_MAX_PATH];
	time_t	m_nNewFileTime;		// ���������SetFileTime������������
#ifdef	WIN32
	bool	m_bJustRread;		// �ոս����˶�����
	bool	m_bJustWrite;		// �ոս�����Щ����
#endif
public:
	whfile_i_std(FILE *fp, const char *cszFileName);
	~whfile_i_std();
public:
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		Write(const void *pBuf, int nSize);
	int		Flush();
	FILE *	GetFILE() const	// Ĭ��ʵ���Ƿ���NULL����whfile
	{
		return	m_fp;
	}
};

////////////////////////////////////////////////////////////////////
// �ڴ��ļ���ʵ��
////////////////////////////////////////////////////////////////////
class	whfile_i_mem	: public whfile_i
{
private:
	// ָ�򻺳��ָ��
	char		*m_pszStr;
	// ���峤��
	int			m_nSize;
	// ��ǰ��ȡ��λ��
	int			m_nOffset;
public:
	whfile_i_mem(char *pszBuf, int nSize);
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	// ���д���ᳬ��m_nSize
	int		Write(const void *pBuf, int nSize);
	int		Flush()
	{
		return	0;
	}
public:
	// ��õ�ǰָ��λ��
	inline char *	GetCurBufPtr()
	{
		return	m_pszStr + m_nOffset;
	}
	// ���ʣ�µĳ���
	inline int		GetSizeLeft() const
	{
		return	m_nSize - m_nOffset;
	}
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
};

////////////////////////////////////////////////////////////////////
// ����whvector���ļ�ʵ��
////////////////////////////////////////////////////////////////////
class	whfile_i_byvector	: public whfile_i
{
private:
	// ָ�򻺳��ָ��
	whvector<char>	*m_pvect;
	bool			m_bReadOnly;
public:
	inline void	SetVector(whvector<char> &vect, bool bReadOnly)
	{
		m_pvect		= &vect;
		m_bReadOnly	= bReadOnly;
	}
	inline whvector<char> &	GetVector()
	{
		return	*m_pvect;
	}
public:
	whfile_i_byvector()
		: m_pvect(NULL), m_bReadOnly(true)
	{
	}
	whfile_i_byvector(whvector<char> &vect, bool bReadOnly)
		: m_pvect(&vect), m_bReadOnly(bReadOnly)
	{
	}
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const
	{
		return	m_pvect->size();
	}
	time_t	FileTime() const
	{
		return	0;
	}
	void	SetFileTime(time_t t)
	{
	}
	int		Read(void *pBuf, int nSize)
	{
		return	m_pvect->f_read(pBuf, nSize);
	}
	int		Seek(int nOffset, int nOrigin)
	{
		return	m_pvect->f_seek(nOffset, nOrigin, m_bReadOnly);
	}
	bool	IsEOF()
	{
		return	m_pvect->f_iseof();
	}
	int		Tell()
	{
		return	m_pvect->f_tell();
	}
	int		Write(const void *pBuf, int nSize)
	{
		return	m_pvect->f_write(pBuf, nSize);
	}
	int		Flush()
	{
		return	0;
	}
};

////////////////////////////////////////////////////////////////////
// �ļ��ڵ��ļ���ʵ��
// ��ΪwhfileĿǰ��֧�ֶ��̷߳��ʣ��������Ҳ��֧�ֶ��̷߳���
////////////////////////////////////////////////////////////////////
class	whfile_i_fileinfile	: public whfile_i
{
private:
	// �������ļ�����
	whfile		*m_file;
	// ��m_file�е���ʼ��
	int			m_nStartOffset;
	// �Լ��ĳ���
	int			m_nSize;
	// �����Լ���ͷ��ƫ��
	int			m_nOffset;
public:
	whfile_i_fileinfile();
	void	SelfDestroy()
	{
		delete	this;
	}
	// ���ù������ļ�(ÿ�ι����൱�������һ�ι���)
	int		AssociateFile(whfile *file, int nStartOffset, int nSize);
	// ��ҪΪ����ʵ�ֵ�
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
};

////////////////////////////////////////////////////////////////////
// ���ļ��ϲ�
////////////////////////////////////////////////////////////////////
class	whmultifile
{
public:
	////////////////////////////////////////////////////////
	// ���Ͷ���
	////////////////////////////////////////////////////////
	#pragma pack(1)
	// �����ļ�ͷ�ṹ
	struct	HDR_T
	{
		enum
		{
			VER				= 0,							// ��ǰ�汾
		};
		struct	CMN_T
		{
			char			szMagic[16];					// ħ����
			int				nVer;							// �汾(��һ�����ܵ���)
			int				nFileNum;						// �������ļ�����
			int				nEndOffset;						// �����ļ����ݽ�����λ��
		}cmn;
		char	junk[64-sizeof(CMN_T)];						// �������ݣ����㡣
		HDR_T()
		{
			Reset();
		}
		inline void	Reset()
		{
			memset(this, 0, sizeof(*this));
			strcpy(cmn.szMagic, "whmultifile");
			cmn.nVer		= VER;
			cmn.nEndOffset	= sizeof(*this);
		}
		inline bool	IsGood() const							// �ж�ͷ�Ƿ�Ϸ�
		{
			return	VER==cmn.nVer
				&&  strcmp("whmultifile", cmn.szMagic)==0;
		}
	};
	// ÿ���ڲ��ļ���ͷ
	struct	IFILEHDR_T
	{
		struct	CMN_T
		{
			char			szName[32];						// �ļ���(����·���ģ�����̫��)
			unsigned char	MD5[WHMD5LEN];					// �ļ���MD5
			int				nSize;							// �ļ�����
			time_t			nTime;							// �ļ�(�޸�)ʱ��
		}cmn;
		char	junk[64-sizeof(CMN_T)];						// �������ݣ����㡣
		IFILEHDR_T()
		{
			memset(this, 0, sizeof(*this));
		}
	};
	#pragma pack()
public:
	////////////////////////////////////////////////////////
	// ��������
	////////////////////////////////////////////////////////
	HDR_T					m_hdr;							// �ļ�ͷ
	int						m_nOpenMode;					// ��ģʽ(ǣ�����ر�ʱ�Ƿ�Ҫ��)
	whfile					*m_basefile;					// �ײ��ļ�
	bool					m_bAutoCloseBaseFile;			// �Ƿ���Ҫ�Զ��رյײ��ļ�
public:
	whmultifile();
	~whmultifile();
	// ��
	enum
	{
		OPENMODE_NOTOPEN	= 0,							// û�д�
		OPENMODE_CREATE		= 1,							// �����ļ�(ԭ���о��滻֮)
		OPENMODE_RDONLY		= 2,							// ֻ�����ļ�(ԭ���������)
		OPENMODE_RDWR		= 3,							// ��д���ļ�(ԭ���������)
	};
	int		OpenFile(const char *cszMainFile, int nOpenMode);
	// �����Ѿ��򿪵��ļ������
	int		OpenFile(whfile *file, int nOpenMode, bool bAutoCloseBaseFile);
	// �ر�
	int		CloseFile();
	// ���һ���ļ���ע��Ϊ�˼򵥣��ļ�ֻ����Ӳ���ɾ�������Ҫɾ����������Create�ļ���
	int		Append(const char *cszFile, whfile *file);
	// �������д����ļ�����
	whfile *	GetFileToAppend(const char *cszFile, time_t nFileTime);
	// �����ļ�����ȡһ��ֻ�����ڲ��ļ�����(������������ļ����Ժ�)��
	whfile *	GetFileToRead(const char *cszFile);
	// ����ļ����б�
	int			GetFileHdrList(whvector<IFILEHDR_T> &vect);
protected:
	inline bool	IsModifyOpen() const
	{
		switch( m_nOpenMode )
		{
			case	OPENMODE_CREATE:
			case	OPENMODE_RDWR:
			return	true;
		}
		return		false;
	}
};

////////////////////////////////////////////////////////////////////
// �����ڱ���ļ�β�����ļ�
////////////////////////////////////////////////////////////////////
class	whtailfile	: public whfile
{
protected:
	// β����Ϣ
	struct	TAIL_T
	{
		enum
		{
			VER		= 0
		};
		struct	CMN_T
		{
			char	szMagic[16];
			int		nVer;
			int		nHostSize;								// �����ļ��ĳ���
			int		nTailSize;								// ��whtailfile�ļ������ݳ���
			time_t	nFTime;									// �ļ�ʱ��
			unsigned char	MD5[WHMD5LEN];					// ר����������MD5��ģ���Ϊtail�ļ�Ҳ����������������صĸ��³���������Ҫ����У�飩
		}cmn;
		char	junk[64-sizeof(CMN_T)];
		TAIL_T()
		{
			Reset();
		}
		inline void	Reset()
		{
			memset(this, 0, sizeof(*this));
			strcpy(cmn.szMagic, "whtailfile");
			cmn.nVer	= VER;
		}
		inline bool	IsGood() const
		{
			return	VER==cmn.nVer
				&&  strcmp("whtailfile", cmn.szMagic)==0;
		}
	};
protected:
	// ��Ա
	whfileman::OP_MODE_T	m_nOPMode;
	bool	m_bModified;
	bool	m_bAutoCloseHostFile;
	whfile	*m_hostfile;
	TAIL_T	m_tail;
public:
	whtailfile();
	virtual ~whtailfile();
	int		Open(whfileman *pFM, const char *cszHostFile, whfileman::OP_MODE_T nOPMode);
	int		Open(whfile *pHostFile, bool bAutoCloseHostFile, whfileman::OP_MODE_T nOPMode);
	int		Close();
	inline void	SetModified(bool bModified)
	{
		m_bModified	= bModified;
	}
	// SetMD5��GetMD5�����ڴ�֮��ر�֮ǰ���ܵ��ã�����Ӧ�ø�����Ҫ��д������м���MD5��
	inline void	SetMD5(unsigned char *MD5)
	{
		memcpy(m_tail.cmn.MD5, MD5, sizeof(m_tail.cmn.MD5));
	}
	inline const unsigned char *	GetMD5() const
	{
		return	m_tail.cmn.MD5;
	}
public:
	void	SelfDestroy()
	{
		delete	this;
	}
	// ����ļ��ܳ���(����ڹ����ļ����ܱ��ֳɶ��ٿ����ʱ������)
	virtual int		FileSize() const;
	// ����ļ�ʱ�䣨�������һ���޸�ʱ�䣩
	virtual time_t	FileTime() const;
	// �����ļ�ʱ��
	virtual void	SetFileTime(time_t t);
	// ����ʵ�ʵõ����ֽ�����0��ʾû�������ˣ�<0��ʾ������
	virtual int		Read(void *pBuf, int nSize);
	// �ƶ��ļ�ָ��
	// SEEK�Ĳ�����fseekһ�£��ṩ����seek
	// ����0��ʾ�ɹ���<0��ʾ������
	// nOriginʹ�ú�fseek����һ����SEEK_SET��SEEK_CUR��SET_END
	virtual int		Seek(int nOffset, int nOrigin);
	// �ж��Ƿ����ļ�β
	virtual bool	IsEOF();
	// ��õ�ǰ�ļ�ָ��λ��
	virtual int		Tell();
	// д������
	virtual int		Write(const void *pBuf, int nSize);
	// ���嵽Ӳ�̣����غ�fflush�Ľ��һ�£�
	virtual int		Flush();
protected:
	inline int	GetCurOffset() const
	{
		return	m_hostfile->Tell() - m_tail.cmn.nHostSize;
	}
public:
	static bool	IsFileOK(whfile *fp);
};

////////////////////////////////////////////////////////////////////
// �����ļ����������б�
////////////////////////////////////////////////////////////////////
class	whlistinfile
{
public:
	// ��ʼ����Ϣ
	struct	INFO_T
	{
		int	nMaxMsgNum;										// ������Ϣ����Ŀ��
		int	nMsgTypeNum;									// ��Ϣ��������
		int	nUnitDataSize;									// ÿ����Ŀ�����ݲ��ֳ���
	};
	#pragma pack(1)
	struct	HDR_T
	{
		static const char *cszMagic;
		enum
		{
			MAGICSIZE	= 5,
			MAXEXT		= 4,
			VER			= 1,
		};
		// 2005-10-12 ��ԭ��8�ֽڵ�szMagic��Ϊ�����szMagic/nReserved/nCRC16
		char	szMagic[MAGICSIZE];							// �ļ�ħ����
		char	nReserved;									// ����������
		unsigned short	nCRC16;								// У����(����m_listhdrs��m_list�����ݲ���CRCУ��Ľ��)
		short	nVer;										// �汾
		short	nListTypeNum;								// �����б�ĸ���
		int		nUnitSize;									// �ڲ��õĵ�Ԫȫ��
		int		nUnitDataSize;								// ���������ݲ��ֵĳ���
		void	clear()
		{
			memset(this, 0, sizeof(*this));
			memcpy(szMagic, cszMagic, MAGICSIZE);
			nVer	= VER;
		}
		inline bool	isgood() const
		{
			if( memcmp(szMagic, cszMagic, MAGICSIZE)==0
			&&  nVer == VER
			&&  nListTypeNum>0
			&&  nUnitDataSize>0
			&&  nUnitSize>nUnitDataSize
			)
			{
				return	true;
			}
			return		false;
		}
	};
	struct	ONELIST_T
	{
		int		nHead;										// �б�ͷ
		int		nTail;										// �б�β
		int		nTotal;										// ������
		void	clear()
		{
			nHead	= whalist::INVALIDIDX;
			nTail	= whalist::INVALIDIDX;
			nTotal	= 0;
		}
	};
	// �ļ��У�HDR_T�������nListTypeNum��LISTHDR_T
	// �ڱ����Ŀ��������У����Խ���Խ���Ȳ���
	struct	LISTHDR_T
	{
		int			nCurIdxInner;							// ��ǰ�ɶ������Ѿ�Read���ڲ����
		ONELIST_T	UnReadList;
		ONELIST_T	ReadList;
		void	clear()
		{
			nCurIdxInner	= whalist::INVALIDIDX;
			UnReadList.clear();
			ReadList.clear();
		}
		void	reset()										// ���¿�ʼ��
		{
			nCurIdxInner	= ReadList.nTail;
		}
	};
	// һ�����Ԫ
	struct	UNIT_T
	{
		enum
		{
			PROP_READ	= 0x01,								// �Ѷ�
		};
		int				nType;
		int				nPrev;								// ��Ϊalistֻ�ṩnext��Ϊ����˫������
		unsigned char	nProp;
		char			data[1];
		//��ʼ��
		inline void	Reset(int __nType)
		{
			nType	= __nType;
			nProp	= 0;
		}
		// ���ʷ���
		inline bool	IsRead() const
		{
			return	(nProp & PROP_READ) != 0;
		}
		inline void	SetRead()
		{
			nProp	|= PROP_READ;
		}
		inline void	SetUnRead()
		{
			nProp	^= PROP_READ;
		}
		static inline int	GetTotalSize(int nDSize)
		{
			return	nDSize + (sizeof(UNIT_T)-wh_sizeinclass(UNIT_T, data));
		}
		static inline int	GetDataSize(int nTSize)
		{
			return	nTSize - (sizeof(UNIT_T)-wh_sizeinclass(UNIT_T, data));
		}
	};
	#pragma pack()
protected:
	// �Լ����ļ�ͷ
	HDR_T				m_hdr;
	// ������ͷ
	whvector<LISTHDR_T>	m_listhdrs;
	// ���������ݵ��ڴ�ӳ��
	whvector<char>		m_vectFileData;
	// ������Ϣ����ı�
	whalist				m_list;
	// �Ƿ��޸�������(�޸��˾���Close�д���)
	bool				m_bModified;
	// �ײ��ļ�
	whfile				*m_pfile;
public:
	inline LISTHDR_T *	GetListHdr(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_hdr.nListTypeNum )
		{
			return	NULL;
		}
		return	m_listhdrs.getptr(nIdx);
	}
	inline whalist *	GetAList()
	{
		return	&m_list;
	}
	inline void			SetModified()
	{
		m_bModified		= true;
	}
	inline bool			GetModified() const
	{
		return	m_bModified;
	}
public:
	whlistinfile();
	~whlistinfile();
	// ���ھ����ļ��Ĵ�/�ر����ϲ㸺��
	// �����ļ�
	int		Create(whfile *file, INFO_T *pInfo);
	// ���ļ�
	int		Open(whfile *file, int nNewMaxNum, int nNewnUnitDataSize);
	// �ر��ļ�
	int		Close();
	// ���루�ᶥ���Ѿ������ģ�����Լ�nTypeС��δ�����ݣ�
	int		push_back(int nType, void *pData, int nSize);
	// ����±�ΪnIdx������(nIdx���б�ͷ��ʼΪ0)
	int		getbegin(int nType);
	int		getnext(int nType, void *pData, int *pnIdxInner);
	// ɾ��ĳ���ڲ�id������
	int		delinner(int nIdxInner);
	// ���ĳ���ڲ�idΪδ��(�����ԭ�����Ѷ��Ļ�)
	int		unreadinner(int nIdxInner);
	// �����ļ������𻵵Ĳ��÷�ʽ
	int		savebadfile();
	// �����Լ�������������дINFO_T
	int		refillinfo(INFO_T *pInfo);
	// ����CRCУ����
	unsigned short	CalcCRC() const;
	// У���ļ��������Ƿ�Ϸ�
	enum
	{
		VERIFY_RST_OK	= 0,
		VERIFY_RST_FAIL	= -1,
	};
	int		verifydata();
private:
	// ��鲢���������������Ҫ����Ŀ
	// �������ʾ���Բ���
	bool	checkandkick(int nTypeToAdd);
	// ��һ����Ԫ����һ������
	int		push_back(ONELIST_T *pOL, int nIdxInner);
	//int		push_front(ONELIST_T *pOL, int nIdxInner);
	// ��һ����Ԫ�������в�����ϲ��Ҫ��֤nIdxInner�����pOL��
	int		remove(ONELIST_T *pOL, int nIdxInner);
};

////////////////////////////////////////////////////////////////////
// ֱ�Ӵ���ͨ�ļ�����whfile����
// szModeһ����Ƕ����ƵĴ򿪷�ʽ������"rb"��"wb"�ȵ�
// �����Ͽ��Բ�����Close������ֻ��Ϊ�˺���ͨ���ļ�����һ�²�д��
////////////////////////////////////////////////////////////////////
whfile *	whfile_OpenCmnFile(const char *szFileName, const char *szMode);
// �ر���ͨ�ļ��������ֻ��Ϊ����Ͼɵ���дϰ�ߣ��ڲ�ʵ���Ͼ���ɾ��file����
void		whfile_CloseCmnFile(whfile *file);

// ���ڴ洴��whfile����
whfile *	whfile_OpenMemBufFile(char *szMemBuf, size_t nMemSize);
void		whfile_CloseMemBufFile(whfile *file);

// ����file�������md5ֵ
unsigned char	*whfile_CalcMD5(whfile *file, unsigned char *MD5);

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHCMN_FILE_STATIC_INFO_Out();
void	WHCMN_FILE_STATIC_INFO_In(void *pInfo);
// ���ȫ�ֵ��ļ��������������Ҫ�Ǹ�һЩ����ģ���Ը�������ļ��������Ķ����õģ�
whfileman *	WHCMN_FILEMAN_GET();
void	WHCMN_FILEMAN_SET(whfileman *pFM);

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILE_H__
