// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstring.h
// Creator      : Wei Hua (魏华)
// Comment      : 字串相关类。如：大型字串数组
//              : wh_isstrtoolong是用来检测字串是否太长的函数，主要用于网络放黑
//              : 对whstrinvector4hash切记不要put(const char *)这样的操作!!!!
// CreationDate : 2003-05-16
// ChangeLog    : 2004-03-16 wh_strsplit增加了""代表所有空白字符
//                2004-05-20 增加了wh_strlike(const char *cszPattern, const char *cszStr);
//                2004-07-26 增加了whstrlike可以自己设分隔符
//                2004-08-13 wh_strsplittoarray修改了对前后空白字符的处理
//                2004-12-13 split系列函数修改为只有""代表所有空白字符。如果分隔符为"\t"，则某个'\t'会分隔一次
//                2006-03-07 增加了wh_strsplit的int64参数：I。wh_strsetvalue增加了对int64的支持。
//                2006-05-30 增加了$(XX)处理函数
//                2006-07-12 取消了whstr4hash和whstrptr4hash中的m_bIC，今后如果需要使用大小写不敏感的直接使用xxxNocase即可
//                           _whstr_hashfuncbase被取消，直接使用_whstr_hashfunc或_whstr_hashfuncNocase即可
//                2006-07-25 增加了wh_strsplittovector方法
//                2006-08-08 本来想取消whstrinvector4hash中的使用const char *的构造函数，这样可以禁止在put的时候直接使用const char *，造成数据不一致（对于一个hash应该存入的所有str都是显式的whstrinvector）
//                           但是这样做却会造成find和erase的手动构造whstrinvector4hash的麻烦，所以还是放弃了，但是要切记不要put(const char *)这样的操作!!!!
//                2006-12-18 增加了utf8相关的函数。把原来wchar_t转换相关的函数中增加了长度参数，并且把nWC的意义从字节数改为wchar_t的个数。
//                2006-12-20 让wh_char2wchar和wh_wchar2char内部具有自动添加0结尾的功能。
//                2006-12-21 增加了WH_WCHAR2MULTICHAR0 和 WH_MULTICHAR2WCHAR0 两个宏
//                2007-03-14 把原来的str4hash等对象的字串比较用模板仿函数来写
//                2007-04-28 增加了strupr和strlwr的Src/Dst形式
//                2007-10-25 增加了参数分析函数wh_parse_opt
//                2008-09-09 增加了whstr_Check_MultiChar

#ifndef	__WHSTRING_H__
#define	__WHSTRING_H__

#include <stdlib.h>
#include <string.h>
#include <string>
#include "whvector.h"
#include "whcmn_def.h"

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

// 之所以放在这里是因为本来stricmp等就是全局函数
#ifdef	__GNUC__
inline int stricmp(const char *__s1, const char *__s2)
{
	return	strcasecmp(__s1, __s2);
}
inline int strnicmp(const char *__s1, const char *__s2, size_t __size)
{
	return	strncasecmp(__s1, __s2, __size);
}
#endif
// WHLINEEND的意义在于linux和windows下的文本换行符不统一，如果都使用\r\n则在windows下用notepad打开这样的文本并改写会出现错误(行错乱)
#ifdef	__GNUC__
#define	WHLINEEND	"\r\n"
#else
#define	WHLINEEND	"\n"
#endif

#define	WHLINEENDLEN	(sizeof(WHLINEEND)-1)

using std::string;

namespace n_whcmn
{

// 空白字符
extern const char *WHSTR_WHITESPACE;

// 将字串变成小写
// 返回字串长度
int	wh_strlwr(const char *cszSrc, char *szDst);
inline int	wh_strlwr(char *szStr)
{
	return	wh_strlwr(szStr, szStr);
}
// 将字串变成大写
// 返回字串长度
int	wh_strupr(const char *cszSrc, char *szDst);
inline int	wh_strupr(char *szStr)
{
	return	wh_strupr(szStr, szStr);
}
// 带长度检测的
int	wh_strlwr(const char *cszSrc, char *szDst, int nMaxSize);
inline int	wh_strlwr(char *szStr, int nMaxSize)
{
	return	wh_strlwr(szStr, szStr, nMaxSize);
}
int	wh_strupr(const char *cszSrc, char *szDst, int nMaxSize);
inline int	wh_strupr(char *szStr, int nMaxSize)
{
	return	wh_strupr(szStr, szStr, nMaxSize);
}

// 比较字串是否和字串数组中任何一个相同，有就返回序号>=0，否则返回-1
// apszStr是以NULL作为最后的结尾的
int	wh_strcmparray(const char *apszStr[], const char *pszOther);
int	wh_stricmparray(const char *apszStr[], const char *pszOther);

////////////////////////////////////////////////////////////////////
// 变长字串栈
////////////////////////////////////////////////////////////////////
class	whstrstack
{
protected:
	whvector<char>	m_strbuf;					// 用于存放字串
	whvector<int>	m_idxbuf;					// 用于存放各个字串的起始点索引
	char			m_sep;						// 分隔符，m_strbuf中的各个字串成员间通过该字符隔开
												// 其实这个类的初衷就是为文件路径的一些操作设计的
public:
	whstrstack(char sep);
	~whstrstack();
	void	clear();							// 清空，但不释放内存
	void	destroy();							// 释放所有内存
	const char *	push(const char *szStr);	// 压入一个字串（返回内部的指针）
	char *	pop(int *pnIdx=NULL);				// 弹出一个字串(返回NULL表示没有了)，该指针在下次push前有效。pnIdx中存放相应的其实点位置。
	int		reinitfrom(const char *szStr, const char *seps);
												// 从一个字串初始化堆栈，seps中有的元素都可能是分隔符
public:
	// 信息访问
	inline void		setsep(char sep)			// 设置分隔符
	{
		m_sep	= sep;
	}
	inline size_t	size() const				// 返回目前存储的元素个数
	{
		return	m_idxbuf.size();
	}
	inline const char *	getwholestr() const		// 获得栈内所有字串首尾相接并有分隔符的字串
	{
		return	m_strbuf.getbuf();
	}
	inline char *	operator[] (int idx) const	// 按照入栈的先后顺序为下标顺序
	{
		return	m_strbuf.getptr(m_idxbuf[idx]);
	}
	inline char *	getlaststr() const
	{
		if( size() == 0 )
		{
			return	NULL;
		}
		return	m_strbuf.getptr(m_idxbuf[size()-1]);
	}
};

////////////////////////////////////////////////////////////////////
// 等长的字串数组
////////////////////////////////////////////////////////////////////
class	whstrarr
{
protected:
	char	*m_pBuf;
	char	**m_ppBuf;
	int		m_nTotal;
	int		m_nUnitLen;
public:
	whstrarr();
	// 使用非默认构造函数初始化了就不用init了
	whstrarr(int nTotal, int nUnitLen);					// 功能同init
	whstrarr(const char **aszList, int nUnitLen);		// 直接从一个NULL结尾常数串中初始化
														// nUnitLen表示至少这么长，如果其中有字串超过就用超过的
	~whstrarr();
	int		init(int nTotal, int nUnitLen);
	int		release();
	int		clearstr(int nIdx);
	int		strcpy(int nIdx, const char *pszStr);		// 拷贝pszStr到nIdx号元素
	int		findstr(const char *pszStr, bool bIC);		// 查找pszStr，返回其索引，找不到返回-1。
														// bIC表示忽略大小写

