// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfile.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的内存文件功能
//                由内存中的一大块预先申请好的区域构成
//                该区域将被划分为三个部分：
//                1. 信息头：MEMINFO_T，里面记录了全局的信息
//                2. 文件信息区：里面是whalist管理的一段内存，存储所有文件信息
//                3. 文件块区：里面是whalist管理的一段内存，存储所有文件块信息
//                强烈注意，如果有文件被打开则不要删除它。这个我是没有判断的。会出错！！！！
// CreationDate : 2004-01-18
// ChangeLOG    : 2004-11-07 修改了FILEINFO_T中nProperty的范围定义
//              : 2005-02-10 取消了whmemfile中的MODE_XXX定义

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

// 内存文件管理器
class	whmemfileman
{
	friend	class whmemfile;
public:
	enum
	{
		MODE_READ		= 0,			// 只读
		MODE_CREATE		= 1,			// 创建文件(原来有就清空，既可读又可写)
		MODE_READWRITE	= 2,			// 改写打开(不清空原来的，原来没有就出错，既可读又可写)
	};

	#pragma pack(1)						// BEGIN OF pack	{
	struct	INFO_T						// 初始化信息
	{
		void		*pBuf;				// 将要被使用的内存区
		size_t		nMaxFile;			// 最多可以容纳的文件数
		size_t		nFragSize;			// 文件块的尺寸
		size_t		nTotalFrag;			// 所有文件块总数
										// nFragSize * nTotalFrag决定了文件部分的最多能容纳的大小 (在一点都不浪费的情况下)
	};
	struct	MEMINFO_T					// pBuf将会被转换为这个头
	{
		// 运行期数据
		size_t		offFileSect;		// 文件信息区的开头偏移量
		size_t		offFragSect;		// 文件块区的开头偏移量
		// 2005-02-18 原来这后面的长度是64个int即512字节，保持它，添加新信息
		// 改名为nExtInfo，以后自己按应用设置相应的单元(注意一个应用中下标不重复即可)
		enum
		{
			EXTIDX_PROPERTY		= 63,	// 最后一个保留作自己用途，标记一些内部属性
		};
		enum
		{
			PROPERTY_MARKCLEAN	= 1,	// (在结束的时候标明)这样在结束非法的时候重启就可以不继承了
		};
		int			nExtInfo[64];		// 可以随便存储
	};
	struct	FILEINFO_T					// 文件信息块
	{
		enum
		{
			PROPERTY_WR		= 0x0001,	// 被写入，没有关闭(这种情况在程序死机重新继承后就需要检验，不能发回)
			PROPERTY_DEL	= 0x0002,	// 标记被删除
		};
		int				nIdx;			// 文件索引，即在m_alFileInfo中的索引
		int				nName;			// 文件名，为了简单就取整数
		unsigned int	nProperty;		// 附加属性(参考上面的PROPERTY_XXX，低16位位whmemfileman使用，高16位为应用程序自行定义)
		int				nHeadIdx;		// 文件起始块的索引
		int				nTailIdx;		// 文件结尾块的索引
		size_t			nSize;			// 文件长度
		unsigned		nCRC32;			// CRC校验(这个一般是在写文件关闭后写入的，如果文件没有正常关闭，则需要校验)
		time_t			nLastModify;	// 最后一次更新文件的时间(这个在文件关闭时才记录)
		unsigned char	MD5[16];		// 上次的MD5值(这个并不是在关闭时自动算的，一般都不算，根据需要算)
		unsigned char	tmpMD5[16];		// 临时寄存的MD5值
		void *			aExtPtr[4];		// 附属指针(上层自由设置，用于保存和文件名相关的东东)
										// !!!! 注意：如果文件原来存在，则create不会影响aExtPtr的内容!!!!
		void	clear()					// 将文件状态清空为初始状态
		{
			// nIdx和nName都需要上层在分配之后填写的，不能变的，所以也不能轻易清除
			nProperty	= 0;
			nHeadIdx	= whalist::INVALIDIDX;
			nTailIdx	= whalist::INVALIDIDX;
			nSize		= 0;
			nCRC32		= 0;
			nLastModify	= 0;
			memset(MD5, 0, sizeof(MD5));
			memset(tmpMD5, 0, sizeof(tmpMD5));
			// memset(aExtPtr, 0, sizeof(aExtPtr)); !!!!这个不清空，只让上层手动清空(只有在从无到有的时候清一次，以后就保留)。
		}
	};
	// 统计
	struct	STATISTIC_T
	{
		int				nFileNum;		// 已有的文件总数
		int				nFileMax;		// 总共可以分配的文件总数
		int				nFragNum;		// 文件块总数
		int				nFragMax;		// 总共可以分配的文件块总数
	};
	#pragma pack()						// BEGIN OF pack	}

protected:
	INFO_T				m_info;
	MEMINFO_T			*m_pMemHdr;		// 上层系统提供的内存开始点
	whhash<int, int>	m_mapFileName2Idx;
										// 文件名和FILEINFO_T单元序号的对应关系
	whalist				m_alFileInfo;	// 用于处理各个FILEINFO_T单元
	whalist				m_alFragInfo;	// 用于处理文件块
public:
	// 通过最大的文件数知道文件信息部分需要的尺寸
	// 通过片尺寸和总片数知道后面需要的尺寸
	// 然后加上MEMINFO_T尺寸返回总的需要的尺寸
	// pInfo中的pBuf不需要管，将会由外界去申请(直接申请内存或者从共享内存申请都可以)
	static size_t	CalcMemSizeNeeded(INFO_T *pInfo);
	inline size_t	CalcMemSizeNeeded()
	{
		return	CalcMemSizeNeeded(&m_info);
	}
public:
	whmemfileman();
	virtual ~whmemfileman();
	// 初始化(因为里面都是自动对象和上层分配的内存，所以不需要对应release了)
	int		Init(INFO_T *pInfo);
	// 清空所有东西，恢复到什么也没有的状态
	inline void	Reset()
	{
		Init(&m_info);
	}
	// 清空所有文件
	// 注意，这个必须只能在Init或者Inherit之后调用
	inline void	MarkClean()
	{
		whbit_ulong_set((unsigned int *)&m_pMemHdr->nExtInfo[MEMINFO_T::EXTIDX_PROPERTY], MEMINFO_T::PROPERTY_MARKCLEAN);
	}
	inline bool	IsMarkedClean() const
	{
		return	whbit_ulong_chk((unsigned int *)&m_pMemHdr->nExtInfo[MEMINFO_T::EXTIDX_PROPERTY], MEMINFO_T::PROPERTY_MARKCLEAN);
	}
	// 从一段已有的内存继承过来(Init和Inherit只需要调用一个都可以完成初始化工作)
	int		Inherit(void *pBuf);
	// 判断一个文件是否存在
	bool	IsFileExist(int nFileName) const;
	// 返回文件长度
	// 返回<0表示文件不存在
	int		GetFileSize(int nFileName);
	// 获取文件信息结构
	FILEINFO_T *	GetFileInfo(int nFileName);
	inline FILEINFO_T *	GetFileInfoByIdx(int nIdx)
	{
		return	(FILEINFO_T *)m_alFileInfo.GetDataUnitPtr(nIdx);
	}
	// 打开一个文件(whmemfile关闭必须通过Close，不能直接删除)
	whmemfile *	Open(int nFileName, int nMode=MODE_READ);
	// OpenByIdx的pName只有nIdx不存在且nMode为写方式的时候才需要
	inline whmemfile *	OpenByIdx(int nIdx, int nMode, void *pName)
	{
		return	OpenFile(GetFileInfoByIdx(nIdx), nMode, pName);
	}
	// 关闭一个文件(关闭后whmemfile对象即失效)
	// 使用Close的原因是因为对于写打开的文件需要在关闭时清空文件状态
	// 还有注意，我不会自动释放没有关闭的文件
	// 如果pFile为NULL不会出错，内部不进行任何操作
	int		Close(whmemfile *pFile);
	// 删除一个文件
	int		Delete(int nFileName);
	inline int		DeleteByIdx(int nIdx)
	{
		return	DeleteFile(GetFileInfoByIdx(nIdx));
	}
	// 标记删除并清空文件
	int		MarkDelAndClean(int nFileName);
	// 返回统计信息
	void	GetStatistic(STATISTIC_T *pStat) const;
	// 获得存在的文件名列表
	void	GetFileNameList(whvector<int> &vectList) const;
	// 获得存在的文件个数
	inline int		GetFileNum() const
	{
		return	m_alFileInfo.GetHdr()->nCurNum;
	}
	// 获得还可以创建的文件个数
	inline int		GetFileNumLeft() const
	{
		return	m_alFileInfo.GetHdr()->nMaxNum - m_alFileInfo.GetHdr()->nCurNum;
	}
	// 获得文件块个数
	inline int		GetFragNum() const
	{
		return	m_alFragInfo.GetHdr()->nCurNum;
	}
	// 获得内存头
	inline MEMINFO_T *	GetMemInfo()
	{
		return	m_pMemHdr;
	}
protected:
	// 计算文件的CRC
	unsigned	CalcCRC32(FILEINFO_T *pFileInfo) const;
	// 清空文件内容
	int			EmptyFile(FILEINFO_T *pFileInfo);
	// 删除文件
	int			DeleteFile(FILEINFO_T *pFileInfo);
	// 打开文件(pName是附加信息，比如nFileName)
	whmemfile *	OpenFile(FILEINFO_T *pFileInfo, int nMode, void *pName);
private:
	// Init的附加操作
	virtual int	Init_Ext()
	{
		return	0;
	}
	// Inherit的附加操作
	virtual int	Inherit_Ext()
	{
		return	0;
	}
	// 在Inherit一个文件成功后，附加要做的事情
	virtual int	Inherit_AfterGood(FILEINFO_T *pFInfo, int nIdx)
	{
		m_mapFileName2Idx.put(pFInfo->nName, nIdx);
		return	0;
	}
	// 设置名字对文件信息单元的映射
	virtual int	Open_SetNameMap(void *pName, FILEINFO_T *pFInfo)
	{
		pFInfo->nName	= (int)pName;
		m_mapFileName2Idx.put(pFInfo->nName, pFInfo->nIdx);
		return	0;
	}
	// 在DeleteFile里面删除其他东西之前做的事情
	virtual int	DeleteFile_BeforeRealDel(FILEINFO_T *pFInfo)
	{
		m_mapFileName2Idx.erase(pFInfo->nName);
		return	0;
	}
};

