// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataini.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ����whdataprop��ini�ļ�����
// CreationDate : 2003-11-08
// ChangeLOG    : 2005-03-23 whdataini::analyzefile�޸�Ϊ���Զ�㸽�Ӻ�׺�ļ���
//              : 2006-06-20 ��㸽�Ӻ�׺�ļ��Ĺ���ȡ��
//              : 2007-01-15 ������#define����Ϊ�յ��±�������һ��defineֵ������
//              : 2007-07-05 �޸��˵��Ⱥ��е�ֵ��λ��Ϊ�ջᱻ����Ϊ�ǶεĿ�ʼ�������Ե����⡣
//              : 2008-06-26 ��������WIN32��LINUX��__GNUC__�£���ϵͳ�궨��

#include "../inc/whdataini.h"
#include "../inc/whstring.h"
#include "../inc/whfile_util.h"
#include <assert.h>

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whdataini::objgetter
////////////////////////////////////////////////////////////////////
whdataini::objgetter::objgetter()
{
}
whdataini::objgetter::~objgetter()
{
}
int	whdataini::objgetter::addobj(const char *cszName, obj *pObj)
{
	assert(cszName);
	m_mapobj.insert(std::make_pair(cszName, pObj));
	return		0;
}
whdataini::obj *	whdataini::objgetter::getobj(const char *cszName)
{
	assert(cszName);
	MAP_T::iterator	it = m_mapobj.find(cszName);
	if( it!=m_mapobj.end() )
	{
		return	(*it).second;
	}
	return		NULL;
}

////////////////////////////////////////////////////////////////////
// whdataini::dftlineanalyzer
////////////////////////////////////////////////////////////////////
int		whdataini::dftlineanalyzer::analyze(const char *cszLine)
{
	if( cszLine[0]==';'
	||  cszLine[0]=='#'
	)
	{
		return	TYPE_COMMENT;
	}

	int		rst = wh_strsplit("sa", cszLine, "=", m_szParam[0], m_szParam[1]);
	if( rst==1 )
	{
		// �����[xxx]�ĸ�ʽ�����Ƕ���ʼ�ı�־��Ϊ�˼���Ҿ�ֻ�жϵ�һ����
		if( m_szParam[0][0]=='[' )
		{
			return	TYPE_OBJENDBEGIN;
		}
	}
	else if( rst==2 )
	{
		return		TYPE_VAL;
	}
	return			TYPE_NOTHING;
}

