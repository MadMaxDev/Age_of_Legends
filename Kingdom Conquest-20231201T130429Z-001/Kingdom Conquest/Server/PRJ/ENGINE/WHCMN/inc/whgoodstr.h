// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgoodstr.h
// Creator      : Wei Hua (κ��)
// Comment      : ��Ҫ�����ж��ִ��Ƿ�Ϸ�
// CreationDate : 2007-07-11 Transformer������ӳ��(*^__^*) ��������
// ChangeLog    : 

#ifndef	__WHGOODSTR_H__
#define	__WHGOODSTR_H__

#include "whfile_base.h"

namespace n_whcmn
{

// �жϵ����ַ��ĺϷ���
class	whgoodchar
{
protected:
	whgoodchar()	{}
public:
	static whgoodchar *	Create();
	virtual ~whgoodchar()	{}
	virtual	void	SelfDestroy()											= 0;
	// �ļ���һ��һ�е��ַ���ɣ�ÿ�г��Ȳ��ܳ���4096�ַ������س���β��
	struct	INIT_RST_T
	{
		int			nErrLine;			// ������������ʾ�����г���
		int			nTotal;				// �ܹ����ַ�����
		int			nFitFactor;			// ���շ���hash��ĺ���̶ȣ����10000�����0
		int			nDupCount;			// ���ظ��ֵļ�����������big5������ܲ�ͬ����ת��wchar�������ͬ�ģ����磩

		INIT_RST_T()
		{
			WHMEMSET0THIS();
		}
	};
	virtual int		Init(const char *cszCharFile, const char *cszCharSet, INIT_RST_T *pRst)	= 0;
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)				= 0;
	virtual int		Release()												= 0;
	// �ж�cszStr���Ƿ���������ַ�
	virtual bool	IsGood(const char *cszStr)								= 0;
	// ���˲��õ��ַ������ع��Եĸ������Ѳ��Ե��ַ�����ΪcReplace
	virtual int		FilterStr(char *szStr, char cReplace)					= 0;
};

// �ж����ʵķǷ��ԣ���Ϊ�ʾ�ı仯̫���ˣ����Բ��ܸ���һ���õļ��ϣ�ֻ�ܸ������õļ��ϣ�
// �ڲ���md5�����ִ������ܴ�ɱ�����᲻ɱ
class	whbadword
{
protected:
	whbadword()	{}
public:
	static whbadword *	Create();
	virtual ~whbadword()	{}
	virtual	void	SelfDestroy()											= 0;
	// �ļ���һ��һ�е��ַ���ɣ�ÿ�г��Ȳ��ܳ���4096�ַ������س���β��
	struct	INIT_RST_T
	{
		int			nErrLine;			// ������������ʾ�����г���
		int			nTotal;				// �ܹ��Ĵ�����
		int			nTotalEffective;	// �ܹ�Ч�ִ�����
		int			nTotalChar;			// ����Ч�ַ���
		int			nFitFactor;			// ���շ���hash��ĺ���̶ȣ����10000�����0
	};
	virtual int		Init(const char *cszBadWordFile, const char *cszCharSet, INIT_RST_T *pRst)
																	= 0;
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)
																	= 0;
	virtual int		Release()										= 0;
	// �ж�cszStr���Ƿ��зǷ��Ĵ�
	virtual bool	IsBad(const char *cszStr)						= 0;
	// �ѻ��ʹ���Ϊָ�����ַ����棬����true��ʾ�����зǷ��ʻ�szDst��Ϊ���˺���ִ�������false�Ļ���ʾû�зǷ��ʻ㣬szDst�ڲ�����д���ϲ����ֱ��ʹ��cszSrc��Ϊ������ڲ�������һ�ο�����
	virtual bool	FilterBad(const char *cszSrc, char *szDst, int nDstSize, char cX='*')
																	= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHGOODSTR_H__
