// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstring.h
// Creator      : Wei Hua (κ��)
// Comment      : �ִ�����ࡣ�磺�����ִ�����
//              : wh_isstrtoolong����������ִ��Ƿ�̫���ĺ�������Ҫ��������ź�
//              : ��whstrinvector4hash�мǲ�Ҫput(const char *)�����Ĳ���!!!!
// CreationDate : 2003-05-16
// ChangeLog    : 2004-03-16 wh_strsplit������""�������пհ��ַ�
//                2004-05-20 ������wh_strlike(const char *cszPattern, const char *cszStr);
//                2004-07-26 ������whstrlike�����Լ���ָ���
//                2004-08-13 wh_strsplittoarray�޸��˶�ǰ��հ��ַ��Ĵ���
//                2004-12-13 splitϵ�к����޸�Ϊֻ��""�������пհ��ַ�������ָ���Ϊ"\t"����ĳ��'\t'��ָ�һ��
//                2006-03-07 ������wh_strsplit��int64������I��wh_strsetvalue�����˶�int64��֧�֡�
//                2006-05-30 ������$(XX)������
//                2006-07-12 ȡ����whstr4hash��whstrptr4hash�е�m_bIC����������Ҫʹ�ô�Сд�����е�ֱ��ʹ��xxxNocase����
//                           _whstr_hashfuncbase��ȡ����ֱ��ʹ��_whstr_hashfunc��_whstr_hashfuncNocase����
//                2006-07-25 ������wh_strsplittovector����
//                2006-08-08 ������ȡ��whstrinvector4hash�е�ʹ��const char *�Ĺ��캯�����������Խ�ֹ��put��ʱ��ֱ��ʹ��const char *��������ݲ�һ�£�����һ��hashӦ�ô��������str������ʽ��whstrinvector��
//                           ����������ȴ�����find��erase���ֶ�����whstrinvector4hash���鷳�����Ի��Ƿ����ˣ�����Ҫ�мǲ�Ҫput(const char *)�����Ĳ���!!!!
//                2006-12-18 ������utf8��صĺ�������ԭ��wchar_tת����صĺ����������˳��Ȳ��������Ұ�nWC��������ֽ�����Ϊwchar_t�ĸ�����
//                2006-12-20 ��wh_char2wchar��wh_wchar2char�ڲ������Զ����0��β�Ĺ��ܡ�
//                2006-12-21 ������WH_WCHAR2MULTICHAR0 �� WH_MULTICHAR2WCHAR0 ������
//                2007-03-14 ��ԭ����str4hash�ȶ�����ִ��Ƚ���ģ��º�����д
//                2007-04-28 ������strupr��strlwr��Src/Dst��ʽ
//                2007-10-25 �����˲�����������wh_parse_opt
//                2008-09-09 ������whstr_Check_MultiChar

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

// ֮���Է�����������Ϊ����stricmp�Ⱦ���ȫ�ֺ���
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
// WHLINEEND����������linux��windows�µ��ı����з���ͳһ�������ʹ��\r\n����windows����notepad���������ı�����д����ִ���(�д���)
#ifdef	__GNUC__
#define	WHLINEEND	"\r\n"
#else
#define	WHLINEEND	"\n"
#endif

#define	WHLINEENDLEN	(sizeof(WHLINEEND)-1)

using std::string;