////////////////////////////////////////////////////////////////////
// whdataini::cpplineanalyzer
////////////////////////////////////////////////////////////////////
struct	CB_INFO_T
{
	whdataini::cpplineanalyzer	*pHost;
	whvector<char>				vectBuf;
	whvector<char>				str;
};
static int _str_mp_cb(int __type, const char *__str, int __len, void *__param)
{
	CB_INFO_T	*pInfo	= (CB_INFO_T *)__param;
	switch( __type )
	{
	case	WH_STR_MP_TYPE_CMN:
		{
			pInfo->vectBuf.push_back(__str, __len);
		}
		break;
	case	WH_STR_MP_TYPE_MACRO:
		{
			// ��pHost�в���__str��Ӧ�ĺ���
			pInfo->str.resize(__len+1);
			memcpy(pInfo->str.getbuf(), __str, __len);
			pInfo->str[__len]	= 0;
			whdataini::cpplineanalyzer::MAP_T::iterator	it	= pInfo->pHost->m_mapMACRO.find(pInfo->str.getbuf());
			if( pInfo->pHost->m_mapMACRO.end() == it )
			{
				// �Ҳ���keyֵ
                return	-1;
			}
            else
            {
                pInfo->vectBuf.push_back((*it).second.c_str(), (*it).second.length());
            }
		}
		break;
	case	WH_STR_MP_TYPE_END:
		{
			// �ִ�����
			pInfo->vectBuf.push_back('\0');
		}
		break;
	}
	return	0;
}
whdataini::cpplineanalyzer::cpplineanalyzer()
{
	m_vectIF.reserve(16);
	// ����ϵͳĬ�ϵĺ궨�壨�����Ϳ���ֱ���������ļ�������LINUX�º�WINDOWS�²�ͬ�������ˣ�
#ifdef	__GNUC__
	m_mapMACRO["LINUX"]		= "";
	m_mapMACRO["__GNUC__"]	= "";
	m_mapMACRO["HOME"]  	= getenv("HOME");
#endif
#ifdef	WIN32
	m_mapMACRO["WIN32"]		= "";
	m_mapMACRO["HOME"]  	= "/";
#endif
}
void	whdataini::cpplineanalyzer::reset()
{
	m_szLineTmp[0]	= 0;
	m_vectIF.clear();
}
int		whdataini::cpplineanalyzer::analyze(const char *cszLine)
{
	int		rst;
	// ���ո�ֽ�
	m_szParam[0][0]	= 0;
	m_szParam[1][0]	= 0;
	rst		= wh_strsplit("sa", cszLine, "", m_szParam[1], m_szParam[0]);
	if( m_szParam[1][0]==0 )
	{
		// ����
		return	TYPE_NOTHING;
	}
	if( m_szParam[1][0]=='/' && m_szParam[1][1]=='/' )
	{
		// ע����
		return	TYPE_COMMENT;
	}
	// �ȿ��Ƿ���#else����#endif�������ս�ifdef/ifndef��Ķ���
	if( strcmp(m_szParam[1], "#else")==0 )
	{
		if( m_vectIF.size()==0 )
		{
			return	TYPE_BADLINE;
		}
		switch( m_vectIF.getlast() )
		{
		case	IF_IFDEF_YES:
			{
				m_vectIF.getlast()	= IF_IFDEF_NO;
			}
			break;
		case	IF_IFDEF_IGNORE:
			{
				m_vectIF.getlast()	= IF_IFDEF_IGNORE;
			}
			break;
		case	IF_IFDEF_NO:
			{
				m_vectIF.getlast()	= IF_IFDEF_YES;
			}
			break;
		default:
			{
				// ���������������
				assert(0);
			}
			break;
		}
		return	TYPE_ELSE;
	}
	if( strcmp(m_szParam[1], "#endif")==0 )
	{
		m_vectIF.pop_back();
		return	TYPE_ENDIF;
	}
	// �ж������Ƿ���Ҫ����
	bool	bIgnore	= false;
	if( m_vectIF.size()>0 )
	{
		switch( m_vectIF.getlast() )
		{
		case	IF_IFDEF_NO:
		case	IF_IFDEF_IGNORE:
			{
				bIgnore	= true;
			}
			break;
		}
	}
	// ifdef��ifndef�����������&&��ʾ�����ȫ��Ҫ���㣬||��ʾ����������һ������
	// �����Ƿ���ifdef��ʼ
	if( strcmp(m_szParam[1], "#ifdef")==0 )
	{
		if( bIgnore )
		{
			m_vectIF.push_back(IF_IFDEF_IGNORE);
		}
		else
		{
			bool	bYes	= false;
			char	szKey[1024]	= "";
			int		nOffset	= 0;
			wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey);
			if( strcmp(szKey, "||")==0 )
			{
				while( wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey)>0 )
				{
					// �жϺ궨�����Ƿ����
					if( m_mapMACRO.find(szKey)!=m_mapMACRO.end() )
					{
						// ��һ�����ھ���
						bYes	= true;
						break;
					}
				}
			}
			else if( strcmp(szKey, "&&")==0 )
			{
				bYes		= true;
				while( wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey)>0 )
				{
					// �жϺ궨�����Ƿ����
					if( m_mapMACRO.find(szKey)==m_mapMACRO.end() )
					{
						// ��һ�������ھ���
						bYes	= false;
						break;
					}
				}
			}
			else
			{
				// �жϺ궨�����Ƿ����
				if( m_mapMACRO.find(m_szParam[0])!=m_mapMACRO.end() )
				{
					bYes	= true;
				}
			}
			// �жϺ궨�����Ƿ����
			if( bYes )
			{
				m_vectIF.push_back(IF_IFDEF_YES);
			}
			else
			{
				m_vectIF.push_back(IF_IFDEF_NO);
			}
		}
		return	TYPE_IFDEF;
	}
	if( strcmp(m_szParam[1], "#ifndef")==0 )
	{
		if( bIgnore )
		{
			m_vectIF.push_back(IF_IFDEF_IGNORE);
		}
		else
		{
			bool	bYes	= false;
			char	szKey[1024]	= "";
			int		nOffset	= 0;
			wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey);

			if( strcmp(szKey, "||")==0 )
			{
				while( wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey)>0 )
				{
					// �жϺ궨�����Ƿ����
					if( m_mapMACRO.find(szKey)==m_mapMACRO.end() )
					{
						// ��һ�����ھ���
						bYes	= true;
						break;
					}
				}
			}
			else if( strcmp(szKey, "&&")==0 )
			{
				bYes	= true;
				while( wh_strsplit(&nOffset, "s", m_szParam[0], "", szKey)>0 )
				{
					// �жϺ궨�����Ƿ�ȫ����
					if( m_mapMACRO.find(szKey)!=m_mapMACRO.end() )
					{
						// ��һ�����ھ�����
						bYes	= false;
						break;
					}
				}
			}
			else
			{
				// �жϺ궨�����Ƿ񲻴���
				if( m_mapMACRO.find(m_szParam[0])==m_mapMACRO.end() )
				{
					bYes	= true;
				}
			}
			// �жϺ궨�����Ƿ񲻴���
			if( bYes )
			{
				m_vectIF.push_back(IF_IFDEF_YES);
			}
			else
			{
				m_vectIF.push_back(IF_IFDEF_NO);
			}
		}
		return	TYPE_IFNDEF;
	}
	// ���Ƿ����������Ҫ������
	if( bIgnore )
	{
		return	TYPE_IGNORE;
	}
	// ���к��滻(�����������$(xxx)���ӵĶ����Ļ�)
	CB_INFO_T			info;
	info.pHost			= this;
	if( wh_strmacroprocess(cszLine, _str_mp_cb, &info)<0 )
	{
		return	TYPE_BADLINE;
	}
	// �ж�����м��С�//���ͰѺ����ע��ȥ�����������ڵ�ע�ͣ�
	char	*pComment	= strstr(info.vectBuf.getbuf(), "//");
	if( pComment )
	{
		*pComment	= 0;
	}
	cszLine		= info.vectBuf.getbuf();
	// ���Ƿ���include
	if( strcmp(m_szParam[1], "#include")==0 )
	{
		// ��ʱm_szParam[0]���������Ӧ���ļ����ˣ�����Ӧ����trim�����ִ���
		return	TYPE_INCLUDE;
	}
	// ���Ƿ��Ǻ궨��
	if( strcmp(m_szParam[1], "#define")==0 )
	{
		m_szParam[0][0]	= 0;
		m_szParam[1][0]	= 0;
		rst		= wh_strsplit("*sa", cszLine, "", m_szParam[0], m_szParam[1]);
		if( m_szParam[0][0] == 0 )
		{
			// define��������������
			return	TYPE_BADLINE;
		}
		// ��������Ķ�����Գ��ǰ��Ķ���
		m_mapMACRO[m_szParam[0]]	= m_szParam[1];
		return	TYPE_MACRODEFINE;
	}
	// ���к���ķ���
	if( strchr(cszLine,'=')==NULL )
	{
		// �����"{"�ĸ�ʽ�����Ƕ���ʼ�ı�־
		if( cszLine[0]=='{' )
		{
			if( cszLine[1]==0 )
			{
				// �Ѵ洢����һ����Ϊ������
				strcpy(m_szParam[0], m_szLineTmp);
				return	TYPE_OBJBEGIN;
			}
			return		TYPE_BADLINE;
		}
		else if( cszLine[0]=='}' )
		{
			if( cszLine[1]==0 )
			{
				return		TYPE_OBJEND;
			}
			return		TYPE_BADLINE;
		}
		else
		{
			// ��Ϊ���ǿ��У����Կ������������洢����
			strcpy(m_szLineTmp, cszLine);
		}
	}
	else
	{
		m_szParam[1][0]	= 0;
		wh_strsplit("sa", cszLine, "=", m_szParam[0], m_szParam[1]);
		return		TYPE_VAL;
	}
	return			TYPE_NOTHING;
}

