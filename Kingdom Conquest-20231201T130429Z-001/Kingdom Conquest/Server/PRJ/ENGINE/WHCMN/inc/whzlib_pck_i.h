// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whzlib
// File         : whzlib_pck_i.h
// Creator      : Wei Hua (魏华)
// Comment      : 和具体打包相关的功能实现
//                如果需要对包进行具体操作，比如交互地添加或删除文件，则需要include这个文件
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

// 文件后缀
extern const char *cstIdxFileExt;
extern const char *cstDatFileExt;
extern const char *cstNamFileExt;

// 和pck相关的类型定义
// 数据单元长度
typedef	unsigned int	whzlib_unitsize_t;
typedef unsigned char	whzlib_filenamesize_t;
// 各种结构
#pragma pack(1)
// 信息文件头(里面主要为文件基本信息和文件索引)
struct	infofile_hdr_t
{
	static const char *	CSZ_MAGIC;
	struct	hdr_t
	{
		char	szMagic[16];				// 文件标志
		unsigned int	nVer;				// 版本
		unsigned int	nUnitSize;			// 单块数据未压缩前的尺寸，除了最后一块一般都一样，默认为65536
											// 这里使用unsigned int是为了免得将来万一想whzlib_unitsize_t变小
		unsigned int	nWastedSize;		// 废弃的空间字节数(一般是由于删除和替换文件造成的)
		// 现在已经有32字节了
	}hdr;
	char	junk[64-sizeof(hdr_t)];			// 垃圾数据，保持头为一定长度(64字节)
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
// 信息文件中的－－文件信息单元
struct	infofile_fileunit_t
{
	enum
	{
		TYPE_NOTHING		= 0,			// 未知类型
		TYPE_FILE			= 1,			// 文件
		TYPE_DIR			= 2,			// 目录
		TYPE_EOD			= 3,			// 目录结尾(End Of Dir)
	};
	unsigned char			type;			// 类型(文件/目录)
	unsigned int			namepos;		// 名字在名字文件中的位置
	time_t					time;			// 文件时间
	unsigned char			md5[16];		// 文件md5校验核
	union
	{
		struct
		{
			unsigned int	datapos;		// 数据在数据文件中的起始位置
			unsigned int	totaldatasize;	// 文件的总数据长度
		}file;
		struct
		{
			unsigned int	entrynum;		// 该目录下的本级入口数
			unsigned int	totalentrynum;	// 该目录下的各级入口总数(不包括该目录项和EOD)
											// 如果目录下没有子目录则entrynum==totalentrynum
		}dir;
	}info;
};
// 数据文件－－文件数据总头
// 这个总头后边会跟着nParts个整数，表示后面的各段压缩数据的起始位置（第一段为0）
// 除了最后一段，每段数据解压后的长度都应该为infofile_hdr_t::nUnitSize
// 所以想要读取某个位置可以迅速通过这张表获得数据的具体位置，因此所有的seek都可以实现了
// 唯一需要的是访问文件时要将该表读入内存。不过即使是4G的文件，如果按64k分段，
// 这张表也不过是不到300k的样子，用空间换时间嘛，还是可以接受的。
struct	datafile_file_hdr_t
{
	enum
	{
		PROPERTY_DELETED	= 0x01,			// 已经被删除的文件数据
	};
	struct	hdr_t
	{
		unsigned char		nProperty;		// 属性
		int					nParts;			// 总共被分割的数量
	}hdr;
	char	junk[16-sizeof(hdr_t)];			// 垃圾数据，保持头为一定长度
};
// 数据文件－－单段数据头
struct	datafile_dataunit_hdr_t
{
	unsigned char			nMode;			// 压缩模式(对于无法压缩的数据会自动使用非压缩模式)
	whzlib_unitsize_t		nSize;			// 后面跟的数据长度
};
#pragma pack()

// 扩展的用于内存中的文件数据单元
struct	EXTFILEUNIT_T
{
	struct	DIRINFO
	{
		whhash<whstr4hash, int, whcmnallocationobj, _whstr_hashfunc>	mapName2ID;
											// 本目录下文件名到入口的映射关系
	};
	infofile_fileunit_t		infile;
	DIRINFO					*pDirInfo;		// 有必要才会有这个信息
	EXTFILEUNIT_T()
	: pDirInfo(NULL)
	{
	}
};
struct	EXTUNIT_T
{
	int		nFatherID;						// 父目录的ID
};
typedef	whvector<EXTFILEUNIT_T>				FILEEXTINFOLIST_T;
typedef	whvector<infofile_fileunit_t>		FILEINFOLIST_T;
typedef	whvector<EXTUNIT_T>					FILEEXTLIST_T;

class	whzlib_pck_reader_i;
class	whzlib_pck_modifier_i;

// 通过打包文件的whzlib_file_i实现
class	whzlib_file_i_pck	: public whzlib_file_i
{
public:
	struct	INFO_T
	{
		unsigned int	totaldatasize;		// 文件的总数据长度
		unsigned int	nUnitSize;			// 单段未压缩数据的长度
		whzlib_pck_reader_i	*pReader;		// pck文件由这个对象打开，其具体数据访问通过它进行
		int				nFileID;			// 在Reader中的文件ID
	};
	INFO_T				m_info;				// 信息数据
private:
	whvector<char>		m_vectbuf;			// 用于承载一段解压后数据的buffer分配器(只在init时分配一次，所以指针永远有效)
	char				*m_buf;				// 指向buffer的指针
	int					m_bufsize;			// 目前buffer中数据的总长度
	int					m_leftsize;			// 目前buffer中数据还可读的长度
	int					m_curpart;			// 当前的部分
	int					m_totalpart;		// 总共的部分数量
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
	// 初始化(在m_info被设置之后调用这个)
	int		Init();
	// 销毁
	int		Release();
};

// 在浏览目录时可以把包当作目录的
class	WHDirCarePck	: public n_whcmn::WHDir
{
public:
	// 里面遇到.idx和.nam文件都忽略，遇到.dat读入
	// 当然还要判断三个文件是否都同时存在
	ENTRYINFO_T *	Read();
};

// 通过打包文件的WHDirBase实现
// 给reader用的
class	WHDirInPck		: public n_whcmn::WHDirBase
{
public:
	static whzlib_fileman	*m_pFMan;		// 如果写了Open就可以通过这个打开
	struct	INFO_T
	{
		whzlib_pck_reader_i	*pReader;		// pck文件由这个对象打开，其具体数据访问通过它进行
		int				nDirID;				// 目录中第一个文件在Reader中的文件ID
		INFO_T() : pReader(NULL), nDirID(-1)
		{}
	};
	INFO_T				m_info;				// 信息数据
private:
	n_whcmn::WHDirBase::ENTRYINFO_T	m_EntryInfo;
											// 用于向外界返回一个结果
	EXTFILEUNIT_T		*m_pUnit;			// 第一个Unit
	int					m_nIdx;				// 当前搜索到的条目的索引
	int					m_nLevel;			// 目前搜索到的级别，在0级发现EOD就说明结束了
public:
	WHDirInPck();
	~WHDirInPck();
	void			SelfDestroy()	{delete this;}
	const char *	GetDir() const	{assert(0); return NULL;}	// 这个只是为了兼容旧代码加上的，不应该被调用
	int				Close();
	ENTRYINFO_T *	Read();
	void			Rewind();
	static WHDirBase *	Create(const char *szDir);
};
// 给modifier用的
class	WHDirInPck4Mod	: public n_whcmn::WHDirBase
{
public:
	struct	INFO_T
	{
		whzlib_pck_modifier_i	*pReader;	// pck文件由这个对象打开，其具体数据访问通过它进行
		int				nDirID;				// 目录中第一个文件在Reader中的文件ID
	};
	INFO_T				m_info;				// 信息数据
private:
	n_whcmn::WHDirBase::ENTRYINFO_T	m_EntryInfo;
											// 用于向外界返回一个结果
	infofile_fileunit_t	*m_pUnit;			// 第一个Unit
	int					m_nIdx;				// 当前搜索到的条目的索引
	int					m_nLevel;			// 目前搜索到的级别，在0级发现EOD就说明结束了
public:
	WHDirInPck4Mod();
	void			SelfDestroy()	{delete this;}
	const char *	GetDir() const	{assert(0); return NULL;}	// 这个只是为了兼容旧代码加上的，不应该被调用
	int				Close();
	ENTRYINFO_T *	Read();
	void			Rewind();
};


// 压缩文件读取器(因为外界不需要，所以就不在上层提供接口了，加_i是为了在本层看起来统一)
class	whzlib_pck_reader_i
{
private:
	n_whcmn::WHSimpleDES	m_des;
public:
	whzlib_pck_reader_i();
	~whzlib_pck_reader_i();
private:
	// 索引文件头
	infofile_hdr_t		m_infohdr;
	// 密码，如果有的话(空字串表示没有密码)
	char				m_szPassword[PCKPASSWORD_MAXLEN];
	// 存放文件索引表(里面的存放形式决定了目录结构，文件排列顺序由插入顺序决定)
	FILEEXTINFOLIST_T	m_finfolist;
	// 三个相关文件的指针
	whfile	*m_fileIdx, *m_fileDat, *m_fileNam;
	// 是否自动删除文件句柄
	bool	m_bAutoClosePackFile;
	// 是否大小写敏感
	bool	m_bIC;
	// 用于读入一个数据单元
	whvector<char>		m_vectbuf;
	char	*m_buf;
	// 存放所有文件名(直接读入文件名内容到这个缓冲)
	whvector<char>		m_vectnames;
public:
	// 设置是否大小写敏感(这个会影响到所有使用到whstr4hash的地方)
	// inline static void	SetIC(bool bIC)
	// {
	// 	n_whcmn::whstr4hash::m_bIC	= bIC;
	// }	2006-07-12 取消这个设置，文件名在windows下也强制大小写敏感
	// 设置解压密码
	void	SetPassword(const char *szPassword);

