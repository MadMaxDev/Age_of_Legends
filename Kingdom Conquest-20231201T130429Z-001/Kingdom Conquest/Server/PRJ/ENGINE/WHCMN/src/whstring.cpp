// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstring.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 字串相关类。如：大型字串数组
// CreationDate : 2003-05-16
// ChangeLOG    : 2004-05-26 纠正了原来whstrmatch没有析构，而导致内存泄漏的错误(没有删除list中的new出来的指针)。
//                2004-06-15 修正了wh_strsplit对连续分隔符下读不出空字串的bug。原来比如1,,234会被分割为1、,234、空串
//                2004-07-08 增加了whstrlike对只有前后有通配符的按普通方式处理
//                2004-07-31 修正了用空白做分隔符时连接的空白被认为是多个分隔符而导致的取得数据为0的错误
//                2004-08-25 修正了wh_strsplit(int *pnOffset, ... ...)中处理最后一个参数时增加offset的错误
//                2005-05-30 修正了wh_strsetvalue(bool...)中清除前面空字符时使用绝对下标同时又指针加加的错误。
//                2005-10-01 在_wh_isspliter中增加了如果第一个字符等于1则
//                2005-12-07 修正了wh_strchrreplace中没有增加i的错误
//                2006-04-24 增加了wh_strsplit对不认识的格式报assert的能力
//                2006-05-31 端午节哦^O^。增加了字串解析中简单的整数的顺序四则运算功能（由后向前的顺序）。
//                2007-01-17 让整数四则运算支持了优先级（乘除高于加减）
//                2007-01-23 把wh_strchr中abMatchChar[(int)*__str]改为abMatchChar[(unsigned char)*__str]
//                2007-03-02 修正了wh_strcontinuouspatternreplace中没有把重复串替换为__to的bug（原来的结果是替换为重复串的第一个字符）
//                2007-04-30 修正了wh_strlwr等函数中在判断字符不属于大写范围时没有拷贝字符的错误

#include "../inc/whstring.h"
#include "../inc/whtime.h"
#include "../inc/wh_platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#ifdef	__GNUC__
#include <iconv.h>
#endif

namespace n_whcmn
{

// 空白字符
const char	*WHSTR_WHITESPACE	= " \t\r\n";

int	wh_atoi(const char *cszVal)
{
	int		i=0;
	sscanf(cszVal, "%u", &i);
	return	i;
}

int	whstr2int(const char *cszVal)
{
	// 支持顺序四则运算吧
	char	c;
	// 优先找加减
	const char	*pPos	= wh_strrchr(cszVal, "+-", c);
	if( pPos )
	{
		char	i1[1024];
		char	i2[1024];
		int		nLen = pPos - cszVal;
		memcpy(i1, cszVal, nLen);
		i1[nLen]	= 0;
		strcpy(i2, pPos+1);
		switch( c )
		{
		case	'+':
			return	whstr2int(i1) + whstr2int(i2);
		case	'-':
			return	whstr2int(i1) - whstr2int(i2);
		default:
			assert(0);
			return	0;
		}
	}
	// 然后找乘除
	pPos	= wh_strrchr(cszVal, "*/", c);
	if( pPos )
	{
		char	i1[1024];
		char	i2[1024];
		int		nLen = pPos - cszVal;
		memcpy(i1, cszVal, nLen);
		i1[nLen]	= 0;
		strcpy(i2, pPos+1);
		switch( c )
		{
		case	'*':
			return	whstr2int(i1) * whstr2int(i2);
		case	'/':
			{
				int	nDivisor	= whstr2int(i2);	// 解决除0的问题
				if( nDivisor==0 )
				{
					return	0;
				}
				return	whstr2int(i1) / nDivisor;
			}
		default:
			assert(0);
			return	0;
		}
	}

	// 消除前面的空字符
	bool	bStop = false;
	while( !bStop )
	{
		switch(*cszVal)
		{
			case	' ':
			case	'\t':
			case	'\r':
			case	'\n':
				// 跳过
				cszVal	++;
			break;
			case	0:
				return	0;
			break;
			default:
				bStop	= true;
			break;
		}
	}
	if( cszVal[0]=='0' )
	{
		switch(cszVal[1])
		{
			case	'x':
			case	'X':
				int		nRst;
				sscanf(cszVal+2, "%x", &nRst);
				return	nRst;
			break;
		}
	}
	// 原来直接用atoi在整数大于0x7FFFFFFF时会返回0x7FFFFFFF
	return	wh_atoi(cszVal);
}
const char *	whint2str(int nVal, char *pszVal)
{
	if( pszVal==NULL )
	{
		static char	szVal[64];
		pszVal	= szVal;
	}
	sprintf(pszVal, "%d", nVal);
	return	pszVal;
}

whint64	whstr2int64(const char *cszVal)
{
	// 支持顺序四则运算吧
	char	c;
	const char	*pPos	= wh_strrchr(cszVal, "+-", c);
	if( pPos )
	{
		char	i1[1024];
		char	i2[1024];
		int		nLen = pPos - cszVal;
		memcpy(i1, cszVal, nLen);
		i1[nLen]	= 0;
		strcpy(i2, pPos+1);
		switch( c )
		{
		case	'+':
			return	whstr2int64(i1) + whstr2int64(i2);
		case	'-':
			return	whstr2int64(i1) - whstr2int64(i2);
		default:
			assert(0);
			return	0;
		}
	}
	// 然后找乘除
	pPos	= wh_strrchr(cszVal, "*/", c);
	if( pPos )
	{
		char	i1[1024];
		char	i2[1024];
		int		nLen = pPos - cszVal;
		memcpy(i1, cszVal, nLen);
		i1[nLen]	= 0;
		strcpy(i2, pPos+1);
		switch( c )
		{
		case	'*':
			return	whstr2int64(i1) * whstr2int64(i2);
		case	'/':
			{
				whint64	nDivisor	= whstr2int64(i2);	// 解决除0的问题
				if( nDivisor==0 )
				{
					return	0;
				}
				return	whstr2int64(i1) / nDivisor;
			}
		default:
			assert(0);
			return	0;
		}
	}

	// 消除前面的空字符
	bool	bStop = false;
	while( !bStop )
	{
		switch(*cszVal)
		{
		case	' ':
		case	'\t':
		case	'\r':
		case	'\n':
			// 跳过
			cszVal	++;
			break;
		case	0:
			return	0;
			break;
		default:
			bStop	= true;
			break;
		}
	}
	if( cszVal[0]=='0' )
	{
		switch(cszVal[1])
		{
		case	'x':
		case	'X':
			whint64	nRst;
			sscanf(cszVal+2, "%"WHINT64PRFX"x", &nRst);
			return	nRst;
			break;
		}
	}
#ifdef	WIN32
	return	_atoi64(cszVal);
#endif
#ifdef	__GNUC__
	return	atoll(cszVal);
#endif
}
const char *	whint642str(whint64 nVal, char *pszVal)
{
	if( pszVal==NULL )
	{
		static char	szVal[64];
		pszVal	= szVal;
	}
	sprintf(pszVal, "0x%"WHINT64PRFX"X", nVal);
	return	pszVal;
}

bool	whstr2bool(const char *cszVal)
{
	bool	bval = false;
	// 保证前面没有空字符
	for(int i=0;i<64 && cszVal;i++)
	{
		// 原来是这么写的 :( if( !strchr(WHSTR_WHITESPACE, cszVal[i]) )
		if( !strchr(WHSTR_WHITESPACE, *cszVal) )
		{
			break;
		}
		cszVal	++;
	}
	if( strnicmp(cszVal, "true", 4)==0
		||  cszVal[0] == '1'
		||  cszVal[0] == 'Y'
		||  cszVal[0] == 'y'
		)
	{
		bval	= true;
	}
	return	bval;
}
const char *	whbool2str(bool bVal, char *pszVal)
{
	if( pszVal==NULL )
	{
		static char	szVal[64];
		pszVal	= szVal;
	}
	if( bVal )
	{
		strcpy(pszVal, "true");
	}
	else
	{
		strcpy(pszVal, "false");
	}
	return	pszVal;
}

int	wh_strlwr(const char *cszSrc, char *szDst)
{
	int		i=0;
	if( cszSrc==szDst )
	{
		while(cszSrc[i])
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				i	++;
			}
			else
			{
				if( cszSrc[i]>='A' && cszSrc[i]<='Z' )
				{
					szDst[i]	= cszSrc[i]+('a'-'A');
				}
			}
			i	++;
		}
	}
	else
	{
		while(cszSrc[i])
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				szDst[i]	= cszSrc[i];
				i	++;
				szDst[i]	= cszSrc[i];
			}
			else
			{
				if( cszSrc[i]>='A' && cszSrc[i]<='Z' )
				{
					szDst[i]	= cszSrc[i]+('a'-'A');
				}
				else
				{
					szDst[i]	= cszSrc[i];
				}
			}
			i	++;
		}
		szDst[i]	= 0;
	}
	return	i;
}
int	wh_strupr(const char *cszSrc, char *szDst)
{
	int		i=0;
	if( cszSrc==szDst )
	{
		while(cszSrc[i])
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				i	++;
			}
			else
			{
				if( cszSrc[i]>='a' && cszSrc[i]<='z' )
				{
					szDst[i]	= cszSrc[i]+('A'-'a');
				}
			}
			i	++;
		}
	}
	else
	{
		while(cszSrc[i])
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				szDst[i]	= cszSrc[i];
				i	++;
				szDst[i]	= cszSrc[i];
			}
			else
			{
				if( cszSrc[i]>='a' && cszSrc[i]<='z' )
				{
					szDst[i]	= cszSrc[i]+('A'-'a');
				}
				else
				{
					szDst[i]	= cszSrc[i];
				}
			}
			i	++;
		}
		szDst[i]	= 0;
	}
	return	i;
}
int	wh_strlwr(const char *cszSrc, char *szDst, int nMaxSize)
{
	int		i=0;
	if( cszSrc==szDst )
	{
		while(cszSrc[i] && nMaxSize>0)
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				i	++;
				nMaxSize	--;
			}
			else
			{
				if( cszSrc[i]>='A' && cszSrc[i]<='Z' )
				{
					szDst[i]	= cszSrc[i]+('a'-'A');
				}
			}
			i	++;
			nMaxSize	--;
		}
	}
	else
	{
		while(cszSrc[i])
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				szDst[i]	= cszSrc[i];
				i	++;
				nMaxSize	--;
				szDst[i]	= cszSrc[i];
			}
			else
			{
				if( cszSrc[i]>='A' && cszSrc[i]<='Z' )
				{
					szDst[i]	= cszSrc[i]+('a'-'A');
				}
				else
				{
					szDst[i]	= cszSrc[i];
				}
			}
			i	++;
			nMaxSize	--;
		}
		szDst[i]	= 0;
	}
	return	i;
}
int	wh_strupr(const char *cszSrc, char *szDst, int nMaxSize)
{
	int		i=0;
	if( cszSrc==szDst )
	{
		while(cszSrc[i] && nMaxSize>0)
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				i	++;
				nMaxSize	--;
			}
			else
			{
				if( cszSrc[i]>='a' && cszSrc[i]<='z' )
				{
					szDst[i]	= cszSrc[i]+('A'-'a');
				}
			}
			i	++;
			nMaxSize	--;
		}
	}
	else
	{
		while(cszSrc[i] && nMaxSize>0)
		{
			if( whstr_Check_MultiChar(&cszSrc[i]) )
			{
				// 跳过这个字符
				szDst[i]	= cszSrc[i];
				i	++;
				nMaxSize	--;
				szDst[i]	= cszSrc[i];
			}
			else
			{
				if( cszSrc[i]>='a' && cszSrc[i]<='z' )
				{
					szDst[i]	= cszSrc[i]+('A'-'a');
				}
				else
				{
					szDst[i]	= cszSrc[i];
				}
			}
			i	++;
			nMaxSize	--;
		}
		szDst[i]	= 0;
	}
	return	i;
}