// 内存文件对象，相当于FILE
class	whmemfile
{
	friend	class whmemfileman;
protected:
	whmemfileman				*m_pMan;
	whmemfileman::FILEINFO_T	*m_pFileInfo;
										// 在管理器中对应的对象指针，到时候删除就通过这个了
	int				m_nOpenMode;		// 文件的打开模式whmemfileman::MODE_XXX
	int				m_nCurOffset;		// 当前文件指针的位置
	int				m_nCurBlockIdx;		// 当前文件块的索引
public:
	whmemfile();
protected:
	~whmemfile();						// 这样就不允许直接删除
public:
	typedef	whmemfileman::FILEINFO_T	FILEINFO_T;
	// 读出数据，返回读出的长度
	size_t	Read(void *pBuf, size_t nSize);
	// 写入数据，返回写入的长度
	size_t	Write(const void *pBuf, size_t nSize);
	// Seek的参数和fseek完全相同
	int		Seek(int nOffset, int nOrigin);
	// 将剩下的文件内容读入一个vector
	size_t	ReadToVector(whvector<char> &vect);
	// 和数据比较
	int		CmpWithData(const void *pData, int nSize);
	// 判断文件是否到了结尾
	inline bool	IsEOF() const
	{
		return	m_nCurOffset == (int)m_pFileInfo->nSize;
		//return	m_nCurBlockIdx == whalist::INVALIDIDX 
		//	||	m_nCurOffset == (int)m_pFileInfo->nSize
		//	;
	}
	// 获得文件信息头
	inline FILEINFO_T *	GetInfo() const
	{
		return	m_pFileInfo;
	}
	// 获得文件长度
	inline size_t	GetFileSize() const
	{
		return	m_pFileInfo->nSize;
	}
	// 获得当前文件指针的位置
	inline size_t	Tell() const
	{
		return	m_nCurOffset;
	}
	// 关闭文件(内部实际上是调用m_pMan->Close，这样做只是为了方便)
	// 注意，Close之后this指针就失效了!!
	inline int	Close()
	{
		return	m_pMan->Close(this);
	}
	// 针对已知类型的简单读写
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
	// 针对whvector的读写，一次读到底
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

// 带名字的文件管理器
// !!!!注意：不要使用whmemfileman中的方法
class	whmemfilemanWithName	: public whmemfileman
{
public:
	#pragma pack(1)
	struct	FILENAME_T
	{
		char	szFName[WH_MAX_PATH];			// 文件名
		// 下面成员都是给上层使用的。whmemfileman内部不使用这些。
		int		nInWhatList;					// 处于哪个双向链表内(-1表示没有)
		int		nPrevIdx, nNextIdx;				// 这两个可以上层用来实现双向链表
		int		nExt[8];						// 今后用于扩充
		void	clear()							// 将文件状态清空为初始状态
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
	whalist		m_alFileName;					// 用于存放各个文件名。它和m_alFileInfo的元素数量一致。所以如果统一行动，则向他们申请的ID应该也一致。
	whhash<whstrptr4hash, int>	m_mapFileNameStr2Idx;
												// 文件名字串和FILEINFO_T单元序号的对应关系
public:
	// 计算需要的尺寸
	// 最开始是whmemfileman需要的，后面是whmemfilemanWithName需要的
	// 即FILENAME_T部分
	static size_t	CalcMemSizeNeeded(INFO_T *pInfo);
	inline size_t	CalcMemSizeNeeded()
	{
		return	whmemfilemanWithName::CalcMemSizeNeeded(&m_info);
	}
public:
	// 打开一个文件(whmemfile关闭必须通过Close，不能直接删除)
	whmemfile *	Open(const char *cszFileName, int nMode=MODE_READ);
	// 删除一个文件
	int		Delete(const char * cszFileName);
	// 文件是否存在
	bool	IsFileExist(const char * cszFileName) const;
	// 获取文件信息结构
	FILEINFO_T *	GetFileInfo(const char * cszFileName);
	inline FILENAME_T *	GetFileNameByIdx(int nIdx)
	{
		return	(FILENAME_T *)m_alFileName.GetDataUnitPtr(nIdx);
	}
	// 获得存在的文件索引号列表
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