	// 打开一个包
	int	OpenPck(const char *szPckFile);
	// 打开一个包，如果内存cache中有就优先打开内存中的文件
	int	OpenPck(const char *szPckFile, whsmpfilecache *pCache);
	// 比较低级的根据文件句柄打开包
	int	OpenPck(whfile *pfileIdx, whfile *pfileDat, whfile *pfileNam, bool bAutoClosePackFile);
	// 关闭包
	int	ClosePck();

	// 创建nFileID对应的whzlib_file_i_pck对象(对象用完后删除即可)
	whzlib_file_i_pck *	OpenFile(int nFileID);
	// 注意：下面两个函数中的szFile都是相对于包根部的路径名，前后都没有"/"或"\"
	// 根据文件名创建whzlib_file_i_pck对象
	whzlib_file_i_pck *	OpenFile(const char *szFile);

	// 创建目录对象
	WHDirInPck *		OpenDir(int nDirID);
	WHDirInPck *		OpenDir(const char *szDir);

	// 根据文件名获得文件或目录ID
	int	GetPathID(const char *szPath);
	// 根据文件名获得文件长度，返回-1表示文件不存在
	int	GetFileSize(const char *szFile);
	// 读取一个文件对象的第N段数据(缓冲长度足够)
	// 返回实际数据长度
	// 返回0表示已经没有这样的数据了，或者表示出错
	int	GetDataPart(int nFileID, int nPart, char *pBuf);
	// 获得名字指针
	char *	GetFileNamePtr(int nOff);
	// 获得指定序号的文件入口的指针
	inline EXTFILEUNIT_T *	GetUnitPtr(int nFileID) const
	{
		return	m_finfolist.getptr(nFileID);
	}
private:
	// 以一定模式打开所有的相关文件
	int	OpenAllFileOnDisk(const char *szPckFile, const char *szMode);
	// 从cache或硬盘上打开所有相关文件
	int	OpenAllFileFromMemOrOnDisk(const char *szPckFile, whsmpfilecache *pCache);
	// 在所有文件已经打开并且设置好各个必要参数的前提下，打开包
	int	OpenPack();
	// 从一个目录单元开始找本级下第一个匹配的入口名
	// 成功：返回值为找到的序号，失败：-1
	int	FindEntry(int nDirID, const char *szName);
};

// 包修改对象
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

