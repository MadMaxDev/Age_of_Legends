// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlog.h
// Creator      : Wei Hua (魏华)
// Comment      : 日志文件相关的功能
// CreationDate : 2004-02-20
// ChangeLOG    : 2007-04-18 修改了一旦超过nLethalPageSize就一定新建文件
//              : 取消了文件序号m_nFIdx。因为在重启glogger程序的时候，继续原来的序号会比较麻烦。
//              : 取消nLethalPageSize，OFOD保持原来的设置

#ifndef	__WHLOG_H__
#define	__WHLOG_H__

#include "whfile.h"
#include "whstring.h"
#include "whtime.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 简单的（打开/append/关闭）模式
////////////////////////////////////////////////////////////////////
int	whlog_writefmt(const char *cszLogFile, const char *cszFmt, ...);

////////////////////////////////////////////////////////////////////
// 日志文件书写器
////////////////////////////////////////////////////////////////////
class	whlogwriter
{
public:
	enum
	{
		MAXMSGLEN				= 32768,					// 最长的一行信息，也是最长的单一二进制消息的长度
	};
	struct	INFO_T
	{
		enum
		{
			OPENMODE_TEXT		= 0x0000,					// 文本写入
			OPENMODE_BINARY		= 0x0001,					// 纯二进制写入
		};
		enum
		{
			PROP_FNAME_DATE		= 0x0001,					// 文件名中是否有日期
			PROP_FNAME_TIME		= 0x0002,					// 文件名中是否有当前小时
			PROP_FNAME_EACHDAY	= 0x0004,					// 如果两天交替就新打开一个文件
			PROP_FNAME_OFOD		= 0x0008,					// 一天一个文件(有了这个就没有文件后面的index了)
															// 这个也相当于PROP_FNAME_EACHDAY起作用，并且PROP_FNAME_TIME不起作用
															// 同时也相当于PROP_FNAME_DATE起作用
			PROP_LINE_TIME		= 0x0010,					// 在每行增加日期和时间
			PROP_LINE_SMPTIME	= 0x0020,					// 在每行增加简单的日期和时间
			PROP_LINE_MS		= 0x0040,					// 在每行增加毫秒数
			PROP_LINE_QUICKFLUSH	= 0x0080,				// 每次写一行都flush一下
			PROP_LINE_AUTORN	= 0x0100,					// 日志行是否自动附加\r\n结尾，这个只对文本写入有效
			PROP_PRINTINSCREEN	= 0x1000,					// 是否同时在Console屏幕打印(这个对于主程序是daemon模式应该没有结果)
			PROP_LOGOPENCLOSE	= 0x2000,					// 在打开关闭文件的时候多打一行log

			PROP_DEFAULT		= PROP_FNAME_DATE
								| PROP_FNAME_TIME
								| PROP_FNAME_EACHDAY
								| PROP_LINE_SMPTIME
								| PROP_LINE_AUTORN
								,							// 默认的属性集和
			PROP_SOHU_DEFAULT	= PROP_FNAME_OFOD
								| PROP_LINE_SMPTIME
								| PROP_LINE_AUTORN
								,							// 给运营商的默认配置
		};
		char	szLogFilePrefix[WH_MAX_PATH];				// 文件名前缀(可以含路径)
		char	szLogFileSuffix[16];						// 文件最后缀
		int		nOpenMode;									// 文件打开模式
		int		nProp;										// 属性
		int		nCloseTimeOut;								// 这么长时间(毫秒)收不到信息就关闭文件
		int		nFileStartIdx;								// 文件的起始序号(主要为了中途中止logger继续启动使用新的序号)
		int		nMaxPageSize;								// 如果是文本文件就代表最多这么多行
															// 如果是二进制文件就代表最多写这个多Bytes
															// 超过了这么多行，并且关闭文件超时达到后，就需要关闭文件并重新取名
		int		nEachNHour;									// 每隔nEachNHour小时就生成一个新文件（这个只在PROP_FNAME_TIME有的情况下生效）

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
	INFO_T		m_info;										// 存储初始化参数
	char		m_szCurLogName[WH_MAX_PATH];				// 存放当前的日志文件名
															// 文件名的格式：prefix[date][hour]_idx_suffix
	char		m_szOpenMode[4];							// 打开文件的模式"at"或"ab"
	whlooper	m_writeto;									// 用于判断文件是否已经比较长时间没有写入操作了
	FILE		*m_fplog;									// 文件指针
	int			m_nCurPageSize;								// 当前已有的文件尺寸(无论是文本还是二进制都是表示字节数)
	int			m_nLastClosePageSize;						// 上次关闭文件时的尺寸
	int			m_nIdxOfDay;								// 每天的序号（主要用于强制在一天生成多个文件）
	int			m_nLastDay;									// 用于判断两天交替的时刻
	whdaytimecrontab	m_wdtc;								// 用于每天一个文件
	whdaytimelooper		m_wdtl;								// 用于每隔几个小时一个文件
public:
	whlogwriter();
	~whlogwriter();
	int		Init(INFO_T *pInfo);							// 初始化
	int		Release();										// 终结
	int		Tick();											// 逻辑运行一下
	int		WriteBin(const void *pData, size_t nSize);		// 写二进制数据，这个必须以二进制方式打开
	int		WriteLine(const char *szStr);					// 写入文本行，这个必须以文本方式打开
	int		WriteFmtLine(const char *szFmt, ...);			// 写入格式文本，这个必须以文本方式打开
	// 这个可以直接用来输出上层绝对控制的文字(比如可执行程序的文件名)
	int		RawPrintf(const char *szFmt, ...);				// 里面可能同时向屏幕输出
private:
	void	MakeCurLogName(bool bFirst);					// 生成当前的日志文件名，放在m_szCurLogName中
	int		OpenCurLog();									// 以指定模式打开当前文件(如果已经打开则不会重复打开)
	int		CloseCurLog();									// 关闭当前文件(如果已经关闭则不会重复关闭)
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHLOG_H__
