// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstring.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �ִ�����ࡣ�磺�����ִ�����
// CreationDate : 2003-05-16
// ChangeLOG    : 2004-05-26 ������ԭ��whstrmatchû���������������ڴ�й©�Ĵ���(û��ɾ��list�е�new������ָ��)��
//                2004-06-15 ������wh_strsplit�������ָ����¶��������ִ���bug��ԭ������1,,234�ᱻ�ָ�Ϊ1��,234���մ�
//                2004-07-08 ������whstrlike��ֻ��ǰ����ͨ����İ���ͨ��ʽ����
//                2004-07-31 �������ÿհ����ָ���ʱ���ӵĿհױ���Ϊ�Ƕ���ָ��������µ�ȡ������Ϊ0�Ĵ���
//                2004-08-25 ������wh_strsplit(int *pnOffset, ... ...)�д������һ������ʱ����offset�Ĵ���
//                2005-05-30 ������wh_strsetvalue(bool...)�����ǰ����ַ�ʱʹ�þ����±�ͬʱ��ָ��ӼӵĴ���
//                2005-10-01 ��_wh_isspliter�������������һ���ַ�����1��
//                2005-12-07 ������wh_strchrreplace��û������i�Ĵ���
//                2006-04-24 ������wh_strsplit�Բ���ʶ�ĸ�ʽ��assert������
//                2006-05-31 �����Ŷ^O^���������ִ������м򵥵�������˳���������㹦�ܣ��ɺ���ǰ��˳�򣩡�
//                2007-01-17 ��������������֧�������ȼ����˳����ڼӼ���
//                2007-01-23 ��wh_strchr��abMatchChar[(int)*__str]��ΪabMatchChar[(unsigned char)*__str]
//                2007-03-02 ������wh_strcontinuouspatternreplace��û�а��ظ����滻Ϊ__to��bug��ԭ���Ľ�����滻Ϊ�ظ����ĵ�һ���ַ���
//                2007-04-30 ������wh_strlwr�Ⱥ��������ж��ַ������ڴ�д��Χʱû�п����ַ��Ĵ���

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

// �հ��ַ�
const char	*WHSTR_WHITESPACE	= " \t\r\n";

int	wh_atoi(const char *cszVal)
{
	int		i=0;
	sscanf(cszVal, "%u", &i);
	return	i;
}

