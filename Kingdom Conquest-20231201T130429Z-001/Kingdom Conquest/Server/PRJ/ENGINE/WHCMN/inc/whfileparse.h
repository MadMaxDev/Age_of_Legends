// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfileparse.h
// Creator      : Wei Hua (魏华)
// Comment      : 文件内容通用解析功能
// CreationDate : 2004-05-28
// ChangeLOG    : 

#ifndef	__WHFILEPARSE_H__
#define	__WHFILEPARSE_H__

#include "whfile.h"
#include "whstring.h"

namespace n_whcmn
{

// 注释分析器接口(内部实现普通的shell脚本注释方式"//、#")
class	whfilecommentchecker
{
private:
	int		m_nCommentBeginNum;
public:
	whfilecommentchecker();
	virtual ~whfilecommentchecker()	{}
	virtual void	Reset();
	virtual bool	CheckComment(const char *cszLine);
};

// 解析器接口
class	whfileparser
{
protected:
	int		m_nErrorLine;											// 出错的行号
	char	m_szHeadOfErrorLine[256];								// 出错行的前面部分内容
	whfilecommentchecker	m_DftCommentChecker;
	whfilecommentchecker	*m_pCommentChecker;						// 识别注释(因为主持方法很可能是通用的)
public:
	// 
	enum
	{
		RST_OK							= 0,						// 成功
		RST_ERR_CANNOTPARSELINE			= -1,						// 无法解析当前行
	};
public:
	whfileparser()
	: m_nErrorLine(-1)
	, m_pCommentChecker(&m_DftCommentChecker)
	{
		m_szHeadOfErrorLine[0]	= 0;
	}
	inline void	SetErrorLine(int nLine)								// 这个一般是外界设置的
	{
		m_nErrorLine	= nLine;
	}
	inline int	GetErrorLine() const
	{
		return	m_nErrorLine;
	}
	inline void	SetHeadOfErrorLine(const char *cszLine)
	{
		WH_STRNCPY0(m_szHeadOfErrorLine, cszLine);
	}
	inline const char *	GetHeadOfErrorLine() const
	{
		return	m_szHeadOfErrorLine;
	}
	inline void	SetCommentChecker(whfilecommentchecker *pCC)
	{
		m_pCommentChecker	= pCC;
	}
public:
	virtual ~whfileparser()	{}
	// 重新开始
	virtual void	Reset()											= 0;
	// 解析一行
	// 返回值为RST_XXX
	virtual int		ParseLine(const char *cszLine, int nLen)		= 0;
};

// 多部分解析(以XXX_BEGIN开头XXX_END结尾)
class	whfileparser_MULTIPART	: public whfileparser
{
public:
	virtual void	Reset();										// 这个Reset需要在AddWorker都结束后调用才能完全有效
	virtual	int		ParseLine(const char *cszLine, int nLen);
public:
	// 具体每部分的解析者接口
	class	Worker
	{
	public:
		whfileparser_MULTIPART	*m_pHost;							// 附着的whfileparser_MULTIPART的指针，可以它的方法，如SetLastError
																	// 调用whfileparser_MULTIPART::AddWorker的时候会设置
	public:
		virtual			~Worker()									{}
		virtual bool	SelfTakeCareOfComment() const				{ return false; }
																	// 表示在自己范围内的注释是否让自己解释，一般都不用
		virtual	bool	ItIsMyBegin(const char *cszLine, int nLen)	= 0;
																	// 只要找到了标明这是开头的数据即可，不用深入解析
		virtual	bool	ItIsMyEnd(const char *cszLine, int nLen)	= 0;
																	// 只要找到了标明这是结尾的数据即可，不用深入解析
		virtual	void	Reset()										= 0;
		virtual	int		ParseLine(const char *cszLine, int nLen)	= 0;
																	// 返回值为whfileparser::RST_XXX
	};
	// 用首尾字符串分隔的部分
	// 如：
	// Server_Begin
	// ...
	// Server_End
	// 本类的基本功能就是跳过内容
	class	Worker_BeginEndWithString	: public Worker
	{
	private:
		const char	*m_cszBegin;
		const char	*m_cszEnd;
	public:
		Worker_BeginEndWithString(const char *cszBegin ,const char *cszEnd)
		: m_cszBegin(cszBegin), m_cszEnd(cszEnd)
		{
		}
		virtual	bool	ItIsMyBegin(const char *cszLine, int nLen)
		{
			return	strcmp(cszLine, m_cszBegin) == 0;
		}
		virtual	bool	ItIsMyEnd(const char *cszLine, int nLen)
		{
			return	strcmp(cszLine, m_cszEnd) == 0;
		}
		virtual	void	Reset()
		{
		}
		virtual	int		ParseLine(const char *cszLine, int nLen)
		{
			return	0;
		}
	};
private:
	whvector<Worker *>	m_vectWorker;
	Worker	*m_pCurWorker;
public:
	whfileparser_MULTIPART();
	void	AddWorker(Worker *pWorker);
};

// 函数中会循序读出每行的内容，trim之，然后交给分析器处理
// 上层如果要提供pszLineBuf，则必须保证它足够大
// 返回值为whfileparser::RST_XXX
int		whfileparse(whfile *file, whfileparser *pParser, char *pszLineBuf=NULL, int nBufSize=0);

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILEPARSE_H__
