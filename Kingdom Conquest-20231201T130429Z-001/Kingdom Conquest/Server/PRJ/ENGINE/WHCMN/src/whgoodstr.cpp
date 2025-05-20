// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgoodstr.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 主要用于判断字串是否合法
// CreationDate : 2007-07-11 Transformer国内首映，(*^__^*) 嘻嘻……
// ChangeLog    : 

#include "../inc/whgoodstr.h"
#include "../inc/whstring.h"
#include "../inc/whhash2.h"
#include "../inc/whfile.h"

namespace n_whcmn
{

// whgoodchar
class	whgoodchar_i	: public whgoodchar
{
private:
	typedef	whhash2<wchar_t,unsigned char>	MYHASH_T;
	char		m_szCharSet[16];
	MYHASH_T	m_setGood;
	whvector<wchar_t>	m_wbuf;
	whvector<wchar_t>	m_wbufdst;
public:
	whgoodchar_i()
	{
		m_wbuf.reserve(128);
		m_wbufdst.reserve(128);
		strcpy(m_szCharSet, "GBK");
	}
	virtual ~whgoodchar_i()
	{
		Release();
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	virtual int		Init(const char *cszCharFile, const char *cszCharSet, INIT_RST_T *pRst);
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst);
	virtual int		Release();
	virtual bool	IsGood(const char *cszStr);
	virtual int		FilterStr(char *szStr, char cReplace);
};
whgoodchar *	whgoodchar::Create()
{
	return	new whgoodchar_i;
}

int		whgoodchar_i::Init(const char *cszCharFile, const char *cszCharSet, INIT_RST_T *pRst)
{
	whfile	*fp	= whfile_OpenCmnFile(cszCharFile, "rb");
	if( !fp )
	{
		return	-1;
	}
	int	rst	= Init(fp, cszCharSet, pRst);
	fp->SelfDestroy();
	return	rst;
}
int		whgoodchar_i::Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)
{
	if( cszCharSet && cszCharSet[0] )
	{
		WH_STRNCPY0(m_szCharSet, cszCharSet);
	}
	// 获得文件长度
	int	nFSize	= fp->FileSize();
	if( nFSize<=0 )
	{
		return	-11;
	}
	// 给hash表预留空间
	int	nMax	= nFSize/2*10/9;			// 因为大部分应该是双字节的
	if( nMax<256 )
	{
		nMax	= 256;						// 至少给所有的字幕留下空位置
	}
	int	nStep1	= _next_prime(nMax*5/4);	// 打出一半的空富裕
	int	nStep2	= _next_prime(nStep1/12);	// 这个是我随便写的
	if( nStep1<=nStep2 )
	{
		nStep1	= _next_prime(nStep2+1);
	}
	if( m_setGood.Init(nStep1, nStep2, nMax)<0 )
	{
		return	-12;
	}
	// 打开文件
	pRst->nErrLine	= 0;
	pRst->nTotal	= 0;
	pRst->nDupCount	= 0;
	while(!fp->IsEOF())
	{
		char	buf[4096];
		// 一行一行读入字串
		pRst->nErrLine	++;
		if( fp->ReadLine(buf, sizeof(buf)-1)<=0 )
		{
			break;
		}
		// 每个字都转换为wchar（但是不要保存最后的回车）
		int	nLen	= wh_strtrim(buf);
		if( nLen==0 )
		{
			// 空行应该跳过
			continue;
		}
		if( buf[0]=='/' && buf[1]=='/' )
		{
			// 说明是注释
			continue;
		}
		wchar_t	wbuf[4096];
		int	nNum	= wh_multichar2wchar(m_szCharSet, buf, nLen, wbuf, WHNUMOFARRAYUNIT(wbuf)-1);
		if( nNum<0 )
		{
			return	-21;
		}
		// 保存到hash表中
		for(int i=0;i<nNum;i++)
		{
			if( m_setGood.put(wbuf[i], 0)==m_setGood.end() )
			{
				// 说明有重复的键值（实践证明，big5码中可能不同的编码，转成wchar却是相同的，比如“十”）
				pRst->nDupCount	++;
				//return	-22;
			}
			else
			{
				pRst->nTotal	++;
			}
		}
	}
	// 统计一下分布程度
	MYHASH_T::STAT_T	s;
	m_setGood.stat(&s);
	assert( pRst->nTotal == s.nGood + s.nBad );
	if( pRst->nTotal==0 )
	{
		pRst->nFitFactor	= 0;
		assert(0);
	}
	else
	{
		pRst->nFitFactor	= s.nGood * 10000 / pRst->nTotal;
	}
	if( pRst->nDupCount )
	{
		// 需要记录一下日志
		WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_ERROR, WHCMN_LOG_STD_HDR(1062,NP)"whgoodchar,Dup,%d,%s", pRst->nDupCount, cszCharSet);
	}
	return	0;
}
int		whgoodchar_i::Release()
{
	m_setGood.Release();
	return	0;
}
bool	whgoodchar_i::IsGood(const char *cszStr)
{
	// 把字串变成wchar
	int	nLen	= strlen(cszStr);
	m_wbuf.resize(nLen+1);
	int	nNum	= wh_multichar2wchar(m_szCharSet, cszStr, nLen, m_wbuf.getbuf(), m_wbuf.size()-1);
	if( nNum<=0 )
	{
		return	false;
	}
	for(int i=0;i<nNum;i++)
	{
		if( !m_setGood.has(m_wbuf[i]) )
		{
			return	false;
		}
	}
	return	true;
}
int		whgoodchar_i::FilterStr(char *szStr, char cReplace)
{
	int	nCount	= 0;
	// 把字串变成wchar
	int	nLen	= strlen(szStr);
	m_wbuf.resize(nLen+1);
	m_wbufdst.reserve(nLen+1);
	m_wbufdst.clear();
	// 把replace也变成wchar
	wchar_t		wcReplace;
	wh_multichar2wchar(m_szCharSet, &cReplace, 1, &wcReplace, 1);
	int	nNum	= wh_multichar2wchar(m_szCharSet, szStr, nLen, m_wbuf.getbuf(), m_wbuf.size()-1);
	if( nNum<=0 )
	{
		return	false;
	}
	for(int i=0;i<nNum;i++)
	{
		if( m_setGood.has(m_wbuf[i]) )
		{
			m_wbufdst.push_back(m_wbuf[i]);
		}
		else
		{
			nCount	++;
			m_wbufdst.push_back(wcReplace);
		}
	}
	if( nCount>0 )
	{
		// 把新的字串转换回去
		wh_wchar2multichar(m_szCharSet, m_wbufdst.getbuf(), m_wbufdst.size(), szStr, nLen+1);
	}
	return	nCount;
}