int	whstr2int(const char *cszVal)
{
	// ֧��˳�����������
	char	c;
	// �����ҼӼ�
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
	// Ȼ���ҳ˳�
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
				int	nDivisor	= whstr2int(i2);	// �����0������
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

	// ����ǰ��Ŀ��ַ�
	bool	bStop = false;
	while( !bStop )
	{
		switch(*cszVal)
		{
			case	' ':
			case	'\t':
			case	'\r':
			case	'\n':
				// ����
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
	// ԭ��ֱ����atoi����������0x7FFFFFFFʱ�᷵��0x7FFFFFFF
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
	// ֧��˳�����������
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
	// Ȼ���ҳ˳�
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
				whint64	nDivisor	= whstr2int64(i2);	// �����0������
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

	// ����ǰ��Ŀ��ַ�
	bool	bStop = false;
	while( !bStop )
	{
		switch(*cszVal)
		{
		case	' ':
		case	'\t':
		case	'\r':
		case	'\n':
			// ����
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
	// ��֤ǰ��û�п��ַ�
	for(int i=0;i<64 && cszVal;i++)
	{
		// ԭ������ôд�� :( if( !strchr(WHSTR_WHITESPACE, cszVal[i]) )
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
				// ��������ַ�
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
		// ���Ƿ�ƥ��
		if( memcmp(pStr, pSubStr, nSublen)==0 )
		{
			// ƥ����
			return	(char *)pStr;
		}
		// �ж��ǲ�������
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
	// �����ж��ٸ���
	int	nCount = 0;
	const char **ppList = aszList;
	while(*ppList)
	{
		int	nLen = strlen(*ppList)+1;
		if( nLen > nUnitLen )
		{
			// ����Ϊ��󳤶�
			nUnitLen	= nLen;
		}
		nCount	++;
		ppList	++;
	}
	// ֮���Բ�ȡ�ִ��е����ֵ������Ϊ�����ܻ�Ҫ�����ĳ��ȵĴ�
	// ��ʼ��
	init(nCount, nUnitLen);
	// ��д��
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
// �ȳ���b�ִ�����
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
// �䳤�ִ�ջ
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
		// ������ǵ�һ���������һ����ĩβ���Ϸָ���
		m_strbuf[nOff-1]	= m_sep;
	}
	// ���ϱ��ִ�(������\0)
	memcpy(m_strbuf.getptr(nOff), szStr, nLen);
	// ��������
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
	char	*pStr = m_strbuf.getptr(nOff);		// ����д����Ϊresize����ȡ�ͻᱨassert��
	m_strbuf.resize(nOff);
	if( m_strbuf.size() )						// ������ж����
	{
		m_strbuf.getlast()	= 0;				// �ִ���β���㣨���Ǹ��ָ����պ��滻Ϊ0�ˣ�
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
	// ���ԭ��������
	clear();

	int		len;
	if( !szStr || (len=strlen(szStr))==0 )
	{
		// �մ���ô��ʼ����
		return	0;
	}

	// ��ʼ������󳤶�
	m_strbuf.resize(len+1);

	int		last = 0;
	int		i;
	for(i=0;i<len;i++)
	{
		char	c = szStr[i];
		if( strchr(seps, c) )
		{
			// �ҵ���һ���ָ���
			c		= m_sep;
			// ���һ������
			m_idxbuf.push_back(last);
			last	= i+1;
		}
		// ���ַ���ֵ
		m_strbuf[i]	= c;
	}
	// ������һ������
	m_idxbuf.push_back(last);
	// ���һ��\0
	m_strbuf[i]		= 0;

	return	0;
}

////////////////////////////////////////////////////////////////////
// whvarstrarr �䳤�ĳ�ʼ���󲻻�ɾ�����ִ�����
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
// �����ִ�ƥ��
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

	// �����Ƿ���ģ������
	if( m_bLikeMatch )
	{
		if( nLen==1 && cszWord[0]==whstrlike::DFTSEP )
		{
			return	-1;
		}
		// ���ǰ���е����м�û�оͰ�����ͨ�Ĵ���
		if( cszWord[0]==whstrlike::DFTSEP
		&&  strchr(cszWord+1, whstrlike::DFTSEP) == (cszWord+nLen-1)
		)
		{
			// �ú������������������
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
	// �ּ�����
	for(i=0;i<nLen;i++)
	{
		idx			= findchar_in_chlid(idx, cszWord[i]);
		if( idx<0 )
		{
			// ˵�����µ�
			bNew	= true;
			break;
		}
		inspnt		= idx;
	}
	if( !bNew )
	{
		// �ж����һ���Ƿ���last
		UNIT_T	*pUnit	= m_vectUnit.getptr(inspnt);
		if( pUnit && !pUnit->islast )
		{
			// ��ô��˵��ԭ������һ���Ƚϳ���
			pUnit->islast	= true;
			return	ADDSTR_RST_OK;
		}
		// ԭ��������
		return		ADDSTR_RST_DUP;
	}
	// ��ǰ��i��ʾ��һ��δƥ���ַ���λ�á��򣬴�����ʼ���롣
	for(;i<nLen;i++)
	{
		inspnt	= insertchar_in_chlid(inspnt, cszWord[i]);
	}
	// �������һ���ַ�Ϊ��������
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
			// �����Ҫ��ȫƥ�������ֱ�ӷ�����
			if( m_bWholeMatch )
			{
				bFound	= false;
				goto	End;
			}
			// û����ȫƥ��
			i0		++;
			idx		= 0;
			// �����ͷ�����Ŀ�ͷ��������һ��
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
			// ���ǵ�һ��ƥ��㣬��¼�¿�ʼλ��
			startidx	= i;
		}
		if( pUnit->islast )
		{
			if( m_bWholeMatch )
			{
				// ���ƥ�䵽��β�˾ͳɹ����������
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
			// �ж��Ƿ��ģ����ѯ���
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
	// ��ʼ��ض��Ǵ��ڵ�
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
		// �±곬����
		return	NULL;
	}
	return	m_vectUnit.getptr(nIdx);
}

////////////////////////////////////////////////////////////////////
// whstrlike �ж�һ���ִ��Ƿ������ƥ��
////////////////////////////////////////////////////////////////////
class	whstrlike_i	: public whstrlike
{
protected:
	struct	PATTERN_T
	{
		enum
		{
			TYPE_BEGIN	= 0,			// ��Ҫ��ͷƥ���Ӵ�
			TYPE_SEARCH	= 1,			// ��Ҫ�Ӻ������в����Ӵ�
			TYPE_END	= 2,			// ƥ�䵽��β�ķ����ִ�
		};
		unsigned char	nType;			// ����
		unsigned char	nLen;			// �Ӵ�����
		const char		*pStr;			// �Ӵ���ָ��(�����TYPE_END������ΪNULL)
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
	// ��pattern�����Ա��޸�
	int		nLen	= strlen(cszPattern);
	m_PatternStr.resize(nLen+1);
	char	*pStr	= m_PatternStr.getbuf();
	memcpy(pStr, cszPattern, m_PatternStr.size());

	// ���ԭ����ģʽ
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
				// ��ʾһ��SEP������һ���ַ����������ǰ�ƶ�
				--	nLen;
				memmove(pStr+i+1, pStr+i+2, nLen-i);
				bIsSEP	= false;
			}
			else
			{
				// �ɵĽ���
				if( pPattern )
				{
					if( pPattern->pStr )
					{
						pPattern->nLen	= pStr+i - pPattern->pStr;
					}
				}
				pStr[i]		= 0;
				// �µĿ�ʼ
				pPattern	= m_PatternArray.push_back();
				// ����һ���µ�Ԫ�Ŀ�ʼ
				if( pStr[i+1]==0 )
				{
					// ����β���κ��ַ���ƥ��
					pPattern->nType		= PATTERN_T::TYPE_END;
					pPattern->pStr		= NULL;
					pPattern->nLen		= 0;
				}
				else
				{
					// һ����
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
				// ���ǵ�һ����Ҫ���ͷƥ���
				pPattern		= m_PatternArray.push_back();
				pPattern->nType	= PATTERN_T::TYPE_BEGIN;
				pPattern->pStr	= pStr;
			}
			else
			{
				// ����ĳ�����������м䲿��
			}
		}
	}
	// �����β����%����
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
				// �������ĵ��ִ�����
				char	*pStrMatch	= wh_strstr_cn(cszStr, pPattern->pStr);
				if( !pStrMatch )
				{
					return	false;
				}
				if( m_nMaxClearance>0 )
				{
					if( pStrMatch-cszStr > m_nMaxClearance )
					{
						// ��϶���󣬲�����Ϊ��ƥ���
						return	false;
					}
				}
				cszStr	= pStrMatch + pPattern->nLen;
			}
			break;
			case	PATTERN_T::TYPE_END:
			{
				// ֱ��ƥ�䵽��β��
				return	true;
			}
			break;
		}
	}
	// ���һ��ƥ������Ƿ�END�ģ����Ƿ�ƥ�䵽�˽�β
	if( pPattern )
	{
		if( pPattern->pStr )
		{
			// cszStr��ʱӦ�������һ��ƥ�䴮����һ���ַ�
			int		nLenLeft	= strlen(cszStr0) - (cszStr - cszStr0);
			if( nLenLeft!=0 )
			{
				// û����ȫƥ��
				return	false;
			}
		}
	}
	// û�г�����ƥ��
	return	true;
}

////////////////////////////////////////////////////////////////////
// �ִ��� whstrstream
////////////////////////////////////////////////////////////////////
whstrstream::whstrstream()
: m_pszStr(NULL)
, m_nSize(0)
, m_nOffset(0)
, m_bNeedThrowErrStr(false)
, m_bNoExtendOnWrite(false)
{
}
// �����ִ�(�����þͲ��ܽ��ж�ȡ����)
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
// ��ʼ��ͷ��
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
// ��ȡָ���ֽڵ�����
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
// ��ȡһ���ַ�
char	whstrstream::getc()
{
	if( iseos() )
	{
		return	0;
	}
	return	m_pszStr[m_nOffset++];
}
// ��ȡ�䳤�ִ�
int		whstrstream::readvstrasstr(char *szStr)
{
	// �Ȼ�ȡ����
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
		// ��ʾû�ж���
		return	0;
	}
}

// ��ȡһ�����ļ����ַ�������������ľͷ������ֽ����ģ�
const char *	whstrstream::getchnc(int *pnLen)
{
	if( iseos() )
	{
		return	NULL;
	}
	// �жϵ�ǰ�ַ��Ƿ�������
	m_szCHNC[0]	= m_pszStr[m_nOffset];
	m_nOffset	++;
	*pnLen		= 1;
	if( whstr_Check_MultiChar(m_szCHNC) )
	{
		// �����Ŀ�ͷ���Ͷ��һ��
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
			// ������չ
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
	// �Ȼ�ȡ����
	unsigned char	nLen = strlen(cszStr);
	// д����
	int	rst = 0;
	rst	+= write(&nLen);
	// д�ִ�
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
// �ִ�����
////////////////////////////////////////////////////////////////////
// �����ִ���β�İ�����szKick����ַ�
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
		// ȫ������Ҫtrim
		szStr[0]	= 0;
		return		0;
	}
	end	= begin;							// �������ѭ���Ҳ�����szKick���ַ�������¾������ֵ
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
		// ��������Ҫ�ƶ�
		memmove(szStr, szStr+begin, l);
	}
	szStr[l]	= 0;

	return		l;
}
// �����ִ���β�Ŀո��س�
int		wh_strtrim(char *szStr)
{
	return	wh_strtrimc(szStr, WHSTR_WHITESPACE);
}
// ȥ���ִ�β�İ�����szKick����ַ�
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
// ȥ���ִ�β��\��/
int		wh_strkickendslash(char *szStr)
{
	return	wh_strendtrimc(szStr, "\\/");
}
// ��֤�ִ�ĩβ��ĳ���ַ�
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
		// �ж��Ƿ��ǿ��ֽ�
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
	// ���һ���ֽ�
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
		// �ض�ext ����Ϊԭ���Ǿ������ƻ�ԭ�������֣�
		nExtLen		-= nDiff;
		if( nExtLen )
		{
			// �Ƿ���ɰ������
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
		// �ж��ǲ��Ǻ���	2005-04-05��
		if( whstr_Check_MultiChar(szTarget+nTargetLen-nDiff-1) )
		{
			szTarget[nTargetLen-nDiff-1]	= '0';
		}
		// �����������ĵط�
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

// ��ȫ��strcpy����������涨���ȾͲ�����(����NULL)
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
	char	*buf = NULL, bufi[256];	// bufi����ʱ�õ�
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

	// ȥ����β�Ŀհ�
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
					// ����ʶ�ĸ�ʽ
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
						// ����ʶ�ĸ�ʽ
						assert(0);
					break;
				}
			}
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
			i	++;			// �����ַ��ִ��������һ����ѭ�����滹���ټ�һ����
		}
	}

	// ���һ������
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
				// ����ʶ�ĸ�ʽ
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
					// ����ʶ�ĸ�ʽ
					assert(0);
				break;
			}
		}
		else
		{
			noarg	= 1;
		}
		// ����Ϊ"*"ҲҪ��count
		count	++;
	}
	rcount	= count;
	// �������û�и�ֵ��buf������ԭ����ֵ����Щ2003-07-15�����Ժ���&&0��
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
				// ����ʶ�ĸ�ʽ
				assert(0);
			break;
		}
		count ++;
	};

	return	rcount;
}

