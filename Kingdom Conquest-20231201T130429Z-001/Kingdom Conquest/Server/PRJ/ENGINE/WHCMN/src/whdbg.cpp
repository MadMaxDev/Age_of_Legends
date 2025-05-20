// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ������صĹ���
// CreationDate : 2003-05-10
// ChangeLOG    : 2004-09-11 dbgtimeuse::Disp�У����nNum<0���ʾ��ӡ���иո�mark������Ŀ

#include "../inc/wh_platform.h"
#include "../inc/whdbg.h"
#include "../inc/whstring.h"
#include <stdio.h>

namespace n_whcmn
{

void	dbg_ouput_to_dbgwin(const char *szMsg)
{
#ifdef	WIN32
	OutputDebugString(szMsg);
#else
	fprintf(stderr, "%s%s", szMsg, WHLINEEND);
#endif
}

void	dbg_ouput_to_msgbox(const char *szTitle, const char *szMsg)
{
#ifdef	WIN32
	MessageBox(NULL, szMsg, szTitle, MB_OK);
#else
	fprintf(stderr, "%s:%s%s", szTitle, szMsg, WHLINEEND);
#endif
}

void	dbg_append_to_file(const char *szFile, const char *szMsg)
{
	FILE	*fp;
	fp	= fopen(szFile, "at");
	if( !fp )
	{
		return;
	}
	fprintf(fp, szMsg);
	fclose(fp);
}

// ����һ�еĳ���
static const int	CSTNUMPERLINE	= 16;
int	dbg_calc_line_len(int nNum)
{
	return	8+1+1+1			// 00000000h: <-������һ���ո�
			+ 3*nNum		// nNum��hex��
			+ 1+1+nNum		// ; nNum���ַ�
			+ 2				// \r\n
			;
}
// ע��pszMem��szBuf����ı�
void	dbg_printmem_line(unsigned char	*&pszMem, int nNum, char *&szBuf)
{
	int		i;
	// ��ַ + ���
	sprintf(szBuf, "%p : ", pszMem);
	szBuf	+= 8+1+1+1;
	// nNum��hex��
	for(i=0;i<nNum;i++)
	{
		sprintf(szBuf, "%02X ", pszMem[i]);
		szBuf	+= 3;
	}
	// �����Ƿ��п���
	int		nLeft = CSTNUMPERLINE-nNum;
	for(i=0;i<nLeft;i++)
	{
		sprintf(szBuf, "   ");
		szBuf	+= 3;
	}
	// ; 
	sprintf(szBuf, "; ");
	szBuf	+= 1+1;
	// nNum���ַ�
	for(i=0;i<nNum;i++)
	{
		if( whstr_Check_MultiChar((char *)pszMem+i) )
		{
			// ֱ�Ӵ�ӡ
			sprintf(szBuf, "%c%c", pszMem[i], pszMem[i+1]);
			szBuf	+= 2;
			// ����һ�����
			i++;
			continue;
		}
		char	c			= pszMem[i];
		if( 0
		||  ( c>='a' && c<='z' )
		||  ( c>='A' && c<='Z' )
		||  ( c>='0' && c<='9' )
		||  ( c!=0 && strchr("`~!@#$%^&*()-=_+|[]{};':,./<>?\\\"", c) )
		)
		{
			// c���Ѿ�����ȷֵ��
		}
		else if( strchr(" \t", c) )
		{
			c	= ' ';
		}
		// ����Ͳ���ӡ������
		else
		{
			c	= '.';
		}
		sprintf(szBuf, "%c", c);
		szBuf	+= 1;
	}
	// \r\n
	sprintf(szBuf, WHLINEEND);
	szBuf		+= WHLINEENDLEN;
	pszMem		+= nNum;
}
const char *	dbg_printmem(const void *pMem, int nLen, char *szBuf)
{
	unsigned char	*pszMem = (unsigned char *)pMem;
	int		nLine	= nLen/CSTNUMPERLINE;
	int		nExt	= nLen%CSTNUMPERLINE;
	int		i;
	char	szTmpBuf[256];
	char	*pszBuf0	= szBuf;
	bool	bPrintToScreen	= false;
	if( !szBuf )
	{
		bPrintToScreen		= true;
	}
	else
	{
		// ��õ�nLenΪ0��ʱ��ʲôҲû�У����������ʾ���ڴ��е���Ч����
		szBuf[0]			= 0;
	}
	for(i=0;i<nLine;i++)
	{
		if( bPrintToScreen )
		{
			szBuf	= szTmpBuf;
			dbg_printmem_line(pszMem, CSTNUMPERLINE, szBuf);
			puts(szTmpBuf);	// 2006-01-04 ��ԭ����printf(szTmpBuf)��Ϊputs(szTmpBuf)����Ϊ���szTmpBuf����%�Ļ�������ܵ��±�����
		}
		else
		{
			dbg_printmem_line(pszMem, CSTNUMPERLINE, szBuf);
		}
	}
	if( nExt>0 )
	{
		if( bPrintToScreen )
		{
			szBuf	= szTmpBuf;
			dbg_printmem_line(pszMem, nExt, szBuf);
			printf(szTmpBuf);
		}
		else
		{
			dbg_printmem_line(pszMem, nExt, szBuf);
		}
	}
	return	pszBuf0;
}
const char *	dbg_printmem(const void *pMem, int nLen, whvector<char> &vect)
{
	vect.clear();
	// �����ܳ��ȣ��������һ��'\0'��β
	vect.resize(dbg_calc_line_len(CSTNUMPERLINE) * (nLen/CSTNUMPERLINE+1) + 1);
	dbg_printmem(pMem, nLen, vect.getbuf());
	return	vect.getbuf();
}
int		dbg_printmem(const void *pMem, int nLen, FILE *pout)
{
	unsigned char	*pszMem = (unsigned char *)pMem;
	int		nLine	= nLen/CSTNUMPERLINE;
	int		nExt	= nLen%CSTNUMPERLINE;
	int		i;
	char	szTmpBuf[256];
	for(i=0;i<nLine;i++)
	{
		char	*szBuf	= szTmpBuf;
		dbg_printmem_line(pszMem, CSTNUMPERLINE, szBuf);
		fputs(szTmpBuf, pout);
	}
	if( nExt>0 )
	{
		char	*szBuf	= szTmpBuf;
		dbg_printmem_line(pszMem, nExt, szBuf);
		fputs(szTmpBuf, pout);
		nLine	++;
	}
	return	nLine;
}
int		dbg_printmemashexarray(const void *pMem, int nLen, FILE *pout, int nByteInLine)
{
	printf("{%s", WHLINEEND);
	int	nByte					= 0;
	unsigned char	*pszMem		= (unsigned char *)pMem;
	for(int i=0;i<nLen;i++)
	{
		printf("0x%02X, ", *pszMem);
		++pszMem;
		if( ++nByte>=nByteInLine )
		{
			printf(WHLINEEND);
			nByte	= 0;
		}
	}
	if( nByte>0 )
	{
		printf(WHLINEEND);
	}
	printf("}%s", WHLINEEND);
	return	0;
}

////////////////////////////////////////////////////////////////////
// ��ʱͳ��
////////////////////////////////////////////////////////////////////
// dbgtimeuse
////////////////////////////////////////////////////////////////////
DBGTIMEUSE_DECLARE(g_dtu);
dbgtimeuse::dbgtimeuse()
: m_nCurIdx(0)
{
}
void	dbgtimeuse::BeginMark(int nIdx)
{
	m_nCurIdx	= nIdx;
}
void	dbgtimeuse::Mark(int nIdx)
{
	if( nIdx>=(int)m_vT.size() )
	{
		m_vT.resize(nIdx+1);
	}
	m_vT[nIdx]	= wh_gettickcount();
}
void	dbgtimeuse::Mark()
{
	Mark(m_nCurIdx);
	m_nCurIdx	++;
}
char *	dbgtimeuse::Disp(int nIdx0, int nNum, char *szStr)
{
	const int	CUNITSTRLEN	= 40;

	if( nNum<0 )
	{
		// 2004-09-11 ��Ϊд�������Ǽ�
		nNum	= m_nCurIdx-nIdx0-1;
		if( nNum<0 )
		{
			static char	szNothing[]	= "Nothing!";
			return	szNothing;
		}
	}
	else if( nIdx0+nNum+1>(int)m_vT.size() )
	{
		// ������
		assert(0);
		return	NULL;
	}
	if( !szStr )
	{
		m_vStr.resize(nNum*CUNITSTRLEN);
		szStr	= m_vStr.getbuf();
	}

	char	szTmp[CUNITSTRLEN];
	char	*pszStr	= szStr;
	for(int i=0;i<nNum;i++)
	{
		sprintf(szTmp, "%d,%d ", nIdx0+i, wh_tickcount_diff(m_vT[nIdx0+i+1], m_vT[nIdx0+i]));
		int	nLen	= strlen(szTmp);
		assert(nLen<CUNITSTRLEN);
		memcpy(pszStr, szTmp, nLen+1);	// +1��Ϊ�˿������һ��0
		pszStr		+= nLen;
	}
	
	return	szStr;
}
int		dbgtimeuse::GetDiff(int nIdx0, int nIdx1)
{
	if( nIdx1<0 )
	{
		nIdx1	= m_nCurIdx-1;
	}
	return	wh_tickcount_diff(m_vT[nIdx1], m_vT[nIdx0]);
}

////////////////////////////////////////////////////////////////////
// dbgtimeuse2
////////////////////////////////////////////////////////////////////
dbgtimeuse2::dbgtimeuse2(int nTotal)
: m_vT(nTotal), m_vL(nTotal), m_vStr(4096)
{
	memset(m_vT.getbuf(), 0, m_vT.totalbytes());
	memset(m_vL.getbuf(), 0, m_vL.totalbytes());
}
void	dbgtimeuse2::Begin(int nIdx)
{
	m_vT[nIdx]	= wh_gettickcount();
}
void	dbgtimeuse2::End(int nIdx)
{
	m_vL[nIdx]	= wh_tickcount_diff(wh_gettickcount(), m_vT[nIdx]);
}
char *	dbgtimeuse2::Disp(int nIdx0, int nNum, char *szStr)
{
	const int	CUNITSTRLEN	= 40;
	if( nNum<0 )
	{
		nNum	= m_vT.size() - nIdx0;
	}

	if( szStr==NULL )
	{
		m_vStr.resize(nNum*CUNITSTRLEN);
		szStr	= m_vStr.getbuf();
	}
	char	*pszStr	= szStr;
	char	szTmp[CUNITSTRLEN];
	szStr[0]	= 0;
	for(int i=nIdx0;i<nIdx0+nNum;i++)
	{
		if( m_vL[i]>0 )
		{
			sprintf(szTmp, "%d,%d ", i, m_vL[i]);
			int	nLen	= strlen(szTmp);
			assert(nLen<CUNITSTRLEN);
			memcpy(pszStr, szTmp, nLen+1);	// +1��Ϊ�˿������һ��0
			pszStr		+= nLen;
		}
	}
	
	return	szStr;
}
int		dbgtimeuse2::GetDiff(int nIdx0, int nIdx1)
{
	if( nIdx1<0 )
	{
		nIdx1	= m_vT.size()-1;
	}
	return	wh_tickcount_diff(m_vT[nIdx1], m_vT[nIdx0]);
}


void	whdbg_makebad(int nType)
{
	switch( nType )
	{
		case	0:
		default:
		{
			const char	*pszStr = (const char *)0x1234;
			printf("%s", pszStr);
		}
		break;
	}
}

}		// EOF namespace n_whcmn
