// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_base.h
// Creator     : Wei Hua (魏华)
// Comment     : 基础的包结构
// CreationDate: 2006-07-18
// ChangeLOG   : 2006-07-20 取消了BLOCK_HDR_T中的nBlocknumOrSize。在文件的第一块中增加了附加数据，这样可以减少后面块中浪费的数据。
//             : 2006-07-21 取消了m_nLastBlockDataMaxSize和m_nLastBlockDataSize和m_nCurOffsetInBlock，这些可以直接计算出来，不会废很多时间。
//             : 2006-07-31 增加了rawfile的文件时间功能
//             : 2007-03-27 取消了m_setOpenedFileStart为了能够把一个文件打开多次（的确有这种需求）
//			   : 2008-06-06	增加一个接口，用于直接读取原始的压缩数据（by 钟文杰）

#ifndef	__WH_PACKAGE_BASE_H__
#define __WH_PACKAGE_BASE_H__

#include "whcmn_def.h"
#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "whstring.h"
#include "whhash.h"
#include "wh_package_singlefile.h"
#include "wh_package_def.h"

#define FILEINPACKAGE_NOCASE

namespace n_whcmn
{

// 简单的块文件
// 
class	wh_package_base
{
public:
	typedef	int			blockoffset_t;								// 块偏移，不是文件的的偏移量，而是块的序号
																	// 这里单独定义出来只是为了怕将来改成更大的文件，不过要改也比较困难，因为很多其他参数还是int，比如seek的参数

#ifdef	FILEINPACKAGE_NOCASE										// 对包中的文件不区分大小写
	typedef	whstrinvector4hashNocase			STRINVECT;
	typedef	_whstr_hashfuncNocase				STRHASHFUNC;
#else																// 对包中的文件区分大小写
	typedef	whstrinvector4hash					STRINVECT;
	typedef	_whstr_hashfunc						STRHASHFUNC;
#endif
	typedef	whhashset<STRINVECT, whcmnallocationobj, STRHASHFUNC>	STRSET_T;

	struct	INFO_T
	{
		whfileman::OP_MODE_T	nOPMode;							// 打开包的方式（有时候也是需要只读打开的，比如游戏中的时候）
		whfile					*pBaseFile;							// 最最底层的文件（也可能是一大块内存变成的内存文件）
		WHCompress				*pWHCompress;						// 压缩器
		ICryptFactory::ICryptor	*pEncryptor;						// 加密器
		ICryptFactory::ICryptor	*pDecryptor;						// 解密器（注意：加密器和解密器可不能一样）
		int						nBlockSize;							// 基本文件块的大小（越大对于小文件浪费越大，越小对于大文件浪费越大）
		char					szPass[128];						// 密码（如果没有密码则后的长度为0）
		int						nPassLen;							// 密码长度
		char					szDir[WH_MAX_PATH];					// 自己代表的目录名
		bool					bAutoCloseBaseFile;					// 是否自动关闭底层基础文件
		bool					bCheckFileBlockNumOnOpen;			// 是否在打开文件的时候检验文件块数是否正常（因为在程序异常的时候很可能导致文件没有写入，但是文件体已经有部分更新了，所以为了能够尽可能打开文件就不用检查了）
		bool					bNoErrOnDelFile;					// 删除文件如果出错就忽略（可能会浪费一些空间）

		INFO_T()
			: nOPMode(whfileman::OP_MODE_BIN_READONLY)
			, pBaseFile(NULL)
			, pWHCompress(NULL)
			, pEncryptor(NULL), pDecryptor(NULL)
			, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
			, nPassLen(0)
			, bAutoCloseBaseFile(false)
			, bCheckFileBlockNumOnOpen(false)
			, bNoErrOnDelFile(true)
		{
			memset(szDir, 0, sizeof(szDir));
		}
		// 从另外的相似结构中获取自己需要的信息
		template<typename _TyOther>
		void	ReadFromOther(const _TyOther &other)
		{
			// pBaseFile不是通用数据，可能别人不提供，所以就不要写了
			nOPMode				= other.nOPMode;
			pWHCompress			= other.pWHCompress;
			pEncryptor			= other.pEncryptor;
			pDecryptor			= other.pDecryptor;
			nBlockSize			= other.nBlockSize;
			memcpy(szPass, other.szPass, other.nPassLen);
			nPassLen			= other.nPassLen;
		}
	};
	#pragma pack(1)
	struct	PACKAGE_HDR_T 
	{
		enum
		{
			VER			= 2
		};
		char			szMagic[16];								// 魔法串，用于标识文件类型
		int				nVer;										// 版本
		int				nBlockSize;									// 基本文件块的大小（越大对于小文件浪费越大，越小对于大文件浪费越大）
		blockoffset_t	nAvailIdx;									// 可用块的链表头
		blockoffset_t	nMaxBlockIdx;								// 最后一个可用块序号+1（也就是可以新分配的可用块序号）
																	// 这个在打开文件的时候可能根据文件长度算出来，如果文件长度不是块长的整倍数，或者计算出的块数和这个不符合，则可能文件坏了。
		unsigned char	PassMD5[16];								// 加密密码的MD5校验核
		char			junk[16+64];								// 这样前面凑够128字节以留做以后扩充
		enum
		{
			RF_IDX_GOD		= 0,									// 这个位置先保留，没想好作什么用
			RF_IDX_FINFO	= 1,									// 文件信息表
			RF_IDX_FNAME	= 2,									// 文件名表
			RF_IDX_TOTAL	= 8										// 总数
		};
		blockoffset_t	anRawfile[RF_IDX_TOTAL];					// 用于存储重要的RAW基础文件的信息