int		wh_strsplit(int *pnOffset, const char *__format, const char *__src, const char *__splitter, ...)
{
	char	*buf = NULL, bufi[256];	// bufi����ʱ�õ�
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

	// ȥ����β�Ŀհ�
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
			// ԭ���ǣ�i += sl;start = i; �����ڷָ�������ʱ�������Ϊ��
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
			i	++;			// �����ַ��ִ��������һ����ѭ�����滹���ټ�һ����
		}
	}
	(*pnOffset)	+= start;

	// ���һ������
	if( start<l && count<__count && !noarg )
	{
		// ��ôoffset�Ϳ���ָ���ִ���β��
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
				// ����ʶ�ĸ�ʽ
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
					// ����ʶ�ĸ�ʽ
					assert(0);
				break;
			}
		}
		else
		{
			noarg	= 1;
		}
		// ����Ϊ"*"ҲҪ��count
		count	++;
	}
	rcount	= count;
	// �������û�и�ֵ��buf������ԭ����ֵ����Щ2003-07-15�����Ժ���&&0��
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
				// ����ʶ�ĸ�ʽ
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
	// һ��ָ�붼��16���Ƶ�(��Ҳ������10���Ƶ�)
	rfValue	= (voidptr)whstr2int(cszVal);
}
void	wh_strsetvalue(time_t &rfValue, const char *cszVal)
{
	// ��׼��ʽ: YYYY-MM-DD [HH:MM:SS]   (�μ�wh_gettimestr��˵��)
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

// ���ִ����ҳ���ĳЩ�ַ���ס���Ӵ�
// ����ҵ��򷵻�__substr�����򷵻�NULL
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

// ��һ��ͨ���ִ��ֽ�Ϊ���ɲ���
// __src���ֽ���ִ�
// __dst����״̬�ֽⲿ����'\0'�ָ��ʵ�壬�䳤�Ȳ�������__src
// __ptr��������Ÿ������ֵ�ͷָ���ָ������
// *__numԭ����__ptr�������ɵ�Ԫ����������󷵻�ʵ�ʵ�Ԫ�ظ���
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
				// �ִ���������������
				goto	End;
			}
			if( count>=(*__num) )
			{
				// ����������ɵ�������
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

// �����ɷֽ���ִ���ϳ�һ���ִ�
// __ptr��������Ÿ������ֵ�ͷָ���ָ������
// __numԭ����__ptr���ɵ�Ԫ������
// __sep���ڷָ������ִ�
// __dst��������ɵ��ִ����䳤�����ϲ㱣֤�㹻
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
		abMatches[(unsigned char)__matches[i]] = true;		//��Ȼ����ʱ����ָ����±�
		i++;
	}
	i = strlen(__str) - 1;
	while (i >= 0)
	{
		if (abMatches[(unsigned char)__str[i]])				//��Ȼ����ʱ����ָ����±�
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
	// ����д�˵��ԭ���ڴ�ʹ���
	assert( nStrLen<nMaxLen );
	if( nFromLen==0 )
	{
		// ���Ӧ���ǲ������
		return	0;
	}
	// �ҳ��ж��ٸ��滻��
	int		nCount		= 0;
	char	*pSubStr	= __str;
	while( (pSubStr=strstr(pSubStr, __from))!=NULL )
	{
		nCount	++;
		pSubStr	+=nFromLen;
	}
	if( nCount==0 )
	{
		// û����Ҫ�滻��
		return	0;
	}
	// �жϳ����Ƿ�
	if( nStrLen+nCount*nDiff >= nMaxLen )
	{
		// ���Ȳ���
		return	-1;
	}
	// ���0��λ��
	char	*pLast	= __str+nStrLen;
	// ������ʼ��λ��
	char	*pNext	= __str;
	nCount		= 0;
	pSubStr		= __str;
	while( (pSubStr=strstr(pNext, __from))!=NULL )
	{
		// ��һ������λ��
		pNext	= pSubStr+nFromLen;
		nStrLen	= pLast-pNext+1;
		// �����ƶ�
		memmove(pNext+nDiff, pNext, nStrLen);
		// Ŀ�꿽��
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
				// �Ѿ����ˣ����������
				continue;
			}
			// û�У���ǿ�Ʊ��__to
			__dst[nDstIdx]	= __to;
			// ����ע�Ѿ�����
			bHas	= true;
		}
		else
		{
			__dst[nDstIdx]	= __src[i];
			bHas	= false;
		}
		nDstIdx	++;
	}
	// ���һ������
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
				// ��ͨ״̬
				// ���������$����0���̽���
				if( c=='$' )
				{
					if( len>0 )
					{
						// ��ǰ����ִ������ص�����
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
				// ��$��ϣ���ҵ���(����״̬
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
						// �൱������ͨ�ִ�
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
				// �ҵ��ˡ�$(�����ռ�������ִ�
				// ������ˡ�)���ͽ���
				if( c==')' )
				{
					int	rst	= __cb(WH_STR_MP_TYPE_MACRO, __src+start, len, __param);
					if( rst<0 )
					{
						return	-200 + rst;
					}
					// �����ͨ״̬
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
		// ��һ���ַ�
		i++;
	}
	switch( step )
	{
	case	0:
		{
			// ���һ���ַ�
			int	rst	= __cb(WH_STR_MP_TYPE_CMN, __src+start, len, __param);
			if( rst<0 )
			{
				return	-300 + rst;
			}
		}
		break;
	case	1:
		{
			// ˵������$��β����
			int	rst	= __cb(WH_STR_MP_TYPE_CMN, "$", 1, __param);
			if( rst<0 )
			{
				return	-400 + rst;
			}
		}
		break;
	case	2:
		// ˵���к�û�н���
		return	-2;
	}
	// ���߻ص�������
	__cb(WH_STR_MP_TYPE_END, NULL, 0, __param);
	return	0;
}

// ����һ���ı�������ΪKey=Value����ʽ
int		wh_strlineparse(const char *szLine, const char *szSep, const WH_LINE_PARSE_INFO_T *pInfo, bool bIC)
{
	char	szKey[256], szVal[1024];
	int		rst;

	rst		= wh_strsplit("sa", szLine, szSep, szKey, szVal);
	if( rst<2 )
	{
		// ��ʽ����
		return	-1;
	}
	if( szKey[0]==';'
	||  szKey[0]=='#'
	|| (szKey[0]=='/' && (szKey[1]=='/'))
	)
	{
		// ע��
		return	0;
	}

	// �ж��Ƿ��ǿ��ϵ�key
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

		// ��һ��
		pInfo	++;
	}

	// û��ƥ��
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
			// �������Ҫ�����
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
		// ��һ��
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
						// ����Ԫ��
						return	nIdx;
					}
				}
			break;
		}
		i	++;
	}
	if( nStep == FIND_FIRST_SPACE )
	{
		// �����һ��
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
	// �Ȱ��ִ�������ʱ���壬��Ϊ��Ҫ�����м���ܶ��β0
	vectTmp.resize(nSize+1);
	memcpy(vectTmp.getbuf(), szStr, nSize+1);
	char	*p0	= vectTmp.getptr(nSize);
	char	*pC	= vectTmp.getbuf();
	vectOpt.reserve(16);	// ����˵������8����λ
	int		nStep		= 0;
	bool	bQuotation	= false;
	for(int i=0;i<nSize;i++)
	{
		switch( nStep )
		{
		case	0:	// ����opt��ʼ��'-'
			{
				// �ȴ���һ��'-'
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
						// �ո��������������
					}
					break;
				default:
					{
						// ��Ŀ�ʼ�Ĳ���
						sprintf(pszErr, "no begin opt,pos:%d,%s", i, pC);
						return	-1;
					}
					break;
				}
			}
			break;
		case	1:	// ȷ��opt�ַ���ʼ��λ��
			{
				switch( *pC )
				{
				case	' ':
					{
						// ˵����һ���յ�'-'��û��ʲô����
						sprintf(pszErr, "bad option -,pos:%d,%s", i, pC);
						return	-11;
					}
					break;
				case	'-':
					{
						// ˵����������'--'������������
					}
					break;
				default:
					{
						// �����κ�һ���ַ�����˵����opt��һ����
						nStep	= 2;
					}
					break;
				}
			}
			break;
		case	2:	// ����opt������λ��
			{
				if( (*pC)==' ' )
				{
					// ˵��Opt���ֽ��������Ӧ���ǲ���
					*pC	= 0;
					nStep	= 3;
				}
			}
			break;
		case	3:	// ����ֵ���ֵĿ�ʼ
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
								// ˵��ֵ����'-'
								vectOpt.push_back(pC);
								nStep	= 4;
							}
							break;
						default:
							{
								// ˵��û�в������֣�ֱ�ӵ�����һ���Ŀ�ʼ
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
						// �ո���Ҫ����
					}
					break;
				case	'"':
					{
						// ���ſ�ʼ��Ҳ����ֵ�Ŀ�ʼ��
						bQuotation	= true;
						++pC;
						++i;
					}
				default:
					{
						// �������κ�һ���ַ�˵����ֵ���ֵĿ�ʼ
						vectOpt.push_back(pC);
						nStep	= 4;
					}
					break;
				}
			}
			break;
		case	4:	// ����ֵ���ֵĽ���
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

