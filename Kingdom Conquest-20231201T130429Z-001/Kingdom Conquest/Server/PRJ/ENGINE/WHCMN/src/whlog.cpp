// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlog.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 日志文件相关的功能
// CreationDate : 2004-02-20
// ChangeLog    :
//                2004-09-23 修正了whlogwriter::RawPrintf中对空文件指针的判断
//                2006-02-14 whlogwriter::~whlogwriter中加入了Release

#include "../inc/whlog.h"
#include "../inc/whtime.h"
#include "../inc/whdir.h"
#include <stdarg.h>

namespace n_whcmn
{

int		whlog_writefmt(const char *cszLogFile, const char *cszFmt, ...)
{
	char	szMsg[4096];
	va_list	arglist;
	va_start( arglist, cszFmt );
	vsprintf( szMsg, cszFmt, arglist );
	va_end( arglist );

	FILE	*fp;
	fp		= fopen(cszLogFile, "at");
	if( !fp )
	{
		return	-1;
	}
	fprintf(fp, szMsg);
	fclose(fp);

	return	0;
}

}

using namespace n_whcmn;

whlogwriter::whlogwriter()
: m_fplog(NULL)
, m_nCurPageSize(0)
, m_nLastClosePageSize(0)
, m_nIdxOfDay(0)
, m_nLastDay(0)
{
	m_szCurLogName[0]	= 0;
	strcpy(m_szOpenMode, "at");
	m_wdtc.SetDays(1);
	m_wdtc.settimeofday(0, true);
}
whlogwriter::~whlogwriter()
{
	Release();
}
int		whlogwriter::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_nCurPageSize			= 0;
	m_nLastClosePageSize	= 0;
	m_writeto.setinterval(m_info.nCloseTimeOut);

	m_wdtl.setinterval(m_info.nEachNHour*3600);

	// 确保目标目录存在
	whdir_SureMakeDirForFile(m_info.szLogFilePrefix);
	// 开初始的文件
	MakeCurLogName(true);
	// 以指定模式打开当前文件
	switch( m_info.nOpenMode )
	{
		case	INFO_T::OPENMODE_TEXT:
			strcpy(m_szOpenMode, "at");
		break;
		case	INFO_T::OPENMODE_BINARY:
			strcpy(m_szOpenMode, "ab");
		break;
	}
	if( OpenCurLog()<0 )
	{
		return	-1;
	}
	return	0;
}
int		whlogwriter::Release()
{
	// 如果文件没有关闭则关闭之。
	CloseCurLog();
	return	0;
}
int		whlogwriter::Tick()
{
	bool	bTimeOut		= m_writeto.check();
	bool	bShouldClose	= false;
	bool	bNewFile		= false;

	// 首先进行关闭判断
	if( bTimeOut )
	{
		if( m_nCurPageSize != m_nLastClosePageSize )
		{
			// 有新的信息写入，应当关闭
			bShouldClose	= true;
			m_writeto.reset();
		}
	}
	// 只有不是PROP_FNAME_OFOD时才判断文件长度是否过长，是否应该生成新文件
	if( (m_info.nProp & INFO_T::PROP_FNAME_OFOD) == 0
	&&  (m_info.nMaxPageSize>0 && m_nCurPageSize >= m_info.nMaxPageSize)
	)
	{
		bShouldClose	= true;
		bNewFile		= true;
		m_nIdxOfDay		++;
	}

	if( (m_info.nProp & (INFO_T::PROP_FNAME_EACHDAY | INFO_T::PROP_FNAME_OFOD)) )
	{
		if( m_wdtc.check() )
		{
			bShouldClose	= true;
			bNewFile		= true;
			m_nIdxOfDay		= 0;
			m_wdtc.gonext();
		}
	}
	if( m_info.nProp & INFO_T::PROP_FNAME_TIME )
	{
		// 要在文件名中显示时间
		if( m_wdtl.check() )
		{
			bShouldClose	= true;
			bNewFile		= true;
			m_nIdxOfDay		= 0;
			m_wdtl.gonext();
		}
	}

	// 如果前面需要，就关闭文件
	if( bShouldClose )
	{
		CloseCurLog();

		// 序号递增判断 (这个一定包括超过Lethal的状况了)
		if( bNewFile )
		{
			// 生成新文件名
			MakeCurLogName(false);
			// 重置本页尺寸
			m_nCurPageSize			= 0;
			// 这个也要清空，否则就会导致下次循环过来再次关闭/打开文件
			m_nLastClosePageSize	= 0;
		}

		OpenCurLog();
	}

	return	0;
}
int		whlogwriter::WriteBin(const void *pData, size_t nSize)
{
	if( m_info.nOpenMode != INFO_T::OPENMODE_BINARY
	||  !m_fplog
	)
	{
		return	-1;
	}
	nSize				= fwrite(pData, 1, nSize, m_fplog);
	m_nCurPageSize		+= nSize;
	m_writeto.reset();
	return	0;
}
int		whlogwriter::WriteLine(const char *szStr)
{
	if( m_info.nOpenMode != INFO_T::OPENMODE_TEXT
	||  !m_fplog
	)
	{
		return	-1;
	}

	// 这两个是不能同时出现的
	if( m_info.nProp & INFO_T::PROP_LINE_SMPTIME )
	{
		// 2005-08-31 应sohu要求把时间长度缩短到只有日
		RawPrintf("%s", wh_getsmptimestr(wh_time())+6);
	}
	else if( m_info.nProp & INFO_T::PROP_LINE_TIME )
	{
		RawPrintf("%s", wh_gettimestr(wh_time()));
	}
	if( m_info.nProp & INFO_T::PROP_LINE_MS )
	{
		RawPrintf(":%010lu,", wh_gettickcount());
	}
	else
	{
		RawPrintf(",");
	}


	RawPrintf("%s", szStr);
	if( m_info.nProp & INFO_T::PROP_LINE_AUTORN )
	{
		// 判断如果最后不是\r\n就添加
		int	nLen	= strlen(szStr);
		// 原来是nLen>=2，地址长度为1的log加不上回车
		if( nLen>=1
		&&  (szStr[nLen-1]!='\r' && szStr[nLen-1]!='\n')
		)
		{
			RawPrintf(WHLINEEND);
		}
	}
	m_nCurPageSize	= ftell(m_fplog);
	m_writeto.reset();
	return	0;
}
int		whlogwriter::WriteFmtLine(const char *szFmt, ...)
{
	char	szMsg[MAXMSGLEN];
	va_list	arglist;
	va_start( arglist, szFmt );
	vsprintf( szMsg, szFmt, arglist );
	va_end( arglist );

	return	WriteLine(szMsg);
}

