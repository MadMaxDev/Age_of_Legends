// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfileparse.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 文件内容通用解析功能
// CreationDate : 2004-05-28
// ChangeLOG    : 

#include "../inc/whfileparse.h"
#include "../inc/whstring.h"

namespace n_whcmn
{

int		whfileparse(whfile *file, whfileparser *pParser, char *pszLineBuf, int nBufSize)
{
	char	szLocalBuf[1024];	// 如果外界不指定buffer就用这个，应该足够大了吧
	if( !pszLineBuf )
	{
		pszLineBuf	= szLocalBuf;
		nBufSize	= sizeof(szLocalBuf)-1;
	}
	int	nLine		= 0;
	// 重新定位到文件开头
	file->Rewind();
	while( !file->IsEOF() )
	{
		nLine		++;
		int	nLen	= file->ReadLine(pszLineBuf, nBufSize);
		if( nLen<=0 )
		{
			// 没有数据可读了
			// 因为就算是空行也会有长度的
			break;
		}
		// trim了好分析
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
	// 看看是否是注释头
	bool	bIs	= false;
	if( memcmp("/*", cszLine, 2)==0 )
	{
		m_nCommentBeginNum	++;
		bIs		= true;
	}
	// 看看是否是注释尾
	if( strstr(cszLine, "*/") )
	{
		m_nCommentBeginNum	--;
		if( m_nCommentBeginNum<0 )
		{
			// 这种不匹配的不能算一行注释
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
	// 看看是否是其他注释
	if( (cszLine[0]=='/' && (cszLine[1]=='/'))
	||  cszLine[0]=='#'
	)
	{
		// 是类似Php脚本的注释
		return	true;
	}
	return		false;
}

////////////////////////////////////////////////////////////////////
// whfileparser_MULTIPART
////////////////////////////////////////////////////////////////////
// 为接口实现的
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
		// 如果是空行则跳过
		if( cszLine[0]==0 )
		{
			return	RST_OK;
		}
		// 找到何时的解析器
		for(size_t i=0;i<m_vectWorker.size();++i)
		{
			Worker	*pWorker		= m_vectWorker[i];
			if( pWorker->ItIsMyBegin(cszLine, nLen) )
			{
				m_pCurWorker		= pWorker;
				return	RST_OK;
			}
		}
		// 没有找到合适的。跳过吧(原来是返回错误，不过其实没必要)
		return	RST_OK;
	}
	if( m_pCurWorker->ItIsMyEnd(cszLine, nLen) )
	{
		// 解析到部分结尾。重新开始。
		m_pCurWorker	= NULL;
		return	RST_OK;
	}
	return	m_pCurWorker->ParseLine(cszLine, nLen);;
}
// 自己用的
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