namespace n_whcmn
{

// �հ��ַ�
extern const char *WHSTR_WHITESPACE;

// ���ִ����Сд
// �����ִ�����
int	wh_strlwr(const char *cszSrc, char *szDst);
inline int	wh_strlwr(char *szStr)
{
	return	wh_strlwr(szStr, szStr);
}
// ���ִ���ɴ�д
// �����ִ�����
int	wh_strupr(const char *cszSrc, char *szDst);
inline int	wh_strupr(char *szStr)
{
	return	wh_strupr(szStr, szStr);
}
// �����ȼ���
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

// �Ƚ��ִ��Ƿ���ִ��������κ�һ����ͬ���оͷ������>=0�����򷵻�-1
// apszStr����NULL��Ϊ���Ľ�β��
int	wh_strcmparray(const char *apszStr[], const char *pszOther);
int	wh_stricmparray(const char *apszStr[], const char *pszOther);

////////////////////////////////////////////////////////////////////
// �䳤�ִ�ջ
////////////////////////////////////////////////////////////////////
class	whstrstack
{
protected:
	whvector<char>	m_strbuf;					// ���ڴ���ִ�
	whvector<int>	m_idxbuf;					// ���ڴ�Ÿ����ִ�����ʼ������
	char			m_sep;						// �ָ�����m_strbuf�еĸ����ִ���Ա��ͨ�����ַ�����
												// ��ʵ�����ĳ��Ծ���Ϊ�ļ�·����һЩ������Ƶ�
public:
	whstrstack(char sep);
	~whstrstack();
	void	clear();							// ��գ������ͷ��ڴ�
	void	destroy();							// �ͷ������ڴ�
	const char *	push(const char *szStr);	// ѹ��һ���ִ��������ڲ���ָ�룩
	char *	pop(int *pnIdx=NULL);				// ����һ���ִ�(����NULL��ʾû����)����ָ�����´�pushǰ��Ч��pnIdx�д����Ӧ����ʵ��λ�á�
	int		reinitfrom(const char *szStr, const char *seps);
												// ��һ���ִ���ʼ����ջ��seps���е�Ԫ�ض������Ƿָ���
public:
	// ��Ϣ����
	inline void		setsep(char sep)			// ���÷ָ���
	{
		m_sep	= sep;
	}
	inline size_t	size() const				// ����Ŀǰ�洢��Ԫ�ظ���
	{
		return	m_idxbuf.size();
	}
	inline const char *	getwholestr() const		// ���ջ�������ִ���β��Ӳ��зָ������ִ�
	{
		return	m_strbuf.getbuf();
	}
	inline char *	operator[] (int idx) const	// ������ջ���Ⱥ�˳��Ϊ�±�˳��
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
// �ȳ����ִ�����
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
	// ʹ�÷�Ĭ�Ϲ��캯����ʼ���˾Ͳ���init��
	whstrarr(int nTotal, int nUnitLen);					// ����ͬinit
	whstrarr(const char **aszList, int nUnitLen);		// ֱ�Ӵ�һ��NULL��β�������г�ʼ��
														// nUnitLen��ʾ������ô��������������ִ��������ó�����
	~whstrarr();
	int		init(int nTotal, int nUnitLen);
	int		release();
	int		clearstr(int nIdx);
	int		strcpy(int nIdx, const char *pszStr);		// ����pszStr��nIdx��Ԫ��
	int		findstr(const char *pszStr, bool bIC);		// ����pszStr���������������Ҳ�������-1��
														// bIC��ʾ���Դ�Сд

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
// �ȳ���bin�ִ�����
////////////////////////////////////////////////////////////////////
class	whbstrarr	: public whstrarr
{
public:
	int		strcpy(int nIdx, char *pszStr);
};

////////////////////////////////////////////////////////////////////
// �䳤�ĳ�ʼ���󲻻�ɾ�����ִ�����
////////////////////////////////////////////////////////////////////
class	whvarstrarr
{
protected:
	whvector<char>	m_bufs;						// �洢�����ִ�
	whvector<int>	m_offsets;					// �洢�����ִ�����ʼƫ����
public:
	void	reserve(int nMaxSize, int nMaxNum);	// Ԥ������ܳ��Ⱥ������Ŀ
	int		add(const char *cszStr);			// ���һ���ִ������������
	const char *	get(int nIdx) const;		// ��ȡָ���±���ִ�

	inline const char *	safeget(int nIdx) const	// ��֤�������
	{
		if( nIdx<0 || nIdx>=(int)m_offsets.size() )
		{
			return	NULL;
		}
		return	get(nIdx);
	}
	inline const char *	safegetlast() const		// ��֤������ģ���ȡ���һ��
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
	inline size_t	availnum() const	// �����пռ��ϻ����Է���ĸ���
	{
		return	m_offsets.capacity() - m_offsets.size();
	}
	inline void		clear()
	{
		m_offsets.clear();
		m_bufs.clear();
	}

	// ���й��ܺ���
	// �ж��ִ����Ƿ������еĴʻ㣬������֮(Ŀǰ������Ч��2004-02-10)
	const char *	hasstrin(const char *cszStr);
};

////////////////////////////////////////////////////////////////////
// �ж�һ���ִ��Ƿ������ƥ��
// ���磺%��%��%��% ���ͱ�ʾ�������ƹ������ľ��ӡ�
////////////////////////////////////////////////////////////////////
// ��Ϊ�ڲ��ķ������ܶ�䣬��������ʹ�ýӿ�
class	whstrlike
{
public:
	char		SEP;		// ��������(SEPSEP=�ַ�SEP)
	static char	DFTSEP;		// Ĭ�ϵģ�'%'
	static whstrlike *		Create();
	whstrlike()	: SEP('%')	{}
	virtual ~whstrlike()	{}
public:
	// ����ƥ��ģʽ
	virtual bool	SetPattern(const char *cszPattern)		= 0;
	// ����ģ�����ҵļ�϶����%���Դ������󳤶ȡ�Ĭ��Ϊ0�������޳���
	virtual bool	SetMaxClearance(int nClearance)			= 0;
	// �ж��Ƿ�ƥ��
	virtual bool	IsLike(const char *cszStr)				= 0;
};

////////////////////////////////////////////////////////////////////
// �����ִ�ƥ��(����һ���ִ��Ƿ�Ͷ����ѡ�е�����һ��ƥ��)
////////////////////////////////////////////////////////////////////
class	whstrmatch
{
protected:
	struct	UNIT_T
	{
		// ���ڵ���ַ�
		char	c;
		bool	islast;							// �Ƿ������һ���ַ�
		// �±�<0��ʾ�Ƿ�����û��ָ��Ľڵ�
		int		next;							// ��һ�����нڵ���±�
		int		firstchild;						// ��һ�����ӵ��±�
		int		ext;							// ��������
		void	reset(char _c)
		{
			c			= _c;
			islast		= false;
			next		= -1;
			firstchild	= -1;
			ext			= 0;
		}
	};
	whvector<UNIT_T>	m_vectUnit;				// ��ʼ��ʱ�ͻᱣ��һ����Ϊ�����ڵ�
	bool				m_bWholeMatch;			// �Ƿ�Ҫȫ��ƥ��(��hasstrinʹ�õ�)
	bool				m_bLikeMatch;			// �Ƿ���Ҫģ��ƥ��
	whvector<whstrlike *>	m_vectLike;			// �����Ҫģ��ƥ�䣬�����ƥ�������������
public:
	inline void	reserve(int nNum)
	{
		m_vectUnit.reserve(nNum);
		m_vectLike.reserve(nNum);
	}
	inline int	size() const					// ���ڵ�Ԫ������
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
	whstrmatch(int nReserveNum = 1024);			// �������������һ�����ȵ��ڴ�(nReserveNum��ʾ�ִ�����)
	~whstrmatch();
	// �����Ƿ���Ҫģ��ƥ��
	void	setlikecheck(bool bSet);

