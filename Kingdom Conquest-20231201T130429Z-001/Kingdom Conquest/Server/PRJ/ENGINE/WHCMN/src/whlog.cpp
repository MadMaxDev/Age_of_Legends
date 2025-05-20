// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlog.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��־�ļ���صĹ���
// CreationDate : 2004-02-20
// ChangeLog    :
//                2004-09-23 ������whlogwriter::RawPrintf�жԿ��ļ�ָ����ж�
//                2006-02-14 whlogwriter::~whlogwriter�м�����Release

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

	// ȷ��Ŀ��Ŀ¼����
	whdir_SureMakeDirForFile(m_info.szLogFilePrefix);
	// ����ʼ���ļ�
	MakeCurLogName(true);
	// ��ָ��ģʽ�򿪵�ǰ�ļ�
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
	// ����ļ�û�йر���ر�֮��
	CloseCurLog();
	return	0;
}
int		whlogwriter::Tick()
{
	bool	bTimeOut		= m_writeto.check();
	bool	bShouldClose	= false;
	bool	bNewFile		= false;

	// ���Ƚ��йر��ж�
	if( bTimeOut )
	{
		if( m_nCurPageSize != m_nLastClosePageSize )
		{
			// ���µ���Ϣд�룬Ӧ���ر�
			bShouldClose	= true;
			m_writeto.reset();
		}
	}
	// ֻ�в���PROP_FNAME_OFODʱ���ж��ļ������Ƿ�������Ƿ�Ӧ���������ļ�
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
		// Ҫ���ļ�������ʾʱ��
		if( m_wdtl.check() )
		{
			bShouldClose	= true;
			bNewFile		= true;
			m_nIdxOfDay		= 0;
			m_wdtl.gonext();
		}
	}

	// ���ǰ����Ҫ���͹ر��ļ�
	if( bShouldClose )
	{
		CloseCurLog();

		// ��ŵ����ж� (���һ����������Lethal��״����)
		if( bNewFile )
		{
			// �������ļ���
			MakeCurLogName(false);
			// ���ñ�ҳ�ߴ�
			m_nCurPageSize			= 0;
			// ���ҲҪ��գ�����ͻᵼ���´�ѭ�������ٴιر�/���ļ�
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

	// �������ǲ���ͬʱ���ֵ�
	if( m_info.nProp & INFO_T::PROP_LINE_SMPTIME )
	{
		// 2005-08-31 ӦsohuҪ���ʱ�䳤�����̵�ֻ����
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
		// �ж���������\r\n�����
		int	nLen	= strlen(szStr);
		// ԭ����nLen>=2����ַ����Ϊ1��log�Ӳ��ϻس�
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
	// �ļ����ĸ�ʽ��prefix[date][hour]_idx_suffix
	strcpy(m_szCurLogName, m_info.szLogFilePrefix);
	// ��ȡ��ǰʱ��
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
	// �����PROP_FNAME_OFOD����PROP_FNAME_TIMEʧЧ���Ҳ����ں����index
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
			// һ�����ļ���û��ʱ���ǣ�����Ҫ���ļ����
			sprintf(m_szCurLogName+strlen(m_szCurLogName), "-%04d", m_nIdxOfDay);
			m_nIdxOfDay	++;
		}
	}
	// ������һ����ֻ��Ҫ���Ӻ�׺��
	strcat(m_szCurLogName, m_info.szLogFileSuffix);
}
int		whlogwriter::OpenCurLog()
{
	// ����Ѿ����˾Ͳ��ô򿪵ڶ���
	if( m_fplog )
	{
		return	0;
	}
	// ��¼��ǰ������
	int	nPrevDay	= wh_gettoday();
	// ���ļ�
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
		// ��¼��ιر�ʱ
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
		// puts �������ӻ��з���ע�⣺fputs���������ӻ��еģ�
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
		// ���´�һ��
		OpenCurLog();
	}

	return		0;
}