	inline int		GetTotal() const
	{
		return	m_nTotal;
	}

	inline char *	safegetunit(int nIdx) const
	{
		if( nIdx<0 || nIdx>=m_nTotal )
		{
			return	0;
		}
		return	getunit(nIdx);
	}
	inline char *	getunit(int nIdx) const
	{
		return	m_ppBuf[nIdx];
	}
	inline char *	operator[](int nIdx) const
	{
		return	getunit(nIdx);
	}
	inline char **	getppbuf() const
	{
		return	m_ppBuf;
	}
};

////////////////////////////////////////////////////////////////////
// 等长的bin字串数组
////////////////////////////////////////////////////////////////////
class	whbstrarr	: public whstrarr
{
public:
	int		strcpy(int nIdx, char *pszStr);
};

////////////////////////////////////////////////////////////////////
// 变长的初始化后不会删减的字串数组
////////////////////////////////////////////////////////////////////
class	whvarstrarr
{
protected:
	whvector<char>	m_bufs;						// 存储所有字串
	whvector<int>	m_offsets;					// 存储各个字串的起始偏移量
public:
	void	reserve(int nMaxSize, int nMaxNum);	// 预留最大总长度和最多数目
	int		add(const char *cszStr);			// 添加一条字串，返回其序号
	const char *	get(int nIdx) const;		// 获取指定下标的字串

	inline const char *	safeget(int nIdx) const	// 保证不出错的
	{
		if( nIdx<0 || nIdx>=(int)m_offsets.size() )
		{
			return	NULL;
		}
		return	get(nIdx);
	}
	inline const char *	safegetlast() const		// 保证不出错的，获取最后一个
	{
		if( totalnum()==0 )
		{
			return	NULL;
		}
		return	get(totalnum()-1);
	}
	inline const char *	operator[] (int nIdx) const
	{
		return	get(nIdx);
	}
	inline size_t	totalsize() const
	{
		return	m_bufs.size();
	}
	inline size_t	totalnum() const
	{
		return	m_offsets.size();
	}
	inline size_t	availnum() const	// 在已有空间上还可以分配的个数
	{
		return	m_offsets.capacity() - m_offsets.size();
	}
	inline void		clear()
	{
		m_offsets.clear();
		m_bufs.clear();
	}

	// 集中功能函数
	// 判断字串中是否含有其中的词汇，并返回之(目前不考虑效率2004-02-10)
	const char *	hasstrin(const char *cszStr);
};

////////////////////////////////////////////////////////////////////
// 判断一个字串是否和描述匹配
// 比如：%共%产%党% ，就表示所有类似共产党的句子。
////////////////////////////////////////////////////////////////////
// 因为内部的方法可能多变，所以这里使用接口
class	whstrlike
{
public:
	char		SEP;		// 可以随便改(SEPSEP=字符SEP)
	static char	DFTSEP;		// 默认的：'%'
	static whstrlike *		Create();
	whstrlike()	: SEP('%')	{}
	virtual ~whstrlike()	{}
public:
	// 设置匹配模式
	virtual bool	SetPattern(const char *cszPattern)		= 0;
	// 设置模糊查找的间隙（即%可以代表的最大长度。默认为0，即无限长）
	virtual bool	SetMaxClearance(int nClearance)			= 0;
	// 判断是否匹配
	virtual bool	IsLike(const char *cszStr)				= 0;
};

////////////////////////////////////////////////////////////////////
// 用于字串匹配(检验一个字串是否和多个备选中的任意一个匹配)
////////////////////////////////////////////////////////////////////
class	whstrmatch
{
protected:
	struct	UNIT_T
	{
		// 本节点的字符
		char	c;
		bool	islast;							// 是否是最后一个字符
		// 下标<0表示非法，即没有指向的节点
		int		next;							// 下一个并列节点的下标
		int		firstchild;						// 第一个孩子的下标
		int		ext;							// 附加数据
		void	reset(char _c)
		{
			c			= _c;
			islast		= false;
			next		= -1;
			firstchild	= -1;
			ext			= 0;
		}
	};
	whvector<UNIT_T>	m_vectUnit;				// 初始化时就会保留一个作为最后根节点
	bool				m_bWholeMatch;			// 是否要全部匹配(给hasstrin使用的)
	bool				m_bLikeMatch;			// 是否需要模糊匹配
	whvector<whstrlike *>	m_vectLike;			// 如果需要模糊匹配，则各个匹配规则在这里面
public:
	inline void	reserve(int nNum)
	{
		m_vectUnit.reserve(nNum);
		m_vectLike.reserve(nNum);
	}
	inline int	size() const					// 现在的元素总数
	{
		return	m_vectUnit.size();
	}
	inline int	sizeoflike() const
	{
		return	m_vectLike.size();
	}
	inline void	setwholematch(bool bSet)
	{
		m_bWholeMatch	= bSet;
	}
public:
	whstrmatch(int nReserveNum = 1024);			// 可以在最初保留一定长度的内存(nReserveNum表示字串数量)
	~whstrmatch();
	// 设置是否需要模糊匹配
	void	setlikecheck(bool bSet);

	// 清空原来的列表
	void	clear();