		static const char	*CSZMAGIC;

		void	reset()
		{
			WHMEMSET0THIS();
			nVer			= VER;
			strcpy(szMagic, CSZMAGIC);
			nMaxBlockIdx	= 1;
		}
		bool	IsOK() const										// 判断从文件中读出的头是否正确
		{
			return	nVer == VER && strcmp(szMagic, CSZMAGIC)==0;
		}
		void	print(FILE *fp) const
		{
			fprintf(fp, "%s,%d,%d,%d,%d,%s", szMagic, nVer, nBlockSize, nAvailIdx, nMaxBlockIdx, wh_hex2str(PassMD5,sizeof(PassMD5)));
		}
	};
	#pragma pack()
	// 检查文件是否是包文件
	enum	enumFILEISWHAT
	{
		FILEISWHAT_ERROR	= -1,		// 文件打开错误
		FILEISWHAT_COMMON	= 0,		// 一般文件
		FILEISWHAT_PCK		= 1,		// 一般包文件
		FILEISWHAT_TAILPCK	= 2,		// tail包文件
	};
	static enumFILEISWHAT	FileIsWhat(whfile *fp);
	static enumFILEISWHAT	FileIsWhat(whfileman *pFM, const char *cszFName);
public:
	#pragma pack(1)
	// 每个块的头
	struct	BLOCK_HDR_T
	{
		blockoffset_t			nPrev;								// 前一块序号（0表示没有）
		blockoffset_t			nNext;								// 后一块序号（0表示没有）
	};
	// 第一块头部的附加数据
	struct	FIRST_BLOCK_EXT_T
	{
		blockoffset_t			nEnd;								// 最后后一块序号
		int						nFileSize;							// 文件长度
		time_t					nFileTime;							// 文件最后修改时间
		char					junk[32-12-8];						// 填充垃圾用的（把总的设置成32字节（包括前面的BLOCK_HDR_T））
								// 12是FIRST_BLOCK_EXT_T中前面数据的大小
								// 8是BLOCK_HDR_T中数据的大小
	};
	#pragma pack()
	// 内部RAW文件
	class	rawfile	: public whfile
	{
		// 如果文件长度变了，则相关的变量也会变化，它们是：
		// m_nEnd还有m_FirstBlockExt.nFileSize
		// 如果文件指针变化了，则相关的变量也会变化，它们是：
		// m_nCurOffset、m_nCurBlock、m_nCurOffsetInBlock
	public:
		rawfile(wh_package_base	*pPB);
		~rawfile();													// 析构里面会调用Close
		int		Open(whfileman::OP_MODE_T nOPMode, blockoffset_t &nStart);
																	// 根据mode选择是否创建文件，如果是创建则nStart被忽略，并用它返回文件的起始块号
																	// 如果是打开文件则nStart就是文件起始块号，内部会查找文件的结束块并保存下来。
																	// (注意：这个创建打开和普通文件的w打开不同，不会删除已有的文件。)
		int		Close();											// 感觉上没啥可做的，因为所有写操作都是即使生效的
		int		Delete();											// 删除文件内容（之后这个文件的所有数据就不存在了，而且不能继续做任何读写操作!!!）
																	// 同事文件对象指针也被删除，请不要继续引用!!!!
		int		Extend(int nSize);									// 扩大文件长度（如果成功返回新的文件长度）
																	// 注意：如果成功，Extend不改变当前的Offset，如果失败，那Offset就不知道在什么地方了
		void	SetFileTime(time_t t);
		inline blockoffset_t	GetStart() const
		{
			return	m_nStart;
		}
		inline blockoffset_t	GetEnd() const
		{
			return	m_FirstBlockExt.nEnd;
		}
		inline blockoffset_t	GetBlockDataMaxSize() const
		{
			return	m_pPB->GetBlockDataMaxSize();
		}
		inline blockoffset_t	GetFirstBlockDataMaxSize() const
		{
			return	m_pPB->GetFirstBlockDataMaxSize();
		}
		inline bool	IsCurBlockLastOne() const						// 判断当前的block是否是最后一块
		{
			return	m_nCurBlock == GetEnd();
		}
		inline bool	IsReadOnly() const								// 判断文件是否是只读的
		{
			return	whfileman::IsReadOnlyMode(m_nOPMode);
		}
		// 获得在一个块内的偏移（如果是第一块会加上附加数据的长度）
		// *pnBlockNum是总快数减一，*pnOffsetInBlock是在当前块内的便宜（*pnOffsetInBlock只有最后一块才会可能指向块尾）
		inline void	CalcBlocknumAndOffsetinblock(int nOffset, int *pnBlockNum, int *pnOffsetInBlock) const
		{
			int	nRawOffset		= nOffset + sizeof(FIRST_BLOCK_EXT_T);
			*pnBlockNum			= nRawOffset/GetBlockDataMaxSize();
			*pnOffsetInBlock	= nRawOffset%GetBlockDataMaxSize();
			if( (*pnOffsetInBlock)==0 )
			{
				if( nOffset>=FileSize() )
				{
					(*pnBlockNum)		--;
					(*pnOffsetInBlock)	= GetBlockDataMaxSize();
				}
			}
		}
		// 增加文件长度
		inline void	AddFileSize(int nAdd)
		{
			m_FirstBlockExt.nFileSize	+= nAdd;
			m_bShouldSaveFirstBlockExt	= true;
		}
		///////////////////////////////////
		// 为whfile接口实现的方法
		///////////////////////////////////
		void	SelfDestroy()
		{
			delete	this;
		}
		int		FileSize() const;									// 在打开文件时可以直接从第一块中读出文件尺寸来。
		// RAW文件就不需要时间这个功能了
		time_t	FileTime() const;
		int		Read(void *pBuf, int nSize);
		int		Seek(int nOffset, int nOrigin);
		bool	IsEOF();
		int		Tell();
		int		Write(const void *pBuf, int nSize);
		int		Flush();
	protected:
		// 内用数据
		// 包指针
		wh_package_base	*m_pPB;
		// 打开模式
		whfileman::OP_MODE_T	m_nOPMode;
		// 当前块的头信息
		BLOCK_HDR_T		m_CurBlockHdr;
		// 第一块的附加信息
		FIRST_BLOCK_EXT_T		m_FirstBlockExt;
		// 需要保存附加信息
		bool			m_bShouldSaveFirstBlockExt;
		// 文件起始块号（结束块号在m_FirstBlockExt中有存储）
		blockoffset_t	m_nStart;
		// 位置指针（包括当前块和块内的偏移（块内偏移是只除去头（BLOCK_HDR_T）之外的偏移））
		blockoffset_t	m_nCurBlock;
		// 文件全局的Offset
		int				m_nCurOffset;
	protected:
		// 内用方法
		// 把指针移到指定块真正的头，不读出任何数据
		int	SeekToBlockHdr(blockoffset_t nBlock);
		// 把指针移到指定块，并读出头到m_CurBlockHdr
		int	SeekToBlockAndReadHdr(blockoffset_t nBlock);
		// 利用m_pPB将文件指针移动到本文件当前注视的位置，因为里面会计算在块内的偏移，所以把这个偏移返回给上层
		int	SeekToMyCurPos(int *pnOffsetInBlock);
	};
	// 让rawfile能使用wh_package_base中的方法
	friend class wh_package_base::rawfile;
	// 扩展的最终给外界的单包文件
	class	file4out	: public wh_package_singlefile
	{
	private:
		wh_package_base	*m_pPB;
		wh_package_base::blockoffset_t	m_nStartBlock;
	public:
		file4out(wh_package_base *pPB)
			: m_pPB(pPB)
			, m_nStartBlock(0)
		{
		}
		~file4out()
		{
		}
		void	SelfDestroy()
		{
			delete	this;
		}
	};
	friend class wh_package_base::file4out;
protected:
	// 是否被初始化了
	bool						m_bInited;
	bool						m_bShouldSavePackageHdr;			// m_package_hdr改变了，需要存盘
	bool						m_bShouldSaveFileList;				// 文件信息表改变了，需要存盘
	bool						m_bShouldSaveVectorFileName;		// 文件名表改变了，需要存盘
	// 内用数据
	INFO_T						m_info;								// 记录一下初始化的数据
	PACKAGE_HDR_T				m_package_hdr;						// 文件头，打开包的时候读入，如果发生变动在关闭包的时候保存
	// 文件中存储的文件列表单元
	// 首先是列表文件中的单元类型(unsigned char)
	enum
	{
		TYPE_FILE				= 1,								// 文件
		TYPE_DIRBEGIN			= 2,								// 目录开始
		TYPE_DIREND				= 3,								// 目录结束
	};
	// 然后是名字的开始偏移(int，因为whstrinvector4hash里面的offset就是整数，所以就这么用吧)
	// 后面是文件或目录数据(文件就需要文件的起始块号blockoffset_t nStartBlock（至于文件是否压缩加密之类的直接在文件数据中看）；目录现在好像还没想好有什么数据...)
	// 内存中的目录结构大map
	struct	FILENODE_T 
	{
	public:
		typedef	whhash<STRINVECT, FILENODE_T, whcmnallocationobj, STRHASHFUNC>			STR2NODE_T;
	private:
		bool	bIsDir;												// 表名这个是否是目录
	public:
		union	_U_T
		{
			struct	_D_T
			{
				STR2NODE_T		*pContent;							// 目录内容（一般都不为空，即便目录下没有文件也只是map为空）
			}d;
			struct	_F_T
			{
				blockoffset_t	nStartBlock;						// 文件起始块号
			}f;
		}u;
		FILENODE_T()
		{
			WHMEMSET0THIS();
		}
		inline bool IsDir() const
		{
			return	bIsDir;
		}
		inline void	SetIsDir(bool bSet)
		{
			bIsDir	= bSet;
		}
	};
	typedef	FILENODE_T::STR2NODE_T	STR2NODE_T;
	FILENODE_T					m_RootNode;							// 对应根节点的node
	whvector<char>				m_vectFNAME;						// 存放文件名的大缓冲
	STRSET_T					m_setFNAME;							// 已经存在的文件名表
	// 给外界用的Dir对象
	class	dir4out		: public WHDirBase
	{
	private:
		STR2NODE_T				*m_pDir;							// 对应的目录内容映射对象
		STR2NODE_T::kv_iterator	m_it;								// 浏览过程中的迭代器
		ENTRYINFO_T				m_EntryInfo;
		char					m_szBaseDir[WH_MAX_PATH];			// 相应的目录名
	public:
		dir4out(STR2NODE_T *pDir, const char *cszBaseDir);
		virtual ~dir4out();
		virtual void			SelfDestroy()	{delete this;}
		virtual const char *	GetDir() const	{return m_szBaseDir;}
		virtual int				Close();
		virtual ENTRYINFO_T *	Read();
		virtual void			Rewind();
	};
protected:
	// 注意：所有这些被保护的方法不能被外界使用，如果使用要考虑多线程的时候就比较麻烦。因为这些方法可能会被文件基本操作分别调用。
	// 比如比如两个线程都调用了AllocBlock然后立即写入块头，那么就可能出问题了。