// whbadword
class	whbadword_i	: public whbadword
{
private:
	struct	MYWSTR_T
	{
		whvector<wchar_t>	*pvect;
		int			nStart;
		int			nLen;
		inline bool operator == (const MYWSTR_T & other) const
		{
			if( other.nLen != nLen )
			{
				return	false;
			}
			return	memcmp(other.GetBuf(), GetBuf(), TotalBytes())==0;
		}
		inline wchar_t *	GetBuf() const
		{
			return	pvect->getptr(nStart);
		}
		inline int	TotalBytes() const
		{
			return	nLen * sizeof(wchar_t);
		}
	};
	struct	_my_hashfunc
	{
		_my_hashfunc() {}

		inline unsigned int	operator()(const MYWSTR_T &s)		const
		{
			unsigned int h = 0;
			for(int i=0;i<s.nLen;i++)
			{
				h = h * 31 + (unsigned int)(s.GetBuf()[i]);
			}
			return h;
		}
	};

	typedef	whhash2<MYWSTR_T,unsigned char,_my_hashfunc>	MYHASH_T;
	char		m_szCharSet[16];
	MYHASH_T	m_setBad;
	whvector<wchar_t>	m_wWholeStr;
	whvector<wchar_t>	m_wbuf;
public:
	whbadword_i()
	{
		m_wWholeStr.reserve(16384);
		m_wbuf.reserve(128);
		strcpy(m_szCharSet, "GBK");
	}
	virtual ~whbadword_i()
	{
		Release();
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	virtual int		Init(const char *cszBadWordFile, const char *cszCharSet, INIT_RST_T *pRst);
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst);
	virtual int		Release();
	virtual bool	IsBad(const char *cszStr);
	virtual bool	FilterBad(const char *cszSrc, char *szDst, int nDstSize, char cX);
private:
	// 判断m_wbuf中从nStart开始有哪些
	// 如果有，则通过*pnStart和*pnLen传回坏词还是的位置和长度，以最大匹配为准
	// 如果wcX不为0则说明需要把相应的位置替换掉
	bool	_IsBad(int nStart, int *pnStart=NULL, int *pnLen=NULL, wchar_t wcX=0);
};
whbadword *	whbadword::Create()
{
	return	new whbadword_i;
}