// ���ֱ�Ӵ�base64.cpp�п����ģ��򵥣��Ǻ�
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
			// �����������������ѭ��
			break;
		}
		i	++;
	}
	// ��һ�������ֻ���󳬳����ȵĲ�����0��β
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
		// ��ӡ˵����
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
	// ���cszKey�ǿմ���Ҳ����ȫ�����г���
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
		// û�����ñ����ַ������Ͳ��������
		return	0;
	}
	if( szC[1]==0 )
	{
		// ���ֽڵľͲ��ù���������
		return	0;
	}
	// Ϊ�˼򵥾��ȼ�������ֽڰɣ�������������룬Ӧ�ÿ���ת������һ��wchar�����Է���Ӧ����1��
	wchar_t	szWC[3];
	if( wh_multichar2wchar(cszCharSet, szC, 2, szWC, sizeof(szWC))!=1 )
	{
		return	0;
	}
	return	1;
}

// WCHAR���
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
// windows�µ�ʵ��һ����Ϊ�˼���wh_getcodepage_number�Ĺ���
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
	memcpy(szWC, szTmpC, rst+1);	// �������һ��0d
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
	memcpy(szC, szTmpC, (rst+1)*sizeof(wchar_t));	// �������һ��0
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
	whvector<wchar_t>	vect(nUTF8Len+1);	// 2007-04-05 ��������ȼ�һ�ˣ�Ϊ�����ٿ���������0
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
	whvector<wchar_t>	vect(nC+1);	// 2007-04-05 ��������ȼ�һ�ˣ�Ϊ�����ٿ���������0
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