	// 获取一个可用的新块偏移（返回0表示无法获取，一般应该不会出现这样情况的，除非硬盘满了或者文件了）
	// 应该是先从可用表中找，如果没有再新分配
	// 如果成功，同时内部的whfile指针也会移动到相应块的block开头位置（但是没有读取块头数据，所以上层可以立即写入新的块头数据）
	blockoffset_t	AllocBlock();
	// 创建一个rawblock
	int				MakeRawBlock(blockoffset_t nBlock);
	// 把内部文件指针移动到指定块的开头（是真开头，和AllocBlock成功后的位置使一样的）
	int				SeekToBlockHdr(blockoffset_t nBlock);
	// 把内部文件指针移动到指定块的指定位置，其中nOffset是指BLOCK_HDR_T之后的偏移
	int				SeekToBlockAndOffset(blockoffset_t nBlock, blockoffset_t nOffset);
	// 从某个块到某个块之间的一段链（当然上层需要保证这段链是有效的。）
	int				FreeBlock(blockoffset_t nStart, blockoffset_t nEnd);
	// 从文件f中读出目录项，以pRoot为根节点创建目录树（可递归）
	int				ReadDirTreeFromFile(STR2NODE_T *pRoot, whfile *f);
	// 把目录树内容存入文件（可递归）
	int				SaveDirTreeToFile(STR2NODE_T *pRoot, whfile *f);
	// 释放目录树（可递归）
	int				FreeDirTree(STR2NODE_T *&pRoot);
	// 删除目录数（包括各级目录和文件）（可递归）
	int				DelDirTree(STR2NODE_T *&pRoot);
	// 对有名文件的处理
	// 通过文件名查找对应的节点（这个也可能是目录）
	// cszPath的样子是xxx/yyy/zzz这样的（其中/也可能是\），代表从根目录开始的相对路径
	struct	FINDNODEBYPATHNAME_RST_T
	{
		// 如果找到文件或目录
		FILENODE_T			*pFN;									// 则这是最终的节点
		