	// 加入一个匹配串
	// 返回
	enum
	{
		ADDSTR_RST_OK		= 0,				// 成功
		ADDSTR_RST_DUP		= 1,				// 有重复串
		ADDSTR_RST_ERR		= -1,				// 出错
	};
	// nExt主要是为了将来做映射用的
	int		addstr(const char *cszWord, int nLen, int nExt);
	inline int	addstr(const char *cszWord, int nExt)
	{
		return	addstr(cszWord, strlen(cszWord), nExt);
	}
	// 查找匹配，判断有字串是否在
	// 如果有，则pnPos中返回匹配的起始位置，pnLen中为匹配长度
	bool	hasstrin(const char *cszStr, int nLen, int *pnPos = NULL, int *pnLen = NULL, int *pnExt=NULL);
	inline bool	hasstrin(const char *cszStr, int *pnPos = NULL, int *pnLen = NULL, int *pnExt=NULL)
	{
		return	hasstrin(cszStr, strlen(cszStr), pnPos, pnLen, pnExt);
	}
	// 集中功能函数
protected:
	// 从nStartIdx开始在同级查找字符
	// 如果找到，返回位置。
	// 否则返回<0
	int		findchar_in_chlid(int nIdx, char c, UNIT_T **ppUnit=NULL);
	// 将字符插入nStartIdx的同级中
	// 返回插入位置索引（出错返回<0）
	int		insertchar_in_chlid(int nIdx, char c);
	// 根据序号返回指针
	UNIT_T *	getptr(int nIdx);
};

////////////////////////////////////////////////////////////////////
// 字串最前面一个字节表示字串长度(今后不要改了，如果要加长最好另写一个whbstr2)
////////////////////////////////////////////////////////////////////
typedef	unsigned char	whbstrsize_t;

inline const char *	whbstrgetstr(const char *s)
{
	return	s + sizeof(whbstrsize_t);
}
inline whbstrsize_t	whbstrgetsize(const char *s)
{
	return	*(whbstrsize_t *)s;
}
inline void	whbstrcpy(char *pszDst, const char *pszSrc)
{
	memcpy(pszDst, pszSrc, whbstrgetsize(pszSrc)+1);
}
inline void	whbstrcpy_2cmn(char *pszDst, const char *pszSrc)
{
	whbstrsize_t	nSize = whbstrgetsize(pszSrc);
	memcpy(pszDst, whbstrgetstr(pszSrc), nSize);
	pszDst[nSize]	= 0;
}
inline unsigned int	whbstrhash(const char *s)
{
	whbstrsize_t	size, i;
	unsigned int	h = 0;
	size	= *(whbstrsize_t *)s;
	s		+= sizeof(whbstrsize_t);
	for(i=0;i<size;i++)
	{
		h	= h * 31 + *(unsigned char *)s;
		s	++;
	}
	return h;
}

////////////////////////////////////////////////////////////////////
// 本地字符判断
////////////////////////////////////////////////////////////////////
// 判断下面的字符是否是本地多字节字符（最多检查两个字节）
// 如果是则返回字符的长度-1，否则返回0
int	whstr_Check_MultiChar(const char *szC);

// 快速strstr（适合内部多重复常字串查找）
char *	wh_strstr(const char *pStr, const char *pSubStr);
// 考虑中文的字串查找
char *	wh_strstr_cn(const char *pStr, const char *pSubStr);

////////////////////////////////////////////////////////////////////
// 字串流 whstrstream
// 也可用作二进制读/写流
// 如果写超过了setbin设置的长度，则长度会扩展（所以上层要么留出足够的缓冲，或者使用不可扩展选项）
////////////////////////////////////////////////////////////////////
class	whstrstream
{
protected:
	// 指向字串缓冲的指针
	char		*m_pszStr;
	// 字串长度(不含最后的'\0')
	int			m_nSize;
	// 当前读取的位置
	int			m_nOffset;
	// 用于存储getchnc的结果
	char		m_szCHNC[3];
	// 是否需要throw (如果为真，则在read出错时抛出字串异常)
	bool		m_bNeedThrowErrStr;
	// 在写时不可扩展
	bool		m_bNoExtendOnWrite;
public:
	whstrstream();
	// 设置字串(不设置就不能进行读取操作)
	// 注意，里面会使用strlen函数获取字串长度，所以只有真正有字串内容后才能使用setstr，否则得到的字串长度不对!!!!
	void	setstr(const char *pszStr);
	// 设置二进制数据
	void	setbin(const void *pszBin, int nSize);
	// 开始从头读
	void	rewind();
	// 移动指针
	// nOrigin使用和fseek参数一样的SEEK_SET、SEEK_CUR和SET_END
	int		seek(int nOffset, int nOrigin);
	// 获取指定字节的数据，返回得到的字节数
	// pBuf可以为空，为空则跳过那么多字节，相当于seekcur
	int		read(void *pBuf, int nSize);
	// 获取一个字符
	char	getc();
	// 获取一个中文兼容字符（即如果是中文就返回两字节中文）
	const char *	getchnc(int *pnLen);
	// 固定类型的获取(返回得到的字节数)
	template<typename _Ty>
	inline int	read(_Ty *pVar)
	{
		int	rst	= read(pVar, sizeof(_Ty));
		if( m_bNeedThrowErrStr
		&&  rst!=sizeof(_Ty) )
		{
			throw	"BAD Size";
		}
		return	rst;
	}
	// 获取数组(返回元素个数，注意：不是总bytes)
	template<typename _Ty>
	inline int	readarray(_Ty *pArr, int nNum)
	{
		int	rst	= read(pArr, sizeof(_Ty)*nNum) / sizeof(_Ty);
		if( m_bNeedThrowErrStr
		&&  rst!=nNum )
		{
			throw	"BAD Size";
		}
		return	rst;
	}
	// 获取变长字串并保存为普通字串(返回字串长度--strlen)
	// szStr可以为空，则跳过这个字串
	int		readvstrasstr(char *szStr);

