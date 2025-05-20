// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whfileparse.h
// Creator      : Wei Hua (κ��)
// Comment      : �ļ�����ͨ�ý�������
// CreationDate : 2004-05-28
// ChangeLOG    : 

#ifndef	__WHFILEPARSE_H__
#define	__WHFILEPARSE_H__

#include "whfile.h"
#include "whstring.h"

namespace n_whcmn
{

// ע�ͷ������ӿ�(�ڲ�ʵ����ͨ��shell�ű�ע�ͷ�ʽ"//��#")
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

// �������ӿ�
class	whfileparser
{
protected:
	int		m_nErrorLine;											// ������к�
	char	m_szHeadOfErrorLine[256];								// �����е�ǰ�沿������
	whfilecommentchecker	m_DftCommentChecker;
	whfilecommentchecker	*m_pCommentChecker;						// ʶ��ע��(��Ϊ���ַ����ܿ�����ͨ�õ�)
public:
	// 
	enum
	{
		RST_OK							= 0,						// �ɹ�
		RST_ERR_CANNOTPARSELINE			= -1,						// �޷�������ǰ��
	};
public:
	whfileparser()
	: m_nErrorLine(-1)
	, m_pCommentChecker(&m_DftCommentChecker)
	{
		m_szHeadOfErrorLine[0]	= 0;
	}
	inline void	SetErrorLine(int nLine)								// ���һ����������õ�
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
	// ���¿�ʼ
	virtual void	Reset()											= 0;
	// ����һ��
	// ����ֵΪRST_XXX
	virtual int		ParseLine(const char *cszLine, int nLen)		= 0;
};

// �ಿ�ֽ���(��XXX_BEGIN��ͷXXX_END��β)
class	whfileparser_MULTIPART	: public whfileparser
{
public:
	virtual void	Reset();										// ���Reset��Ҫ��AddWorker����������ò�����ȫ��Ч
	virtual	int		ParseLine(const char *cszLine, int nLen);
public:
	// ����ÿ���ֵĽ����߽ӿ�
	class	Worker
	{
	public:
		whfileparser_MULTIPART	*m_pHost;							// ���ŵ�whfileparser_MULTIPART��ָ�룬�������ķ�������SetLastError
																	// ����whfileparser_MULTIPART::AddWorker��ʱ�������
	public:
		virtual			~Worker()									{}
		virtual bool	SelfTakeCareOfComment() const				{ return false; }
																	// ��ʾ���Լ���Χ�ڵ�ע���Ƿ����Լ����ͣ�һ�㶼����
		virtual	bool	ItIsMyBegin(const char *cszLine, int nLen)	= 0;
																	// ֻҪ�ҵ��˱������ǿ�ͷ�����ݼ��ɣ������������
		virtual	bool	ItIsMyEnd(const char *cszLine, int nLen)	= 0;
																	// ֻҪ�ҵ��˱������ǽ�β�����ݼ��ɣ������������
		virtual	void	Reset()										= 0;
		virtual	int		ParseLine(const char *cszLine, int nLen)	= 0;
																	// ����ֵΪwhfileparser::RST_XXX
	};
	// ����β�ַ����ָ��Ĳ���
	// �磺
	// Server_Begin
	// ...
	// Server_End
	// ����Ļ������ܾ�����������
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

// �����л�ѭ�����ÿ�е����ݣ�trim֮��Ȼ�󽻸�����������
// �ϲ����Ҫ�ṩpszLineBuf������뱣֤���㹻��
// ����ֵΪwhfileparser::RST_XXX
int		whfileparse(whfile *file, whfileparser *pParser, char *pszLineBuf=NULL, int nBufSize=0);

}		// EOF namespace n_whcmn

#endif	// EOF __WHFILEPARSE_H__