char *	wh_strstr_cn(const char *pStr, const char *pSubStr)
{
	int		nSublen	= strlen(pSubStr);
	while( pStr[0] != 0 )
	{
		// 看是否匹配
		if( memcmp(pStr, pSubStr, nSublen)==0 )
		{
			// 匹配了
			return	(char *)pStr;
		}
		// 判断是不是中文
		if( whstr_Check_MultiChar(pStr) )
		{
			pStr	+= 2;
		}
		else
		{
			pStr	++;
		}
	}
	return	NULL;
}

int	wh_strcmparray(const char *apszStr[], const char *pszOther)
{
	int	i=0;
	while( apszStr[i]!=NULL )
	{
		if( strcmp(apszStr[i], pszOther)==0 )
		{
			return	i;
		}
		i++;
	}
	return	-1;
}
int	wh_stricmparray(const char *apszStr[], const char *pszOther)
{
	int	i=0;
	while( apszStr[i]!=NULL )
	{
		if( stricmp(apszStr[i], pszOther)==0 )
		{
			return	i;
		}
		i++;
	}
	return	-1;
}

////////////////////////////////////////////////////////////////////
// whstrarr
////////////////////////////////////////////////////////////////////
whstrarr::whstrarr()
: m_pBuf(NULL), m_ppBuf(NULL)
, m_nTotal(0)
, m_nUnitLen(0)
{
}
whstrarr::whstrarr(int nTotal, int nUnitLen)
{
	init(nTotal, nUnitLen);
}
whstrarr::whstrarr(const char **aszList, int nUnitLen)
{
	m_pBuf		= NULL;
	m_ppBuf		= NULL;
	m_nTotal	= 0;
	m_nUnitLen	= 0;
	if( !aszList )
	{
		return;
	}
	// 先找有多少个串
	int	nCount = 0;
	const char **ppList = aszList;
	while(*ppList)
	{
		int	nLen = strlen(*ppList)+1;
		if( nLen > nUnitLen )
		{
			// 更新为最大长度
			nUnitLen	= nLen;
		}
		nCount	++;
		ppList	++;
	}
	// 之所以不取字串中的最大值，是因为今后可能还要插入别的长度的串
	// 初始化
	init(nCount, nUnitLen);
	// 填写串
	int	i;
	for(i=0;i<nCount;i++)
	{
		::strcpy(m_ppBuf[i], aszList[i]);
	}
}
whstrarr::~whstrarr()
{
	release();
}
int		whstrarr::init(int nTotal, int nUnitLen)
{
	int	nTotalBytes;
	nTotalBytes	= nTotal * nUnitLen;
	m_pBuf		= new char [nTotalBytes];
	if( !m_pBuf )
	{
		return	-1;
	}
	m_ppBuf		= new char* [nTotal];
	if( !m_ppBuf )
	{
		return	-1;
	}
	m_nTotal	= nTotal;
	m_nUnitLen	= nUnitLen;

	char	*pBuf = m_pBuf;
	for(int i=0;i<m_nTotal;i++)
	{
		m_ppBuf[i]	= pBuf;
		pBuf		+= m_nUnitLen;
	}
	return		0;
}
int		whstrarr::release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf	= 0;
	}
	if( m_ppBuf	)
	{
		delete []	m_ppBuf;
		m_ppBuf	= 0;
	}
	return	true;
}
int		whstrarr::clearstr(int nIdx)
{
	char	* pBuf;
	pBuf	= safegetunit(nIdx);
	if( pBuf )
	{
		pBuf[0]	= 0;
	}
	return	0;
}
int		whstrarr::strcpy(int nIdx, const char *pszStr)
{
	char	* pBuf;
	pBuf	= safegetunit(nIdx);
	if( !pBuf )
	{
		return	-1;
	}
	wh_strncpy0(pBuf, pszStr, m_nUnitLen-1);
	return	0;
}
int		whstrarr::findstr(const char *pszStr, bool bIC)
{
	int		i;
	if( bIC )
	{
		for(i=0;i<m_nTotal;i++)
		{
			if( stricmp(pszStr, m_ppBuf[i])==0 )
			{
				return	i;
			}
		}
	}
	else
	{
		for(i=0;i<m_nTotal;i++)
		{
			if( strcmp(pszStr, m_ppBuf[i])==0 )
			{
				return	i;
			}
		}
	}
	return	-1;
}

////////////////////////////////////////////////////////////////////
// 等长的b字串数组
////////////////////////////////////////////////////////////////////
int		whbstrarr::strcpy(int nIdx, char *pszStr)
{
	char	* pBuf;
	pBuf	= safegetunit(nIdx);
	if( !pBuf )
	{
		return	-1;
	}
	wh_strncpy0(pBuf, pszStr, m_nUnitLen-1);
	return	0;
}

////////////////////////////////////////////////////////////////////
// 变长字串栈
////////////////////////////////////////////////////////////////////
whstrstack::whstrstack(char sep)
: m_sep(sep)
{
}
whstrstack::~whstrstack()
{
	destroy();
}
void	whstrstack::clear()
{
	m_strbuf.clear();
	m_idxbuf.clear();
}
void	whstrstack::destroy()
{
	m_strbuf.destroy();
	m_idxbuf.destroy();
}
const char *	whstrstack::push(const char *szStr)
{
	int	nLen = strlen(szStr)+1;
	int	nOff = m_strbuf.size();
	m_strbuf.resize(nOff+nLen);
	if( nOff>0 )
	{
		// 如果不是第一个，则给上一个的末尾填上分隔符
		m_strbuf[nOff-1]	= m_sep;
	}
	// 填上本字串(含最后的\0)
	memcpy(m_strbuf.getptr(nOff), szStr, nLen);
	// 加入索引
	m_idxbuf.push_back(nOff);
	return	m_strbuf.getptr(nOff);
}
char *	whstrstack::pop(int *pnIdx)
{
	if( m_idxbuf.size()==0 )
	{
		return	NULL;
	}
	int		nOff  = m_idxbuf.getlast();
	char	*pStr = m_strbuf.getptr(nOff);		// 在这写是因为resize后再取就会报assert了
	m_strbuf.resize(nOff);
	if( m_strbuf.size() )						// 如果不判断这个
	{
		m_strbuf.getlast()	= 0;				// 字串结尾清零（把那个分隔符刚好替换为0了）
	}
	m_idxbuf.pop_back();
	if( pnIdx )
	{
		*pnIdx	= nOff;
	}
	return	pStr;
}
int		whstrstack::reinitfrom(const char *szStr, const char *seps)
{
	// 清空原来的数据
	clear();

	int		len;
	if( !szStr || (len=strlen(szStr))==0 )
	{
		// 空串怎么初始化啊
		return	0;
	}

	// 初始化到最大长度
	m_strbuf.resize(len+1);

	int		last = 0;
	int		i;
	for(i=0;i<len;i++)
	{
		char	c = szStr[i];
		if( strchr(seps, c) )
		{
			// 找到了一个分隔符
			c		= m_sep;
			// 添加一个索引
			m_idxbuf.push_back(last);
			last	= i+1;
		}
		// 本字符赋值
		m_strbuf[i]	= c;
	}
	// 添加最后一个索引
	m_idxbuf.push_back(last);
	// 最后一个\0
	m_strbuf[i]		= 0;

	return	0;
}

////////////////////////////////////////////////////////////////////
// whvarstrarr 变长的初始化后不会删减的字串数组
////////////////////////////////////////////////////////////////////
void	whvarstrarr::reserve(int nMaxSize, int nMaxNum)
{
	m_bufs.reserve(nMaxSize);
	m_offsets.reserve(nMaxNum);
}
int		whvarstrarr::add(const char *cszStr)
{
	m_offsets.push_back(m_bufs.size());
	m_bufs.pushn_back(cszStr, strlen(cszStr)+1);
	return		m_offsets.size()-1;
}
const char *	whvarstrarr::get(int nIdx) const
{
	return	m_bufs.getptr(m_offsets[nIdx]);
}
const char *	whvarstrarr::hasstrin(const char *cszStr)
{
	const char *	strCharSet;
	int				i = 0;
	while( (strCharSet=safeget(i++)) != NULL )
	{
		if( strstr(cszStr, strCharSet) )
		{
			break;
		}
	}
	return			strCharSet;
}

////////////////////////////////////////////////////////////////////
// 用于字串匹配
////////////////////////////////////////////////////////////////////
whstrmatch::whstrmatch(int nReserveNum)
: m_bWholeMatch(false)
, m_bLikeMatch(true)
{
	m_vectUnit.reserve(nReserveNum+1);
	m_vectLike.reserve(nReserveNum);
	clear();
}
whstrmatch::~whstrmatch()
{
	clear();
}
void	whstrmatch::setlikecheck(bool bSet)
{
	m_bLikeMatch	= bSet;
}
void	whstrmatch::clear()
{
	m_vectUnit.resize(1);
	m_vectUnit[0].reset(0);
	for(size_t i=0;i<m_vectLike.size();i++)
	{
		delete	m_vectLike[i];
	}
	m_vectLike.clear();
}
int		whstrmatch::addstr(const char *cszWord, int nLen, int nExt)
{
	if(nLen<=0)
	{
		assert(0);
		return	-1;
	}

	// 看看是否是模糊规则
	if( m_bLikeMatch )
	{
		if( nLen==1 && cszWord[0]==whstrlike::DFTSEP )
		{
			return	-1;
		}
		// 如果前后有但是中间没有就按照普通的处理
		if( cszWord[0]==whstrlike::DFTSEP
		&&  strchr(cszWord+1, whstrlike::DFTSEP) == (cszWord+nLen-1)
		)
		{
			// 用后面的正常方法处理啦
			cszWord	++;
			nLen	-= 2;
		}
		else if( strchr(cszWord, whstrlike::DFTSEP) )
		{
			whstrlike	*pLike	= whstrlike::Create();
			pLike->SetPattern(cszWord);
			m_vectLike.push_back(pLike);
			return	0;
		}
	}

	int	i;
	int	inspnt		= 0;
	int	idx			= 0;
	bool	bNew	= false;
	// 分级查找
	for(i=0;i<nLen;i++)
	{
		idx			= findchar_in_chlid(idx, cszWord[i]);
		if( idx<0 )
		{
			// 说明是新的
			bNew	= true;
			break;
		}
		inspnt		= idx;
	}
	if( !bNew )
	{
		// 判断最后一个是否是last
		UNIT_T	*pUnit	= m_vectUnit.getptr(inspnt);
		if( pUnit && !pUnit->islast )
		{
			// 那么就说明原来加了一个比较长的
			pUnit->islast	= true;
			return	ADDSTR_RST_OK;
		}
		// 原来都有了
		return		ADDSTR_RST_DUP;
	}
	// 当前的i表示第一个未匹配字符的位置。则，从它开始插入。
	for(;i<nLen;i++)
	{
		inspnt	= insertchar_in_chlid(inspnt, cszWord[i]);
	}
	// 设置最后一个字符为结束属性
	m_vectUnit.getlast().islast	= true;
	return	ADDSTR_RST_OK;
}
bool	whstrmatch::hasstrin(const char *cszStr, int nLen, int *pnPos, int *pnLen, int *pnExt)
{
	int		i;
	int		i0			= 0;
	int		startidx	= -1;
	int		idx			= 0;
	bool	bFound		= false;

again:
	for(i=i0;i<nLen;i++)
	{
		UNIT_T		*pUnit;
		idx			= findchar_in_chlid(idx, cszStr[i], &pUnit);
		if( idx<0 )
		{
			// 如果需要完全匹配则可以直接返回了
			if( m_bWholeMatch )
			{
				bFound	= false;
				goto	End;
			}
			// 没有完全匹配
			i0		++;
			idx		= 0;
			// 如果开头是中文开头则跳过下一个
			if( i0<nLen
			&&  whstr_Check_MultiChar(cszStr+i0-1)
			)
			{
				i0	++;
			}
			goto	again;
		}
		else if( i==i0 )
		{
			// 这是第一个匹配点，记录下开始位置
			startidx	= i;
		}
		if( pUnit->islast )
		{
			if( m_bWholeMatch )
			{
				// 如果匹配到结尾了就成功，否则继续
				if( i == nLen-1 )
				{
					if( pnExt )
					{
						*pnExt	= pUnit->ext;
					}
					bFound	= true;
					break;
				}
			}
			else
			{
				bFound	= true;
				break;
			}
		}
	}
	if( pnPos )
	{
		*pnPos		= startidx;
	}
	if( pnLen )
	{
		*pnLen		= i-i0+1;
	}
End:
	if( !bFound )
	{
		if( m_bLikeMatch )
		{
			// 判断是否和模糊查询配合
			for(size_t i=0;i<m_vectLike.size();i++)
			{
				if( m_vectLike[i]->IsLike(cszStr) )
				{
					if(pnPos)	*pnPos	= 0;
					if(pnLen)	*pnLen	= 0;
					if(pnExt)	*pnExt	= 0;
					return	true;
				}
			}
		}
	}
	return			bFound;
}
int		whstrmatch::findchar_in_chlid(int nIdx, char c, UNIT_T **ppUnit)
{
	UNIT_T *pUnit;
	// 起始点必定是存在的
	pUnit			= getptr(nIdx);
	assert( pUnit );
	nIdx			= pUnit->firstchild;
	pUnit			= getptr(nIdx);
	while( pUnit )
	{
		if( pUnit->c == c )
		{
			break;
		}
		nIdx		= pUnit->next;
		pUnit		= getptr(nIdx);
	}
	if( ppUnit )
	{
		*ppUnit		= pUnit;
	}
	return	nIdx;
}
int		whstrmatch::insertchar_in_chlid(int nIdx, char c)
{
	int		nNewIdx	= m_vectUnit.size();

	UNIT_T	*pUnit	= m_vectUnit.push_back();
	pUnit->reset(c);

	UNIT_T	*pFatherUnit	= m_vectUnit.getptr(nIdx);

	pUnit->next		= pFatherUnit->firstchild;
	pFatherUnit->firstchild	= nNewIdx;

	return	nNewIdx;
}
whstrmatch::UNIT_T *	whstrmatch::getptr(int nIdx)
{
	if( nIdx<0 || nIdx>=((int)m_vectUnit.size()) )
	{
		// 下标超界了
		return	NULL;
	}
	return	m_vectUnit.getptr(nIdx);
}