	// ���ԭ�����б�
	void	clear();

	// ����һ��ƥ�䴮
	// ����
	enum
	{
		ADDSTR_RST_OK		= 0,				// �ɹ�
		ADDSTR_RST_DUP		= 1,				// ���ظ���
		ADDSTR_RST_ERR		= -1,				// ����
	};
	// nExt��Ҫ��Ϊ�˽�����ӳ���õ�
	int		addstr(const char *cszWord, int nLen, int nExt);
	inline int	addstr(const char *cszWord, int nExt)
	{
		return	addstr(cszWord, strlen(cszWord), nExt);
	}
	// ����ƥ�䣬�ж����ִ��Ƿ���
	// ����У���pnPos�з���ƥ�����ʼλ�ã�pnLen��Ϊƥ�䳤��
	bool	hasstrin(const char *cszStr, int nLen, int *pnPos = NULL, int *pnLen = NULL, int *pnExt=NULL);
	inline bool	hasstrin(const char *cszStr, int *pnPos = NULL, int *pnLen = NULL, int *pnExt=NULL)
	{
		return	hasstrin(cszStr, strlen(cszStr), pnPos, pnLen, pnExt);
	}
	// ���й��ܺ���
protected:
	// ��nStartIdx��ʼ��ͬ�������ַ�
	// ����ҵ�������λ�á�
	// ���򷵻�<0
	int		findchar_in_chlid(int nIdx, char c, UNIT_T **ppUnit=NULL);
	// ���ַ�����nStartIdx��ͬ����
	// ���ز���λ��������������<0��
	int		insertchar_in_chlid(int nIdx, char c);
	// ������ŷ���ָ��
	UNIT_T *	getptr(int nIdx);
};

////////////////////////////////////////////////////////////////////
// �ִ���ǰ��һ���ֽڱ�ʾ�ִ�����(���Ҫ���ˣ����Ҫ�ӳ������дһ��whbstr2)
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
// �����ַ��ж�
////////////////////////////////////////////////////////////////////
// �ж�������ַ��Ƿ��Ǳ��ض��ֽ��ַ�������������ֽڣ�
// ������򷵻��ַ��ĳ���-1�����򷵻�0
int	whstr_Check_MultiChar(const char *szC);

// ����strstr���ʺ��ڲ����ظ����ִ����ң�
char *	wh_strstr(const char *pStr, const char *pSubStr);
// �������ĵ��ִ�����
char *	wh_strstr_cn(const char *pStr, const char *pSubStr);

////////////////////////////////////////////////////////////////////
// �ִ��� whstrstream
// Ҳ�����������ƶ�/д��
// ���д������setbin���õĳ��ȣ��򳤶Ȼ���չ�������ϲ�Ҫô�����㹻�Ļ��壬����ʹ�ò�����չѡ�
////////////////////////////////////////////////////////////////////
class	whstrstream
{
protected:
	// ָ���ִ������ָ��
	char		*m_pszStr;
	// �ִ�����(��������'\0')
	int			m_nSize;
	// ��ǰ��ȡ��λ��
	int			m_nOffset;
	// ���ڴ洢getchnc�Ľ��
	char		m_szCHNC[3];
	// �Ƿ���Ҫthrow (���Ϊ�棬����read����ʱ�׳��ִ��쳣)
	bool		m_bNeedThrowErrStr;
	// ��дʱ������չ
	bool		m_bNoExtendOnWrite;
public:
	whstrstream();
	// �����ִ�(�����þͲ��ܽ��ж�ȡ����)
	// ע�⣬�����ʹ��strlen������ȡ�ִ����ȣ�����ֻ���������ִ����ݺ����ʹ��setstr������õ����ִ����Ȳ���!!!!
	void	setstr(const char *pszStr);
	// ���ö���������
	void	setbin(const void *pszBin, int nSize);
	// ��ʼ��ͷ��
	void	rewind();
	// �ƶ�ָ��
	// nOriginʹ�ú�fseek����һ����SEEK_SET��SEEK_CUR��SET_END
	int		seek(int nOffset, int nOrigin);
	// ��ȡָ���ֽڵ����ݣ����صõ����ֽ���
	// pBuf����Ϊ�գ�Ϊ����������ô���ֽڣ��൱��seekcur
	int		read(void *pBuf, int nSize);
	// ��ȡһ���ַ�
	char	getc();
	// ��ȡһ�����ļ����ַ�������������ľͷ������ֽ����ģ�
	const char *	getchnc(int *pnLen);
	// �̶����͵Ļ�ȡ(���صõ����ֽ���)
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
	// ��ȡ����(����Ԫ�ظ�����ע�⣺������bytes)
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
	// ��ȡ�䳤�ִ�������Ϊ��ͨ�ִ�(�����ִ�����--strlen)
	// szStr����Ϊ�գ�����������ִ�
	int		readvstrasstr(char *szStr);