int		whbadword_i::Init(const char *cszBadWordFile, const char *cszCharSet, INIT_RST_T *pRst)
{
	whfile	*fp	= whfile_OpenCmnFile(cszBadWordFile, "rb");
	if( !fp )
	{
		return	-1;
	}
	int	rst	= Init(fp, cszCharSet, pRst);
	fp->SelfDestroy();
	return	rst;
}

int		whbadword_i::Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)
{
	if( cszCharSet && cszCharSet[0] )
	{
		WH_STRNCPY0(m_szCharSet, cszCharSet);
	}
	// 获得文件长度
	int	nFSize	= fp->FileSize();
	if( nFSize<0 )
	{
		return	-11;
	}
	// 给hash表预留空间
	int	nMax	= nFSize;					// 虽然大部分应该是双字节的，但是如果内容大部分为英文的话，就可能等于文件字符数了
	if( nMax<256 )
	{
		nMax	= 256;						// 至少给所有的字幕留下空位置
	}
	int	nStep1	= _next_prime(nMax*5/4);	// 打出一部分的空富裕
	int	nStep2	= _next_prime(nStep1/12);	// 这个是我随便写的
	if( nStep1<=nStep2 )
	{
		nStep1	= _next_prime(nStep2+1);
	}
	if( m_setBad.Init(nStep1, nStep2, nMax)<0 )
	{
		return	-12;
	}

	pRst->nErrLine	= 0;
	pRst->nTotal	= 0;
	pRst->nTotalEffective	= 0;
	m_wWholeStr.clear();
	MYWSTR_T		mws;
	mws.pvect		= &m_wWholeStr;
	while(!fp->IsEOF())
	{
		char	buf[4096];
		// 一行一行读入字串
		pRst->nErrLine	++;
		if( fp->ReadLine(buf, sizeof(buf)-1)<=0 )
		{
			break;
		}
		// 每个字都转换为wchar（但是不要保存最后的回车）
		int	nLen	= wh_strtrim(buf);
		if( nLen==0 )
		{
			// 空行应该跳过
			continue;
		}
		if( buf[0]=='/' && buf[1]=='/' )
		{
			// 说明是注释
			continue;
		}
		// 然后把前后的引号trim掉（因为可能需要用引号来扩上一些非显示字符，如空格，等等）
		nLen	= wh_strtrimc(buf, "\"");
		if( nLen==0 )
		{
			// 空行应该跳过
			continue;
		}

		wchar_t	wbuf[4096];
		int	nNum	= wh_multichar2wchar(m_szCharSet, buf, nLen, wbuf, WHNUMOFARRAYUNIT(wbuf)-1);
		if( nNum<0 )
		{
			return		-11;
		}
		// 增加计数
		pRst->nTotal	++;
		// 把每段保存到hash表中
		mws.nStart		= m_wWholeStr.size();
		m_wWholeStr.pushn_back(wbuf, nNum);
		for(int i=1;i<=nNum;i++)
		{
			mws.nLen	= i;
			// 看看原来有没有
			MYHASH_T::iterator	it	= m_setBad.find(mws);
			if( it!=m_setBad.end() )
			{
				// 说明有重复的键值
				if( (*it).GetVal()!=0 && i==nNum )
				{
					// 说明上一个词汇这里不是最后，那么就改成最后
					(*it).SetVal(0);
				}
				// 如果原来存在则说明可能两个词有相同的前面的部分，那么就当作插入成功
				continue;
			}
			// 真正的插入
			if( i==nNum )
			{
				// 最后一个，设置值为0，表示结束
				it	= m_setBad.put(mws, 0);
				assert(it!=m_setBad.end());
				pRst->nTotalEffective	++;
			}
			else
			{
				// 不是最后一个，设置值为1，表示中间
				it	= m_setBad.put(mws, 1);
				assert(it!=m_setBad.end());
			}
		}
	}
	// 统计一下分布程度
	MYHASH_T::STAT_T	s;
	m_setBad.stat(&s);
	if( pRst->nTotal==0 )
	{
		pRst->nFitFactor	= 0;
		assert(0);
	}
	else
	{
		if( s.nGood+s.nBad <= 0 )
		{
			pRst->nFitFactor	= 0;
		}
		else
		{
			pRst->nFitFactor	= s.nGood * 10000 / (s.nGood+s.nBad);
		}
	}
	pRst->nTotalChar		= m_wWholeStr.size();
	return	0;
}
int		whbadword_i::Release()
{
	m_setBad.Release();
	return	0;
}
bool	whbadword_i::IsBad(const char *cszStr)
{
	// 把字串变成wchar
	int	nLen	= strlen(cszStr);
	m_wbuf.resize(nLen+1);
	int	nNum	= wh_multichar2wchar(m_szCharSet, cszStr, nLen, m_wbuf.getbuf(), m_wbuf.size());
	if( nNum<=0 )
	{
		return	false;
	}
	// 让m_wbuf的长度等于字串长度，反正肯定内部够长，最后有0结尾的
	m_wbuf.resize(nNum);
	return	_IsBad(0);
}
bool	whbadword_i::_IsBad(int nStart, int *pnStart, int *pnLen, wchar_t wcX)
{
	MYWSTR_T	mws;
	mws.pvect	= &m_wbuf;
	int	nLen	= 0;		// nLen在后面用作最大匹配长度
	int	nNum	= m_wbuf.size();
	for(int i=nStart;i<nNum;i++)
	{
		mws.nStart	= i;
		int		nMaxLen		= nNum - i;
		for(int j=1;j<=nMaxLen;j++)
		{
			mws.nLen		= j;
			// 判断这段是否存在
			MYHASH_T::iterator	it	= m_setBad.find(mws);
			if( it != m_setBad.end() )
			{
				if( (*it).GetVal() == 0 )
				{
					// 说明存在
					nLen	= j;
					// 继续向下看看还没有没有更大的匹配
				}
				else
				{
					// 否则说明这还只是部分，应该继续向下找
				}
			}
			else
			{
				// 没有找到，说明从这次的i开始后面不存在更多的匹配了
				// 应该继续增加i进行后面的比较
				break;
			}
		}
		if( nLen>0 )
		{
			// 说明曾经找到了匹配的
			if( pnStart!=NULL )
			{
				*pnStart	= mws.nStart;
			}
			if( pnLen!=NULL )
			{
				*pnLen		= nLen;
			}
			if( wcX!=0 )
			{
				// 填充一下
				for(int nIdx=0;nIdx<nLen;nIdx++)
				{
					m_wbuf[mws.nStart+nIdx]	= wcX;
				}
			}
			return	true;
		}
	}
	return	false;
}
bool	whbadword_i::FilterBad(const char *cszSrc, char *szDst, int nDstSize, char cX)
{
	// 先把
	wchar_t	wszX[2];
	if( cX==0 )
	{
		wszX[0]	= 0;
	}
	else
	{
		wh_multichar2wchar(m_szCharSet, &cX, 1, wszX, 2);
	}

	int	nLen	= strlen(cszSrc);
	m_wbuf.resize(nLen+1);
	int	nNum	= wh_multichar2wchar(m_szCharSet, cszSrc, nLen, m_wbuf.getbuf(), m_wbuf.size());
	if( nNum<=0 )
	{
		return	false;
	}
	// 让m_wbuf的长度等于字串长度，反正肯定内部够长，最后有0结尾的
	m_wbuf.resize(nNum);

	// 一点一点过滤
	int		nStart	= 0;
	nLen	= 0;
	bool	bRst	= false;
	while( _IsBad(nStart, &nStart, &nLen, wszX[0]) )
	{
		bRst	= true;
		nStart	+= nLen;
		// 继续往后查
	}
	if( bRst )
	{
		// 这个重新转回本地字符集
		wh_wchar2multichar(m_szCharSet, m_wbuf.getbuf(), m_wbuf.size(), szDst, nDstSize);
	}

	return	bRst;
}

}		// EOF namespace n_whcmn