void	whlogwriter::MakeCurLogName(bool bFirst)
{
	// 文件名的格式：prefix[date][hour]_idx_suffix
	strcpy(m_szCurLogName, m_info.szLogFilePrefix);
	// 获取当前时间
	time_t		t		= wh_time();
	struct tm	*ptm	= localtime(&t);

	if( m_nLastDay != ptm->tm_mday )
	{
		m_nLastDay	= ptm->tm_mday;
		m_nIdxOfDay	= 0;
	}
	if( m_info.nProp & (whlogwriter::INFO_T::PROP_FNAME_OFOD|whlogwriter::INFO_T::PROP_FNAME_DATE) )
	{
		sprintf(m_szCurLogName+strlen(m_szCurLogName),  "%04d%02d%02d"
			, 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday
			);
	}
	// 如果有PROP_FNAME_OFOD，则PROP_FNAME_TIME失效，且不用在后面加index
	if( ((m_info.nProp & whlogwriter::INFO_T::PROP_FNAME_OFOD) == 0) )
	{
		if( m_info.nProp & whlogwriter::INFO_T::PROP_FNAME_TIME )
		{
			if( m_nIdxOfDay==0 )
			{
				sprintf(m_szCurLogName+strlen(m_szCurLogName), "[%02d]"
					, ptm->tm_hour
					);
			}
			else
			{
				sprintf(m_szCurLogName+strlen(m_szCurLogName), "[%02d]-%02d"
					, ptm->tm_hour
					, m_nIdxOfDay
					);
			}
		}
		else
		{
			if( bFirst )
			{
				for(;;)
				{
					char	szTmp[WH_MAX_PATH];
					sprintf(szTmp, "%s-%04d", m_szCurLogName, m_nIdxOfDay);
					if( !whfile_ispathexisted(szTmp) )
					{
						if( m_nIdxOfDay>0 )
						{
							m_nIdxOfDay	--;
						}
						break;
					}
					m_nIdxOfDay	++;
				}
			}
			// 一天多个文件且没有时间标记，则需要加文件标号
			sprintf(m_szCurLogName+strlen(m_szCurLogName), "-%04d", m_nIdxOfDay);
			m_nIdxOfDay	++;
		}
	}
	// 到了这一步就只需要增加后缀了
	strcat(m_szCurLogName, m_info.szLogFileSuffix);
}
int		whlogwriter::OpenCurLog()
{
	// 如果已经打开了就不用打开第二次
	if( m_fplog )
	{
		return	0;
	}
	// 记录当前的日子
	int	nPrevDay	= wh_gettoday();
	// 打开文件
	m_fplog		= fopen(m_szCurLogName, m_szOpenMode);
	if( !m_fplog )
	{
		return	-1;
	}
	if( nPrevDay<=0 )
	{
		RawPrintf("%s%s**** Log Open [day:%d IS INVALID!] ***%s"
			, WHLINEEND, WHLINEEND
			, nPrevDay
			, WHLINEEND
			);
	}
	else
	{
		if( m_info.nProp & INFO_T::PROP_LOGOPENCLOSE )
		{
			RawPrintf("%s%s**** Log Open [day:%d] ***%s"
				, WHLINEEND, WHLINEEND
				, nPrevDay
				, WHLINEEND
				);
		}
	}
	return		0;
}
int		whlogwriter::CloseCurLog()
{
	if( m_fplog )
	{
		fclose(m_fplog);
		m_fplog	= NULL;
		// 记录这次关闭时
		m_nLastClosePageSize	= m_nCurPageSize;
	}
	return		0;
}
int		whlogwriter::RawPrintf(const char *szFmt, ...)
{
	char	szMsg[MAXMSGLEN];
	va_list	arglist;
	va_start( arglist, szFmt );
	vsprintf( szMsg, szFmt, arglist );
	va_end( arglist );

	if( m_info.nProp & INFO_T::PROP_PRINTINSCREEN )
	{
		// puts 会在最后加换行符（注意：fputs不会在最后加换行的）
		fputs(szMsg, stdout);
	}
	if( m_fplog )
	{
		fputs(szMsg, m_fplog);
		if( m_info.nProp & INFO_T::PROP_LINE_QUICKFLUSH )
		{
			fflush(m_fplog);
		}
	}
	else
	{
		printf("%s m_fplog==NULL! msg:%s%s", wh_gettimestr(), szMsg, WHLINEEND);
		// 重新打开一次
		OpenCurLog();
	}

	return		0;
}