	// д������
	// ��!!!!ע�⣬д������ǰ���Ǳ�֤setbin��buffer�㹻�������Խ���ǲ��Ǳ����!!!!��
	int		write(const void *pBuf, int nSize);
	// д�̶����ͱ���
	template<typename _Ty>
	inline int	write(_Ty *pVar)
	{
		return	write(pVar, sizeof(_Ty));
	}
	// д������(����д���Ԫ�ظ�����ע�⣺������bytes)
	template<typename _Ty>
	inline int	writearray(_Ty *pArr, int nNum)
	{
		return	write(pArr, sizeof(_Ty)*nNum) / sizeof(_Ty);
	}
	// ����ͨ�ִ�����vstrд
	int		writestrasvstr(const char *cszStr);
	// ��ӡ�ִ�
	int	sprintf(const char *cszFmt, ...);
	// ���ò�����չ
	inline void	setnoextendonwrite(bool bSet)
	{
		m_bNoExtendOnWrite	= bSet;
	}
public:
	// ����ִ�����
	inline int		getlen() const
	{
		return	m_nSize;
	}
	// �Ƿ��˽�β(End Of Stream)
	inline bool		iseos() const
	{
		return	m_nOffset == m_nSize;
	}
	// ��õ�ǰ�����ָ��λ��
	inline char *	getcurptr()
	{
		return	(char*)(m_pszStr+m_nOffset);
	}
	// ��õ�ǰ��/д����ƫ��
	inline int		getoffset() const
	{
		return	m_nOffset;
	}
	// ����throw
	inline void		setneedthrowerrstr(bool bSet)
	{
		m_bNeedThrowErrStr	= bSet;
	}
};

////////////////////////////////////////////////////////////////////
// �ִ��Ƚ���
////////////////////////////////////////////////////////////////////
// ���ִ�Сд
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
// ����hash����ִ�ָ�����
////////////////////////////////////////////////////////////////////
// һ��Ӧ��ʹ���������Ϊ����Сд����
// ��ס�����ֻ����ָ�룬ָ����뱣֤���ܱ䣬����ָ��ֲ�����
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
				return	false;	// �и��ǿ�
		else if( other )
			return	false;		// �и��ǿ�
		else
			return	true;		// �������ǿ�
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
				return	false;	// ���еķǿ��ִ������ڿ�
		else if( other )
			return	true;		// ��С�����зǿ��ִ�
		else
			return	false;		// ������ȣ����ǿ�
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

// �����ִ�Сд
// ��vector�е��ִ�(������whstrptrxxx��_StrCmp�����Ƿ��Сд����)
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

// ��ָ��
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
				return	false;	// �и��ǿ�
		else if( other )
			return	false;		// �и��ǿ�
		else
			return	true;		// �������ǿ�
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
				return	false;				// ���еķǿ��ִ������ڿ�
		else if( other )
			return	true;					// ��С�����зǿ��ִ�
		else
			return	false;					// ������ȣ����ǿ�
	}
	inline const char * GetPtr() const
	{
		return	m_vectStr.getbuf();
	}
	inline operator const char * () const
	{
		return	GetPtr();
	}
	inline int	GetLen() const				// ����������\0
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
				//Сд���д
				c	-= ('a'-'A');
			}
			h = h * 31 + c;
		}
		return h;
	}
};


// �䳤���ݶ�д
// ��
// �������ݳ���
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
// �������ݲ�����ʼ��ַ(pszDst)�����ݲ��ֳ���
template<typename _LenTy>
int		whvldata_read_ref(const _LenTy *cszSrc, const void *&pszDst)
{
	_LenTy	nLen	= *cszSrc;
	cszSrc			++;
	pszDst			= cszSrc;
	return	(int)nLen;
}
// д
// �������ݳ���
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
// �ִ�����
////////////////////////////////////////////////////////////////////
// ���¼����������ؽ���ִ��ĳ���
// �����ִ���β�İ�����szKick����ַ�
int		wh_strtrimc(char *szStr, const char *szKick);
// �����ִ���β�Ŀո��س�
int		wh_strtrim(char *szStr);
// ȥ���ִ�β�İ�����szKick����ַ�
int		wh_strendtrimc(char *szStr, const char *szKick);
// ȥ���ִ�β��\��/
int		wh_strkickendslash(char *szStr);