////////////////////////////////////////////////////////////////////
// whstrlike 判断一个字串是否和描述匹配
////////////////////////////////////////////////////////////////////
class	whstrlike_i	: public whstrlike
{
protected:
	struct	PATTERN_T
	{
		enum
		{
			TYPE_BEGIN	= 0,			// 需要从头匹配子串
			TYPE_SEARCH	= 1,			// 需要从后续串中查找子串
			TYPE_END	= 2,			// 匹配到结尾的符合字串
		};
		unsigned char	nType;			// 类型
		unsigned char	nLen;			// 子串长度
		const char		*pStr;			// 子串首指针(如果是TYPE_END类型则为NULL)
	};
	whvector<char>		m_PatternStr;
	whvector<PATTERN_T>	m_PatternArray;
	int					m_nMaxClearance;
public:
	whstrlike_i();
	virtual bool	SetPattern(const char *cszPattern);
	virtual bool	SetMaxClearance(int nClearance);
	virtual bool	IsLike(const char *cszStr);
};
char	whstrlike::DFTSEP	= '%';
whstrlike *		whstrlike::Create()
{
	return	new whstrlike_i;
}

whstrlike_i::whstrlike_i()
: m_nMaxClearance(0)
{
}
bool	whstrlike_i::SetPattern(const char *cszPattern)
{
	// 将pattern保存以便修改
	int		nLen	= strlen(cszPattern);
	m_PatternStr.resize(nLen+1);
	char	*pStr	= m_PatternStr.getbuf();
	memcpy(pStr, cszPattern, m_PatternStr.size());

	// 清空原来的模式
	m_PatternArray.clear();

	int			i;
	PATTERN_T	*pPattern = NULL;
	for(i=0;i<nLen;i++)
	{
		bool	bIsSEP	= false;
		if( pStr[i]==SEP )
		{
			bIsSEP		= true;
			if( pStr[i+1]==SEP )
			{
				// 表示一个SEP，当作一个字符，后面的向前移动
				--	nLen;
				memmove(pStr+i+1, pStr+i+2, nLen-i);
				bIsSEP	= false;
			}
			else
			{
				// 旧的结束
				if( pPattern )
				{
					if( pPattern->pStr )
					{
						pPattern->nLen	= pStr+i - pPattern->pStr;
					}
				}
				pStr[i]		= 0;
				// 新的开始
				pPattern	= m_PatternArray.push_back();
				// 这是一个新单元的开始
				if( pStr[i+1]==0 )
				{
					// 到结尾的任何字符都匹配
					pPattern->nType		= PATTERN_T::TYPE_END;
					pPattern->pStr		= NULL;
					pPattern->nLen		= 0;
				}
				else
				{
					// 一个串
					pPattern->nType		= PATTERN_T::TYPE_SEARCH;
					pPattern->pStr		= pStr + i+1;
					++		i;
				}
			}
		}
		if( !bIsSEP )
		{
			if( i==0 )
			{
				// 这是第一个，要求从头匹配的
				pPattern		= m_PatternArray.push_back();
				pPattern->nType	= PATTERN_T::TYPE_BEGIN;
				pPattern->pStr	= pStr;
			}
			else
			{
				// 这是某个连续串的中间部分
			}
		}
	}
	// 如果结尾不是%，则
	if( pPattern->pStr )
	{
		pPattern->nLen	= pStr+nLen - pPattern->pStr;
	}

	return	true;
}
bool	whstrlike_i::SetMaxClearance(int nClearance)
{
	m_nMaxClearance	= nClearance;
	return	true;
}
bool	whstrlike_i::IsLike(const char *cszStr)
{
	const char	*cszStr0	= cszStr;
	PATTERN_T	*pPattern	= NULL;

	for(size_t i=0;i<m_PatternArray.size();i++)
	{
		pPattern	= m_PatternArray.getptr(i);
		switch( pPattern->nType )
		{
			case	PATTERN_T::TYPE_BEGIN:
			{
				if( memcmp(cszStr, pPattern->pStr, pPattern->nLen)!=0 )
				{
					return	false;
				}
				cszStr		+= pPattern->nLen;
			}
			break;
			case	PATTERN_T::TYPE_SEARCH:
			{
				// 考虑中文的字串查找
				char	*pStrMatch	= wh_strstr_cn(cszStr, pPattern->pStr);
				if( !pStrMatch )
				{
					return	false;
				}
				if( m_nMaxClearance>0 )
				{
					if( pStrMatch-cszStr > m_nMaxClearance )
					{
						// 间隙过大，不用认为是匹配的
						return	false;
					}
				}
				cszStr	= pStrMatch + pPattern->nLen;
			}
			break;
			case	PATTERN_T::TYPE_END:
			{
				// 直接匹配到结尾了
				return	true;
			}
			break;
		}
	}
	// 最后一个匹配如果是非END的，则看是否匹配到了结尾
	if( pPattern )
	{
		if( pPattern->pStr )
		{
			// cszStr此时应该是最后一个匹配串的下一个字符
			int		nLenLeft	= strlen(cszStr0) - (cszStr - cszStr0);
			if( nLenLeft!=0 )
			{
				// 没有完全匹配
				return	false;
			}
		}
	}
	// 没有出错，则匹配
	return	true;
}

////////////////////////////////////////////////////////////////////
// 字串流 whstrstream
////////////////////////////////////////////////////////////////////
whstrstream::whstrstream()
: m_pszStr(NULL)
, m_nSize(0)
, m_nOffset(0)
, m_bNeedThrowErrStr(false)
, m_bNoExtendOnWrite(false)
{
}
// 设置字串(不设置就不能进行读取操作)
void	whstrstream::setstr(const char *pszStr)
{
	setbin(pszStr, strlen(pszStr));
}
void	whstrstream::setbin(const void *pszBin, int nSize)
{
	m_pszStr	= (char *)pszBin;
	m_nSize		= nSize;
	rewind();
}
// 开始从头读
void	whstrstream::rewind()
{
	m_nOffset	= 0;
}
int		whstrstream::seek(int nOffset, int nOrigin)
{
	int			nNewOffset	= m_nOffset;
	switch( nOrigin )
	{
		case	SEEK_SET:
			nNewOffset		= nOffset;
		break;
		case	SEEK_CUR:
			nNewOffset		+= nOffset;
		break;
		case	SEEK_END:
			nNewOffset		= m_nSize+nOffset;
		break;
	}
	if( nNewOffset<0 )
	{
		nNewOffset	= 0;
	}
	m_nOffset		= nNewOffset;
	return	0;
}
// 获取指定字节的数据
int		whstrstream::read(void *pBuf, int nSize)
{
	int	nDiff = m_nSize - m_nOffset;
	if( m_bNeedThrowErrStr )
	{
		if( nDiff<=0 )
		{
			throw	"EOF";
		}
	}
	if( nSize>nDiff )
	{
		nSize	= nDiff;
	}
	if( nSize>0 )
	{
		if( pBuf )
		{
			memcpy(pBuf, m_pszStr+m_nOffset, nSize);
		}
		m_nOffset	+= nSize;
	}
	return	nSize;
}
// 获取一个字符
char	whstrstream::getc()
{
	if( iseos() )
	{
		return	0;
	}
	return	m_pszStr[m_nOffset++];
}
// 获取变长字串
int		whstrstream::readvstrasstr(char *szStr)
{
	// 先获取长度
	unsigned char	nLen;
	if( read(&nLen)>0 )
	{
		read(szStr, nLen);
		if( szStr )
		{
			szStr[nLen]	= 0;
		}
		return	nLen;
	}
	else
	{
		// 表示没有读到
		return	0;
	}
}

// 获取一个中文兼容字符（即如果是中文就返回两字节中文）
const char *	whstrstream::getchnc(int *pnLen)
{
	if( iseos() )
	{
		return	NULL;
	}
	// 判断当前字符是否是中文
	m_szCHNC[0]	= m_pszStr[m_nOffset];
	m_nOffset	++;
	*pnLen		= 1;
	if( whstr_Check_MultiChar(m_szCHNC) )
	{
		// 是中文开头，就多读一个
		if( !iseos() )
		{
			m_szCHNC[1]	= m_pszStr[m_nOffset];
			(*pnLen)	++;
			m_nOffset	++;
		}
	}
	m_szCHNC[*pnLen]	= 0;
	return	m_szCHNC;
}

int		whstrstream::write(const void *pBuf, int nSize)
{
	int	nTotal = m_nOffset + nSize;
	if( nTotal>m_nSize )
	{
		if( m_bNoExtendOnWrite )
		{
			// 不可扩展
			return	-1;
		}
		m_nSize	= nTotal;
	}
	if( pBuf )
	{
		memcpy(m_pszStr+m_nOffset, pBuf, nSize);
	}
	m_nOffset	+= nSize;
	assert(m_nOffset<m_nSize);
	return	nSize;
}
int		whstrstream::writestrasvstr(const char *cszStr)
{
	// 先获取长度
	unsigned char	nLen = strlen(cszStr);
	// 写长度
	int	rst = 0;
	rst	+= write(&nLen);
	// 写字串
	rst	+= write(cszStr, nLen);
	return	rst;
}
int		whstrstream::sprintf(const char *cszFmt, ...)
{
	va_list	arglist;
	va_start(arglist, cszFmt);
	vsprintf(m_pszStr+m_nOffset, cszFmt, arglist);
	va_end(arglist);
	int	nSize	= strlen(m_pszStr+m_nOffset);
	m_nOffset	+= nSize;
	assert(m_nOffset<m_nSize);
	return		nSize;
}

