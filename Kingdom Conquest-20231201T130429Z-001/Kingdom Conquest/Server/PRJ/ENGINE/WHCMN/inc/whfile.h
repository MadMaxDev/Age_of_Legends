// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfile.h
// Creator      : Wei Hua (魏华)
// Comment      : 文件功能
// CreationDate : 2003-06-13
// ChangeLOG    : 2005-05-28 取消了iostream头文件，因为它和afx.h一起会导致很奇怪的现象
//                2005-10-12 给whlistinfile增加了文件损坏的补救措施（保存并清空旧文件）。并增加了文件校验码。
//                2006-02-17 把linux下的WH_MAX_PATH也改成260，和windows下面一样。
//                           给whfile_i_mem增加了GetCurBufPtr和GetSizeLeft两个方法。增加了Read和Write的模板函数。
//                2006-06-12 whfile_cpy的返回增加了定义。增加了whfile_cpy_calcmd5和whfile_cpy_cmp函数。

#ifndef	__WHFILE_H__
#define	__WHFILE_H__

#include "whfile_base.h"
#include "whcmn_def.h"
#include "whlist.h"
#include "whmd5.h"
//#include <iostream>

// 针对windows的定义
#if defined( WIN32 )
// 给access用的
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

// 是否是绝对路径
bool	whfile_isabspath(const char *__path);

// 判断路径是否是目录
bool	whfile_ispathdir(const char *__path);
// 判断路径是否存在
bool	whfile_ispathexisted(const char *__path);
// 判断文件的读写属性(注意，写属性在windows下还会受到是系统文件或隐藏文件的影响)
bool	whfile_ispathreadable(const char *__path);
bool	whfile_ispathwritable(const char *__path);
// 是否是系统文件(这个应该只在windows下有用)
bool	whfile_ispathsysfile(const char *__path);
// 获得可能超过4G的文件长度
whint64	whfile_getfilebigsize(const char *__path);
// 获得文件长度(因为老的程序中有很多应用到这个的地方，所以还是继续留着)
inline int		whfile_getfilesize(const char *__path)
{
	return	(int)whfile_getfilebigsize(__path);
}

// 支持4G以上的seek
int		whfseek(FILE *fp, whint64 nOffset, int nOrigin);

// 设置文件/目录可写(返回是否设置成功)
bool	whfile_makefilewritable(const char *__path);

// 获得文件的路径(结尾必须没有"/"或"\"，如果有就先主动调用wh_strkickendslash消除)
// 返回false表示没有路径，里面就是一个文件名
// __file和__path可以是同一个指针
bool	whfile_getfilepath(const char *__file, char *__path = 0);
// 获得文件名
// __file和__fname可以是同一个指针
const char *	whfile_getfilename(const char *__file, char *__fname);
// 获得路径和文件名(全路径名buffer会被改变)
// __fpath最后没有'/'或'\'结尾
// 如：c:\winnt\system32\something.dll会被分解为：c:\winnt\system32和something.dll
bool	whfile_splitfilenameandpath(char *__fullfilepath, const char **__fpath, const char **__fname);
// 将一个相对路径组合到一个基准路径上
const char *	whfile_makerealpath(const char *__basepath, const char *__reltivepath, char *__realpath);

