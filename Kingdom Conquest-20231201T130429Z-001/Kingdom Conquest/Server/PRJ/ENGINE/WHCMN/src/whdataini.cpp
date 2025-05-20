// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataini.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于whdataprop的ini文件访问
// CreationDate : 2003-11-08
// ChangeLOG    : 2005-03-23 whdataini::analyzefile修改为可以多层附加后缀文件的
//              : 2006-06-20 多层附加后缀文件的功能取消
//              : 2007-01-15 修正了#define内容为空导致保留了上一个define值的问题
//              : 2007-07-05 修改了当等号行的值的位置为空会被误认为是段的开始而被忽略的问题。
//              : 2008-06-26 增加上了WIN32和LINUX（__GNUC__下）的系统宏定义

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
		// 如果是[xxx]的格式，就是对象开始的标志，为了简便我就只判断第一个了
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
			// 从pHost中查找__str对应的宏结果
			pInfo->str.resize(__len+1);
			memcpy(pInfo->str.getbuf(), __str, __len);
			pInfo->str[__len]	= 0;
			whdataini::cpplineanalyzer::MAP_T::iterator	it	= pInfo->pHost->m_mapMACRO.find(pInfo->str.getbuf());
			if( pInfo->pHost->m_mapMACRO.end() == it )
			{
				// 找不到key值
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
			// 字串结束
			pInfo->vectBuf.push_back('\0');
		}
		break;
	}
	return	0;
}
whdataini::cpplineanalyzer::cpplineanalyzer()
{
	m_vectIF.reserve(16);
	// 设置系统默认的宏定义（这样就可以直接在配置文件中配置LINUX下和WINDOWS下不同的配置了）
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
	// 按空格分解
	m_szParam[0][0]	= 0;
	m_szParam[1][0]	= 0;
	rst		= wh_strsplit("sa", cszLine, "", m_szParam[1], m_szParam[0]);
	if( m_szParam[1][0]==0 )
	{
		// 空行
		return	TYPE_NOTHING;
	}
	if( m_szParam[1][0]=='/' && m_szParam[1][1]=='/' )
	{
		// 注释行
		return	TYPE_COMMENT;
	}
	// 先看是否是#else或者#endif这样会终结ifdef/ifndef块的东西
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
				// 不会有其他情况的
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
	// 判断现在是否需要忽略
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
	// ifdef和ifndef如果后面跟多项。&&表示后的是全部要满足，||表示后面至少有一个满足
	// 看看是否是ifdef开始
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
					// 判断宏定义项是否存在
					if( m_mapMACRO.find(szKey)!=m_mapMACRO.end() )
					{
						// 有一个存在就行
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
					// 判断宏定义项是否存在
					if( m_mapMACRO.find(szKey)==m_mapMACRO.end() )
					{
						// 有一个不存在就行
						bYes	= false;
						break;
					}
				}
			}
			else
			{
				// 判断宏定义项是否存在
				if( m_mapMACRO.find(m_szParam[0])!=m_mapMACRO.end() )
				{
					bYes	= true;
				}
			}
			// 判断宏定义项是否存在
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
					// 判断宏定义项是否存在
					if( m_mapMACRO.find(szKey)==m_mapMACRO.end() )
					{
						// 有一个存在就行
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
					// 判断宏定义项是否全存在
					if( m_mapMACRO.find(szKey)!=m_mapMACRO.end() )
					{
						// 有一个存在就跳过
						bYes	= false;
						break;
					}
				}
			}
			else
			{
				// 判断宏定义项是否不存在
				if( m_mapMACRO.find(m_szParam[0])==m_mapMACRO.end() )
				{
					bYes	= true;
				}
			}
			// 判断宏定义项是否不存在
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
	// 看是否下面的行需要被忽略
	if( bIgnore )
	{
		return	TYPE_IGNORE;
	}
	// 进行宏替换(如果本行中有$(xxx)样子的东西的话)
	CB_INFO_T			info;
	info.pHost			= this;
	if( wh_strmacroprocess(cszLine, _str_mp_cb, &info)<0 )
	{
		return	TYPE_BADLINE;
	}
	// 判断如果中间有“//”就把后面的注释去掉（即单行内的注释）
	char	*pComment	= strstr(info.vectBuf.getbuf(), "//");
	if( pComment )
	{
		*pComment	= 0;
	}
	cszLine		= info.vectBuf.getbuf();
	// 看是否是include
	if( strcmp(m_szParam[1], "#include")==0 )
	{
		// 这时m_szParam[0]里面就是相应的文件名了（而且应该是trim过的字串）
		return	TYPE_INCLUDE;
	}
	// 看是否是宏定义
	if( strcmp(m_szParam[1], "#define")==0 )
	{
		m_szParam[0][0]	= 0;
		m_szParam[1][0]	= 0;
		rst		= wh_strsplit("*sa", cszLine, "", m_szParam[0], m_szParam[1]);
		if( m_szParam[0][0] == 0 )
		{
			// define后面必须跟东西的
			return	TYPE_BADLINE;
		}
		// 这样后面的定义可以冲掉前面的定义
		m_mapMACRO[m_szParam[0]]	= m_szParam[1];
		return	TYPE_MACRODEFINE;
	}
	// 进行后面的分析
	if( strchr(cszLine,'=')==NULL )
	{
		// 如果是"{"的格式，就是对象开始的标志
		if( cszLine[0]=='{' )
		{
			if( cszLine[1]==0 )
			{
				// 把存储的上一行作为对象名
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
			// 因为不是空行，所以可能是类名，存储起来
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
				// 完全跳过了
				m_bSkip	= false;
				return	0;
			}
			break;
		case	lineanalyzer::TYPE_BADLINE:
			m_errno	= ERRNO_BADLINE;
			return	-1;
			break;
		}
		// 继续跳过
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
				// 是相对路径，那么就需要根据当前文件名算出真是的相对或绝对路径
				// 基准路径，从当前ini文件得到
				char	szBasePath[WH_MAX_PATH];
				whfile_getfilepath(m_ssFNames.getlaststr(), szBasePath);
				// 最终的真实路径
				char	szRealPath[WH_MAX_PATH]="";
				whfile_makerealpath(szBasePath, m_analyzer->m_szParam[0], szRealPath);
				// 一般直接分析文件对象的就不应该有这个了，否则必须是FileMan能打开的。
				return	analyzefile(szRealPath, false, nReloadMask);
			}
			break;
		case	lineanalyzer::TYPE_OBJENDBEGIN:
			{
				// 先结束上个同级对象(如果有的话)
				if( m_ObjStack.size()>0 )
				{
					m_ObjStack.top()->end();
					m_ObjStack.pop();
				}
				// 然后再通过TYPE_OBJBEGIN的内容创建新对象
			}
		case	lineanalyzer::TYPE_OBJBEGIN:
			{
				m_nLevel	++;
				obj *pObj = NULL;
				// 获得相应的对象
				if( m_ObjStack.size()>0 )
				{
					// 这个不是在根部的对象，那么就是栈顶对象的子对象
					obj *pTop = m_ObjStack.top();
					pObj	= (obj *)pTop->findvalueptr(m_analyzer->m_szParam[0]);
				}
				else
				{
					// 这个是根部的对象，从getter中获取
					pObj	= m_getter->getobj(m_analyzer->m_szParam[0]);
				}
				if( !pObj )
				{
					m_nLevel	--;
					if( m_nLevel < m_nIgnoreBadObjLevel )
					{
						// 这个对象不是我关心的，跳过去
						m_bSkip			= true;
						m_nSkipLevel	= 1;
						return	0;
					}
					else
					{
						// 报错
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
					// 如果括号配对就不会出现这样的情况了
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
						if( !m_bIgnoreSetValueErr )	// 这种错一般都是由于key不认识造成的，为了避免由于其他人修改配置文件导致的不认识的key，所以加上这个。比如用于商城文件
						{
							m_errno	= ERRNO_BADLINE;
							return	-13;
						}
					}
				}
				else
				{
					// 如果当前没有对象就忽略（这个主要是为了在老配置文件中加上新内容，过渡时期的需要啊）
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
		// 不用配置
		return	0;
	}

	if( m_setFNames.has(cszFile) )
	{
		// 说明文件被循环include了
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

	// 加到监控列表中
	m_mfcd.AddFile(cszFile, 0);

	int	rst	= analyzefile(fp, bReset, nReloadMask);
	delete	fp;

	// 原来有附加文件，现在增加了include就不用附加文件了
	if( rst==0 )
	{
		// 只有成功了才把当前文件弹出
		m_setFNames.erase(m_ssFNames.pop());
	}
	return	rst;
}
// 如果考虑include功能，那么可能还需要考虑在包中的文件读取功能。所以可能需要有一个系统文件管理器接口。
int	whdataini::analyzefile(whfile *file, bool bReset, unsigned int nReloadMask)
{
	if( bReset )
	{
		m_ObjStack.clear();
		m_analyzer->reset();
	}
	// 开始分析
	int	line	= 0;
	while( !file->IsEOF() )
	{
		line	++;
		if( file->ReadLine(m_szCurLine, sizeof(m_szCurLine)-1)>0 )
		{
			wh_strtrim(m_szCurLine);
			m_errline	= line;	// 先在这里保存错误行号，一旦出错这个数就不会变了
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