		STR2NODE_T			*pDir;									// 如果只找到部分，则pDir是最远的一个目录的对象
																	// 如果是找到全部，则pDir上一层目录对象
		int					nOffset;								// 如果是找到部分，这是没有实现部分的名字的开始偏移
																	// 比如：a/b/c/d/e.txt，如果目前只存在目录a/b/c，则pDir代表a/b/c这个目录，且nOffset=6，即d/e.txt开始的位置，
																	// 如果是找到全部，则nOffset就是最后一段的开始偏移
	};
	enum
	{
		FINDNODEBYPATHNAME_RST_OK					= 0,			// 成功找到文件
		FINDNODEBYPATHNAME_RST_PARTIAL				= 1,			// 找到部分路径
		FINDNODEBYPATHNAME_RST_ERR_MIDDLEFILEINPATH	= -1,			// 路径中间有文件，不能继续检索
	};
	int				FindNodeByPathName(const char *cszPath, FINDNODEBYPATHNAME_RST_T *pRst);
	// 从pRoot相对开始，创建多级目录（或最后是文件），cszPath是相对pRoot的路径
	FILENODE_T *	CreateNodeByPathName(const char *cszPath, STR2NODE_T *pRoot, bool bIsFile);
public:
	wh_package_base();
	~wh_package_base();
	void	SelfDestroy()	{delete this;}
	int	Init(INFO_T *pInfo);
	int	Release();
	int	SavePackageHdr();											// 如果包头变化了就保存之
	int	SaveInfoFiles();											// 保存两个基本文件
	int	Flush();													// 里面会先把包头存盘，然后再flush整个文件内容
	inline rawfile *	NewRawFile()
	{
		return	new rawfile(this);
	}
	inline blockoffset_t	GetBlockDataMaxSize() const				// 计算一块中可以写数据的空间的大小
	{
		return	m_package_hdr.nBlockSize - sizeof(BLOCK_HDR_T);
	}
	inline blockoffset_t	GetFirstBlockDataMaxSize() const		// 计算第一块中可以写数据的空间的大小
	{
		return	GetBlockDataMaxSize() - sizeof(FIRST_BLOCK_EXT_T);
	}
	inline whfile *	GetBaseFile()									// 外面主要是rawfile可能会使用到这个
	{
		return	m_info.pBaseFile;
	}
	inline bool	IsReadOnly() const									// 判断文件是否是只读的
	{
		return	whfileman::IsReadOnlyMode(m_info.nOPMode);
	}
	inline const PACKAGE_HDR_T &	GetPackageHdr() const
	{
		return	m_package_hdr;
	}
	inline const INFO_T &	GetInitInfo() const
	{
		return	m_info;
	}
	// 按块号删除文件
	int			DelFileByBlockIdx(blockoffset_t nStart);
	// 对有文件名的文件的处理（这些文件名或者目录名都是相对与包根的相对路径）
	whfile *	OpenFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart=NULL);
	WHDirBase *	OpenDir(const char *szDir);
	int			MakeDir(const char *szDir);
	// 删除路径（文件或目录）
	int			DelPath(const char *szPath);
	// 获取文件信息（主要是为了不解压具体数据就获得基本信息）
	int			GetPathInfo(const char *cszPath, whfileman::PATHINFO_T *pPathInfo);
	// 调试用
	// 返回连续的文件块序号列表（这个会改变当前的block）
	int			GetBlockList(blockoffset_t nStart, whvector<blockoffset_t> &vect);

	// 返回包中文件的原始数据（压缩后的数据），钟文杰添加
	whfile *	OpenRawFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart=NULL);
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_BASE_H__