	// 写操作。
	// （!!!!注意，写操作的前提是保证setbin的buffer足够长，如果越界是不是报错的!!!!）
	int		write(const void *pBuf, int nSize);
	// 写固定类型变量
	template<typename _Ty>
	inline int	write(_Ty *pVar)
	{
		return	write(pVar, sizeof(_Ty));
	}
	// 写入数组(返回写入的元素个数，注意：不是总bytes)
	template<typename _Ty>
	inline int	writearray(_Ty *pArr, int nNum)
	{
		return	write(pArr, sizeof(_Ty)*nNum) / sizeof(_Ty);
	}
	// 将普通字串按照vstr写
	int		writestrasvstr(const char *cszStr);
	// 打印字串
	int	sprintf(const char *cszFmt, ...);
	// 设置不可扩展
	inline void	setnoextendonwrite(bool bSet)
	{
		m_bNoExtendOnWrite	= bSet;
	}
public:
	// 获得字串长度
	inline int		getlen() const
	{
		return	m_nSize;
	}
	// 是否到了结尾(End Of Stream)
	inline bool		iseos() const
	{
		return	m_nOffset == m_nSize;
	}
	// 获得当前缓冲的指针位置
	inline char *	getcurptr()
	{
		return	(char*)(m_pszStr+m_nOffset);
	}
	// 获得当前读/写到的偏移
	inline int		getoffset() const
	{
		return	m_nOffset;
	}
	// 设置throw
	inline void		setneedthrowerrstr(bool bSet)
	{
		m_bNeedThrowErrStr	= bSet;
	}
};

////////////////////////////////////////////////////////////////////
// 字串比较器
////////////////////////////////////////////////////////////////////
// 区分大小写
struct	WHStrCmp
{
	WHStrCmp()	{}
	inline int	operator() (const char *_str1, const char *_str2) const
	{
		return	strcmp(_str1, _str2);
	}
};
struct	WHStrCmp_Nocase
{
	WHStrCmp_Nocase()	{}
	inline int	operator() (const char *_str1, const char *_str2) const
	{
		return	stricmp(_str1, _str2);
	}
};

////////////////////////////////////////////////////////////////////
// 用于hash表的字串指针对象
////////////////////////////////////////////////////////////////////
// 一般应该使用这个，因为它大小写敏感
// 记住：这个只能用指针，指针必须保证不能变，不能指向局部变量
template<class _StrCmp>
class	whstrptr4hashbase
{
protected:
	const char		*m_pszStr;
	_StrCmp			m_strcmp;
public:
	whstrptr4hashbase()
	: m_pszStr(NULL)
	{
	}
	whstrptr4hashbase(const char *szStr)
	: m_pszStr(szStr)
	{
	}
	whstrptr4hashbase(const whstrptr4hashbase & other)
	: m_pszStr(other.m_pszStr)
	{
	}
	~whstrptr4hashbase()
	{
	}
	inline void	SetStr(const char *szStr)
	{
		m_pszStr	= szStr;
	}
	inline bool operator == (const whstrptr4hashbase & other) const
	{
		return	operator == (other.m_pszStr);
	}
	inline bool operator == (const char * other) const
	{
		if( m_pszStr )
			if( other )
				return	m_strcmp(m_pszStr, other)==0;
			else
				return	false;	// 有个是空
		else if( other )
			return	false;		// 有个是空
		else
			return	true;		// 两个都是空
	}
	inline bool operator <	(const whstrptr4hashbase & other) const
	{
		return	operator <	(other.m_pszStr);
	}
	inline bool operator <	(const char * other) const
	{
		if( m_pszStr )
			if( other )
				return	m_strcmp(m_pszStr, other)<0;
			else
				return	false;	// 所有的非空字串都大于空
		else if( other )
			return	true;		// 空小于所有非空字串
		else
			return	false;		// 两个相等，都是空
	}
	inline const char * GetPtr() const
	{
		return	m_pszStr;
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
};
typedef	whstrptr4hashbase<WHStrCmp>			whstrptr4hash;
typedef	whstrptr4hashbase<WHStrCmp_Nocase>	whstrptr4hashNocase;

// 不区分大小写
// 在vector中的字串(类似于whstrptrxxx，_StrCmp决定是否大小写敏感)
template<class _StrCmp>
class	_whstrinvector4hash_Base
{
private:
	whvector<char>	*m_pvect;
	int				m_nOffset;
	bool			m_bInnerVector;
	_StrCmp			m_strcmp;
public:
	_whstrinvector4hash_Base()
		: m_pvect(NULL)
		, m_nOffset(0)
		, m_bInnerVector(false)
	{
	}
	_whstrinvector4hash_Base(whvector<char> *pvect, int nOffset)
		: m_pvect(pvect)
		, m_nOffset(nOffset)
		, m_bInnerVector(false)
	{
	}
	_whstrinvector4hash_Base(const _whstrinvector4hash_Base &other)
		: m_pvect(other.m_pvect)
		, m_nOffset(other.m_nOffset)
		, m_bInnerVector(false)
	{
	}
	_whstrinvector4hash_Base(const char *cszStr)
		: m_pvect(NULL)
		, m_nOffset(0)
		, m_bInnerVector(true)
	{
		m_pvect	= new whvector<char> (strlen(cszStr)+1);
		memcpy(m_pvect->getbuf(), cszStr, m_pvect->size());
	}
	~_whstrinvector4hash_Base()
	{
		if( m_bInnerVector )
		{
			delete	m_pvect;
		}
	}
	inline void	SetStr(whvector<char> *pvect, int nOffset)
	{
		m_pvect		= pvect;
		m_nOffset	= nOffset;
	}
	inline bool operator == (const _whstrinvector4hash_Base & other) const
	{
		return	operator == (other.GetPtr());
	}
	inline bool operator == (const char * other) const
	{
		return	m_strcmp(GetPtr(), other)==0;
	}
	inline bool operator <	(const _whstrinvector4hash_Base & other) const
	{
		return	operator <	(other.GetPtr());
	}
	inline bool operator <	(const char * other) const
	{
		return	m_strcmp(GetPtr(), other)<0;
	}
	inline const char * GetPtr() const
	{
		return	m_pvect->getptr(m_nOffset);
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
	inline int	GetOffset() const
	{
		return	m_nOffset;
	}
};
typedef	_whstrinvector4hash_Base<WHStrCmp>			whstrinvector4hash;
typedef	_whstrinvector4hash_Base<WHStrCmp_Nocase>	whstrinvector4hashNocase;

// 非指针
template<class _StrCmp, class _AllocationObj=whcmnallocationobj>
class	whstr4hashbase
{
protected:
	whvector<char, false, _AllocationObj>	m_vectStr;
	_StrCmp			m_strcmp;
public:
	whstr4hashbase()
	{
	}
	whstr4hashbase(const char *szStr)
	{
		SetStr(szStr);
	}
	whstr4hashbase(const whstr4hashbase & other)
	{
		SetStr(other.GetPtr());
	}
	~whstr4hashbase()
	{
	}
	inline void operator = (const whstr4hashbase & other)
	{
		operator = (other.GetPtr());
	}
	inline void operator = (const char * szStr)
	{
		SetStr(szStr);
	}
	inline void	SetStr(const char *szStr)
	{
		if( szStr )
		{
			int	len = strlen(szStr)+1;
			m_vectStr.resize(len);
			memcpy((void *)GetPtr(), szStr, len);
		}
	}
	inline bool operator == (const whstr4hashbase & other) const
	{
		return	operator == (other.GetPtr());
	}
	inline bool operator == (const char * other) const
	{
		if( m_vectStr.size() )
			if( other )
				return	m_strcmp(GetPtr(), other)==0;
			else
				return	false;	// 有个是空
		else if( other )
			return	false;		// 有个是空
		else
			return	true;		// 两个都是空
	}
	inline bool operator <	(const whstr4hashbase & other) const
	{
		return	operator <	(other.GetPtr());
	}
	inline bool operator <	(const char * other) const
	{
		if( m_vectStr.size() )
			if( other )
				return	m_strcmp(GetPtr(), other)<0;
			else
				return	false;				// 所有的非空字串都大于空
		else if( other )
			return	true;					// 空小于所有非空字串
		else
			return	false;					// 两个相等，都是空
	}
	inline const char * GetPtr() const
	{
		return	m_vectStr.getbuf();
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
	inline int	GetLen() const				// 不包含最后的\0
	{
		return	m_vectStr.size()-1;
	}
};
typedef	whstr4hashbase<WHStrCmp, whcmnallocationobj>			whstr4hash;
typedef	whstr4hashbase<WHStrCmp_Nocase, whcmnallocationobj>		whstr4hashNocase;

struct	_whstr_hashfunc
{
	_whstr_hashfunc() {}

