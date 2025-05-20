// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whzlib
// File        : whzlib_i.h
// Creator     : Wei Hua (魏华)
// Comment     : whzlib的内部实现头文件
// CreationDate: 2003-09-04
// ChangeLOG   :
//               2004-02-16 增加了单一文件压缩 
//               2004-05-31 为了让所有的whfile都可以直接调用delete删除，取消了原来在各个man中的whhashset<whzlib_file_i *>	m_ptrset;

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

// 一些常量
enum
{
	PCKPASSWORD_MAXLEN		= 128,
};

// 通过打包文件的实现(见whzlib_pck_i.h)
typedef	n_whcmn::whfile_i	whzlib_file_i;

// 管理器实现
class	whzlib_fileman_i	: public whzlib_fileman
{
// 为上层派生的
public:
	// 创建与销毁
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
// 自己使用的
private:
	// 是否优先从存在包文件读取(默认是MOD_PCK_FILE)
	int		m_nSearchMode;
	// 解压密码
	char	m_szPassword[PCKPASSWORD_MAXLEN];
	// 判断一个包是否已经被打开
	whhash<whstr4hash, whzlib_pck_reader_i *, whcmnallocationobj, _whstr_hashfunc>	m_mapName2Pck;
	// 文件内存
	whsmpfilecache			m_fcache;
private:
	// 检查并载入Reader
	whzlib_pck_reader_i *	CheckAndLoadReader(const char *szPath);
	// 打开文件
	whzlib_file_i *	OpenFromDisk(const char *szFileName);
	whzlib_file_i *	OpenFromDisk(const char *szFileName, const char *szMode);
	whzlib_file_i *	OpenFromPackage(const char *szFileName);
	// 打开目录
	WHDirBase *	OpenDirFromDisk(const char *szDirName);
	WHDirBase *	OpenDirFromPackage(const char *szDirName);
	// 获得文件长度
	int		GetFileSizeFromPackage(const char *szFileName);
	bool	IsFileExistFromPackage(const char *szFileName);
	// 寻找szFileName是否可能在包文件中，如果是则将文件名分解为包和相对包的文件名
	// 包名后面没有"/"结尾
	// 这个和后面的LookForPackageByfile功能一样，只不过会先从内存中找
	bool	_LookForPackageByfile(const char *szFileName, char *szPath, char *szFile, bool bIsDir);
	// 判断路径是否存在于内存或者硬盘中
	bool	_IsFileExistInMemOrOnDisk(const char *szPath);
};

class	whzlib_filemodifyman_i	: public whzlib_filemodifyman
{
public:
	whzlib_filemodifyman_i();
	~whzlib_filemodifyman_i();
// 自己使用的
public:
private:
	// 解压密码
	char	m_szPassword[PCKPASSWORD_MAXLEN];
	// 判断一个包是否已经被打开
	whhash<whstr4hash, whzlib_pck_modifier_i *, whcmnallocationobj, _whstr_hashfunc>	m_mapName2Pck;
public:
	void	SetPassword(const char *szPassword);
	int		PutFile(const char *szFileName, whzlib_file *file, bool bShouldInPck, int nCompressMode);
	int		DelFile(const char *szFileName);
	int		DelDir(const char *szDir);
	int		FlushAll();
private:
	// 检查并载入Modifier
	// bForceCreate表示如果原来不存在就Create之
	whzlib_pck_modifier_i *		CheckAndLoadModifier(const char *szPath, bool bForceCreate);
	int		PutFileToDisk(const char *szFileName, whzlib_file *file);
};

////////////////////////////////////////////////////////////////////
// 单一文件压缩 single compress
////////////////////////////////////////////////////////////////////
// 单一压缩文件格式简介：
// 头：64字节 (有用+junk)
// 分段索引：nParts个int
// 压缩数据部分
#pragma pack(1)
// 文件头定义
struct	sc_file_hdr_t
{
	static const char *	CSZ_MAGIC;
	struct	hdr_t
	{
		char			szMagic[16];		// 文件标志
		unsigned int	nVer;				// 版本
		unsigned int	nUnitSize;			// 单块数据未压缩前的尺寸，除了最后一块一般都一样，默认为65536
		time_t			time;				// 被压缩前的文件时间
		size_t			nFileSize;			// 文件长度
		int				nParts;				// 总共被分割的数量
		unsigned char	md5[16];			// 文件md5校验核
	}hdr;
	char	junk[64-sizeof(hdr_t)];			// 垃圾数据，保持头为一定长度(64字节)
	sc_file_hdr_t()
	{
		memset(&hdr, 0, sizeof(hdr));		// 清零所有
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
// 数据文件－－单段数据头
struct	sc_dataunit_hdr_t
{
	unsigned char			nMode;			// 压缩模式(对于无法压缩的数据会自动使用非压缩模式)
	size_t					nSize;			// 后面跟的数据长度
};
#pragma pack()
// 将通用zip文件流压缩到指定的磁盘文件
int		whzlib_single_compress_i(whzlib_file *file, const char *cszDstFile, const char *cszPasswd);
// 检查源数据是否是单文件压缩的数据，如果是hdr中就是文件头
bool	whzlib_issinglecompressed_i(whzlib_file *srcfile, sc_file_hdr_t *hdr);
// 
class	whzlib_file_i_sc	: public whzlib_file_i
{
private:
	whzlib_file			*m_srcfile;
	sc_file_hdr_t		m_schdr;

	whvector<char>		m_vectbuf;			// 用于承载一段解压后数据的buffer分配器(只在init时分配一次，所以指针永远有效)
	char				*m_buf;				// 指向buffer的指针
	int					m_bufsize;			// 目前buffer中数据的总长度
	int					m_leftsize;			// 目前buffer中数据还可读的长度
	int					m_curpart;			// 当前的部分
	int					m_totalpart;		// 总共的部分数量

	whvector<char>		m_vectsrcbuf;		// 临时从源文件中读取数据的缓冲
	char				*m_srcbuf;			// 指向srcbuffer的指针

	// 和加解密相关的
	WHSimpleDES			m_des;
	char				m_szPassword[PCKPASSWORD_MAXLEN];
public:
	whzlib_file_i_sc();
	~whzlib_file_i_sc();
public:
	// 从通用流初始化，如果不是压缩文件则返回错
	int		InitFrom(whzlib_file *srcfile);
	// 设置解压密码
	void	SetPassword(const char *szPassword);
public:
	void	SelfDestroy()
	{
		delete	this;
	}
	// 为父类实现的接口
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		GetMD5(unsigned char *MD5);
private:
	// 读取一个文件对象的第N段数据(缓冲长度足够)
	// 返回实际数据长度
	// 返回0表示已经没有这样的数据了，或者表示出错
	int		GetDataPart(int nPart, char *pBuf);
};

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_I_H__