// ��֤�ִ�ĩβ��ĳ���ַ�
int		wh_strsurelast(char *__str, char __c);
// ��֤��β��"/"
int		wh_strsureendslash(char *__str);

// һ����˵__countһ������__dst����󳤶ȼ�һ�����Կ���������ĩβ���һ��0
// ������˵���ٶ�char szDst[32]��ʹ�÷�ʽΪwh_strncpy0(szDst, src, sizeof(szDst)-1);
char *	wh_strncpy0(char *__dst, const char *__src, size_t __count);
char *	wh_strncat0(char *__dst, const char *__tocat, size_t __count);
// DST�����ǿ���sizeof���ִ����������ִ�ָ��
#define	WH_STRNCPY0(DST, SRC)	wh_strncpy0(DST, SRC, sizeof(DST)-1)
#define	WH_STRNCAT0(DST, TOCAT)	wh_strncat0(DST, TOCAT, sizeof(DST)-1)
#define	WH_MEMCPY(DST, SRC)		memcpy(DST, SRC, sizeof(DST))
#define WH_CLEAR_STRNCPY0(DST, SRC) (memset(DST, 0, sizeof(DST)),wh_strncpy0(DST, SRC, sizeof(DST)-1))		// ���������,Ȼ���ٿ���,web��չ��Ҫ,added by yuezhongyue 2011-12-18
// ��֤STR�����һλ��0����ð����ִ�����ʱ��Խ��
#define	WH_STRSURELAST0(STR)	STR[sizeof(STR)-1] = 0;

// �ж�һ���ִ��Ƿ����
// �������ʾ�ִ�������(�������һ��'\0')������__lenlimit
bool	wh_isstrtoolong(const char *__str, int __lenlimit);

// ��һ���ִ����һ����׺�������ܳ��Ȳ��ܳ���һ��ֵ�����nIdx>0���������ֵ��������
// nMaxSize��ʾ�������һ��0�ĳ���
const char *	wh_appendext(char *szTarget, int nMaxSize, const char *cszExt, int nIdx);

// �ж�һ���ִ����Ƿ���ĳ����Χ��[nChar1,nChar2]���ַ������ص�һ���������������ַ������û�з���0��
char	wh_checkcharrange(const char *cszStr, int nChar1, int nChar2);

// ����__splitter�ָ���������ر��ָ�Ĳ�������(������*��Ҳ���ܹ��ҵ��ķֶ�������)
// __splitter���Ϊ""���ʾ�ָ���Ϊ���еĿ��ַ���
// __splitter�����һ���ַ�Ϊ'\1'���ʾʹ�ú�������е��ַ���Ϊ�ָ�����
// ����__splitter������Ϊ�ָ�����
// ��һ����ʽ�ִ���ʾ��������������
// ���û�ж�����ֵ����
// **���õĸ�ʽ**�У�
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
// ���أ�
// �ֽ⵽�Ĳ���������ע�⣺"*"��Ҳ��һ����
// ע��֮����д������������Ϊû�취���ñ�ε��ñ��
int		wh_strsplit(const char *__format, const char *__src, const char *__splitter, ...);
int		wh_strsplit(int *pnOffset, const char *__format, const char *__src, const char *__splitter, ...);

// ���������ִ�
char *	wh_strcat(char *__dst, const char *__str1, const char *__str2);

// ���ӵ�����
typedef	void*	voidptr;
typedef	char*	charptr;

void	wh_strsetvalue_charptr(charptr pValue, int nMaxSize, const char *cszVal);	// �ִ����������һЩ
void	wh_strsetvalue(charptr pValue, const char *cszVal);							// �����Ҫ��array��ص������Ͳ��жϳ�����
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

// whstr2int���ת�����ɹ��򷵻�0
int	whstr2int(const char *cszVal);
const char *	whint2str(int nVal, char *pszVal=NULL);

whint64	whstr2int64(const char *cszVal);
const char *	whint642str(whint64 nVal, char *pszVal=NULL);

bool	whstr2bool(const char *cszVal);
const char *	whbool2str(bool bVal, char *pszVal=NULL);

// �жϵ���spliter��λ��
inline bool	_wh_isspliter(const char *__str, const char *__splitter, int __slen)
{
	switch( __splitter[0] )
	{
		case	0:
		// ���__spliter����Ϊ0������Ϊ���пհ��ַ�����spliter
		return	strchr(WHSTR_WHITESPACE, __str[0]) != NULL;
		case	1:
		// ��˵���ǰ��յ��ַ����Ƚϵ�
		return	strchr(__splitter+1, __str[0]) != NULL;
		default:
		// ��������������Ƚϣ������ִ���Ϊ�ָ�����
		return	strncmp(__str, __splitter, __slen) == 0;
	}
}