	inline unsigned int	operator()(const whstrptr4hash &s)	const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const whstr4hash &s)	const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const whstrinvector4hash &s) const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const char *s)		const
	{
		unsigned int h = 0;
		for(;*s; s++)
		{
			h = h * 31 + *(unsigned char *)s;
		}
		return h;
	}
};

struct	_whstr_hashfuncNocase
{
	_whstr_hashfuncNocase() {}

	inline unsigned int	operator()(const whstrptr4hashNocase &s)	const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const whstr4hashNocase &s)	const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const whstrinvector4hashNocase &s) const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const char *s)		const
	{
		unsigned int	h = 0;
		unsigned char	c = 0;
		for(;*s; s++)
		{
			c	= *(unsigned char *)s;
			if(c>='a' && c<='z')
			{
				//小写变大写
				c	-= ('a'-'A');
			}
			h = h * 31 + c;
		}
		return h;
	}
};


// 变长数据读写
// 读
// 返回数据长度
template<typename _LenTy>
int		whvldata_read(const _LenTy *cszSrc, void *szDst)
{
	_LenTy	nLen	= *cszSrc;
	cszSrc			++;
	if( nLen>0 )
	{
		memcpy(szDst, cszSrc, nLen);
	}
	return	(int)nLen;
}
// 返回数据部分起始地址(pszDst)和数据部分长度
template<typename _LenTy>
int		whvldata_read_ref(const _LenTy *cszSrc, const void *&pszDst)
{
	_LenTy	nLen	= *cszSrc;
	cszSrc			++;
	pszDst			= cszSrc;
	return	(int)nLen;
}
// 写
// 返回数据长度
template<typename _LenTy>
int		whvldata_write(const void *cszSrc, int nLen, _LenTy *szDst)
{
	*szDst			= nLen;
	szDst			++;
	if( nLen>0 )
	{
		memcpy(szDst, cszSrc, nLen);
	}
	return			nLen + sizeof(_LenTy);
}

////////////////////////////////////////////////////////////////////
// 字串处理
////////////////////////////////////////////////////////////////////
// 以下几个函数返回结果字串的长度
// 除掉字串首尾的包含在szKick里的字符
int		wh_strtrimc(char *szStr, const char *szKick);
// 除掉字串首尾的空格或回车
int		wh_strtrim(char *szStr);
// 去除字串尾的包含在szKick里的字符
int		wh_strendtrimc(char *szStr, const char *szKick);
// 去除字串尾的\或/
int		wh_strkickendslash(char *szStr);

// 保证字串末尾有某个字符
int		wh_strsurelast(char *__str, char __c);
// 保证结尾是"/"
int		wh_strsureendslash(char *__str);

// 一般来说__count一定等于__dst的最大长度减一，所以拷贝后在最末尾添加一个0
// 举例来说，假定char szDst[32]，使用方式为wh_strncpy0(szDst, src, sizeof(szDst)-1);
char *	wh_strncpy0(char *__dst, const char *__src, size_t __count);
char *	wh_strncat0(char *__dst, const char *__tocat, size_t __count);
// DST必须是可以sizeof的字串，不能是字串指针
#define	WH_STRNCPY0(DST, SRC)	wh_strncpy0(DST, SRC, sizeof(DST)-1)
#define	WH_STRNCAT0(DST, TOCAT)	wh_strncat0(DST, TOCAT, sizeof(DST)-1)
#define	WH_MEMCPY(DST, SRC)		memcpy(DST, SRC, sizeof(DST))
#define WH_CLEAR_STRNCPY0(DST, SRC) (memset(DST, 0, sizeof(DST)),wh_strncpy0(DST, SRC, sizeof(DST)-1))		// 先清空数组,然后再拷贝,web扩展需要,added by yuezhongyue 2011-12-18
// 保证STR的最后一位是0，免得按照字串处理时不越界
#define	WH_STRSURELAST0(STR)	STR[sizeof(STR)-1] = 0;

// 判断一个字串是否过长
// 返回真表示字串物理长度(包括最后一个'\0')超过了__lenlimit
bool	wh_isstrtoolong(const char *__str, int __lenlimit);

// 给一个字串添加一个后缀，但是总长度不能超过一定值，如果nIdx>0则利用数字递增在最后
// nMaxSize表示包含最后一个0的长度
const char *	wh_appendext(char *szTarget, int nMaxSize, const char *cszExt, int nIdx);

// 判断一个字串中是否有某个范围内[nChar1,nChar2]的字符，返回第一个遇到的这样的字符（如果没有返回0）
char	wh_checkcharrange(const char *cszStr, int nChar1, int nChar2);

// 按照__splitter分割参数，返回被分割的参数个数(即便有*的也按总共找到的分段数返回)
// __splitter如果为""则表示分隔符为所有的空字符；
// __splitter如果第一个字符为'\1'则表示使用后面的所有单字符作为分隔符；
// 否则__splitter整串作为分隔符。
// 用一个格式字串表示各个参数的类型
// 如果没有读到则值不变
// **可用的格式**有：
//	a	string till end
//	s	string
//	d	int
//	o	short
//	y	byte
//	f	float
//	b	bool
//	t	time_t
//  I	int64
//	*	ignore
// 返回：
// 分解到的参数个数（注意："*"的也算一个）
// 注：之所以写两个函数是因为没办法利用变参调用变参
int		wh_strsplit(const char *__format, const char *__src, const char *__splitter, ...);
int		wh_strsplit(int *pnOffset, const char *__format, const char *__src, const char *__splitter, ...);

// 连接两个字串
char *	wh_strcat(char *__dst, const char *__str1, const char *__str2);

// 附加的类型
typedef	void*	voidptr;
typedef	char*	charptr;

void	wh_strsetvalue_charptr(charptr pValue, int nMaxSize, const char *cszVal);	// 字串这个会特殊一些
void	wh_strsetvalue(charptr pValue, const char *cszVal);							// 这个主要给array相关的做，就不判断长度了
void	wh_strsetvalue(whbyte &rfValue, const char *cszVal);
void	wh_strsetvalue(short &rfValue, const char *cszVal);
void	wh_strsetvalue(int &rfValue, const char *cszVal);
void	wh_strsetvalue(unsigned int &rfValue, const char *cszVal);
void	wh_strsetvalue(float &rfValue, const char *cszVal);
void	wh_strsetvalue(bool &rfValue, const char *cszVal);
void	wh_strsetvalue(std::string &rfValue, const char *cszVal);
void	wh_strsetvalue(voidptr &rfValue, const char *cszVal);
void	wh_strsetvalue(time_t &rfValue, const char *cszVal);
void	wh_strsetvalue(whint64 &rfValue, const char *cszVal);

