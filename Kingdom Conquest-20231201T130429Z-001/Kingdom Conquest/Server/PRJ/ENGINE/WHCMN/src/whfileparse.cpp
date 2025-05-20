// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfileparse.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �ļ�����ͨ�ý�������
// CreationDate : 2004-05-28
// ChangeLOG    : 

#include "../inc/whfileparse.h"
#include "../inc/whstring.h"

namespace n_whcmn
{

int		whfileparse(whfile *file, whfileparser *pParser, char *pszLineBuf, int nBufSize)
{
	char	szLocalBuf[1024];	// �����粻ָ��buffer���������Ӧ���㹻���˰�
	if( !pszLineBuf )
	{
		pszLineBuf	= szLocalBuf;
		nBufSize	= sizeof(szLocalBuf)-1;
	}
	int	nLine		= 0;
	// ���¶�λ���ļ���ͷ
	file->Rewind();
	while( !file->IsEOF() )
	{
		nLine		++;
		int	nLen	= file->ReadLine(pszLineBuf, nBufSize);
		if( nLen<=0 )
		{
			// û�����ݿɶ���
			// ��Ϊ�����ǿ���Ҳ���г��ȵ�
			break;
		}
		// trim�˺÷���
		wh_strtrim(pszLineBuf);
		int	rst		= pParser->ParseLine(pszLineBuf, nLen);
		if( rst<0 )
		{
			pParser->SetErrorLine(nLine);
			pParser->SetHeadOfErrorLine(pszLineBuf);
			return	rst;
		}
	}
	return	whfileparser::RST_OK;
}

////////////////////////////////////////////////////////////////////
// whfilecommentchecker
////////////////////////////////////////////////////////////////////
whfilecommentchecker::whfilecommentchecker()
: m_nCommentBeginNum(0)
{
}
void	whfilecommentchecker::Reset()
{
	m_nCommentBeginNum	= 0;
}
bool	whfilecommentchecker::CheckComment(const char *cszLine)
{
	// �����Ƿ���ע��ͷ
	bool	bIs	= false;
	if( memcmp("/*", cszLine, 2)==0 )
	{
		m_nCommentBeginNum	++;
		bIs		= true;
	}
	// �����Ƿ���ע��β
	if( strstr(cszLine, "*/") )
	{
		m_nCommentBeginNum	--;
		if( m_nCommentBeginNum<0 )
		{
			// ���ֲ�ƥ��Ĳ�����һ��ע��
			m_nCommentBeginNum	= 0;
		}
		else
		{
			bIs	= true;
		}
	}
	if( bIs || m_nCommentBeginNum>0 )
	{
		return	true;
	}
	// �����Ƿ�������ע��
	if( (cszLine[0]=='/' && (cszLine[1]=='/'))
	||  cszLine[0]=='#'
	)
	{
		// ������Php�ű���ע��
		return	true;
	}
	return		false;
}

////////////////////////////////////////////////////////////////////
// whfileparser_MULTIPART
////////////////////////////////////////////////////////////////////
// Ϊ�ӿ�ʵ�ֵ�
void	whfileparser_MULTIPART::Reset()
{
	m_pCurWorker	= NULL;
	for(size_t i=0;i<m_vectWorker.size();++i)
	{
		m_vectWorker[i]->Reset();
	}
}
int		whfileparser_MULTIPART::ParseLine(const char *cszLine, int nLen)
{
	if( !m_pCurWorker || !m_pCurWorker->SelfTakeCareOfComment() )
	{
		if( m_pCommentChecker->CheckComment(cszLine) )
		{
			return	RST_OK;
		}
	}

	if( !m_pCurWorker )
	{
		// ����ǿ���������
		if( cszLine[0]==0 )
		{
			return	RST_OK;
		}
		// �ҵ���ʱ�Ľ�����
		for(size_t i=0;i<m_vectWorker.size();++i)
		{
			Worker	*pWorker		= m_vectWorker[i];
			if( pWorker->ItIsMyBegin(cszLine, nLen) )
			{
				m_pCurWorker		= pWorker;
				return	RST_OK;
			}
		}
		// û���ҵ����ʵġ�������(ԭ���Ƿ��ش��󣬲�����ʵû��Ҫ)
		return	RST_OK;
	}
	if( m_pCurWorker->ItIsMyEnd(cszLine, nLen) )
	{
		// ���������ֽ�β�����¿�ʼ��
		m_pCurWorker	= NULL;
		return	RST_OK;
	}
	return	m_pCurWorker->ParseLine(cszLine, nLen);;
}
// �Լ��õ�
whfileparser_MULTIPART::whfileparser_MULTIPART()
: m_pCurWorker(NULL)
{
}
void	whfileparser_MULTIPART::AddWorker(Worker *pWorker)
{
	pWorker->m_pHost	= this;
	m_vectWorker.push_back(pWorker);
}

}		// EOF namespace n_whcmn