// ���ִ��ֽ�Ϊͬ���͵�����
// ���õĸ�ʽ������"**���õĸ�ʽ**"

// ע�⣬���_Ty��char *����ô__arrҪôֱ����char x[n][m]�����ͣ�Ҫô��char *x[n]����
// �����x[n][m]ǿ��ת��Ϊvoid *��ת��Ϊchar**�����Ƿ�����
template<typename _Ty>
int		wh_strsplittoarray(const char *__src, const char *__splitter, _Ty *__arr, int __num)
{
	char	/**buf = NULL, ����ԭ��buf������ʲô��*/ bufi[128];
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
			// ԭ���ǣ�i += sl; �����ڷָ�������ʱ�����
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
			i	++;			// ���ִ���
		}
	}
	// ���һ������(����bWordBegin�������β�Ŀհ�)
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

	char	/**buf = NULL, ����ԭ��buf������ʲô��*/ bufi[128];
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
			// ԭ���ǣ�i += sl; �����ڷָ�������ʱ�����
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
			i	++;			// ���ִ���
		}
	}
	// ���һ������(����bWordBegin�������β�Ŀհ�)
	if( bWordBegin && start<l )
	{
		wh_strncpy0(bufi, __src+start, i-start);
		wh_strtrim(bufi);
		wh_strsetvalue((*__vect.push_back()), bufi);
	}

	return	__vect.size();
}

// ���ִ����ҳ���ĳЩ�ַ���ס���Ӵ�
// ����ҵ��򷵻�__substr�����򷵻�NULL
char *	wh_findquoted(const char *__src, char __left, char __right, char *__substr);
// ���ܺ��������ƣ��������޸�__src�����ݣ���__left��__right��λ����0
char *	wh_findquotedandmodify(char *__src, char __left, char __right, char **__ptr_right);

// ��һ��ͨ���ִ��ֽ�Ϊ���ɲ���
// __src���ֽ���ִ�
// __sep������п��ܵķָ�������������__sep�е��ַ����Ƿָ�����
// __dst������ֽⲿ����'\0'�ָ��ʵ�壬�䳤�Ȳ�������__src���ϲ�Ӧ�ñ�֤
// __ptr��������Ÿ������ֵ�ͷָ���ָ������
// *__numԭ����__ptr�������ɵ�Ԫ����������󷵻�ʵ�ʵ�Ԫ�ظ���
int		wh_strexplode(const char *__src, const char *__sep, char *__dst, char *__ptr[], int *__num);
// �����ɷֽ���ִ���ϳ�һ���ִ�
// __ptr��������Ÿ������ֵ�ͷָ���ָ������
// __numԭ����__ptr���ɵ�Ԫ������
// __sep���ڷָ������ִ����������ִ�(�����explode�еĺ��岻һ������ע��!!)
// __dst��������ɵ��ִ����䳤�����ϲ㱣֤�㹻
int		wh_strimplode(char *__ptr[], int __num, const char *__sep, char *__dst);

// ע�⣺��������wh_strchrreplace�У����__toΪ0��ʾ���滻��ֻ�����ִ���__from�ĸ���
// �滻�ִ��е�ĳ���ַ�����һ���ַ������ر��滻�ĸ�����
int		wh_strchrreplace(char *__str, char __from, char __to);
// __strlen��ʾ����滻�ĳ���(���Ҳ���ر��滻�ĸ���)
int		wh_strchrreplace(char *__str, int __strlen, char __from, char __to);

// �����ִ��е��κκ�ָ���ִ����κ��ַ�ƥ���λ��
// ���ؽ��Ϊƥ��λ��
// __what�з���ƥ�䵽���ַ�
const char *	wh_strchr(const char *__str, const char *__matches, char &__what);
const char *	wh_strrchr(const char *__str, const char *__matches, char &__what);

// �ҵ��ִ��е�N�����ַ�ƥ���λ��
const char *	wh_the_n_strchr(const char *__str, char __c, int __n);

// ���ִ��е�����ĳ���ִ��滻Ϊ��һ���ִ�
// nMaxLenΪ�Ǹ��ִ�������ǵ��ĳ��ȣ��������һ��0��
int		wh_strstrreplace(char *__str, int nMaxLen, const char *__from, const char *__to);

// ��һ���ִ�__src��������ĳ���ַ����κ�__from�е��ַ������滻Ϊһ���ַ�__to��__dst��ȥ����__dst�ĳ��Ȳ��ܳ���nMaxLen
int		wh_strcontinuouspatternreplace(char *__dst, int nMaxLen, const char *__src, const char *__from, char __to);

// �����ִ��е�$(XX)�ִ���������Ӧ���滻����ͨ��__cb���У�
// __cb�ǻص�������__param�ǻص��Ĳ���
enum
{
	WH_STR_MP_TYPE_CMN		= 0,		// ��ͨ�ִ�
	WH_STR_MP_TYPE_MACRO	= 1,		// ���ִ�
	WH_STR_MP_TYPE_END		= 2,		// ����
};
int		wh_strmacroprocess(const char *__src, int (*__cb)(int __type, const char *__str, int __len, void *__param), void *__param);