const char *	wh_strgetvalue(const charptr pValue, char *pszVal);
const char *	wh_strgetvalue(const whbyte &rfValue, char *pszVal);
const char *	wh_strgetvalue(const short &rfValue, char *pszVal);
const char *	wh_strgetvalue(const int &rfValue, char *pszVal);
const char *	wh_strgetvalue(const unsigned int &rfValue, char *pszVal);
const char *	wh_strgetvalue(const float &rfValue, char *pszVal);
const char *	wh_strgetvalue(const bool &rfValue, char *pszVal);
const char *	wh_strgetvalue(const std::string &rfValue, char *pszVal);
const char *	wh_strgetvalue(const voidptr &rfValue, char *pszVal);
const char *	wh_strgetvalue(const time_t &rfValue, char *pszVal);
const char *	wh_strgetvalue(const whint64 &rfValue, char *pszVal);

int	wh_atoi(const char *cszVal);

// whstr2int如果转换不成功则返回0
int	whstr2int(const char *cszVal);
const char *	whint2str(int nVal, char *pszVal=NULL);

whint64	whstr2int64(const char *cszVal);
const char *	whint642str(whint64 nVal, char *pszVal=NULL);

bool	whstr2bool(const char *cszVal);
const char *	whbool2str(bool bVal, char *pszVal=NULL);

// 判断到了spliter的位置
inline bool	_wh_isspliter(const char *__str, const char *__splitter, int __slen)
{
	switch( __splitter[0] )
	{
		case	0:
		// 如果__spliter长度为0，则认为所有空白字符都是spliter
		return	strchr(WHSTR_WHITESPACE, __str[0]) != NULL;
		case	1:
		// 则说明是按照单字符来比较的
		return	strchr(__splitter+1, __str[0]) != NULL;
		default:
		// 其他情况则按正常比较（整个字串作为分隔符）
		return	strncmp(__str, __splitter, __slen) == 0;
	}
}

// 将字串分解为同类型的数组
// 可用的格式见上面"**可用的格式**"

// 注意，如果_Ty是char *，那么__arr要么直接是char x[n][m]的类型，要么是char *x[n]类型
// 如果将x[n][m]强制转换为void *再转换为char**则会出非法操作
template<typename _Ty>
int		wh_strsplittoarray(const char *__src, const char *__splitter, _Ty *__arr, int __num)
{
	char	/**buf = NULL, 忘了原来buf用来做什么了*/ bufi[128];
	int		count, start, i, sl, l, rcount;

	count	= 0;
	rcount	= 0;
	i		= 0;
	sl		= strlen(__splitter);
	start	= 0;

	bool	bWordBegin	= false;

	// find the next splitter
	l	= strlen(__src);
	for(i=0; i<l&&count<__num; i++)
	{
		if( !bWordBegin && __splitter[0]==0 )
		{
			if( strchr(WHSTR_WHITESPACE, __src[i]) )
			{
				continue;
			}
		}
		if( _wh_isspliter(__src+i, __splitter, sl) )
		{
			bWordBegin	= false;
			wh_strncpy0(bufi, __src+start, i-start);
			wh_strtrim(bufi);
			wh_strsetvalue(__arr[count], bufi);
			// 原来是：i += sl; 这样在分隔符连续时会出错
			i		+= (sl-1);
			start	= i+1;
			count	++;
			if( (start)>=l ) break;
		}
		else
		{
			if( !bWordBegin )
			{
				bWordBegin	= true;
			}
		}
		if( whstr_Check_MultiChar(__src+i) )
		{
			i	++;			// 汉字处理
		}
	}
	// 最后一个参数(利用bWordBegin来清除结尾的空白)
	if( bWordBegin && start<l && count<__num )
	{
		wh_strncpy0(bufi, __src+start, i-start);
		wh_strtrim(bufi);
		wh_strsetvalue(__arr[count], bufi);
		count	++;
	}
	rcount	= count;

	return	rcount;
}
template<typename _Ty>
int		wh_strsplittovector(const char *__src, const char *__splitter, whvector<_Ty> &__vect)
{
	__vect.clear();

	char	/**buf = NULL, 忘了原来buf用来做什么了*/ bufi[128];
	int		start, i, sl, l;

	i		= 0;
	sl		= strlen(__splitter);
	start	= 0;

	bool	bWordBegin	= false;

	// find the next splitter
	l	= strlen(__src);
	for(i=0; i<l; i++)
	{
		if( !bWordBegin && __splitter[0]==0 )
		{
			if( strchr(WHSTR_WHITESPACE, __src[i]) )
			{
				continue;
			}
		}
		if( _wh_isspliter(__src+i, __splitter, sl) )
		{
			bWordBegin	= false;
			wh_strncpy0(bufi, __src+start, i-start);
			wh_strtrim(bufi);
			wh_strsetvalue((*__vect.push_back()), bufi);
			// 原来是：i += sl; 这样在分隔符连续时会出错
			i		+= (sl-1);
			start	= i+1;
			if( (start)>=l ) break;
		}
		else
		{
			if( !bWordBegin )
			{
				bWordBegin	= true;
			}
		}
		if( whstr_Check_MultiChar(__src+i) )
		{
			i	++;			// 汉字处理
		}
	}
	// 最后一个参数(利用bWordBegin来清除结尾的空白)
	if( bWordBegin && start<l )
	{
		wh_strncpy0(bufi, __src+start, i-start);
		wh_strtrim(bufi);
		wh_strsetvalue((*__vect.push_back()), bufi);
	}

	return	__vect.size();
}

// 从字串中找出用某些字符括住的子串
// 如果找到则返回__substr，否则返回NULL
char *	wh_findquoted(const char *__src, char __left, char __right, char *__substr);
// 功能和上面类似，不过会修改__src的内容，将__left和__right的位置置0
char *	wh_findquotedandmodify(char *__src, char __left, char __right, char **__ptr_right);

// 将一个通过字串分解为若干部分
// __src被分解的字串
// __sep存放所有可能的分隔符。即存在于__sep中的字符都是分隔符。
// __dst用来存分解部分用'\0'分割的实体，其长度不能少于__src，上层应该保证
// __ptr是用来存放各个部分的头指针的指针数组
// *__num原来是__ptr最多可容纳的元素数量，最后返回实际的元素个数
int		wh_strexplode(const char *__src, const char *__sep, char *__dst, char *__ptr[], int *__num);
// 将若干分解的字串组合成一个字串
// __ptr是用来存放各个部分的头指针的指针数组
// __num原来是__ptr容纳的元素数量
// __sep用于分隔各个字串，可以是字串(这个和explode中的含义不一样，请注意!!)
// __dst是最后生成的字串，其长度由上层保证足够
int		wh_strimplode(char *__ptr[], int __num, const char *__sep, char *__dst);

