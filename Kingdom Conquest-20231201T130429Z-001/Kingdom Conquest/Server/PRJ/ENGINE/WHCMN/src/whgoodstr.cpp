// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgoodstr.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��Ҫ�����ж��ִ��Ƿ�Ϸ�
// CreationDate : 2007-07-11 Transformer������ӳ��(*^__^*) ��������
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
	// ����ļ�����
	int	nFSize	= fp->FileSize();
	if( nFSize<=0 )
	{
		return	-11;
	}
	// ��hash��Ԥ���ռ�
	int	nMax	= nFSize/2*10/9;			// ��Ϊ�󲿷�Ӧ����˫�ֽڵ�
	if( nMax<256 )
	{
		nMax	= 256;						// ���ٸ����е���Ļ���¿�λ��
	}
	int	nStep1	= _next_prime(nMax*5/4);	// ���һ��Ŀո�ԣ
	int	nStep2	= _next_prime(nStep1/12);	// ����������д��
	if( nStep1<=nStep2 )
	{
		nStep1	= _next_prime(nStep2+1);
	}
	if( m_setGood.Init(nStep1, nStep2, nMax)<0 )
	{
		return	-12;
	}
	// ���ļ�
	pRst->nErrLine	= 0;
	pRst->nTotal	= 0;
	pRst->nDupCount	= 0;
	while(!fp->IsEOF())
	{
		char	buf[4096];
		// һ��һ�ж����ִ�
		pRst->nErrLine	++;
		if( fp->ReadLine(buf, sizeof(buf)-1)<=0 )
		{
			break;
		}
		// ÿ���ֶ�ת��Ϊwchar�����ǲ�Ҫ�������Ļس���
		int	nLen	= wh_strtrim(buf);
		if( nLen==0 )
		{
			// ����Ӧ������
			continue;
		}
		if( buf[0]=='/' && buf[1]=='/' )
		{
			// ˵����ע��
			continue;
		}
		wchar_t	wbuf[4096];
		int	nNum	= wh_multichar2wchar(m_szCharSet, buf, nLen, wbuf, WHNUMOFARRAYUNIT(wbuf)-1);
		if( nNum<0 )
		{
			return	-21;
		}
		// ���浽hash����
		for(int i=0;i<nNum;i++)
		{
			if( m_setGood.put(wbuf[i], 0)==m_setGood.end() )
			{
				// ˵�����ظ��ļ�ֵ��ʵ��֤����big5���п��ܲ�ͬ�ı��룬ת��wcharȴ����ͬ�ģ����硰ʮ����
				pRst->nDupCount	++;
				//return	-22;
			}
			else
			{
				pRst->nTotal	++;
			}
		}
	}
	// ͳ��һ�·ֲ��̶�
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
		// ��Ҫ��¼һ����־
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
	// ���ִ����wchar
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
	// ���ִ����wchar
	int	nLen	= strlen(szStr);
	m_wbuf.resize(nLen+1);
	m_wbufdst.reserve(nLen+1);
	m_wbufdst.clear();
	// ��replaceҲ���wchar
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
		// ���µ��ִ�ת����ȥ
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
	// �ж�m_wbuf�д�nStart��ʼ����Щ
	// ����У���ͨ��*pnStart��*pnLen���ػ��ʻ��ǵ�λ�úͳ��ȣ������ƥ��Ϊ׼
	// ���wcX��Ϊ0��˵����Ҫ����Ӧ��λ���滻��
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
	// ����ļ�����
	int	nFSize	= fp->FileSize();
	if( nFSize<0 )
	{
		return	-11;
	}
	// ��hash��Ԥ���ռ�
	int	nMax	= nFSize;					// ��Ȼ�󲿷�Ӧ����˫�ֽڵģ�����������ݴ󲿷�ΪӢ�ĵĻ����Ϳ��ܵ����ļ��ַ�����
	if( nMax<256 )
	{
		nMax	= 256;						// ���ٸ����е���Ļ���¿�λ��
	}
	int	nStep1	= _next_prime(nMax*5/4);	// ���һ���ֵĿո�ԣ
	int	nStep2	= _next_prime(nStep1/12);	// ����������д��
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
		// һ��һ�ж����ִ�
		pRst->nErrLine	++;
		if( fp->ReadLine(buf, sizeof(buf)-1)<=0 )
		{
			break;
		}
		// ÿ���ֶ�ת��Ϊwchar�����ǲ�Ҫ�������Ļس���
		int	nLen	= wh_strtrim(buf);
		if( nLen==0 )
		{
			// ����Ӧ������
			continue;
		}
		if( buf[0]=='/' && buf[1]=='/' )
		{
			// ˵����ע��
			continue;
		}
		// Ȼ���ǰ�������trim������Ϊ������Ҫ������������һЩ����ʾ�ַ�����ո񣬵ȵȣ�
		nLen	= wh_strtrimc(buf, "\"");
		if( nLen==0 )
		{
			// ����Ӧ������
			continue;
		}

		wchar_t	wbuf[4096];
		int	nNum	= wh_multichar2wchar(m_szCharSet, buf, nLen, wbuf, WHNUMOFARRAYUNIT(wbuf)-1);
		if( nNum<0 )
		{
			return		-11;
		}
		// ���Ӽ���
		pRst->nTotal	++;
		// ��ÿ�α��浽hash����
		mws.nStart		= m_wWholeStr.size();
		m_wWholeStr.pushn_back(wbuf, nNum);
		for(int i=1;i<=nNum;i++)
		{
			mws.nLen	= i;
			// ����ԭ����û��
			MYHASH_T::iterator	it	= m_setBad.find(mws);
			if( it!=m_setBad.end() )
			{
				// ˵�����ظ��ļ�ֵ
				if( (*it).GetVal()!=0 && i==nNum )
				{
					// ˵����һ���ʻ����ﲻ�������ô�͸ĳ����
					(*it).SetVal(0);
				}
				// ���ԭ��������˵����������������ͬ��ǰ��Ĳ��֣���ô�͵�������ɹ�
				continue;
			}
			// �����Ĳ���
			if( i==nNum )
			{
				// ���һ��������ֵΪ0����ʾ����
				it	= m_setBad.put(mws, 0);
				assert(it!=m_setBad.end());
				pRst->nTotalEffective	++;
			}
			else
			{
				// �������һ��������ֵΪ1����ʾ�м�
				it	= m_setBad.put(mws, 1);
				assert(it!=m_setBad.end());
			}
		}
	}
	// ͳ��һ�·ֲ��̶�
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
	// ���ִ����wchar
	int	nLen	= strlen(cszStr);
	m_wbuf.resize(nLen+1);
	int	nNum	= wh_multichar2wchar(m_szCharSet, cszStr, nLen, m_wbuf.getbuf(), m_wbuf.size());
	if( nNum<=0 )
	{
		return	false;
	}
	// ��m_wbuf�ĳ��ȵ����ִ����ȣ������϶��ڲ������������0��β��
	m_wbuf.resize(nNum);
	return	_IsBad(0);
}
bool	whbadword_i::_IsBad(int nStart, int *pnStart, int *pnLen, wchar_t wcX)
{
	MYWSTR_T	mws;
	mws.pvect	= &m_wbuf;
	int	nLen	= 0;		// nLen�ں����������ƥ�䳤��
	int	nNum	= m_wbuf.size();
	for(int i=nStart;i<nNum;i++)
	{
		mws.nStart	= i;
		int		nMaxLen		= nNum - i;
		for(int j=1;j<=nMaxLen;j++)
		{
			mws.nLen		= j;
			// �ж�����Ƿ����
			MYHASH_T::iterator	it	= m_setBad.find(mws);
			if( it != m_setBad.end() )
			{
				if( (*it).GetVal() == 0 )
				{
					// ˵������
					nLen	= j;
					// �������¿�����û��û�и����ƥ��
				}
				else
				{
					// ����˵���⻹ֻ�ǲ��֣�Ӧ�ü���������
				}
			}
			else
			{
				// û���ҵ���˵������ε�i��ʼ���治���ڸ����ƥ����
				// Ӧ�ü�������i���к���ıȽ�
				break;
			}
		}
		if( nLen>0 )
		{
			// ˵�������ҵ���ƥ���
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
				// ���һ��
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
	// �Ȱ�
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
	// ��m_wbuf�ĳ��ȵ����ִ����ȣ������϶��ڲ������������0��β��
	m_wbuf.resize(nNum);

	// һ��һ�����
	int		nStart	= 0;
	nLen	= 0;
	bool	bRst	= false;
	while( _IsBad(nStart, &nStart, &nLen, wszX[0]) )
	{
		bRst	= true;
		nStart	+= nLen;
		// ���������
	}
	if( bRst )
	{
		// �������ת�ر����ַ���
		wh_wchar2multichar(m_szCharSet, m_wbuf.getbuf(), m_wbuf.size(), szDst, nDstSize);
	}

	return	bRst;
}

}		// EOF namespace n_whcmn