// ����һ���ı�������ΪKey=Value����ʽ
enum
{
	WHSTR_TYPE_STRING					= 1,
	WHSTR_TYPE_INT						= 2,
	WHSTR_TYPE_FLOAT					= 3,
	WHSTR_TYPE_SHORT					= 4,
	WHSTR_TYPE_BOOL						= 5,
	WHSTR_TYPE_TIME						= 6,
	WHSTR_TYPE_INTARRAY					= 11,		// ע������array���ÿո�ָ��
	WHSTR_TYPE_FLOATARRAY				= 12,
	WHSTR_TYPE_EXTFUNC					= 100,
	WHSTR_TYPE_EXTFUNCPARSEONLY			= 101,		// ֻ��������
	WHSTR_TYPE_INT64					= 102,		// INT64
};
enum
{
	WHSTR_ACT_PARSE						= 0,
	WHSTR_ACT_PRINT						= 1,
	WHSTR_ACT_SAVE						= 2,
};
typedef void (*whstr_ext_func_t)(const char *szStr, void *pValue, int nAct);	// nAct��ʾ�ǽ������Ǵ�ӡ
struct	WH_LINE_PARSE_INFO_T
{
	const char	*cszKey;
	void	*pValue;
	int		nType;						// WHSTR_TYPE_XXX
	int		nExt;						// �����array�����Ԫ����ĿΪnExt��Ԫ�ؼ��Կո�Ϊ�ָ���
										// �������Ҫ���ӽ��������ģ�nExt�Ǻ���ָ��
};
// �����parse�ȽϹ�ʱ�ˣ����ֻ�Ƕ����ݽ����ʹ��whdataini.h���ṩ�Ĺ���!!!!
// ���ǣ����д���ݣ����ǿ���ʹ�õ�wh_parsesave���ܵģ����д�Ƚϼ򵥡�
// �ֽ�һ��
int		wh_strlineparse(const char *szLine, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// �ֽ�һ���ļ�
int		wh_strfileparse(const char *szFile, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// �ֽ��ļ���һ��sect
int		wh_strfilesectparse(const char *szFile, const char *szSectName, const char *szSep, WH_LINE_PARSE_INFO_T *pInfo, bool bIC=true);
// ��ӡ��ֵ�Ͷ�Ӧ����
int		wh_parseprint(WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);		// nTab��ʾ"="���ڵ�λ��
int		wh_parsesave(const char *szFile, WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);
int		wh_parsesave(FILE *fp, WH_LINE_PARSE_INFO_T *pInfo, int nTab = 16);
char *	wh_genspace(char *szDst, const char *szStr, int nTab = 16);		// �������ո�(szDst��szStr������һ��)
// �ִ�ת��Ϊ����(Ԫ���Կո�ָ�)
// ��������ߴ�
int		wh_str2array(int nType, const char *szStr, void *pArr, int nNum);

// ��һ���ִ�ָ������ת��Ϊһ�����ִ��������ִ��ܳ��ȣ�������nMaxSize��
int		wh_array2str(int argc, char *argv[], char *szStr, int nMaxSize);
// �����ִ��еĲ����Σ�-xxx xxx����ʽ�����صõ���opt�ĸ�����
// nSize����Ҫ�������ִ��ĳ���
// vectTmp������ṩ����ʱ���壬���ڰ�szStr�е�����ÿ�κ��涼����0��β��vectOpt�����ָ�����ָ��vectTmp�ڲ���ƫ�Ƶģ�
// vectOpt�������մ�ŵ���ÿ�������ε����ݣ���ָ�룩������ż��λ��Ϊ������������λ��Ϊ����ֵ
int		wh_parse_opt(const char *szStr, int nSize, whvector<char> &vectTmp, whvector<const char *>&vectOpt, char *pszErr);

// ��������������ǰӦ�ñ�֤��������srand
// ��������ִ�(ע�⣺���Զ���szStr������һ��'\0')������Ҫ��֤szStr��������nLen+1��byte
int		wh_randstr(char *szStr, int nLen, bool bAutoAddLast0=true);
// ������������ƴ�
int		wh_randhex(void *szStr, int nLen);

// 16����������ִ�(Ĭ����Сд)
const char *	wh_hex2str(const unsigned char *szHex, int nLen, char *szStr=NULL, int nSpace=0, bool bUPCASE=false);
// �ִ����16������(nSize=0��ʾ�����ִ��ĳ�������>0��ʾ���ɵ�16���ƴ�����󳤶�)
// �������ɵ�16�������ݵĳ���
// ע���ִ���ÿ��byte֮�䲻Ҫ�пո��磺023f3d5e
//     ��Ϊ�����ܳ����СszHex��szStr������һ��ָ��
int				wh_str2hex(const char *szStr, unsigned char *szHex, size_t nSize=0);

// ����һ���ִ���whvector<char>
inline void	wh_copystr2vector(whvector<char> &vect, const char *cszStr)
{
	vect.resize(strlen(cszStr)+1);
	memcpy(vect.getbuf(), cszStr, vect.size());
}

// �ж��ִ��Ƿ��Ǵ�������ɵ�
// nMaxLen��ʾ�ִ��ĳ��Ȳ��ܳ���ĳ��ֵ������Ҳ���Ǵ������۴���nMaxLen���Ϊ0���ʾ���ƽϳ��ȣ�
bool	wh_isstrnumeric(const char *cszStr, int nMaxLen=0);

// ��ӡ�ַ���
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

// ���cszKey��NULL�ҷǿմ�(0�����ִ�)������Ҷ�Ӧ��value�������ӡȫ����
// cszDft��ʾ���ʲôҲû���ҵ����ӡ����ִ�
void	wh_strkeyprint(const char *astr[][2], const char *cszKey=NULL, const char *cszDft=NULL, bool bIC=true);

// �ж���ĸ�Ƿ���16�����е�����0~9 A~F a~f
bool	wh_charishexdigit(char c);
// ���ִ��е�%XX�滻Ϊ��Ӧ��ASCII��ֵ
// ����ת��֮����ִ�����
int		wh_change_percentsymbol(const char *cszSrc, char *pszDst);

#ifdef	WIN32
#define	snprintf	_snprintf
#endif
// ��WCHAR��صģ�����Ĭ�ϱ����ַ�������GBK��
// �����ĸ�ת������������ת��֮����ִ��ĳ��ȣ�ע�⣬��һ����nC��nWC�����ȣ�
// ��WCHARת��Ϊchar�����szCΪ����ֱ�ӽ�ת������ִ��浽ԭ����szWC��ȥ��
// ��nWC��ʾszWC��wchar_t�ĸ���!!!! nC��ʾszC��char�ĸ�����
// WH_WCHAR2MULTICHAR0 �� WH_MULTICHAR2WCHAR0 ���������ڽ�һ��ת������һ�����̶����ȣ���������һ����ָ�루ע��Ŷ��0ֻ�Ǵ����Ǹ��Ƚϵײ�ĺ���������˵����0��0�Ƕ��Զ��ӵģ�
int		wh_wchar2char(char *szWC);
int		wh_wchar2char(const wchar_t *cszWC, int nWC, char *szC, int nC);
int		wh_wchar2multichar(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
const char *	wh_wchar2multichar0(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
#define	WH_WCHAR2MULTICHAR0(cszCodePage, cszWC, szC)	wh_wchar2multichar0(cszCodePage, cszWC, wcslen(cszWC), szC, sizeof(szC)-1)
// ��ȫ��ת�������ʧ�ܷ���""��������NULL
const char *	wh_wchar2multichar0_safe(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC);
#define	WH_WCHAR2MULTICHAR0_safe(cszCodePage, cszWC, szC)	wh_wchar2multichar0_safe(cszCodePage, cszWC, wcslen(cszWC), szC, sizeof(szC)-1)

// ��charת��ΪWCHAR
// ����ʵ��ת����wchar�ĸ�������������β��0����Ȼ0���Զ���ӵģ�
int		wh_char2wchar(char *szC);
int		wh_char2wchar(const char *cszC, int nC, wchar_t *szWC, int nWC);
int		wh_multichar2wchar(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
const wchar_t *	wh_multichar2wchar0(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
#define	WH_MULTICHAR2WCHAR0(cszCodePage, cszC, szWC)	wh_multichar2wchar0(cszCodePage, cszC, strlen(cszC), szWC, sizeof(szWC)/sizeof(szWC[0])-1)
// ��ȫ��ת�������ʧ�ܷ���""��������NULL
const wchar_t *	wh_multichar2wchar0_safe(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC);
#define	WH_MULTICHAR2WCHAR0_safe(cszCodePage, cszC, szWC)	wh_multichar2wchar0_safe(cszCodePage, cszC, strlen(cszC), szWC, sizeof(szWC)/sizeof(szWC[0])-1)


// UTF8���������ת����������ת��֮����ִ����ȣ�
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
// ��ȫ��ת�������ʧ�ܷ���""��������NULL
const char *	wh_utf82multichar0_safe(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC);
const char *	wh_multichar2utf80_safe(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len);
//�޸�����������,��Ȼ��strlen(xxx)==sizeof(xxx)-1��ʱ�����ַ�������'\0'��β,modified by yuezhongyue, 2010-12-31
#define	WH_MULTICHAR2UTF80_safe(cszCodePage, cszC, szUTF8)	(szUTF8[sizeof(szUTF8)-1]=0,wh_multichar2utf80_safe(cszCodePage, cszC, strlen(cszC), szUTF8, sizeof(szUTF8)-1))
#define	WH_UTF82MULTICHAR0_safe(cszCodePage, cszCUTF8, szC)	(szC[sizeof(szC)-1]=0,wh_utf82multichar0_safe(cszCodePage, cszCUTF8, strlen(cszCUTF8), szC, sizeof(szC)-1))

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTRING_H__
