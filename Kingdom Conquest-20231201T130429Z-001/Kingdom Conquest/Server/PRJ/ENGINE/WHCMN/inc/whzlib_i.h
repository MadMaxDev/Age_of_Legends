// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whzlib
// File        : whzlib_i.h
// Creator     : Wei Hua (κ��)
// Comment     : whzlib���ڲ�ʵ��ͷ�ļ�
// CreationDate: 2003-09-04
// ChangeLOG   :
//               2004-02-16 �����˵�һ�ļ�ѹ�� 
//               2004-05-31 Ϊ�������е�whfile������ֱ�ӵ���deleteɾ����ȡ����ԭ���ڸ���man�е�whhashset<whzlib_file_i *>	m_ptrset;

#ifndef	__WHZLIB_I_H__
#define __WHZLIB_I_H__

#include "whzlib.h"
#include "whfile_util.h"
#include "whmd5.h"
#include "whdes.h"

using namespace n_whcmn;

namespace n_whzlib
{

class	whzlib_fileman_i;
class	whzlib_pck_reader_i;
class	whzlib_pck_modifier_i;

// һЩ����
enum
{
	PCKPASSWORD_MAXLEN		= 128,
};

// ͨ������ļ���ʵ��(��whzlib_pck_i.h)
typedef	n_whcmn::whfile_i	whzlib_file_i;

// ������ʵ��
class	whzlib_fileman_i	: public whzlib_fileman
{
// Ϊ�ϲ�������
public:
	// ����������
	whzlib_fileman_i();
	~whzlib_fileman_i();
public:
	void	SetSearchMode(int nMode);
	int		GetSearchMode() const;
	void	SetPassword(const char *szPassword);
	int		LoadPckToMem(const char *szPckName);
	int		UnLoadPckFromMem(const char *szPckName);
	int		LoadFileToMem(const char *szFileName);
	int		UnLoadFileFromMem(const char *szFileName);
	whzlib_file *	Open(const char *szFileName, const char *szMode);
	whzlib_file *	Open(int nID);
	bool	Close(whzlib_file *file);
	int		GetFileSize(const char *szFileName);
	bool	IsFileExist(const char *szFileName);
	n_whcmn::WHDirBase *	OpenDir(const char *szDirName);
// �Լ�ʹ�õ�
private:
	// �Ƿ����ȴӴ��ڰ��ļ���ȡ(Ĭ����MOD_PCK_FILE)
	int		m_nSearchMode;
	// ��ѹ����
	char	m_szPassword[PCKPASSWORD_MAXLEN];
	// �ж�һ�����Ƿ��Ѿ�����
	whhash<whstr4hash, whzlib_pck_reader_i *, whcmnallocationobj, _whstr_hashfunc>	m_mapName2Pck;
	// �ļ��ڴ�
	whsmpfilecache			m_fcache;
private:
	// ��鲢����Reader
	whzlib_pck_reader_i *	CheckAndLoadReader(const char *szPath);
	// ���ļ�
	whzlib_file_i *	OpenFromDisk(const char *szFileName);
	whzlib_file_i *	OpenFromDisk(const char *szFileName, const char *szMode);
	whzlib_file_i *	OpenFromPackage(const char *szFileName);
	// ��Ŀ¼
	WHDirBase *	OpenDirFromDisk(const char *szDirName);
	WHDirBase *	OpenDirFromPackage(const char *szDirName);
	// ����ļ�����
	int		GetFileSizeFromPackage(const char *szFileName);
	bool	IsFileExistFromPackage(const char *szFileName);
	// Ѱ��szFileName�Ƿ�����ڰ��ļ��У���������ļ����ֽ�Ϊ������԰����ļ���
	// ��������û��"/"��β
	// ����ͺ����LookForPackageByfile����һ����ֻ�������ȴ��ڴ�����
	bool	_LookForPackageByfile(const char *szFileName, char *szPath, char *szFile, bool bIsDir);
	// �ж�·���Ƿ�������ڴ����Ӳ����
	bool	_IsFileExistInMemOrOnDisk(const char *szPath);
};

class	whzlib_filemodifyman_i	: public whzlib_filemodifyman
{
public:
	whzlib_filemodifyman_i();
	~whzlib_filemodifyman_i();
// �Լ�ʹ�õ�
public:
private:
	// ��ѹ����
	char	m_szPassword[PCKPASSWORD_MAXLEN];
	// �ж�һ�����Ƿ��Ѿ�����
	whhash<whstr4hash, whzlib_pck_modifier_i *, whcmnallocationobj, _whstr_hashfunc>	m_mapName2Pck;
public:
	void	SetPassword(const char *szPassword);
	int		PutFile(const char *szFileName, whzlib_file *file, bool bShouldInPck, int nCompressMode);
	int		DelFile(const char *szFileName);
	int		DelDir(const char *szDir);
	int		FlushAll();
private:
	// ��鲢����Modifier
	// bForceCreate��ʾ���ԭ�������ھ�Create֮
	whzlib_pck_modifier_i *		CheckAndLoadModifier(const char *szPath, bool bForceCreate);
	int		PutFileToDisk(const char *szFileName, whzlib_file *file);
};

////////////////////////////////////////////////////////////////////
// ��һ�ļ�ѹ�� single compress
////////////////////////////////////////////////////////////////////
// ��һѹ���ļ���ʽ��飺
// ͷ��64�ֽ� (����+junk)
// �ֶ�������nParts��int
// ѹ�����ݲ���
#pragma pack(1)
// �ļ�ͷ����
struct	sc_file_hdr_t
{
	static const char *	CSZ_MAGIC;
	struct	hdr_t
	{
		char			szMagic[16];		// �ļ���־
		unsigned int	nVer;				// �汾
		unsigned int	nUnitSize;			// ��������δѹ��ǰ�ĳߴ磬�������һ��һ�㶼һ����Ĭ��Ϊ65536
		time_t			time;				// ��ѹ��ǰ���ļ�ʱ��
		size_t			nFileSize;			// �ļ�����
		int				nParts;				// �ܹ����ָ������
		unsigned char	md5[16];			// �ļ�md5У���
	}hdr;
	char	junk[64-sizeof(hdr_t)];			// �������ݣ�����ͷΪһ������(64�ֽ�)
	sc_file_hdr_t()
	{
		memset(&hdr, 0, sizeof(hdr));		// ��������
		strcpy(hdr.szMagic, CSZ_MAGIC);
		hdr.nVer		= WHZLIB_SC_VER;
		hdr.nUnitSize	= 0x10000;
		hdr.time		= 0;
		hdr.nFileSize	= 0;
		hdr.nParts		= 0;
	}
	inline bool	IsMagicGood() const
	{
		return	strcmp(CSZ_MAGIC, hdr.szMagic) == 0;
	}
	inline bool	IsVerGood() const
	{
		return	hdr.nVer >= WHZLIB_SC_LOWRVER;
	}
};
// �����ļ�������������ͷ
struct	sc_dataunit_hdr_t
{
	unsigned char			nMode;			// ѹ��ģʽ(�����޷�ѹ�������ݻ��Զ�ʹ�÷�ѹ��ģʽ)
	size_t					nSize;			// ����������ݳ���
};
#pragma pack()
// ��ͨ��zip�ļ���ѹ����ָ���Ĵ����ļ�
int		whzlib_single_compress_i(whzlib_file *file, const char *cszDstFile, const char *cszPasswd);
// ���Դ�����Ƿ��ǵ��ļ�ѹ�������ݣ������hdr�о����ļ�ͷ
bool	whzlib_issinglecompressed_i(whzlib_file *srcfile, sc_file_hdr_t *hdr);
// 
class	whzlib_file_i_sc	: public whzlib_file_i
{
private:
	whzlib_file			*m_srcfile;
	sc_file_hdr_t		m_schdr;