////////////////////////////////////////////////////////////////////
// 字串处理
////////////////////////////////////////////////////////////////////
// 除掉字串首尾的包含在szKick里的字符
int		wh_strtrimc(char *szStr, const char *szKick)
{
	int		i, l, begin, end;
	l		= strlen(szStr);
	begin	= -1;
	for(i=0;i<l;i++)
	{
		if( !strchr(szKick, szStr[i]) )
		{
			begin	= i;
			break;
		}
	}
	if( begin<0 )
	{
		// 全部都需要trim
		szStr[0]	= 0;
		return		0;
	}
	end	= begin;							// 在下面的循环找不到非szKick的字符的情况下就是这个值
	for(i=l-1;i>begin;i--)
	{
		if( !strchr(szKick, szStr[i]) )
		{
			end		= i;
			break;
		}
	}
	l	= end - begin + 1;
	if( begin )
	{
		// 这样才需要移动
		memmove(szStr, szStr+begin, l);
	}
	szStr[l]	= 0;

	return		l;
}
// 除掉字串首尾的空格或回车
int		wh_strtrim(char *szStr)
{
	return	wh_strtrimc(szStr, WHSTR_WHITESPACE);
}
// 去除字串尾的包含在szKick里的字符
int		wh_strendtrimc(char *szStr, const char *szKick)
{
	int		i, l;
	l		= strlen(szStr);
	for(i=l-1;i>=0;i--)
	{
		if( !strchr(szKick, szStr[i]) )
		{
			szStr[i+1]	= 0;
			return	i+1;
		}
	}
	return	0;
}
// 去除字串尾的\或/
int		wh_strkickendslash(char *szStr)
{
	return	wh_strendtrimc(szStr, "\\/");
}
// 保证字串末尾有某个字符
int		wh_strsurelast(char *__str, char __c)
{
	int i = strlen(__str);
	if( __str[i-1]!=__c )
	{
		__str[i]	= __c;
		__str[i+1]	= 0;
	}
	return	0;
}
int		wh_strsureendslash(char *__str)
{
	return	wh_strsurelast(__str, '/');
}

char *	wh_strncpy0(char *__dst, const char *__src, size_t __count)
{
	int			i		= 0;
	const char	*pcszC	= __src;
	char		*pszDst	= __dst;
	while( (*pcszC)!=0 && i<(int)__count )
	{
		// 判断是否是宽字节
		if( whstr_Check_MultiChar(pcszC) )
		{
			if( i+2 > (int)__count )
			{
				break;
			}
			memcpy(pszDst, pcszC, 2);
			i		+= 2;
			pszDst	+= 2;
			pcszC	+= 2;
		}
		else
		{
			*pszDst	= *pcszC;
			i		++;
			pszDst	++;
			pcszC	++;
		}
	}
	// 最后一个字节
	*pszDst	= 0;
	return	__dst;
}
char *	wh_strncat0(char *__dst, const char *__tocat, size_t __count)
{
	strncat(__dst, __tocat, __count);
	__dst[__count]	= 0;
	return	__dst;
}

bool	wh_isstrtoolong(const char *__str, int __lenlimit)
{
	for(int i=0;i<__lenlimit;i++)
	{
		if( __str[i]==0 )
		{
			return	false;
		}
	}
	return	true;
}
const char *	wh_appendext(char *szTarget, int nMaxSize, const char *cszExt, int nIdx)
{
	int	nTargetLen	= strlen(szTarget);
	int	nExtLen		= strlen(cszExt);
	int	nDiff		= nTargetLen+nExtLen+1 - nMaxSize;
	if( nDiff>0 )
	{
		// 截断ext （因为原则是尽量不破还原来的名字）
		nExtLen		-= nDiff;
		if( nExtLen )
		{
			// 是否造成半个汉字
			if( whstr_Check_MultiChar(cszExt+nExtLen-1) )
			{
				nExtLen	--;
			}
		}
	}
	if( nExtLen>0 )
	{
		memcpy(szTarget+nTargetLen, cszExt, nExtLen);
		nTargetLen		+= nExtLen;
		*(szTarget+nTargetLen)	= 0;
	}
	if( nIdx>0 )
	{
		char	szNum[32];
		sprintf(szNum, "%d", nIdx);
		int	nNumLen	= strlen(szNum);
		nDiff		= nTargetLen+nNumLen+1 - nMaxSize;
		if( nDiff<0 )
		{
			nDiff	= 0;
		}
		// 判断是不是汉字	2005-04-05加
		if( whstr_Check_MultiChar(szTarget+nTargetLen-nDiff-1) )
		{
			szTarget[nTargetLen-nDiff-1]	= '0';
		}
		// 放在最紧靠后的地方
		memcpy(szTarget+nTargetLen-nDiff, szNum, nNumLen+1);
	}

	return		szTarget;
}
char	wh_checkcharrange(const char *cszStr, int nChar1, int nChar2)
{
	int i	= 0;
	while( cszStr[i] )
	{
		if( nChar1<=((int)cszStr[i]) && ((int)cszStr[i])<=nChar2 )
		{
			return	cszStr[i];
		}
	}
	return	0;
}

// 安全的strcpy，如果超过规定长度就不拷贝(返回NULL)
char *	wh_safestrcpy(char *__dst, int __size, const char *__src)
{
	int	len;
	if( (len=strlen(__src))>__size-1 )
	{
		return	NULL;
	}
	memcpy(__dst, __src, len);
	__dst[len]	= 0;

	return	__dst;
}
char *	wh_safestrcat(char *__dst, int __size, const char *__tocat)
{
	int	len0	= strlen(__dst);
	int	len1	= strlen(__tocat);
	int	len		= len0 + len1;
	if( len>__size-1 )
	{
		return	NULL;
	}
	memcpy(__dst+len0, __tocat, len1);
	__dst[len]	= 0;

	return	__dst;
}

