// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlog.h
// Creator      : Wei Hua (κ��)
// Comment      : ��־�ļ���صĹ���
// CreationDate : 2004-02-20
// ChangeLOG    : 2007-04-18 �޸���һ������nLethalPageSize��һ���½��ļ�
//              : ȡ�����ļ����m_nFIdx����Ϊ������glogger�����ʱ�򣬼���ԭ������Ż�Ƚ��鷳��
//              : ȡ��nLethalPageSize��OFOD����ԭ��������

#ifndef	__WHLOG_H__
#define	__WHLOG_H__

#include "whfile.h"
#include "whstring.h"
#include "whtime.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �򵥵ģ���/append/�رգ�ģʽ
////////////////////////////////////////////////////////////////////
int	whlog_writefmt(const char *cszLogFile, const char *cszFmt, ...);

////////////////////////////////////////////////////////////////////
// ��־�ļ���д��
////////////////////////////////////////////////////////////////////
class	whlogwriter
{
public:
	enum
	{
		MAXMSGLEN				= 32768,					// ���һ����Ϣ��Ҳ����ĵ�һ��������Ϣ�ĳ���
	};
	struct	INFO_T
	{
		enum
		{
			OPENMODE_TEXT		= 0x0000,					// �ı�д��
			OPENMODE_BINARY		= 0x0001,					// ��������д��
		};
		enum
		{
			PROP_FNAME_DATE		= 0x0001,					// �ļ������Ƿ�������
			PROP_FNAME_TIME		= 0x0002,					// �ļ������Ƿ��е�ǰСʱ
			PROP_FNAME_EACHDAY	= 0x0004,					// ������콻����´�һ���ļ�
			PROP_FNAME_OFOD		= 0x0008,					// һ��һ���ļ�(���������û���ļ������index��)
															// ���Ҳ�൱��PROP_FNAME_EACHDAY�����ã�����PROP_FNAME_TIME��������
															// ͬʱҲ�൱��PROP_FNAME_DATE������
			PROP_LINE_TIME		= 0x0010,					// ��ÿ���������ں�ʱ��
			PROP_LINE_SMPTIME	= 0x0020,					// ��ÿ�����Ӽ򵥵����ں�ʱ��
			PROP_LINE_MS		= 0x0040,					// ��ÿ�����Ӻ�����
			PROP_LINE_QUICKFLUSH	= 0x0080,				// ÿ��дһ�ж�flushһ��
			PROP_LINE_AUTORN	= 0x0100,					// ��־���Ƿ��Զ�����\r\n��β�����ֻ���ı�д����Ч
			PROP_PRINTINSCREEN	= 0x1000,					// �Ƿ�ͬʱ��Console��Ļ��ӡ(���������������daemonģʽӦ��û�н��)
			PROP_LOGOPENCLOSE	= 0x2000,					// �ڴ򿪹ر��ļ���ʱ����һ��log

			PROP_DEFAULT		= PROP_FNAME_DATE
								| PROP_FNAME_TIME
								| PROP_FNAME_EACHDAY
								| PROP_LINE_SMPTIME
								| PROP_LINE_AUTORN
								,							// Ĭ�ϵ����Լ���
			PROP_SOHU_DEFAULT	= PROP_FNAME_OFOD
								| PROP_LINE_SMPTIME
								| PROP_LINE_AUTORN
								,							// ����Ӫ�̵�Ĭ������
		};
		char	szLogFilePrefix[WH_MAX_PATH];				// �ļ���ǰ׺(���Ժ�·��)
		char	szLogFileSuffix[16];						// �ļ����׺
		int		nOpenMode;									// �ļ���ģʽ
		int		nProp;										// ����
		int		nCloseTimeOut;								// ��ô��ʱ��(����)�ղ�����Ϣ�͹ر��ļ�
		int		nFileStartIdx;								// �ļ�����ʼ���(��ҪΪ����;��ֹlogger��������ʹ���µ����)
		int		nMaxPageSize;								// ������ı��ļ��ʹ��������ô����
															// ����Ƕ������ļ��ʹ������д�����Bytes
															// ��������ô���У����ҹر��ļ���ʱ�ﵽ�󣬾���Ҫ�ر��ļ�������ȡ��
		int		nEachNHour;									// ÿ��nEachNHourСʱ������һ�����ļ������ֻ��PROP_FNAME_TIME�е��������Ч��

		INFO_T()
		: nOpenMode(OPENMODE_TEXT)
		, nProp(PROP_SOHU_DEFAULT)
		, nCloseTimeOut(1000)
		, nFileStartIdx(0)
		, nMaxPageSize(0)
		, nEachNHour(1)
		{
			strcpy(szLogFilePrefix, "log");
			strcpy(szLogFileSuffix, ".txt");
		}
	};
private:
	INFO_T		m_info;										// �洢��ʼ������
	char		m_szCurLogName[WH_MAX_PATH];				// ��ŵ�ǰ����־�ļ���
															// �ļ����ĸ�ʽ��prefix[date][hour]_idx_suffix
	char		m_szOpenMode[4];							// ���ļ���ģʽ"at"��"ab"
	whlooper	m_writeto;									// �����ж��ļ��Ƿ��Ѿ��Ƚϳ�ʱ��û��д�������
	FILE		*m_fplog;									// �ļ�ָ��
	int			m_nCurPageSize;								// ��ǰ���е��ļ��ߴ�(�������ı����Ƕ����ƶ��Ǳ�ʾ�ֽ���)
	int			m_nLastClosePageSize;						// �ϴιر��ļ�ʱ�ĳߴ�
	int			m_nIdxOfDay;								// ÿ�����ţ���Ҫ����ǿ����һ�����ɶ���ļ���
	int			m_nLastDay;									// �����ж����콻���ʱ��
	whdaytimecrontab	m_wdtc;								// ����ÿ��һ���ļ�
	whdaytimelooper		m_wdtl;								// ����ÿ������Сʱһ���ļ�
public:
	whlogwriter();
	~whlogwriter();
	int		Init(INFO_T *pInfo);							// ��ʼ��
	int		Release();										// �ս�
	int		Tick();											// �߼�����һ��
	int		WriteBin(const void *pData, size_t nSize);		// д���������ݣ���������Զ����Ʒ�ʽ��
	int		WriteLine(const char *szStr);					// д���ı��У�����������ı���ʽ��
	int		WriteFmtLine(const char *szFmt, ...);			// д���ʽ�ı�������������ı���ʽ��
	// �������ֱ����������ϲ���Կ��Ƶ�����(�����ִ�г�����ļ���)
	int		RawPrintf(const char *szFmt, ...);				// �������ͬʱ����Ļ���
private:
	void	MakeCurLogName(bool bFirst);					// ���ɵ�ǰ����־�ļ���������m_szCurLogName��
	int		OpenCurLog();									// ��ָ��ģʽ�򿪵�ǰ�ļ�(����Ѿ����򲻻��ظ���)
	int		CloseCurLog();									// �رյ�ǰ�ļ�(����Ѿ��ر��򲻻��ظ��ر�)
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHLOG_H__