// 注意：下面两个wh_strchrreplace中，如果__to为0表示不替换，只返回字串中__from的个数
// 替换字串中的某个字符到另一个字符。返回被替换的个数。
int		wh_strchrreplace(char *__str, char __from, char __to);
// __strlen表示最多替换的长度(这个也返回被替换的个数)
int		wh_strchrreplace(char *__str, int __strlen, char __from, char __to);

// 查找字串中的任何和指定字串中任何字符匹配的位置
// 返回结果为匹配位置
// __what中返回匹配到的字符
const char *	wh_strchr(const char *__str, const char *__matches, char &__what);
const char *	wh_strrchr(const char *__str, const char *__matches, char &__what);

// 找到字串中第N个和字符匹配的位置
const char *	wh_the_n_strchr(const char *__str, char __c, int __n);

// 将字串中的所有某个字串替换为另一个字串
// nMaxLen为那个字串最多能涨到的长度（包括最后一个0）
int		wh_strstrreplace(char *__str, int nMaxLen, const char *__from, const char *__to);

// 将一个字串__src中连续的某种字符（任何__from中的字符）都替换为一个字符__to到__dst中去，但__dst的长度不能超过nMaxLen
int		wh_strcontinuouspatternreplace(char *__dst, int nMaxLen, const char *__src, const char *__from, char __to);

// 查找字串中的$(XX)字串，并作相应的替换处理（通过__cb进行）
// __cb是回调函数，__param是回调的参数
enum
{
	WH_STR_MP_TYPE_CMN		= 0,		// 普通字串
	WH_STR_MP_TYPE_MACRO	= 1,		// 宏字串
	WH_STR_MP_TYPE_END		= 2,		// 结束
};
int		wh_strmacroprocess(const char *__src, int (*__cb)(int __type, const char *__str, int __len, void *__param), void *__param);