	// 创建目录对象
	WHDirBase *	OpenDir(int nDirID);
	WHDirBase *	OpenDir(const char *szDir);

	// 设置解压密码(这个必须在CreatePck或OpenPck之后调用)
	void	SetPassword(const char *szPassword);
	// 将内存内容flush到硬盘上
	int		Flush();
// 自己用的
public:
	// 初始化(这里为了避免文件不能打开的状况)
	int	CreatePck(CREATEINFO_T *pInfo);
	int	OpenPck(OPENINFO_T *pInfo);
	int	ClosePck();
	// 根据文件名获得文件或目录ID
	int	GetPathID(const char *szPath);
	// 寻找路径已经存在的部分，返回存在部分的ID，在szNew中返回不存在的部分
	int	LookForExistPath(const char *szPath, char *szNew);
	// 获得名字指针(不过需要立即使用)
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
	// 获得指定序号的文件入口的指针
	inline infofile_fileunit_t *	GetUnitPtr(int nPathID) const
	{
		if( nPathID<0 || nPathID>=int(m_finfolist.size()) )
		{
			return	NULL;
		}
		return	m_finfolist.getptr(nPathID);
	}
private:
	// 是否是刚创建的文件
	bool	m_bJustCreated;
	// 文件是否被修改，是否需要回写索引文件
	bool	m_bModified;
	// 文件名
	char	m_szPckFile[WH_MAX_PATH];
	char	m_szTmpFileName[WH_MAX_PATH];
	// 三个相关文件的指针
	FILE	*m_fpIdx, *m_fpDat, *m_fpNam;
	// 索引文件头
	infofile_hdr_t		m_infohdr;
	// 密码，如果有的话(空字串表示没有密码)
	char				m_szPassword[PCKPASSWORD_MAXLEN];
	// 存放文件索引表(里面的存放形式决定了目录结构，文件排列顺序由插入顺序决定)
	FILEINFOLIST_T		m_finfolist;
	// 附加信息列表，和文件索引列表一一对应
	FILEEXTLIST_T		m_fextlist;
	// 专门为append做的当前目录栈
	whvector<int>		m_vectDirStack;
private:
	// 打开所有相关的三个文件
	int	OpenAllFileOnDisk(const char *szPckFile, const char *szMode);
	// 添加文件数据到数据文件(同时更新infofile_fileunit_t中的md5和文件时间)
	int	WriteData(infofile_fileunit_t *pFUnit, whzlib_file *file, int nCompressMode);
	// 添加文件名到文件名文件
	int	WriteFileName(const char *szFileName);
	// 从特定位置读取文件名，返回文件名全长（包含最后的'\0'）
	// 返回-1表示没有
	int	ReadFileName(int nPos, char *szFileName, int nSize);
	// 从一个目录单元开始找本级下第一个匹配的入口名
	// 成功：返回值为找到的序号，失败：-1
	int	FindEntry(int nDirID, const char *szName);
	// 在一个目录下添加一个子目录(这个保证nDirID绝对存在)，返回新目录的ID
	int	AddDirToDir(int nDirID, const char *szName);
	// 在一个目录下添加一个文件，返回新文件的ID
	int	AddFileToDir(int nDirID, const char *szName, whzlib_file *file, int nCompressMode);
	// 为所有父目录的总入口数添加数量(也包括nDirID这一级)
	int	AddNumToAllFather(int nDirID, int nNum);
	// 即所有在nCheckAfter之后的元素，如果相关ID>nEffectAfter，则需要将需要+nAdjust
	int	AdjustExtUnit(int nCheckAfter, int nEffectAfter, int nAdjust);
	// 删除目录
	int	DelDir(int nDirID);
	// 删除文件
	int	DelFile(int nFileID);
};

// 从特定位置读取文件名，返回文件名全长（包含最后的'\0'）
int	whzlib_GetFilePartNum(int nTotalSize, int nUnitSize);
// 读写文件名
int	whzlib_ReadFileName(FILE *fp, int nPos, char *szFileName, int nSize);
int	whzlib_WriteFileName(FILE *fp, const char *szFileName);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_PCK_I_H__