	whvector<char>		m_vectbuf;			// ���ڳ���һ�ν�ѹ�����ݵ�buffer������(ֻ��initʱ����һ�Σ�����ָ����Զ��Ч)
	char				*m_buf;				// ָ��buffer��ָ��
	int					m_bufsize;			// Ŀǰbuffer�����ݵ��ܳ���
	int					m_leftsize;			// Ŀǰbuffer�����ݻ��ɶ��ĳ���
	int					m_curpart;			// ��ǰ�Ĳ���
	int					m_totalpart;		// �ܹ��Ĳ�������

	whvector<char>		m_vectsrcbuf;		// ��ʱ��Դ�ļ��ж�ȡ���ݵĻ���
	char				*m_srcbuf;			// ָ��srcbuffer��ָ��

	// �ͼӽ�����ص�
	WHSimpleDES			m_des;
	char				m_szPassword[PCKPASSWORD_MAXLEN];
public:
	whzlib_file_i_sc();
	~whzlib_file_i_sc();
public:
	// ��ͨ������ʼ�����������ѹ���ļ��򷵻ش�
	int		InitFrom(whzlib_file *srcfile);
	// ���ý�ѹ����
	void	SetPassword(const char *szPassword);
public:
	void	SelfDestroy()
	{
		delete	this;
	}
	// Ϊ����ʵ�ֵĽӿ�
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		GetMD5(unsigned char *MD5);
private:
	// ��ȡһ���ļ�����ĵ�N������(���峤���㹻)
	// ����ʵ�����ݳ���
	// ����0��ʾ�Ѿ�û�������������ˣ����߱�ʾ����
	int		GetDataPart(int nPart, char *pBuf);
};

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_I_H__