////////////////////////////////////////////////////////////////////
// whdataini
////////////////////////////////////////////////////////////////////
whdataini::whdataini()
: m_nIgnoreBadObjLevel(1)
, m_bSkip(false), m_nSkipLevel(0), m_nLevel(0)
, m_errno(ERRNO_SUCCESS), m_errline(0)
, m_ssFNames('\0')
, m_getter(NULL), m_analyzer(NULL)
, m_bIgnoreSetValueErr(false)
{
	m_szCurLine[0]	= 0;
	m_pFileMan_dft	= whfileman_Cmn_Create();
	m_pFileMan		= m_pFileMan_dft;
}
whdataini::~whdataini()
{
	if( m_pFileMan_dft )
	{
		delete	m_pFileMan_dft;
	}
}
int	whdataini::setgetter(objgetter *getter)
{
	m_getter	= getter;
	return		0;
}
int	whdataini::setlineanalyzer(lineanalyzer *analyzer)
{
	m_analyzer	= analyzer;
	return		0;
}
int	whdataini::analyzeline(const char *cszLine, unsigned int nReloadMask)
{
	int		rst;
	rst		= m_analyzer->analyze(cszLine);
	if( m_bSkip )
	{
		switch(rst)
		{
		case	lineanalyzer::TYPE_OBJENDBEGIN:
			goto	normalwork;
			break;
		case	lineanalyzer::TYPE_OBJBEGIN:
			m_nSkipLevel	++;
			break;
		case	lineanalyzer::TYPE_OBJEND:
			m_nSkipLevel	--;
			assert(m_nSkipLevel>=0);
			if( m_nSkipLevel==0 )
			{
				// ��ȫ������
				m_bSkip	= false;
				return	0;
			}
			break;
		case	lineanalyzer::TYPE_BADLINE:
			m_errno	= ERRNO_BADLINE;
			return	-1;
			break;
		}
		// ��������
		return	0;
	}
	else
	{
normalwork:
		m_bSkip	= false;
		switch(rst)
		{
		case	lineanalyzer::TYPE_INCLUDE:
			{
				// �����·������ô����Ҫ���ݵ�ǰ�ļ���������ǵ���Ի����·��
				// ��׼·�����ӵ�ǰini�ļ��õ�
				char	szBasePath[WH_MAX_PATH];
				whfile_getfilepath(m_ssFNames.getlaststr(), szBasePath);
				// ���յ���ʵ·��
				char	szRealPath[WH_MAX_PATH]="";
				whfile_makerealpath(szBasePath, m_analyzer->m_szParam[0], szRealPath);
				// һ��ֱ�ӷ����ļ�����ľͲ�Ӧ��������ˣ����������FileMan�ܴ򿪵ġ�
				return	analyzefile(szRealPath, false, nReloadMask);
			}
			break;
		case	lineanalyzer::TYPE_OBJENDBEGIN:
			{
				// �Ƚ����ϸ�ͬ������(����еĻ�)
				if( m_ObjStack.size()>0 )
				{
					m_ObjStack.top()->end();
					m_ObjStack.pop();
				}
				// Ȼ����ͨ��TYPE_OBJBEGIN�����ݴ����¶���
			}
		case	lineanalyzer::TYPE_OBJBEGIN:
			{
				m_nLevel	++;
				obj *pObj = NULL;
				// �����Ӧ�Ķ���
				if( m_ObjStack.size()>0 )
				{
					// ��������ڸ����Ķ�����ô����ջ��������Ӷ���
					obj *pTop = m_ObjStack.top();
					pObj	= (obj *)pTop->findvalueptr(m_analyzer->m_szParam[0]);
				}
				else
				{
					// ����Ǹ����Ķ��󣬴�getter�л�ȡ
					pObj	= m_getter->getobj(m_analyzer->m_szParam[0]);
				}
				if( !pObj )
				{
					m_nLevel	--;
					if( m_nLevel < m_nIgnoreBadObjLevel )
					{
						// ����������ҹ��ĵģ�����ȥ
						m_bSkip			= true;
						m_nSkipLevel	= 1;
						return	0;
					}
					else
					{
						// ����
						m_errno	= ERRNO_BADOBJNAME;
						return	-11;
					}
				}
				pObj->begin();
				m_ObjStack.push(pObj);
			}
			break;
		case	lineanalyzer::TYPE_OBJEND:
			{
				m_nLevel	--;
				if( m_ObjStack.size()>0 )
				{
					m_ObjStack.top()->end();
					m_ObjStack.pop();
				}
				else
				{
					// ���������ԾͲ�����������������
					m_errno	= ERRNO_BADLINE;
					return	-12;
				}
			}
			break;
		case	lineanalyzer::TYPE_VAL:
			{
				obj *pObj	= m_ObjStack.top();
				if( pObj )
				{
					if( pObj->setvalue(m_analyzer->m_szParam[0], m_analyzer->m_szParam[1], nReloadMask)<0 )
					{
						if( !m_bIgnoreSetValueErr )	// ���ִ�һ�㶼������key����ʶ��ɵģ�Ϊ�˱��������������޸������ļ����µĲ���ʶ��key�����Լ�����������������̳��ļ�
						{
							m_errno	= ERRNO_BADLINE;
							return	-13;
						}
					}
				}
				else
				{
					// �����ǰû�ж���ͺ��ԣ������Ҫ��Ϊ�����������ļ��м��������ݣ�����ʱ�ڵ���Ҫ����
					//m_errno	= ERRNO_BADLINE;
					//return	-1;
				}
			}
			break;
		case	lineanalyzer::TYPE_BADLINE:
			m_errno	= ERRNO_BADLINE;
			return	-14;
			break;
		}
	}
	return		0;
}
int	whdataini::analyzefile(const char *cszFile, bool bReset, unsigned int nReloadMask)
{
	if( !cszFile || cszFile[0]==0 )
	{
		// ��������
		return	0;
	}

	if( m_setFNames.has(cszFile) )
	{
		// ˵���ļ���ѭ��include��
		m_errno	= ERRNO_DUPINCLUDE;
		return	-1;
	}

	if( bReset )
	{
		m_ssFNames.clear();
		m_setFNames.clear();
	}

	whfile	*fp;
	fp		= m_pFileMan->Open(cszFile, whfileman::OP_MODE_BIN_READONLY);
	if( !fp )
	{
		m_errno	= ERRNO_CANNOTOPENFILE;
		return	-2;
	}

	m_setFNames.put(m_ssFNames.push(cszFile));

	// �ӵ�����б���
	m_mfcd.AddFile(cszFile, 0);

	int	rst	= analyzefile(fp, bReset, nReloadMask);
	delete	fp;

	// ԭ���и����ļ�������������include�Ͳ��ø����ļ���
	if( rst==0 )
	{
		// ֻ�гɹ��˲Űѵ�ǰ�ļ�����
		m_setFNames.erase(m_ssFNames.pop());
	}
	return	rst;
}
// �������include���ܣ���ô���ܻ���Ҫ�����ڰ��е��ļ���ȡ���ܡ����Կ�����Ҫ��һ��ϵͳ�ļ��������ӿڡ�
int	whdataini::analyzefile(whfile *file, bool bReset, unsigned int nReloadMask)
{
	if( bReset )
	{
		m_ObjStack.clear();
		m_analyzer->reset();
	}
	// ��ʼ����
	int	line	= 0;
	while( !file->IsEOF() )
	{
		line	++;
		if( file->ReadLine(m_szCurLine, sizeof(m_szCurLine)-1)>0 )
		{
			wh_strtrim(m_szCurLine);
			m_errline	= line;	// �������ﱣ������кţ�һ������������Ͳ������
			int	rst	= analyzeline(m_szCurLine, nReloadMask);
			if( rst<0 )
			{
				return	rst;
			}
		}
		else
		{
			break;
		}
	}

	return		0;
}
int	whdataini::printincludestack(FILE *fpout)
{
	fprintf(fpout, "[");
	const char	*pFName;
	while( (pFName=m_ssFNames.pop()) != NULL )
	{
		fprintf(fpout, "%s<-", pFName);
	}
	fprintf(fpout, "]");
	return		0;
}
const char *	whdataini::printincludestack(char *pStrOut, int &nMaxSize)
{
	if( pStrOut == NULL )
	{
		static char	szStrOut[1024];
		pStrOut		= szStrOut;
		nMaxSize	= sizeof(szStrOut);
	}
	char		*pTmpOut	= pStrOut;
	int			rst;
	rst			= snprintf(pTmpOut, nMaxSize, "[");
	pTmpOut		+= rst;
	nMaxSize	-= rst;
	const char	*pFName;
	while( (pFName=m_ssFNames.pop()) != NULL )
	{
		rst	= snprintf(pTmpOut, nMaxSize, "%s<-", pFName);
		if( rst<=0 )
		{
			return	pStrOut;
		}
		pTmpOut		+= rst;
		nMaxSize	-= rst;
	}
	rst			= snprintf(pTmpOut, nMaxSize, "]");
	pTmpOut		+= rst;
	nMaxSize	-= rst;
	return		pStrOut;
}
int	whdataini::printerrreport(FILE *fpout)
{
	fprintf(fpout, "fileinclude:");
	printincludestack(fpout);
	fprintf(fpout, " errcode:%d line:%d content:%s", m_errno, m_errline, GetCurLine());
	return	0;
}
const char *	whdataini::printerrreport(char *pStrOut, int &nMaxSize)
{
	if( pStrOut == NULL )
	{
		static char	szStrOut[1024];
		pStrOut		= szStrOut;
		nMaxSize	= sizeof(szStrOut);
	}
	char		*pTmpOut	= pStrOut;
	int			rst;
	rst			= snprintf(pTmpOut, nMaxSize, "fileinclude:");
	pTmpOut		+= rst;
	nMaxSize	-= rst;
	int	nOldSize= nMaxSize;
	printincludestack(pTmpOut, nMaxSize);
	pTmpOut		+= nOldSize-nMaxSize;
	rst			= snprintf(pTmpOut, nMaxSize, " errcode:%d line:%d content:%s", m_errno, m_errline, GetCurLine());
	nMaxSize	-= rst;
	return		pStrOut;
}
