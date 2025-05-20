// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfile.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵��ڴ��ļ�����
// CreationDate : 2004-01-19
// ChangeLOG    : 2007-04-27 ������whmemfile::Write��û���ж��ڴ���Ƿ�����ɹ��������ļ����ݶ�ʧ�Ĵ���

#include "../inc/whmemfile.h"
#include "../inc/whcmn_def.h"
#include <stdio.h>

using namespace n_whcmn;

whmemfile::whmemfile()
: m_pMan(NULL), m_pFileInfo(NULL)
, m_nOpenMode(0), m_nCurOffset(0), m_nCurBlockIdx(whalist::INVALIDIDX)
{
}
whmemfile::~whmemfile()
{
}
size_t	whmemfile::Read(void *pBuf, size_t nSize)
{
	// ��char*�ü���ƫ��
	char	*szBuf	= (char *)pBuf;
	// ��¼�Ѿ������ĳߴ�
	size_t	nReadSize	= 0;
	// nSize���ܳ�����ǰλ�õ��ļ���β�ľ���
	int		nDistFromTail = m_pFileInfo->nSize - m_nCurOffset;
	assert( nDistFromTail>=0 );
	if( nDistFromTail < (int)nSize )
	{
		nSize	= nDistFromTail;
	}

	while( nSize>0 )
	{
		const char	*szFrag		= (const char *)m_pMan->m_alFragInfo.GetDataUnitPtr(m_nCurBlockIdx);
		if( !szFrag )
		{
			// Ӧ���Ѿ������ļ�β��
			break;
		}
		// �ֲ�ƫ��
		int		nLocalOffset	= m_nCurOffset%m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
		// �ж��ڵ�ǰ���п��Կ�������󳤶�
		size_t	nSizeToCopy		= m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize - nLocalOffset;
		if( nSize < nSizeToCopy )
		{
			// ���˵������Ҫ��һ����
			// ע�⣺����ġ�<�������ǡ�<=������Ϊ==��ʱ���ļ�ָ��պû�ָ���¸���Ŀ�ͷ
			nSizeToCopy	= nSize;
		}
		else	// nSize >= nSizeToCopy
		{
			// ˵�����������ˣ�������Ҫ��һ��
			m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		}
		// ��ȡ������Դָ��
		szFrag					+= nLocalOffset;
		// ����֮
		memcpy(szBuf, szFrag, nSizeToCopy);

		// �µ���ƫ��
		m_nCurOffset			+= nSizeToCopy;
		// �д����������ݵ���ʼָ��
		szBuf					+= nSizeToCopy;
		// �Ѿ�����ĳ���
		nReadSize				+= nSizeToCopy;
		// �õ�����Ҫ���͵ĳ���
		nSize					-= nSizeToCopy;
	}

	return	nReadSize;
}
size_t	whmemfile::Write(const void *pBuf, size_t nSize)
{
	// 0�Ƚ����⣬��ֱ���ж���
	if( nSize==0 )
	{
		return	0;
	}

	// �ж��ļ��Ƿ��ǿ�д�򿪵�
	switch( m_nOpenMode )
	{
		case	whmemfileman::MODE_CREATE:
		case	whmemfileman::MODE_READWRITE:
		break;
		default:
			// ��д�򿪣�����д��
			assert(0);
			return	0;
		break;
	}

	// ��char*�ü���ƫ��
	const char	*szBuf	= (const char *)pBuf;
	// ��¼�Ѿ�д��ĳߴ�
	size_t	nWriteSize	= 0;

	while( nSize>0 )
	{
		if( m_nCurBlockIdx == whalist::INVALIDIDX )
		{
			// ���m_nCurBlockIdxָ��INVALIDIDX����Ҫ�����µĿ�
			m_nCurBlockIdx	= m_pMan->m_alFragInfo.Alloc();
			// 2007-04-27���ж������Ƿ�ɹ�
			if( m_nCurBlockIdx == whalist::INVALIDIDX )
			{
				// ���ش���
				return	0;
			}
			if( m_pFileInfo->nTailIdx == whalist::INVALIDIDX )
			{
				// ˵��ԭ���ļ��ǿյģ�����Ҫ��ͷҲ����
				m_pFileInfo->nHeadIdx	= m_nCurBlockIdx;
			}
			else
			{
				// ���¿���ӵ�ԭ�ļ�����β
				 m_pMan->m_alFragInfo.SetNextOf(m_pFileInfo->nTailIdx, m_nCurBlockIdx);
			}
			// ���µ�β
			m_pFileInfo->nTailIdx		= m_nCurBlockIdx;
		}
		// �ҵ���ǰ��
		char	*szFrag			= (char *)m_pMan->m_alFragInfo.GetDataUnitPtr(m_nCurBlockIdx);
		if( !szFrag )
		{
			// Ӧ����ǰ���޷�������
			break;
		}
		// �ֲ�ƫ��
		int		nLocalOffset	= m_nCurOffset%m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
		// �ж��ڵ�ǰ���п��Կ�������󳤶�
		size_t	nSizeToCopy		= m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize - nLocalOffset;
		if( nSize < nSizeToCopy )
		{
			// ���˵������Ҫ��һ����
			// ע�⣺����ġ�<�������ǡ�<=������Ϊ==��ʱ���ļ�ָ��պû�ָ���¸���Ŀ�ͷ
			nSizeToCopy	= nSize;
		}
		else	// nSize >= nSizeToCopy
		{
			// ˵�����������ˣ�������Ҫ��һ��
			m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		}
		// ��ȡ������Ŀ��ָ��
		szFrag					+= nLocalOffset;
		// ����֮
		memcpy(szFrag, szBuf, nSizeToCopy);

		// �µ���ƫ��
		m_nCurOffset			+= nSizeToCopy;
		// �д����������ݵ���ʼָ��
		szBuf					+= nSizeToCopy;
		// �Ѿ�д��ĳ���
		nWriteSize				+= nSizeToCopy;
		// �õ�����Ҫ���͵ĳ���
		nSize					-= nSizeToCopy;
	}

	// ���offset�����˵�ǰ�ļ��ߴ���ı��ļ��ߴ�Ϊ�����
	if( m_nCurOffset > (int)m_pFileInfo->nSize )
	{
		m_pFileInfo->nSize	= m_nCurOffset;
	}

	// ����
	return	nWriteSize;
}
int		whmemfile::Seek(int nOffset, int nOrigin)
{
	// ����ļ��ߴ�Ϊ0������seek
	if( m_pFileInfo->nTailIdx == whalist::INVALIDIDX )
	{
		return	0;
	}

	// ����Ŀ���ļ�ָ���λ��
	int		nDstOffset	= 0;

	switch(nOrigin)
	{
		case	SEEK_CUR:
			nDstOffset	= m_nCurOffset + nOffset;
		break;
		case	SEEK_SET:
			nDstOffset	= nOffset;
		break;
		case	SEEK_END:
			nDstOffset	= m_pFileInfo->nSize + nOffset;
		break;
		default:
			// ����Ĳ������ܿ�����nOffset��nOriginд����
			assert(0);
		break;
	}
	if( nDstOffset<0 )
	{
		nDstOffset	= 0;
	}
	else if( nDstOffset>int(m_pFileInfo->nSize) )
	{
		nDstOffset	= m_pFileInfo->nSize;
	}

	m_nCurOffset	= nDstOffset;

	// �ҵ���Ӧ��block
	int				nBlock	= m_nCurOffset / m_pMan->m_alFragInfo.GetHdr()->nDataUnitSize;
	m_nCurBlockIdx	= m_pFileInfo->nHeadIdx;
	for(int i=0;i<nBlock;i++)
	{
		m_nCurBlockIdx		= m_pMan->m_alFragInfo.GetNextOf(m_nCurBlockIdx);
		// ����ļ����ȸպ��ǵ����鳤�ȵı��������ڽ���ʱm_nCurBlockIdx��ָ��INVALIDIDX
		// ����պñ�ʾEOF��������Write��ʱ�������ͨ�õķ��������¿�
	}

	return			0;
}
size_t	whmemfile::ReadToVector(whvector<char> &vect)
{
	vect.resize( GetFileSize() - Tell() );
	return	Read(vect.getbuf(), vect.size());
}
int		whmemfile::CmpWithData(const void *pData, int nSize)
{
	char	szBuf[4096];
	char	*pszData	= (char *)pData;

	// ������Ȳ�ͬ�����ֱ�ӷ���
	int		nRst		= GetFileSize() - nSize;
	if( nRst!=0 )
	{
		return	nRst;
	}

	while( !IsEOF() && nSize>0 )
	{
		int	nReadSize	= Read(szBuf, sizeof(szBuf));
		if( nReadSize==0 )
		{
			break;
		}
		int	nRst		= memcmp(szBuf, pszData, nReadSize);
		if( nRst!=0 )
		{
			// �������ػ����Է�ӳһЩ�ڴ�����״�����Ǻǣ�����û��̫���Ҫ
			return	nRst;
		}
		// ����Ҫ�Ƚ��ֽ���
		nSize			-= nReadSize;
		pszData			+= nReadSize;
	}
	if( nSize!=0 )
	{
		return	-1;
	}
	return	0;
}