// 删除文件(这个和whdir_sysdelfile功能完全一样，只是历史原因造成的两个名字)
int		whfile_del(const char *__file);
// 文件改名
int		whfile_ren(const char *__oldfile, const char *__newfile);
// 文件拷贝
// __cptime表示是否要设置新文件的时间和旧文件一致
// __nofileok表示没有源文件则拷贝成功(空到空)
// 返回
enum
{
	WHFILE_CPY_RST_OK		= 0,
	WHFILE_CPY_RST_ONSELF	= -1,		// 自己拷贝到自己
	WHFILE_CPY_RST_SRCERR	= -2,		// 源文件错误（读写）
	WHFILE_CPY_RST_DSTERR	= -3,		// 目标文件错误（读写）
	WHFILE_CPY_RST_SRC_O_ERR= -4,		// 源文件错误（打开）
	WHFILE_CPY_RST_DST_C_ERR= -5,		// 目标文件错误（创建）
};
int		whfile_cpy(const char *__dstfile, const char *__srcfile, bool __cptime=false, bool __nofileok=true);
// 拷贝文件并计算MD5码
// 返回同上WHFILE_CPY_RST_XXX
struct	WHFILE_CPY_T
{
	const char	*dstfile;
	const char	*srcfile;
	bool		cptime;
	bool		nofileok;
	unsigned char	MD5[16];
	size_t		cbstep;					// 每拷贝超过这么多字节就调用一下回调函数（如果为0则表示不用调用回调）
	void		*cbparam;				// 回调的参数
	void (*cb_progress)(size_t copied, void *cbparam);
										// 回调函数copied是已经拷贝的字节数，param是回传的参数
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
// 拷贝文件并对比（如果不填，则函数使用内部的回调函数）
// 返回同上WHFILE_CPY_RST_XXX
int		whfile_cpy_cmp(WHFILE_CPY_T *pParam);

// 文件时间操作(我只关心modification time)
// 设置
int		whfile_setmtime(const char *__file, time_t t);
// 获取(如果文件不存在返回0（应该没有别的情况会返回0的）)
time_t	whfile_getmtime(const char *__file);

// 将文件内容读入一个whvector，whvector的长度就是读到的文件长度
int		whfile_readfile(const char *__file, whvector<char> &__vectbuf);
// 将文件内容读入内存。*__size中原来是__data的最大长度，*__size返回读到的数据长度
int		whfile_readfile(const char *__file, void *__data, size_t *__size);

// 将文件内容和数据比较(如果文件出错或不存在比较结果为-1)
// 返回0表示内容一致
int		whfile_cmpdata(const char *__file, const void *__data, int __size);
int		whfile_cmpdata(whfile *__file, const void *__data, int __size);
// 文件对文件比较
// 返回0表示内容一致
// -1表示文件读写出错
// 1表示文件内容不同
// 2表示文件长度不同
int		whfile_cmp(whfile *__file1, whfile *__file2);
int		whfile_cmp(const char *__file1, const char *__file2);

// 同linux下的touch
// __filetime==0表示now
int		whfile_touch(const char *__file, time_t __filetime=0);

// 直接把一段内存写入文件
// 成功，返回写入长度，否则返回<0
int		whfile_writefile(const char *__file, const void *__data, size_t __size);
// 安全的文件替换
// __readagain表示是否需要存完了再读一遍以保证确实可用
int		whfile_safereplace(const char *__file, const void *__data, size_t __size, bool __readagain);

// 创建指定长度的文件（内容为连续的某个数值）
int		whfile_createfile_fill(const char *__file, unsigned char __pad, size_t __size);
// 在指定的文件指针上写入连续__size个byte
int		whfile_file_fill(whfile *__fp, unsigned char __pad, size_t __size);
// 创建指定长度的文件(一直没做好:(，不填数就不行)
//int		whfile_createfile(const char *__file, size_t __size);
// 判断文件名后缀是否符合过滤字串的要求
// cszFilter是".h|.cpp"这样的格式
bool	whfile_ExtIsMatch(const char *cszFileName, const char *cszFilter);
bool	whfile_MakeExtFilter(const char *cszFilter, whvector<EXTINFO_UNIT_T> &vectFilter);
bool	whfile_ExtIsMatch(const char *cszFileName, whvector<EXTINFO_UNIT_T> &vectFilter);

// 加入了校验位的文件读写（CRC头有16字节）
// 返回结果
enum
{
	WHFILE_CRC_READ_OK			= 0,			// 正确读入
	WHFILE_CRC_READ_NOTEXST		= -1,			// 文件不存在
	WHFILE_CRC_READ_ERRFILE		= -2,			// 文件无法读入，硬错误
	WHFILE_CRC_READ_ERRHDR		= -3,			// 文件头错误
	WHFILE_CRC_READ_ERRCRC		= -4,			// 文件CRC校验错误
};
int		whfile_crc_readfile(const char *__file, void *__data, size_t *__size);
int		whfile_crc_readfile(whfile *__file, void *__data, size_t *__size);
int		whfile_crc_readfile(const char *__file, whvector<char> &vectData);
int		whfile_crc_readfile(whfile *__file, whvector<char> &vectData);
// __writetmp表示是否需要先写一个临时文件
int		whfile_crc_writefile(const char *__file, const void *__data, size_t __size, bool __writetmp);

int		whfile_crc_readfromvector(whvector<char> &vectFile, void *__data, size_t *__size);
int		whfile_crc_readfromvector(whvector<char> &vectFile, whvector<char> &vectData);
int		whfile_crc_writetovector(whvector<char> &vectFile, const void *__data, size_t __size);

// 文件MD5的计算
// 获得一个文件的md5
unsigned char *	whfilemd5(const char *szFile, unsigned char *szDst);
unsigned char *	whfilemd5(whfile *fp, unsigned char *szDst);
// 获得一个文件的md5字串
char *	whfilemd5str(const char *szFile, char *szDst);
// 用函数里的静态字串变量
char *	whfilemd5str(const char *szFile);


////////////////////////////////////////////////////////////////////
// 带公开虚析构的实现
////////////////////////////////////////////////////////////////////
class	whfile_i		: public whfile
{
public:
	virtual int		Write(const void *pBuf, int nSize)
	{
		// 默认不实现写的方法
		return	-1;
	}
	virtual int		Flush()
	{
		// 默认也没有flush
		return	0;
	}
};

////////////////////////////////////////////////////////////////////
// 标准文件的实现
////////////////////////////////////////////////////////////////////
class	whfile_i_std	: public whfile_i
{
private:
	FILE	*m_fp;
	int		m_nSize;
	char	m_szFileName[WH_MAX_PATH];
	time_t	m_nNewFileTime;		// 如果调用了SetFileTime，则这个会非零
#ifdef	WIN32
	bool	m_bJustRread;		// 刚刚进行了读操作
	bool	m_bJustWrite;		// 刚刚进行了些操作
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
	FILE *	GetFILE() const	// 默认实现是返回NULL，在whfile
	{
		return	m_fp;
	}
};

////////////////////////////////////////////////////////////////////
// 内存文件的实现
////////////////////////////////////////////////////////////////////
class	whfile_i_mem	: public whfile_i
{
private:
	// 指向缓冲的指针
	char		*m_pszStr;
	// 缓冲长度
	int			m_nSize;
	// 当前读取的位置
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
	// 这个写不会超过m_nSize
	int		Write(const void *pBuf, int nSize);
	int		Flush()
	{
		return	0;
	}
public:
	// 获得当前指针位置
	inline char *	GetCurBufPtr()
	{
		return	m_pszStr + m_nOffset;
	}
	// 获得剩下的长度
	inline int		GetSizeLeft() const
	{
		return	m_nSize - m_nOffset;
	}
	// 针对特殊类型的读写
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
// 基于whvector的文件实现
////////////////////////////////////////////////////////////////////
class	whfile_i_byvector	: public whfile_i
{
private:
	// 指向缓冲的指针
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
// 文件内的文件的实现
// 因为whfile目前不支持多线程访问，所以这个也不支持多线程访问
////////////////////////////////////////////////////////////////////
class	whfile_i_fileinfile	: public whfile_i
{
private:
	// 关联的文件对象
	whfile		*m_file;
	// 在m_file中的起始点
	int			m_nStartOffset;
	// 自己的长度
	int			m_nSize;
	// 距离自己开头的偏移
	int			m_nOffset;
public:
	whfile_i_fileinfile();
	void	SelfDestroy()
	{
		delete	this;
	}
	// 设置关联的文件(每次关联相当于清除上一次关联)
	int		AssociateFile(whfile *file, int nStartOffset, int nSize);
	// 需要为父类实现的
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
};

////////////////////////////////////////////////////////////////////
// 多文件合并
////////////////////////////////////////////////////////////////////
class	whmultifile
{
public:
	////////////////////////////////////////////////////////
	// 类型定义
	////////////////////////////////////////////////////////
	#pragma pack(1)
	// 导出文件头结构
	struct	HDR_T
	{
		enum
		{
			VER				= 0,							// 当前版本
		};
		struct	CMN_T
		{
			char			szMagic[16];					// 魔法串
			int				nVer;							// 版本(不一样则不能导入)
			int				nFileNum;						// 包含的文件个数
			int				nEndOffset;						// 正常文件数据结束的位置
		}cmn;
		char	junk[64-sizeof(CMN_T)];						// 垃圾数据，补足。
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
		inline bool	IsGood() const							// 判断头是否合法
		{
			return	VER==cmn.nVer
				&&  strcmp("whmultifile", cmn.szMagic)==0;
		}
	};
	// 每个内部文件的头
	struct	IFILEHDR_T
	{
		struct	CMN_T
		{
			char			szName[32];						// 文件名(不带路径的，不会太长)
			unsigned char	MD5[WHMD5LEN];					// 文件的MD5
			int				nSize;							// 文件长度
			time_t			nTime;							// 文件(修改)时间
		}cmn;
		char	junk[64-sizeof(CMN_T)];						// 垃圾数据，补足。
		IFILEHDR_T()
		{
			memset(this, 0, sizeof(*this));
		}
	};
	#pragma pack()
public:
	////////////////////////////////////////////////////////
	// 变量定义
	////////////////////////////////////////////////////////
	HDR_T					m_hdr;							// 文件头
	int						m_nOpenMode;					// 打开模式(牵扯到关闭时是否要把)
	whfile					*m_basefile;					// 底层文件
	bool					m_bAutoCloseBaseFile;			// 是否需要自动关闭底层文件
public:
	whmultifile();
	~whmultifile();
	// 打开
	enum
	{
		OPENMODE_NOTOPEN	= 0,							// 没有打开
		OPENMODE_CREATE		= 1,							// 创建文件(原来有就替换之)
		OPENMODE_RDONLY		= 2,							// 只读打开文件(原来必须存在)
		OPENMODE_RDWR		= 3,							// 读写打开文件(原来必须存在)
	};
	int		OpenFile(const char *cszMainFile, int nOpenMode);
	// 根据已经打开的文件对象打开
	int		OpenFile(whfile *file, int nOpenMode, bool bAutoCloseBaseFile);
	// 关闭
	int		CloseFile();
	// 添加一个文件（注意为了简单，文件只能添加不能删除，如果要删除除非重新Create文件）
	int		Append(const char *cszFile, whfile *file);
	// 获得用于写入的文件对象
	whfile *	GetFileToAppend(const char *cszFile, time_t nFileTime);
	// 根据文件名获取一个只读的内部文件对象(这必须是在主文件打开以后)。
	whfile *	GetFileToRead(const char *cszFile);
	// 获得文件名列表
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
// 附着在别的文件尾部的文件
////////////////////////////////////////////////////////////////////
class	whtailfile	: public whfile
{
protected:
	// 尾部信息
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
			int		nHostSize;								// 宿主文件的长度
			int		nTailSize;								// 本whtailfile文件的数据长度
			time_t	nFTime;									// 文件时间
			unsigned char	MD5[WHMD5LEN];					// 专门用来设置MD5码的（因为tail文件也许会用来做网上下载的更新程序，所以需要有所校验）
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
	// 成员
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
	// SetMD5和GetMD5必须在打开之后关闭之前才能调用（外面应该根据需要在写入过程中计算MD5）
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
	// 获得文件总长度(这个在估计文件可能被分成多少块读入时很有用)
	virtual int		FileSize() const;
	// 获得文件时间（就是最后一次修改时间）
	virtual time_t	FileTime() const;
	// 设置文件时间
	virtual void	SetFileTime(time_t t);
	// 返回实际得到的字节数，0表示没有数据了，<0表示出错了
	virtual int		Read(void *pBuf, int nSize);
	// 移动文件指针
	// SEEK的参数和fseek一致，提供反向seek
	// 返回0表示成功，<0表示出错了
	// nOrigin使用和fseek参数一样的SEEK_SET、SEEK_CUR和SET_END
	virtual int		Seek(int nOffset, int nOrigin);
	// 判断是否到了文件尾
	virtual bool	IsEOF();
	// 获得当前文件指针位置
	virtual int		Tell();
	// 写入数据
	virtual int		Write(const void *pBuf, int nSize);
	// 缓冲到硬盘（返回和fflush的结果一致）
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
// 利用文件做基础的列表
////////////////////////////////////////////////////////////////////
class	whlistinfile
{
public:
	// 初始化信息
	struct	INFO_T
	{
		int	nMaxMsgNum;										// 最多的消息总条目数
		int	nMsgTypeNum;									// 消息种类数量
		int	nUnitDataSize;									// 每个条目的数据部分长度
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
		// 2005-10-12 把原来8字节的szMagic改为下面的szMagic/nReserved/nCRC16
		char	szMagic[MAGICSIZE];							// 文件魔法串
		char	nReserved;									// 保留做它用
		unsigned short	nCRC16;								// 校验码(它是m_listhdrs和m_list的数据部分CRC校验的结合)
		short	nVer;										// 版本
		short	nListTypeNum;								// 里面列表的个数
		int		nUnitSize;									// 内部用的单元全长
		int		nUnitDataSize;								// 给外界的数据部分的长度
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
		int		nHead;										// 列表头
		int		nTail;										// 列表尾
		int		nTotal;										// 总数量
		void	clear()
		{
			nHead	= whalist::INVALIDIDX;
			nTail	= whalist::INVALIDIDX;
			nTotal	= 0;
		}
	};
	// 文件中，HDR_T后面跟着nListTypeNum个LISTHDR_T
	// 在表的条目插入过程中，序号越大的越优先插入
	struct	LISTHDR_T
	{
		int			nCurIdxInner;							// 当前可读到的已经Read的内部序号
		ONELIST_T	UnReadList;
		ONELIST_T	ReadList;
		void	clear()
		{
			nCurIdxInner	= whalist::INVALIDIDX;
			UnReadList.clear();
			ReadList.clear();
		}
		void	reset()										// 重新开始读
		{
			nCurIdxInner	= ReadList.nTail;
		}
	};
	// 一个表项单元
	struct	UNIT_T
	{
		enum
		{
			PROP_READ	= 0x01,								// 已读
		};
		int				nType;
		int				nPrev;								// 因为alist只提供next，为了做双向链表
		unsigned char	nProp;
		char			data[1];
		//初始化
		inline void	Reset(int __nType)
		{
			nType	= __nType;
			nProp	= 0;
		}
		// 访问方法
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
	// 自己的文件头
	HDR_T				m_hdr;
	// 各个表头
	whvector<LISTHDR_T>	m_listhdrs;
	// 真正表数据的内存映射
	whvector<char>		m_vectFileData;
	// 用于消息分配的表
	whalist				m_list;
	// 是否修改了内容(修改了就在Close中存盘)
	bool				m_bModified;
	// 底层文件
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
	// 至于具体文件的打开/关闭由上层负责
	// 创建文件
	int		Create(whfile *file, INFO_T *pInfo);
	// 打开文件
	int		Open(whfile *file, int nNewMaxNum, int nNewnUnitDataSize);
	// 关闭文件
	int		Close();
	// 加入（会顶掉已经读过的，或比自己nType小的未读数据）
	int		push_back(int nType, void *pData, int nSize);
	// 获得下标为nIdx的数据(nIdx从列表头开始为0)
	int		getbegin(int nType);
	int		getnext(int nType, void *pData, int *pnIdxInner);
	// 删除某个内部id的数据
	int		delinner(int nIdxInner);
	// 标记某个内部id为未读(如果它原来是已读的话)
	int		unreadinner(int nIdxInner);
	// 对于文件内容损坏的不久方式
	int		savebadfile();
	// 根据自己的内容重新填写INFO_T
	int		refillinfo(INFO_T *pInfo);
	// 生成CRC校验码
	unsigned short	CalcCRC() const;
	// 校验文件内数据是否合法
	enum
	{
		VERIFY_RST_OK	= 0,
		VERIFY_RST_FAIL	= -1,
	};
	int		verifydata();
private:
	// 检查并顶掉比这个更不重要的条目
	// 返回真表示可以插入
	bool	checkandkick(int nTypeToAdd);
	// 把一个单元加入一个链表
	int		push_back(ONELIST_T *pOL, int nIdxInner);
	//int		push_front(ONELIST_T *pOL, int nIdxInner);
	// 把一个单元从链表中拆出。上层可要保证nIdxInner真的在pOL中
	int		remove(ONELIST_T *pOL, int nIdxInner);
};

////////////////////////////////////////////////////////////////////
// 直接从普通文件创建whfile对象
// szMode一般就是二进制的打开方式，即："rb"或"wb"等等
// 理论上可以不调用Close函数，只是为了和普通的文件调用一致才写的
////////////////////////////////////////////////////////////////////
whfile *	whfile_OpenCmnFile(const char *szFileName, const char *szMode);
// 关闭普通文件对象（这个只是为了配合旧的书写习惯，内部实际上就是删除file对象）
void		whfile_CloseCmnFile(whfile *file);

// 从内存创建whfile对象
whfile *	whfile_OpenMemBufFile(char *szMemBuf, size_t nMemSize);
void		whfile_CloseMemBufFile(whfile *file);

// 根据file对象计算md5值
unsigned char	*whfile_CalcMD5(whfile *file, unsigned char *MD5);

////////////////////////////////////////////////////////////////////
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
void *	WHCMN_FILE_STATIC_INFO_Out();
void	WHCMN_FILE_STATIC_INFO_In(void *pInfo);
// 获得全局的文件管理器句柄（主要是给一些懒惰的，不愿意设置文件管理器的对象用的）
whfileman *	WHCMN_FILEMAN_GET();
void	WHCMN_FILEMAN_SET(whfileman *pFM);

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILE_H__