int		wh_strsplit(const char *__format, const char *__src, const char *__splitter, ...)
{
	char	*buf = NULL, bufi[256];	// bufi是临时用的
	va_list	marker;
	int		count, start, i, sl, l, rcount, noarg = 0;
	int		__count;
	int		ignore;

	va_start( marker, __splitter );
	count	= 0;
	rcount	= 0;
	i		= 0;
	sl		= __splitter[0]>1 ? strlen(__splitter) : 1;
	start	= 0;
	__count	= strlen(__format);

	// 去掉结尾的空白
	l	= strlen(__src);
	i	= l-1;
	while( i>=0 && strchr(WHSTR_WHITESPACE, __src[i]) )
	{
		l	--;
		i	--;
	}

	bool	bWordBegin	= false;

	// find the next splitter
	for(i=0; i<l&&count<__count; i++)
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
			ignore		= 0;
			switch(__format[count])
			{
				case	's':
				case	'a':
					buf	= va_arg( marker, char*);
				break;
				case	'd':
					buf	= (char *)va_arg( marker, int* );
				break;
				case	'o':
					buf	= (char *)va_arg( marker, short* );
				break;
				case	'y':
					buf	= (char *)va_arg( marker, whbyte* );
				break;
				case	'f':
					buf	= (char *)va_arg( marker, float* );
				break;
				case	'b':
					buf	= (char *)va_arg( marker, bool* );
				break;
				case	't':
					buf	= (char *)va_arg( marker, time_t* );
				break;
				case	'I':
					buf	= (char *)va_arg( marker, whint64* );
				break;
				case	'*':
					// ignore
					ignore	= 1;
				break;
				default:
					// 不认识的格式
					assert(0);
					buf	= NULL;
				break;
			}
			if( !ignore )
			{
				if( !buf )
				{
					noarg	= 1;
					break;
				}

				int	ilen	= i-start;
				switch(__format[count])
				{
					case	'a':
						strcpy(buf, __src+start);
						wh_strtrim(buf);
						// make it quit from the for circle
						i	= l;
					break;
					case	's':
						wh_strncpy0(buf, __src+start, ilen);
						wh_strtrim(buf);
					break;
					case	'd':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(int *)buf, bufi);
					break;
					case	'o':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(short *)buf, bufi);
					break;
					case	'y':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(whbyte *)buf, bufi);
					break;
					case	'f':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(float *)buf, bufi);
					break;
					case	'b':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(bool *)buf, bufi);
					break;
					case	't':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(time_t *)buf, bufi);
					break;
					case	'I':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(whint64 *)buf, bufi);
					break;
					case	'*':
					break;
					default:
						// 不认识的格式
						assert(0);
					break;
				}
			}
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
			i	++;			// 本地字符字处理（这个加一个，循环里面还会再加一个）
		}
	}

	// 最后一个参数
	if( start<l && count<__count && !noarg )
	{
		buf	= NULL;
		switch(__format[count])
		{
			case	'a':
			case	's':
				buf	= va_arg( marker, char*);
			break;
			case	'd':
				buf	= (char *)va_arg( marker, int* );
			break;
			case	'o':
				buf	= (char *)va_arg( marker, short* );
			break;
			case	'y':
				buf	= (char *)va_arg( marker, whbyte* );
			break;
			case	'f':
				buf	= (char *)va_arg( marker, float* );
			break;
			case	'b':
				buf	= (char *)va_arg( marker, bool* );
			break;
			case	't':
				buf	= (char *)va_arg( marker, time_t* );
			break;
			case	'I':
				buf	= (char *)va_arg( marker, whint64* );
			break;
			case	'*':
			break;
			default:
				// 不认识的格式
				assert(0);
			break;
		}
		if( buf )
		{
			switch(__format[count])
			{
				case	'a':
				case	's':
					strcpy(buf, __src + start);
					wh_strtrim(buf);
				break;
				case	'd':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(int *)buf, bufi);
				break;
				case	'o':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(short *)buf, bufi);
				break;
				case	'y':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(whbyte *)buf, bufi);
				break;
				case	'f':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(float *)buf, bufi);
				break;
				case	'b':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(bool *)buf, bufi);
				break;
				case	't':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(time_t *)buf, bufi);
				break;
				case	'I':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(whint64 *)buf, bufi);
				break;
				case	'*':
				break;
				default:
					// 不认识的格式
					assert(0);
				break;
			}
		}
		else
		{
			noarg	= 1;
		}
		// 就算为"*"也要算count
		count	++;
	}
	rcount	= count;
	// 如果还有没有赋值的buf（保留原来的值更好些2003-07-15，所以后面&&0）
	while( count<__count && !noarg && 0 )
	{
		switch(__format[count])
		{
			case	'a':
			case	's':
				buf	= va_arg( marker, char*);
				if(buf)	strcpy(buf, "");
			break;
			case	'd':
				buf	= (char *)va_arg( marker, int* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'o':
				buf	= (char *)va_arg( marker, short* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'y':
				buf	= (char *)va_arg( marker, whbyte* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'f':
				buf	= (char *)va_arg( marker, float* );
				if(buf)	*(float *)buf = 0;
			break;
			case	'b':
				buf	= (char *)va_arg( marker, bool* );
				if(buf)	*(bool *)buf = 0;
			break;
			case	't':
				buf	= (char *)va_arg( marker, time_t* );
				if(buf)	*(time_t *)buf = 0;
			break;
			case	'I':
				buf	= (char *)va_arg( marker, whint64* );
				if(buf)	*(whint64 *)buf = 0;
			break;
			case	'*':
			break;
			default:
				// 不认识的格式
				assert(0);
			break;
		}
		count ++;
	};

	return	rcount;
}

int		wh_strsplit(int *pnOffset, const char *__format, const char *__src, const char *__splitter, ...)
{
	char	*buf = NULL, bufi[256];	// bufi是临时用的
	va_list	marker;
	int		count, start, i, sl, l, rcount, noarg = 0;
	int		__count;
	int		ignore;

	va_start( marker, __splitter );
	count	= 0;
	rcount	= 0;
	i		= 0;
	sl		= __splitter[0]>1 ? strlen(__splitter) : 1;
	start	= 0;
	__count	= strlen(__format);

	__src	+= (*pnOffset);

	// 去掉结尾的空白
	l	= strlen(__src);
	i	= l-1;
	while( i>=0 && strchr(WHSTR_WHITESPACE, __src[i]) )
	{
		l	--;
		i	--;
	}

	bool	bWordBegin	= false;

	// find the next splitter
	for(i=0; i<l&&count<__count; i++)
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
			ignore		= 0;
			switch(__format[count])
			{
				case	's':
				case	'a':
					buf	= va_arg( marker, char*);
				break;
				case	'd':
					buf	= (char *)va_arg( marker, int* );
				break;
				case	'o':
					buf	= (char *)va_arg( marker, short* );
				break;
				case	'y':
					buf	= (char *)va_arg( marker, whbyte* );
				break;
				case	'f':
					buf	= (char *)va_arg( marker, float* );
				break;
				case	'b':
					buf	= (char *)va_arg( marker, bool* );
				break;
				case	't':
					buf	= (char *)va_arg( marker, time_t* );
				break;
				case	'I':
					buf	= (char *)va_arg( marker, whint64* );
				break;
				case	'*':
					// ignore
					ignore	= 1;
				break;
				default:
					buf	= NULL;
					assert(0);
				break;
			}
			if( !ignore )
			{
				if( !buf )
				{
					noarg	= 1;
					break;
				}

				int	ilen	= i-start;
				switch(__format[count])
				{
					case	'a':
						strcpy(buf, __src+start);
						wh_strtrim(buf);
						// make it quit from the for circle
						i	= l;
					break;
					case	's':
						wh_strncpy0(buf, __src+start, ilen);
						wh_strtrim(buf);
					break;
					case	'd':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(int *)buf, bufi);
					break;
					case	'o':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(short *)buf, bufi);
					break;
					case	'y':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(whbyte *)buf, bufi);
					break;
					case	'f':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(float *)buf, bufi);
					break;
					case	'b':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(bool *)buf, bufi);
					break;
					case	't':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(time_t *)buf, bufi);
					break;
					case	'I':
						wh_strncpy0(bufi, __src+start, ilen);
						wh_strsetvalue(*(whint64 *)buf, bufi);
					break;
					case	'*':
					break;
					default:
						assert(0);
					break;
				}
			}
			// 原来是：i += sl;start = i; 这样在分隔符连续时会出错（因为）
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
			i	++;			// 本地字符字处理（这个加一个，循环里面还会再加一个）
		}
	}
	(*pnOffset)	+= start;

	// 最后一个参数
	if( start<l && count<__count && !noarg )
	{
		// 那么offset就可以指到字串结尾了
		(*pnOffset)	+= strlen(__src+start);

		buf	= NULL;
		switch(__format[count])
		{
			case	'a':
			case	's':
				buf	= va_arg( marker, char*);
			break;
			case	'd':
				buf	= (char *)va_arg( marker, int* );
			break;
			case	'o':
				buf	= (char *)va_arg( marker, short* );
			break;
			case	'y':
				buf	= (char *)va_arg( marker, whbyte* );
			break;
			case	'f':
				buf	= (char *)va_arg( marker, float* );
			break;
			case	'b':
				buf	= (char *)va_arg( marker, bool* );
			break;
			case	't':
				buf	= (char *)va_arg( marker, time_t* );
			break;
			case	'I':
				buf	= (char *)va_arg( marker, whint64* );
			break;
			case	'*':
			break;
			default:
				// 不认识的格式
				assert(0);
			break;
		}
		if( buf )
		{
			switch(__format[count])
			{
				case	'a':
				case	's':
					strcpy(buf, __src + start);
					wh_strtrim(buf);
				break;
				case	'd':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(int *)buf, bufi);
				break;
				case	'o':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(short *)buf, bufi);
				break;
				case	'y':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(whbyte *)buf, bufi);
				break;
				case	'f':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(float *)buf, bufi);
				break;
				case	'b':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(bool *)buf, bufi);
				break;
				case	't':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(time_t *)buf, bufi);
				break;
				case	'I':
					strcpy(bufi, __src+start);
					wh_strsetvalue(*(whint64 *)buf, bufi);
				break;
				case	'*':
				break;
				default:
					// 不认识的格式
					assert(0);
				break;
			}
		}
		else
		{
			noarg	= 1;
		}
		// 就算为"*"也要算count
		count	++;
	}
	rcount	= count;
	// 如果还有没有赋值的buf（保留原来的值更好些2003-07-15，所以后面&&0）
	while( count<__count && !noarg && 0 )
	{
		switch(__format[count])
		{
			case	'a':
			case	's':
				buf	= va_arg( marker, char*);
				if(buf)	strcpy(buf, "");
			break;
			case	'd':
				buf	= (char *)va_arg( marker, int* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'o':
				buf	= (char *)va_arg( marker, short* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'y':
				buf	= (char *)va_arg( marker, whbyte* );
				if(buf)	*(int *)buf = 0;
			break;
			case	'f':
				buf	= (char *)va_arg( marker, float* );
				if(buf)	*(float *)buf = 0;
			break;
			case	'b':
				buf	= (char *)va_arg( marker, bool* );
				if(buf)	*(bool *)buf = 0;
			break;
			case	't':
				buf	= (char *)va_arg( marker, time_t* );
				if(buf)	*(time_t *)buf = 0;
			break;
			case	'I':
				buf	= (char *)va_arg( marker, whint64* );
				if(buf)	*(whint64 *)buf = 0;
			break;
			case	'*':
			break;
			default:
				// 不认识的格式
				assert(0);
			break;
		}
		count ++;
	};

	return	rcount;
}

void	wh_strsetvalue_charptr(charptr pValue, int nMaxSize, const char *cszVal)
{
	strncpy(pValue, cszVal, nMaxSize-1);
	pValue[nMaxSize-1]	= 0;
}
void	wh_strsetvalue(charptr pValue, const char *cszVal)
{
	strcpy(pValue, cszVal);
}
void	wh_strsetvalue(whbyte &rfValue, const char *cszVal)
{
	rfValue	= (whbyte)whstr2int(cszVal);
}
void	wh_strsetvalue(short &rfValue, const char *cszVal)
{
	rfValue	= (short)whstr2int(cszVal);
}
void	wh_strsetvalue(int &rfValue, const char *cszVal)
{
	rfValue	= whstr2int(cszVal);
}
void	wh_strsetvalue(unsigned int &rfValue, const char *cszVal)
{
	rfValue	= whstr2int(cszVal);
}
void	wh_strsetvalue(float &rfValue, const char *cszVal)
{
	rfValue	= (float)atof(cszVal);
}
void	wh_strsetvalue(bool &rfValue, const char *cszVal)
{
	rfValue	= whstr2bool(cszVal);
}
void	wh_strsetvalue(std::string &rfValue, const char *cszVal)
{
	rfValue	= cszVal;
}
void	wh_strsetvalue(voidptr &rfValue, const char *cszVal)
{
	// 一般指针都是16进制的(但也可能是10进制的)
	rfValue	= (voidptr)whstr2int(cszVal);
}
void	wh_strsetvalue(time_t &rfValue, const char *cszVal)
{
	// 标准格式: YYYY-MM-DD [HH:MM:SS]   (参见wh_gettimestr的说明)
	rfValue	= wh_gettimefromstr(cszVal);
}
void	wh_strsetvalue(whint64 &rfValue, const char *cszVal)
{
	rfValue	= whstr2int64(cszVal);
}

const char *	wh_strgetvalue(const charptr pValue, char *pszVal)
{
	strcpy(pszVal, pValue);
	return	pszVal;
}
const char *	wh_strgetvalue(const whbyte &rfValue, char *pszVal)
{
	return	whint2str(rfValue, pszVal);
}
const char *	wh_strgetvalue(const short &rfValue, char *pszVal)
{
	return	whint2str(rfValue, pszVal);
}
const char *	wh_strgetvalue(const int &rfValue, char *pszVal)
{
	return	whint2str(rfValue, pszVal);
}
const char *	wh_strgetvalue(const unsigned int &rfValue, char *pszVal)
{
	sprintf(pszVal, "%u", rfValue);
	return	pszVal;
}
const char *	wh_strgetvalue(const float &rfValue, char *pszVal)
{
	sprintf(pszVal, "%f", rfValue);
	return	pszVal;
}
const char *	wh_strgetvalue(const bool &rfValue, char *pszVal)
{
	return	whbool2str(rfValue, pszVal);
}
const char *	wh_strgetvalue(const std::string &rfValue, char *pszVal)
{
	strcpy(pszVal, rfValue.c_str());
	return	pszVal;
}
const char *	wh_strgetvalue(const voidptr &rfValue, char *pszVal)
{
	sprintf(pszVal, "0x%p", rfValue);
	return	pszVal;
}
const char *	wh_strgetvalue(const time_t &rfValue, char *pszVal)
{
	return	wh_gettimestr(rfValue);
}
const char *	wh_strgetvalue(const whint64 &rfValue, char *pszVal)
{
	return	whint642str(rfValue, pszVal);
}

// 从字串中找出用某些字符括住的子串
// 如果找到则返回__substr，否则返回NULL
char *	wh_findquoted(const char *__src, char __left, char __right, char *__substr)
{
	const char	*pLeft;
	if( (pLeft=strchr(__src, __left)) == NULL )
	{
		return	NULL;
	}
	pLeft	++;
	const char	*pRight;
	if( (pRight=strchr(pLeft, __right)) == NULL )
	{
		return	NULL;
	}
	int		nLen	= pRight-pLeft;
	memcpy(__substr, pLeft, nLen);
	__substr[nLen]	= 0;
	return	__substr;
}
char *	wh_findquotedandmodify(char *__src, char __left, char __right, char **__ptr_right)
{
	char	*pLeft;
	if( (pLeft=strchr(__src, __left)) == NULL )
	{
		return	NULL;
	}
	pLeft	++;
	char	*pRight;
	if( (pRight=strchr(pLeft, __right)) == NULL )
	{
		return	NULL;
	}
	*(pLeft-1)	= 0;
	*pRight		= 0;
	if( __ptr_right )
	{
		*__ptr_right	= pRight+1;
	}
	return	pLeft;
}

// 将一个通过字串分解为若干部分
// __src被分解的字串
// __dst用来状态分解部分用'\0'分割的实体，其长度不能少于__src
// __ptr是用来存放各个部分的头指针的指针数组
// *__num原来是__ptr最多可容纳的元素数量，最后返回实际的元素个数
int		wh_strexplode(const char *__src, const char *__sep, char *__dst, char *__ptr[], int *__num)
{
	int		rst=0;
	int		i=0, j=0, count=0;
	char	*prev;
	prev	= __dst;
	while( 1 )
	{
		if( __src[i]==0 
		||  strchr(__sep, __src[i])
		)
		{
			__dst[j++]		= 0;
			__ptr[count++]	= prev;
			prev			= __dst+j;
			if( __src[i]==0 )
			{
				// 字串结束，工作结束
				goto	End;
			}
			if( count>=(*__num) )
			{
				// 超过最多容纳的数量了
				rst		= -1;
				goto	End;
			}
		}
		else
		{
			__dst[j++]		= __src[i];
		}
		i	++;
	}

End:
	*__num	= count;

	return	rst;
}

// 将若干分解的字串组合成一个字串
// __ptr是用来存放各个部分的头指针的指针数组
// __num原来是__ptr容纳的元素数量
// __sep用于分隔各个字串
// __dst是最后生成的字串，其长度由上层保证足够
int		wh_strimplode(char *__ptr[], int __num, const char *__sep, char *__dst)
{
	if( __num<=0 )
	{
		__dst[0]	= 0;
		return		0;
	}

	int		idx		= 0;
	int		i = 0, j = 0;
	while(1)
	{
		j	= 0;
		while( __ptr[idx][j] )
		{
			__dst[i++]	= __ptr[idx][j++];
		}
		idx	++;
		if( idx>=__num )
		{
			break;
		}
		j	= 0;
		while(__sep[j])
		{
			__dst[i++]	= __sep[j++];
		}
	}
	__dst[i]	= 0;

	return		0;
}

int		wh_strchrreplace(char *__str, char __from, char __to)
{
	int	i=0, count=0;
	while(__str[i])
	{
		if( __str[i]==__from )
		{
			if( __to!=0 )
			{
				__str[i]	= __to;
			}
			count		++;
		}
		i++;
	}
	return		count;
}
int		wh_strchrreplace(char *__str, int __strlen, char __from, char __to)
{
	int	i=0, count=0;
	while(__str[i] )
	{
		if( __str[i]==__from )
		{
			if( __to!=0 )
			{
				__str[i]	= __to;
			}
			count		++;
		}
		i++;
		if( --__strlen <= 0 )
		{
			break;
		}
	}
	return		count;
}
const char *	wh_strchr(const char *__str, const char *__matches, char &__what)
{
	bool	abMatchChar[256];
	memset(abMatchChar, 0, sizeof(abMatchChar));
	int		i=0;
	while(__matches[i]!=0)
	{
		abMatchChar[(int)__matches[i]]	= true;
		++i;
	}
	while( *__str!=0 )
	{
		if( abMatchChar[(unsigned char)*__str] )
		{
			__what	= *__str;
			return	__str;
		}
		++__str;
	}
	return	NULL;
}
const char *	wh_strrchr(const char *__str, const char *__matches, char &__what)
{
	bool abMatches[256];
	memset(abMatches, 0, sizeof(abMatches));
	int i = 0;
	while (__matches[i])
	{
		abMatches[(unsigned char)__matches[i]] = true;		//不然中文时会出现负数下标
		i++;
	}
	i = strlen(__str) - 1;
	while (i >= 0)
	{
		if (abMatches[(unsigned char)__str[i]])				//不然中文时会出现负数下标
		{
			__what = __str[i];
			return &__str[i];
		}
		i--;
	}
	return NULL;
}
const char *	wh_the_n_strchr(const char *__str, char __c, int __n)
{
	const char	*pszRst	= NULL;
	for(int i=0;i<__n;i++)
	{
		pszRst	= strchr(__str, __c);
		if( !pszRst )
		{
			return	NULL;
		}
		__str	= pszRst + 1;
	}
	return	pszRst;
}
int		wh_strstrreplace(char *__str, int nMaxLen, const char *__from, const char *__to)
{
	int		nFromLen	= strlen(__from);
	int		nToLen		= strlen(__to);
	int		nDiff		= nToLen - nFromLen;
	int		nStrLen		= strlen(__str);
	// 这个有错说明原来内存就错了
	assert( nStrLen<nMaxLen );
	if( nFromLen==0 )
	{
		// 这个应该是不允许的
		return	0;
	}
	// 找出有多少个替换处
	int		nCount		= 0;
	char	*pSubStr	= __str;
	while( (pSubStr=strstr(pSubStr, __from))!=NULL )
	{
		nCount	++;
		pSubStr	+=nFromLen;
	}
	if( nCount==0 )
	{
		// 没有需要替换的
		return	0;
	}
	// 判断长度是否够
	if( nStrLen+nCount*nDiff >= nMaxLen )
	{
		// 长度不够
		return	-1;
	}
	// 最后0的位置
	char	*pLast	= __str+nStrLen;
	// 搜索开始的位置
	char	*pNext	= __str;
	nCount		= 0;
	pSubStr		= __str;
	while( (pSubStr=strstr(pNext, __from))!=NULL )
	{
		// 下一个搜索位置
		pNext	= pSubStr+nFromLen;
		nStrLen	= pLast-pNext+1;
		// 后面移动
		memmove(pNext+nDiff, pNext, nStrLen);
		// 目标拷贝
		memcpy(pSubStr, __to, nToLen);
		nCount	++;
		pNext	+= nDiff;
		pLast	+= nDiff;
	}
	return		0;
}
int		wh_strcontinuouspatternreplace(char *__dst, int nMaxLen, const char *__src, const char *__from, char __to)
{
	int		nSrcLen		= strlen(__src);
	int		nDstIdx		= 0;
	bool	bHas		= false;
	for(int i=0;i<nSrcLen;i++)
	{
		if( strchr(__from, __src[i]) )
		{
			if( bHas )
			{
				// 已经有了，就跳过这个
				continue;
			}
			// 没有，就强制变成__to
			__dst[nDstIdx]	= __to;
			// 并标注已经有了
			bHas	= true;
		}
		else
		{
			__dst[nDstIdx]	= __src[i];
			bHas	= false;
		}
		nDstIdx	++;
	}
	// 最后一个清零
	__dst[nDstIdx]		= 0;
	return	0;
}

int		wh_strmacroprocess(const char *__src, int (*__cb)(int __type, const char *__str, int __len, void *__param), void *__param)
{
	int		i=0;
	char	c;
	int		step=0;
	int		start=0,len=0;
	while( (c=__src[i])!=0 )
	{
		switch(step)
		{
		case	0:
			{
				// 普通状态
				// 如果碰到“$”则0过程结束
				if( c=='$' )
				{
					if( len>0 )
					{
						// 把前面的字串传给回调函数
						int	rst	= __cb(WH_STR_MP_TYPE_CMN, __src+start, len, __param);
						if( rst<0 )
						{
							return	-100 + rst;
						}
					}
					step	= 1;
				}
				else
				{
					len		++;
				}
			}
			break;
		case	1:
			{
				// 有$，希望找到“(”的状态
				switch( c )
				{
				case	'(':
					{
						step		= 2;
						start		= i+1;
						len			= 0;
					}
					break;
				default:
					{
						// 相当于是普通字串
						step		= 0;
						start		= i-1;
						len			= 2;
					}
					break;
				}
			}
			break;
		case	2:
			{
				// 找到了“$(”，收集后面的字串
				// 如果有了“)”就结束
				if( c==')' )
				{
					int	rst	= __cb(WH_STR_MP_TYPE_MACRO, __src+start, len, __param);
					if( rst<0 )
					{
						return	-200 + rst;
					}
					// 变回普通状态
					step	= 0;
					start	= i+1;
					len		= 0;
				}
				else
				{
					len		++;
				}
			}
			break;
		}
		// 下一个字符
		i++;
	}
	switch( step )
	{
	case	0:
		{
			// 最后一段字符
			int	rst	= __cb(WH_STR_MP_TYPE_CMN, __src+start, len, __param);
			if( rst<0 )
			{
				return	-300 + rst;
			}
		}
		break;
	case	1:
		{
			// 说明行以$结尾而已
			int	rst	= __cb(WH_STR_MP_TYPE_CMN, "$", 1, __param);
			if( rst<0 )
			{
				return	-400 + rst;
			}
		}
		break;
	case	2:
		// 说明有宏没有结束
		return	-2;
	}
	// 告诉回调结束了
	__cb(WH_STR_MP_TYPE_END, NULL, 0, __param);
	return	0;
}

// 分析一行文本，解析为Key=Value的形式
int		wh_strlineparse(const char *szLine, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC)
{
	char	szKey[256], szVal[1024];
	int		rst;

	rst		= wh_strsplit("sa", szLine, szSep, szKey, szVal);
	if( rst<2 )
	{
		// 格式错误
		return	-1;
	}
	if( szKey[0]==';'
	||  szKey[0]=='#'
	|| (szKey[0]=='/' && (szKey[1]=='/'))
	)
	{
		// 注释
		return	0;
	}

	// 判断是否是可认的key
	while( pInfo->cszKey )
	{
		if( bIC )
		{
			rst	= stricmp(szKey, pInfo->cszKey);
		}
		else
		{
			rst	= strcmp(szKey, pInfo->cszKey);
		}
		if( rst==0 )
		{
			switch( pInfo->nType )
			{
				case	WHSTR_TYPE_STRING:
					wh_strsetvalue((charptr)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_INT:
					wh_strsetvalue(*(int*)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_FLOAT:
					wh_strsetvalue(*(float*)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_SHORT:
					wh_strsetvalue(*(short*)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_BOOL:
					wh_strsetvalue(*(bool*)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_TIME:
					wh_strsetvalue(*(time_t*)pInfo->pValue, szVal);
				break;
				case	WHSTR_TYPE_INTARRAY:
				case	WHSTR_TYPE_FLOATARRAY:
					wh_str2array(pInfo->nType, szVal, pInfo->pValue, pInfo->nExt);
				break;
				case	WHSTR_TYPE_EXTFUNC:
				case	WHSTR_TYPE_EXTFUNCPARSEONLY:
					if( pInfo->nExt )
					{
						((whstr_ext_func_t)pInfo->nExt)(szVal, pInfo->pValue, WHSTR_ACT_PARSE);
					}
					else
					{
						assert(0);
					}
				break;
				case	WHSTR_TYPE_INT64:
					wh_strsetvalue(*(whint64*)pInfo->pValue, szVal);
					break;
			}
			return	0;
		}

		// 下一个
		pInfo	++;
	}

	// 没有匹配
	return	-1;
}
char *	wh_strcat(char *__dst, const char *__str1, const char *__str2)
{
	int	n1	= strlen(__str1);
	int	n2	= strlen(__str2);
	memcpy(__dst, __str1, n1);
	memcpy(__dst+n1, __str2, n2+1);
	return	__dst;
}

char *	wh_genspace(char *szDst, const char *szStr, int nTab)
{
	assert(nTab>0);
	int		nLen  = strlen(szStr);
	int		nSize = nLen / nTab;
	if( nLen%nTab )
	{
		nSize	++;
	}
	nSize	*= nTab;
	memcpy(szDst, szStr, nLen);
	memset(szDst+nLen, ' ', nSize-nLen);
	szDst[nSize]	= 0;
	return	szDst;
}
int		wh_parseprint(WH_LINE_PARSE_INFO_T *pInfo, int nTab)
{
	return	wh_parsesave(stdout, pInfo, nTab);
}
int		wh_parsesave(const char *szFile, WH_LINE_PARSE_INFO_T *pInfo, int nTab)
{
	FILE	*fp = fopen(szFile, "wt");
	if( !fp )
	{
		return	-1;
	}

	if( wh_parsesave(fp, pInfo, nTab)<0 )
	{
		return	-2;
	}

	fclose(fp);
	return	0;
}
int		wh_parsesave(FILE *fp, WH_LINE_PARSE_INFO_T *pInfo, int nTab)
{
	int		i;
	char	szKey[256], szValue[256];
	while( pInfo->cszKey )
	{
		if( pInfo->nType == WHSTR_TYPE_EXTFUNCPARSEONLY )
		{
			// 这个不需要输出的
			pInfo	++;
			continue;
		}
		if( nTab>0 )
		{
			wh_genspace(szKey, pInfo->cszKey, nTab);
			fprintf(fp, "%s = ", szKey);
		}
		else
		{
			fprintf(fp, "%s=", pInfo->cszKey);
		}
		switch( pInfo->nType )
		{
			case	WHSTR_TYPE_STRING:
				fprintf(fp, "%s", wh_strgetvalue((charptr)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_INT:
				fprintf(fp, "%s", wh_strgetvalue(*(int*)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_SHORT:
				fprintf(fp, "%s", wh_strgetvalue(*(short*)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_BOOL:
				fprintf(fp, "%s", wh_strgetvalue(*(bool*)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_FLOAT:
				fprintf(fp, "%s", wh_strgetvalue(*(float*)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_TIME:
				fprintf(fp, "%s", wh_strgetvalue(*(time_t*)pInfo->pValue, szValue));
			break;
			case	WHSTR_TYPE_INTARRAY:
			{
				int	*pArr = (int *)pInfo->pValue;
				for(i=0;i<pInfo->nExt;i++)
				{
					fprintf(fp, "%d ", pArr[i]);
				}
			}
			break;
			case	WHSTR_TYPE_FLOATARRAY:
			{
				float	*pArr = (float *)pInfo->pValue;
				for(i=0;i<pInfo->nExt;i++)
				{
					fprintf(fp, "%f ", pArr[i]);
				}
			}
			break;
			case	WHSTR_TYPE_EXTFUNC:
				if( pInfo->nExt )
				{
					((whstr_ext_func_t)pInfo->nExt)((char *)fp, pInfo->pValue, WHSTR_ACT_SAVE);
				}
				else
				{
					assert(0);
				}
			break;
		}
		fputs(WHLINEEND, fp);
		// 下一个
		pInfo	++;
	}

	return	0;
}
int		wh_str2array(int nType, const char *szStr, void *pArr, int nExt)
{
	int		nIdx	= 0;
	int		i		= 0;
	int		nBegin	= 0;
	int		nStep;

	enum
	{
		FIND_FIRST_NOSPACE	= 0,
		FIND_FIRST_SPACE	= 1,
	};
	nStep	= FIND_FIRST_NOSPACE;
	while( szStr[i] )
	{
		switch( nStep )
		{
			case	FIND_FIRST_NOSPACE:
				if( szStr[i] != ' ' )
				{
					nStep	= FIND_FIRST_SPACE;
					nBegin	= i;
				}
			break;
			case	FIND_FIRST_SPACE:
				if( szStr[i] == ' ' )
				{
					nStep	= FIND_FIRST_NOSPACE;
					switch( nType )
					{
						case	WHSTR_TYPE_INTARRAY:
							((int *)pArr)[nIdx]		= wh_atoi(szStr+nBegin);
						break;
						case	WHSTR_TYPE_FLOATARRAY:
							((float *)pArr)[nIdx]	= (float)atof(szStr+nBegin);
						break;
					}
					nIdx	++;
					if( nExt>0 && nIdx >= nExt )
					{
						// 返回元素
						return	nIdx;
					}
				}
			break;
		}
		i	++;
	}
	if( nStep == FIND_FIRST_SPACE )
	{
		// 最后还有一个
		switch( nType )
		{
			case	WHSTR_TYPE_INTARRAY:
				((int *)pArr)[nIdx]		= wh_atoi(szStr+nBegin);
			break;
			case	WHSTR_TYPE_FLOATARRAY:
				((float *)pArr)[nIdx]	= (float)atof(szStr+nBegin);
			break;
		}
		nIdx	++;
	}
	return	nIdx;
}
int		wh_array2str(int argc, char *argv[], char *szStr, int nMaxSize)
{
	int	nSize	= 0;
	for(int i=0;i<argc;i++)
	{
		int	nLen	= strlen(argv[i]);
		nSize		+= nLen+1;
		if( nSize>nMaxSize )
		{
			return	-1;
		}
		memcpy(szStr, argv[i], nLen);
		szStr		+= nLen;
		*szStr++	= ' ';
	}
	if( nSize>0 )
	{
		nSize	--;
		*(--szStr)	= 0;
	}
	else
	{
		*szStr	= 0;
	}
	return	nSize;
}
int		wh_parse_opt(const char *szStr, int nSize, whvector<char> &vectTmp, whvector<const char *>&vectOpt, char *pszErr)
{
	// 先把字串放入临时缓冲，因为需要在其中加入很多结尾0
	vectTmp.resize(nSize+1);
	memcpy(vectTmp.getbuf(), szStr, nSize+1);
	char	*p0	= vectTmp.getptr(nSize);
	char	*pC	= vectTmp.getbuf();
	vectOpt.reserve(16);	// 即少说可以有8个空位
	int		nStep		= 0;
	bool	bQuotation	= false;
	for(int i=0;i<nSize;i++)
	{
		switch( nStep )
		{
		case	0:	// 查找opt开始的'-'
			{
				// 等待第一个'-'
				switch( (*pC) )
				{
				case	'-':
					{
						vectOpt.push_back(pC);
						nStep	= 1;
					}
					break;
				case	' ':
					{
						// 空格可以跳过，继续
					}
					break;
				default:
					{
						// 别的开始的不行
						sprintf(pszErr, "no begin opt,pos:%d,%s", i, pC);
						return	-1;
					}
					break;
				}
			}
			break;
		case	1:	// 确定opt字符开始的位置
			{
				switch( *pC )
				{
				case	' ':
					{
						// 说明是一个空的'-'，没有什么意义
						sprintf(pszErr, "bad option -,pos:%d,%s", i, pC);
						return	-11;
					}
					break;
				case	'-':
					{
						// 说明是连续的'--'，继续往下找
					}
					break;
				default:
					{
						// 其他任何一个字符，都说明是opt的一部分
						nStep	= 2;
					}
					break;
				}
			}
			break;
		case	2:	// 查找opt结束的位置
			{
				if( (*pC)==' ' )
				{
					// 说明Opt部分结束后面的应该是参数
					*pC	= 0;
					nStep	= 3;
				}
			}
			break;
		case	3:	// 查找值部分的开始
			{
				switch( *pC )
				{
				case	'-':
					{
						switch( *(pC+1) )
						{
						case	' ':
						case	0:
							{
								// 说明值就是'-'
								vectOpt.push_back(pC);
								nStep	= 4;
							}
							break;
						default:
							{
								// 说明没有参数部分，直接到了下一个的开始
								vectOpt.push_back(p0);
								vectOpt.push_back(pC);
								nStep	= 1;
							}
							break;
						}
					}
					break;
				case	' ':
					{
						// 空格需要跳过
					}
					break;
				case	'"':
					{
						// 引号开始（也代表值的开始）
						bQuotation	= true;
						++pC;
						++i;
					}
				default:
					{
						// 其他的任何一个字符说明是值部分的开始
						vectOpt.push_back(pC);
						nStep	= 4;
					}
					break;
				}
			}
			break;
		case	4:	// 查找值部分的结束
			{
				if( bQuotation )
				{
					if( (*pC) == '"' )
					{
						bQuotation	= false;
						*pC			= 0;
						nStep		= 0;
					}
				}
				else if( (*pC)==' ' )
				{
					*pC		= 0;
					nStep	= 0;
				}
			}
			break;
		}
		++pC;
	}
	int	nNum	= vectOpt.size();
	if( (nNum%2)==1 )
	{
		vectOpt.push_back(p0);
		++nNum;
	}
	return	nNum/2;
}

// 这个直接从base64.cpp中拷来的，简单，呵呵
static char base_rand_char[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};
int		wh_randstr(char *szStr, int nLen, bool bAutoAddLast0)
{
	int		i;
	for(i=0;i<nLen;i++)
	{
		szStr[i]	= base_rand_char[rand() % sizeof(base_rand_char)];
	}
	if( bAutoAddLast0 )
	{
		szStr[nLen]	= 0;
	}
	return			0;
}
int		wh_randhex(void *szStr, int nLen)
{
	unsigned char	*pszStr	= (unsigned char *)szStr;
	int		i;
	for(i=0;i<nLen;i++)
	{
		pszStr[i]	= rand() & 0xFF;
	}
	return	0;
}
const char *	wh_hex2str(const unsigned char *szHex, int nLen, char *szStr, int nSpace, bool bUPCASE)
{
	static char	 g_szStr[4096];
	const char * cszLOW	= "%02x";
	const char * cszUP	= "%02X";
	const char * cszFmt = cszLOW;
	if( !szStr )
	{
		szStr	= g_szStr;
	}
	const char * cszStrToReturn	= szStr;
	if( bUPCASE )
	{
		cszFmt	= cszUP;
	}
	for(int i=0;i<nLen;i++)
	{
		sprintf(szStr, cszFmt, szHex[i]);
		for(int j=0;j<nSpace;j++)
		{
			szStr[2+j]	= ' ';
		}
		szStr	+= 2+nSpace;
	}
	*szStr	= 0;
	return	cszStrToReturn;
}
int		wh_str2hex(const char *szStr, unsigned char *szHex, size_t nSize)
{
	char			szUnit[3];
	unsigned		nUnit;
	szUnit[2]		= 0;
	size_t			nLen = strlen(szStr)/2;
	int				nHexSize	= 0;
	if( nSize>0 && nLen>nSize )
	{
		nLen		= nSize;
	}
	for(size_t i=0;i<nLen;i++)
	{
		//
		memcpy(szUnit, szStr, 2);
		sscanf(szUnit, "%x", &nUnit);
		*szHex	= nUnit;
		szHex	++;
		nHexSize++;
		szStr	+= 2;
	}
	return		nHexSize;
}

bool	wh_isstrnumeric(const char *cszStr, int nMaxLen)
{
	int	i=0;
	while( nMaxLen==0 || i<nMaxLen )
	{
		if( cszStr[i]<'0' || cszStr[i]>'9' )
		{
			// 如果不是数字则跳出循环
			break;
		}
		i	++;
	}
	// 第一个非数字或最后超出长度的部分是0结尾
	if( cszStr[i]==0 )
	{
		return	true;
	}
	return		false;
}

int		wh_strfileparse(const char *szFile, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC)
{
	FILE	*fp;
	fp		= fopen(szFile, "r");
	if( !fp )
	{
		return	-1;
	}

	char	buf[1024];
	while(!feof(fp))
	{
		if( fgets(buf, sizeof(buf)-1, fp) )
		{
			wh_strlineparse(buf, szSep, pInfo, bIC);
		}
		else
		{
			break;
		}
	}

	fclose(fp);
	return		0;
}
int		wh_strfilesectparse(char *szFile, char *szSectName, char *szSep, WH_LINE_PARSE_INFO_T *pInfo, bool bIC)
{
	FILE	*fp;
	fp		= fopen(szFile, "r");
	if( !fp )
	{
		return	-1;
	}

	char	buf[1024];
	while(!feof(fp))
	{
		if( fgets(buf, sizeof(buf)-1, fp) )
		{
			wh_strlineparse(buf, szSep, pInfo, bIC);
		}
		else
		{
			break;
		}
	}

	fclose(fp);
	return		0;
}

void	wh_printchartable(FILE *fp)
{
	int	i, j;
	const int	STEP = 8;
	for(i=0;i<256;i+=STEP)
	{
		unsigned char	c;
		char	szTmp[32];
		// 打印说明行
		for(j=0;j<STEP;j++)
		{
			c		= i+j;
			switch(c)
			{
				case	'\0':
					strcpy(szTmp, "\\0");
				break;
				case	' ':
					strcpy(szTmp, "SPC");
				break;
				case	'\a':
					strcpy(szTmp, "\\a");
				break;
				case	'\b':
					strcpy(szTmp, "\\b");
				break;
				case	'\f':
					strcpy(szTmp, "\\f");
				break;
				case	'\t':
					strcpy(szTmp, "\\t");
				break;
				case	'\v':
					strcpy(szTmp, "\\v");
				break;
				case	'\r':
					strcpy(szTmp, "\\r");
				break;
				case	'\n':
					strcpy(szTmp, "\\n");
				break;
				default:
					szTmp[0]	= c;
					szTmp[1]	= 0;
				break;
			}
			fprintf(fp, "%7s,", szTmp);
		}
		fprintf(fp, WHLINEEND);
		for(j=0;j<STEP;j++)
		{
			c		= i+j;
			fprintf(fp, "%7d,", c);
		}
		fprintf(fp, "%s%s", WHLINEEND, WHLINEEND);
	}
}

void	wh_strkeyprint(const char *astr[][2], const char *cszKey, const char *cszDft, bool bIC)
{
#ifdef	__GNUC__
	typedef	int (*STRCMP_T)(const char *, const char *);
#else
	typedef	int (__cdecl *STRCMP_T)(const char *, const char *);
#endif

	int	i=0;
	STRCMP_T	pCmpFunc	= ::strcmp;
	if( bIC )
	{
		pCmpFunc			= ::stricmp;
	}
	// 如果cszKey是空串则也当做全部都列出来
	if( cszKey && cszKey[0]==0 )
	{
		cszKey	= NULL;
	}
	while( astr[i][0] )
	{
		if( cszKey )
		{
			if( (*pCmpFunc)(cszKey, astr[i][0])==0 )
			{
				printf("%s: %s%s", astr[i][0], astr[i][1], WHLINEEND);
				return;
			}
		}
		else
		{
			printf("%s: %s%s", astr[i][0], astr[i][1], WHLINEEND);
		}
		i	++;
	}
	if( cszKey )
	{
		if( cszDft )
		{
			printf("%s%s", cszDft, WHLINEEND);
		}
		else
		{
			printf("(NULL)%s", WHLINEEND);
		}
	}
}

bool	wh_charishexdigit(char c)
{
	char	nDiff	= c-'0';
	if( nDiff>=0 && nDiff<=9 )
	{
		return	true;
	}
	nDiff	= c-'A';
	if( nDiff>=0 && nDiff<=5 )
	{
		return	true;
	}
	nDiff	= c-'a';
	if( nDiff>=0 && nDiff<=5 )
	{
		return	true;
	}
	return	false;
}
int		wh_change_percentsymbol(const char *cszSrc, char *pszDst)
{
	int	i=0;
	int	j=0;
	while(cszSrc[i] != 0)
	{
		if( cszSrc[i] == '%' && wh_charishexdigit(cszSrc[i+1]) && wh_charishexdigit(cszSrc[i+2]) )
		{
			char	szTmp[3];
			szTmp[0]	= cszSrc[i+1];
			szTmp[1]	= cszSrc[i+2];
			szTmp[2]	= 0;
			int		nRst;
			sscanf(szTmp, "%x", &nRst);
			pszDst[j]	= nRst;
			i	+= 3;
		}
		else
		{
			pszDst[j]	= cszSrc[i];
			i	++;
		}
		j	++;
	}
	pszDst[j]	= 0;
	return	j;
}

int		whstr_Check_MultiChar(const char *szC)
{
	const char	*cszCharSet	= WHCMN_getcharset();
	if( cszCharSet[0] == 0 )
	{
		// 没有设置本地字符集，就不管这个了
		return	0;
	}
	if( szC[1]==0 )
	{
		// 单字节的就不用过来捣乱了
		return	0;
	}
	// 为了简单就先检查两个字节吧（如果是正常的码，应该可以转换出来一个wchar，所以返回应该是1）
	wchar_t	szWC[3];
	if( wh_multichar2wchar(cszCharSet, szC, 2, szWC, sizeof(szWC))!=1 )
	{
		return	0;
	}
	return	1;
}

// WCHAR相关
#ifdef	WIN32		// {

static int	wh_getcodepage_number(const char *cszCodePage)
{
	if( stricmp("UTF8", cszCodePage)==0 )
	{
		return	CP_UTF8;
	}
	else if( stricmp("euckr", cszCodePage) == 0)
	{
		return 949;
	}
	else
	{
		int cp = wh_atoi(cszCodePage);
		if( cp > 0 )
		{
			return cp;
		}
	}

	return	CP_ACP;
}
int		wh_wchar2multichar(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC)
{
	int	rst	= WideCharToMultiByte(wh_getcodepage_number(cszCodePage), 0, cszWC, nWC, szC, nC, NULL, NULL);
	if( rst>=0 && rst<nC )
	{
		szC[rst]	= 0;
	}
	return	rst;
}
int		wh_multichar2wchar(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC)
{
	int	rst	= MultiByteToWideChar(wh_getcodepage_number(cszCodePage), 0, cszC, nC, (LPWSTR)szWC, nWC);
	if( rst>=0 && rst<nWC )
	{
		szWC[rst]	= 0;
	}
	return	rst;
}
// windows下单实现一份是为了减少wh_getcodepage_number的工序
int		wh_utf82wchar(const char *cszUTF8, int nUTF8Len, wchar_t *szWC, int nWC)
{
	int	rst	= MultiByteToWideChar(CP_UTF8, 0, cszUTF8, nUTF8Len, szWC, nWC);
	if( rst>=0 && rst<nWC )
	{
		szWC[rst]	= 0;
	}
	return	rst;
}
int		wh_wchar2utf8(const wchar_t *cszWC, int nWC, char *szUTF8, int nUTF8Len)
{
	int	rst	= WideCharToMultiByte(CP_UTF8, 0, cszWC, nWC, szUTF8, nUTF8Len, NULL, NULL);
	if( rst>=0 && rst<nUTF8Len )
	{
		szUTF8[rst]	= 0;
	}
	return	rst;
}
#endif	// EOF WIN32	}

#ifdef	__GNUC__		// {
int		wh_wchar2multichar(const char *cszCodePage, const wchar_t *szWC, int nWC, char *szC, int nC)
{
	iconv_t	ic	= iconv_open(cszCodePage, "WCHAR_T");
	if( (iconv_t)(-1) == ic )
	{
		return	-1;
	}
	size_t	nOut		= nC;
	size_t	nOldLen		= nOut;
	size_t	nIn			= nWC*sizeof(wchar_t);
	char	*pIn		= (char *)szWC;
	char	*pOut		= (char *)szC;
	if( 0 != iconv(ic, &pIn, &nIn, &pOut, &nOut) )
	{
		iconv_close(ic);
		return	-2;
	}

	iconv_close(ic);
	int	rst	= (int)(nOldLen - nOut);
	if( rst>=0 && rst<nC )
	{
		szC[rst]	= 0;
	}
	return	rst;
}
int		wh_multichar2wchar(const char *cszCodePage, const char *szC, int nC, wchar_t *szWC, int nWC)
{
	iconv_t	ic	= iconv_open("WCHAR_T", cszCodePage);
	if( (iconv_t)(-1) == ic )
	{
		return	-1;
	}
	size_t	nOut		= nWC*sizeof(wchar_t);
	size_t	nOldLen		= nOut;
	size_t	nIn			= nC;
	char	*pIn		= (char *)szC;
	char	*pOut		= (char *)szWC;
	if( 0 != iconv(ic, &pIn, &nIn, &pOut, &nOut) )
	{
		iconv_close(ic);
		return	-2;
	}

	iconv_close(ic);
	int	rst	= (int)((nOldLen - nOut)/sizeof(wchar_t));
	if( rst>=0 && rst<nWC )
	{
		szWC[rst]	= 0;
	}
	return	rst;
}
int		wh_utf82wchar(const char *cszUTF8, int nUTF8Len, wchar_t *szWC, int nWC)
{
	return wh_multichar2wchar("UTF8", cszUTF8, nUTF8Len, szWC, nWC);
}
int		wh_wchar2utf8(const wchar_t *cszWC, int nWC, char *szUTF8, int nUTF8Len)
{
	return wh_wchar2multichar("UTF8", cszWC, nWC, szUTF8, nUTF8Len);
}
#endif	// EOF __GNUC__	}

int		wh_wchar2char(char *szWC)
{
	char	szTmpC[4096];
	int	rst	= wh_wchar2char((wchar_t *)szWC, wcslen((wchar_t *)szWC), szTmpC, sizeof(szTmpC)-1);
	if( rst<0 )
	{
		return	rst;
	}
	memcpy(szWC, szTmpC, rst+1);	// 包括最后一个0d
	return	rst;
}
int		wh_char2wchar(char *szC)
{
	char	szTmpC[4096];
	int	rst	= wh_char2wchar(szC, strlen(szC), (wchar_t *)szTmpC, WHNUMOFARRAYUNIT(szTmpC)-1);
	if( rst<0 )
	{
		return	rst;
	}
	memcpy(szC, szTmpC, (rst+1)*sizeof(wchar_t));	// 包括最后一个0
	return	rst;
}
int		wh_wchar2char(const wchar_t *cszWC, int nWC, char *szC, int nC)
{
	return	wh_wchar2multichar(WHCMN_getcharset(), (const wchar_t *)cszWC, nWC, szC, nC);
}
int		wh_char2wchar(const char *szC, int nC, wchar_t *szWC, int nWC)
{
	return	wh_multichar2wchar(WHCMN_getcharset(), szC, nC, szWC, nWC);
}
int		wh_utf82char(const char *cszUTF8, int nUTF8Len, char *szC, int nC)
{
	return	wh_utf82multichar(WHCMN_getcharset(), cszUTF8, nUTF8Len, szC, nC);
}
int		wh_char2utf8(const char *cszC, int nC, char *szUTF8, int nUTF8Len)
{
	return	wh_multichar2utf8(WHCMN_getcharset(), cszC, nC, szUTF8, nUTF8Len);
}
const char *	wh_wchar2multichar0(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC)
{
	if( wh_wchar2multichar(cszCodePage, cszWC, nWC, szC, nC)<0 )
	{
		return	NULL;
	}
	return	szC;
}
const char *	wh_wchar2multichar0_safe(const char *cszCodePage, const wchar_t *cszWC, int nWC, char *szC, int nC)
{
	if( wh_wchar2multichar(cszCodePage, cszWC, nWC, szC, nC)<0 )
	{
		return	"";
	}
	return	szC;
}
const wchar_t *	wh_multichar2wchar0(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC)
{
	if( wh_multichar2wchar(cszCodePage, cszC, nC, szWC, nWC)<0 )
	{
		return	NULL;
	}
	return	szWC;
}
const wchar_t *	wh_multichar2wchar0_safe(const char *cszCodePage, const char *cszC, int nC, wchar_t *szWC, int nWC)
{
	if( wh_multichar2wchar(cszCodePage, cszC, nC, szWC, nWC)<0 )
	{
		return	L"";
	}
	return	szWC;
}

int		wh_utf82multichar(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC)
{
	int	len	= 0;
	whvector<wchar_t>	vect(nUTF8Len+1);	// 2007-04-05 把这个长度加一了，为了至少可以在最后加0
	len		= wh_utf82wchar(cszUTF8, nUTF8Len, vect.getbuf(), vect.size());
	len		= wh_wchar2multichar(cszCodePage, vect.getbuf(), len, szC, nC);
	return	len;
}
const char *	wh_utf82multichar0(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC)
{
	if( wh_utf82multichar(cszCodePage, cszUTF8, nUTF8Len, szC, nC)<0 )
	{
		return	NULL;
	}
	return	szC;
}
int		wh_multichar2utf8(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len)
{
	int	len	= 0;
	whvector<wchar_t>	vect(nC+1);	// 2007-04-05 把这个长度加一了，为了至少可以在最后加0
	len		= wh_multichar2wchar(cszCodePage, cszC, nC, vect.getbuf(), vect.size());
	len		= wh_wchar2utf8(vect.getbuf(), len, szUTF8, nUTF8Len);
	return	len;
}
const char *	wh_multichar2utf80(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len)
{
	if( wh_multichar2utf8(cszCodePage, cszC, nC, szUTF8, nUTF8Len)<0 )
	{
		return	NULL;
	}
	return	szUTF8;
}
const char *	wh_utf82multichar0_safe(const char *cszCodePage, const char *cszUTF8, int nUTF8Len, char *szC, int nC)
{
	if( wh_utf82multichar(cszCodePage, cszUTF8, nUTF8Len, szC, nC)<0 )
	{
		return	"";
	}
	return	szC;
}
const char *	wh_multichar2utf80_safe(const char *cszCodePage, const char *cszC, int nC, char *szUTF8, int nUTF8Len)
{
	if( wh_multichar2utf8(cszCodePage, cszC, nC, szUTF8, nUTF8Len)<0 )
	{
		return	"";
	}
	return	szUTF8;
}

}		// EOF namespace n_whcmn