// 分析一行文本，解析为Key=Value的形式
enum
{
	WHSTR_TYPE_STRING					= 1,
	WHSTR_TYPE_INT						= 2,
	WHSTR_TYPE_FLOAT					= 3,
	WHSTR_TYPE_SHORT					= 4,
	WHSTR_TYPE_BOOL						= 5,
	WHSTR_TYPE_TIME						= 6,
	WHSTR_TYPE_INTARRAY					= 11,		// 注这两个array是用空格分割的
	WHSTR_TYPE_FLOATARRAY				= 12,
	WHSTR_TYPE_EXTFUNC					= 100,
	WHSTR_TYPE_EXTFUNCPARSEONLY			= 101,		// 只做解析用
	WHSTR_TYPE_INT64					= 102,		// INT64
};
enum
{
	WHSTR_ACT_PARSE						= 0,
	WHSTR_ACT_PRINT						= 1,
	WHSTR_ACT_SAVE						= 2,
};
typedef void (*whstr_ext_func_t)(const char *szStr, void *pValue, int nAct);	// nAct表示是解析还是打印
struct	WH_LINE_PARSE_INFO_T
{
	const char	*cszKey;
	void	*pValue;
	int		nType;						// WHSTR_TYPE_XXX
	int		nExt;						// 如果是array，最大元素数目为nExt。元素间以空格为分隔符
										// 如果是需要附加解析函数的，nExt是函数指针
};
// 下面的parse比较过时了，如果只是读数据今后请使用whdataini.h中提供的功能!!!!
// 如果牵扯到写数据，则还是可以使用到wh_parsesave功能的，这个写比较简单。
// 分解一行
int		wh_strlineparse(const char *szLine, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// 分解一个文件
int		wh_strfileparse(const char *szFile, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// 分解文件的一个sect
int		wh_strfilesectparse(const char *szFile, const char *szSectName, const char *szSep, WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// 打印键值和对应数据
int		wh_parseprint(WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);		// nTab表示"="所在的位置
int		wh_parsesave(const char *szFile, WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);
int		wh_parsesave(FILE *fp, WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);
char *	wh_genspace(char *szDst, const char *szStr, int nTab = 16);		// 生成填充空格(szDst和szStr可以是一个)
// 字串转换为数组(元素以空格分隔)
// 返回数组尺寸
int		wh_str2array(int nType, const char *szStr, void *pArr, int nNum);

// 把一个字串指针数组转换为一个整字串（返回字串总长度，不超过nMaxSize）
int		wh_array2str(int argc, char *argv[], char *szStr, int nMaxSize);
// 读出字串中的参数段（-xxx xxx的形式。返回得到的opt的个数）
// nSize是需要解析的字串的长度
// vectTmp是外界提供的临时缓冲，用于把szStr中的数据每段后面都加上0结尾（vectOpt里面的指针就是指向vectTmp内部的偏移的）
// vectOpt里面最终存放的是每个参数段的内容（首指针），其中偶数位置为参数键，奇数位置为参数值
int		wh_parse_opt(const char *szStr, int nSize, whvector<char> &vectTmp, whvector<const char *>&vectOpt, char *pszErr);

// 在下面两个函数前应该保证曾经调用srand
// 生成随机字串(注意：会自动在szStr最后加上一个'\0')，所以要保证szStr最多可容纳nLen+1个byte
int		wh_randstr(char *szStr, int nLen, bool bAutoAddLast0=true);
// 生成随机二进制串
int		wh_randhex(void *szStr, int nLen);

// 16进制数变成字串(默认是小写)
const char *	wh_hex2str(const unsigned char *szHex, int nLen, char *szStr=NULL, int nSpace=0, bool bUPCASE=false);
// 字串变成16进制数(nSize=0表示按照字串的长度做，>0表示生成的16进制串的最大长度)
// 返回生成的16进制数据的长度
// 注：字串中每个byte之间不要有空格，如：023f3d5e
//     因为反正总长会变小szHex和szStr可以是一个指针
int				wh_str2hex(const char *szStr, unsigned char *szHex, size_t nSize=0);

// 拷贝一个字串到whvector<char>
inline void	wh_copystr2vector(whvector<char> &vect, const char *cszStr)
{
	vect.resize(strlen(cszStr)+1);
	memcpy(vect.getbuf(), cszStr, vect.size());
}

// 判断字串是否是纯数字组成的
// nMaxLen表示字串的长度不能超过某个值，否则也按非纯数字论处（nMaxLen如果为0则表示不计较长度）
bool	wh_isstrnumeric(const char *cszStr, int nMaxLen=0);

// 打印字符表
void	wh_printchartable(FILE *fp);
/*  0~127
    ZERO,       .,       .,       .,       .,       .,       .,      \a,
       0,       1,       2,       3,       4,       5,       6,       7,

      \b,      \t,      \n,      \v,      \f,      \r,       .,       .,
       8,       9,      10,      11,      12,      13,      14,      15,

       .,       .,       .,       .,       .,       .,       .,       .,
      16,      17,      18,      19,      20,      21,      22,      23,

       .,       .,       .,       .,       .,       .,       .,       .,
      24,      25,      26,      27,      28,      29,      30,      31,

   SPACE,       !,       ",       #,       $,       %,       &,       ',
      32,      33,      34,      35,      36,      37,      38,      39,

       (,       ),       *,       +,       ,,       -,       .,       /,
      40,      41,      42,      43,      44,      45,      46,      47,

       0,       1,       2,       3,       4,       5,       6,       7,
      48,      49,      50,      51,      52,      53,      54,      55,

       8,       9,       :,       ;,       <,       =,       >,       ?,
      56,      57,      58,      59,      60,      61,      62,      63,

       @,       A,       B,       C,       D,       E,       F,       G,
      64,      65,      66,      67,      68,      69,      70,      71,

       H,       I,       J,       K,       L,       M,       N,       O,
      72,      73,      74,      75,      76,      77,      78,      79,

       P,       Q,       R,       S,       T,       U,       V,       W,
      80,      81,      82,      83,      84,      85,      86,      87,

       X,       Y,       Z,       [,       \,       ],       ^,       _,
      88,      89,      90,      91,      92,      93,      94,      95,

       `,       a,       b,       c,       d,       e,       f,       g,
      96,      97,      98,      99,     100,     101,     102,     103,

       h,       i,       j,       k,       l,       m,       n,       o,
     104,     105,     106,     107,     108,     109,     110,     111,

       p,       q,       r,       s,       t,       u,       v,       w,
     112,     113,     114,     115,     116,     117,     118,     119,

       x,       y,       z,       {,       |,       },       ~,       ,
     120,     121,     122,     123,     124,     125,     126,     127,
*/

// 如果cszKey非NULL且非空串(0长度字串)，则查找对应的value。否则打印全部。
// cszDft表示如果什么也没有找到则打印这个字串
void	wh_strkeyprint(const char *astr[][2], const char *cszKey=NULL, const char *cszDft=NULL, bool bIC=true);

// 判断字母是否是16进制中的数字0~9 A~F a~f
bool	wh_charishexdigit(char c);
// 把字串中的%XX替换为相应的ASCII数值
// 返回转义之后的字串长度
int		wh_change_percentsymbol(const char *cszSrc, char *pszDst);

#ifdef	WIN32
#define	snprintf	_snprintf
#endif
// 和WCHAR相关的（里面默认本次字符集都是GBK）
// 下面四个转换函数都返回转换之后的字串的长度（注意，不一定和nC或nWC成正比）
// 将WCHAR转换为char（如果szC为空则直接将转换后的字串存到原来的szWC中去）
// （nWC表示szWC的wchar_t的个数!!!! nC表示szC中char的个数）
// WH_WCHAR2MULTICHAR0 和 WH_MULTICHAR2WCHAR0 两个宏用于将一方转换到另一方（固定长度）并返回另一方的指针（注意哦，0只是代表是个比较底层的函数，不是说最后加0，0是都自动加的）
int		wh_wchar2char(char *szWC);
int		wh_wchar2char(const wchar_t *cszWC, int nWC, char *szC, int nC);
int		wh_wchar2multichar(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
const char *	wh_wchar2multichar0(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
#define	WH_WCHAR2MULTICHAR0(cszCodePage, cszWC, szC)	wh_wchar2multichar0(cszCodePage, cszWC, wcslen(cszWC), szC, sizeof(szC)-1)
// 安全的转换，如果失败返回""，而不是NULL
const char *	wh_wchar2multichar0_safe(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
#define	WH_WCHAR2MULTICHAR0_safe(cszCodePage, cszWC, szC)	wh_wchar2multichar0_safe(cszCodePage, cszWC, wcslen(cszWC), szC, sizeof(szC)-1)

// 将char转换为WCHAR
// 返回实际转换的wchar的个数（不包括结尾的0，虽然0是自动添加的）
int		wh_char2wchar(char *szC);
int		wh_char2wchar(const char *cszC, int nC, wchar_t *szWC, int nWC);
int		wh_multichar2wchar(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
const wchar_t *	wh_multichar2wchar0(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
#define	WH_MULTICHAR2WCHAR0(cszCodePage, cszC, szWC)	wh_multichar2wchar0(cszCodePage, cszC, strlen(cszC), szWC, sizeof(szWC)/sizeof(szWC[0])-1)
// 安全的转换，如果失败返回""，而不是NULL
const wchar_t *	wh_multichar2wchar0_safe(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
#define	WH_MULTICHAR2WCHAR0_safe(cszCodePage, cszC, szWC)	wh_multichar2wchar0_safe(cszCodePage, cszC, strlen(cszC), szWC, sizeof(szWC)/sizeof(szWC[0])-1)


// UTF8和其他间的转换（都返回转换之后的字串长度）
int		wh_utf82wchar(const char *cszUTF8, int nUTF8Len, wchar_t *szWC, int nWC);
int		wh_wchar2utf8(const wchar_t *cszWC, int nWC, char *szUTF8, int nUTF8Len);
int		wh_utf82char(const char *cszUTF8, int nUTF8Len, char *szC, int nC);
int		wh_char2utf8(const char *cszC, int nC, char *szUTF8, int nUTF8Len);
int		wh_utf82multichar(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC);
const char *	wh_utf82multichar0(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC);
int		wh_multichar2utf8(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len);
const char *	wh_multichar2utf80(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len);
#define	WH_MULTICHAR2UTF80(cszCodePage, cszC, szUTF8)	wh_multichar2utf80(cszCodePage, cszC, strlen(cszC), szUTF8, sizeof(szUTF8)-1)
#define	WH_UTF82MULTICHAR0(cszCodePage, cszCUTF8, szC)	wh_utf82multichar0(cszCodePage, cszCUTF8, strlen(cszCUTF8), szC, sizeof(szC)-1)
// 安全的转换，如果失败返回""，而不是NULL
const char *	wh_utf82multichar0_safe(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC);
const char *	wh_multichar2utf80_safe(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len);
//修改以下两个宏,不然在strlen(xxx)==sizeof(xxx)-1的时候结果字符串不以'\0'结尾,modified by yuezhongyue, 2010-12-31
#define	WH_MULTICHAR2UTF80_safe(cszCodePage, cszC, szUTF8)	(szUTF8[sizeof(szUTF8)-1]=0,wh_multichar2utf80_safe(cszCodePage, cszC, strlen(cszC), szUTF8, sizeof(szUTF8)-1))
#define	WH_UTF82MULTICHAR0_safe(cszCodePage, cszCUTF8, szC)	(szC[sizeof(szC)-1]=0,wh_utf82multichar0_safe(cszCodePage, cszCUTF8, strlen(cszCUTF8), szC, sizeof(szC)-1))

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTRING_H__
