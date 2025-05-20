// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgoodstr.h
// Creator      : Wei Hua (魏华)
// Comment      : 主要用于判断字串是否合法
// CreationDate : 2007-07-11 Transformer国内首映，(*^__^*) 嘻嘻……
// ChangeLog    : 

#ifndef	__WHGOODSTR_H__
#define	__WHGOODSTR_H__

#include "whfile_base.h"

namespace n_whcmn
{

// 判断单个字符的合法性
class	whgoodchar
{
protected:
	whgoodchar()	{}
public:
	static whgoodchar *	Create();
	virtual ~whgoodchar()	{}
	virtual	void	SelfDestroy()											= 0;
	// 文件由一行一行的字符组成（每行长度不能超过4096字符），回车结尾。
	struct	INIT_RST_T
	{
		int			nErrLine;			// 如果出错这里表示是那行出错
		int			nTotal;				// 总共的字符数量
		int			nFitFactor;			// 最终返回hash表的合理程度，最佳10000，最差0
		int			nDupCount;			// 有重复字的计数（发现在big5里面可能不同的字转成wchar结果是相同的，比如）

		INIT_RST_T()
		{
			WHMEMSET0THIS();
		}
	};
	virtual int		Init(const char *cszCharFile, const char *cszCharSet, INIT_RST_T *pRst)	= 0;
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)				= 0;
	virtual int		Release()												= 0;
	// 判断cszStr中是否都是允许的字符
	virtual bool	IsGood(const char *cszStr)								= 0;
	// 过滤不好的字符，返回过略的个数，把不对的字符过滤为cReplace
	virtual int		FilterStr(char *szStr, char cReplace)					= 0;
};

// 判断整词的非法性（因为词句的变化太多了，所以不能给出一个好的集合，只能给出不好的集合）
// 内部用md5代表字串，可能错杀，不会不杀
class	whbadword
{
protected:
	whbadword()	{}
public:
	static whbadword *	Create();
	virtual ~whbadword()	{}
	virtual	void	SelfDestroy()											= 0;
	// 文件由一行一行的字符组成（每行长度不能超过4096字符），回车结尾。
	struct	INIT_RST_T
	{
		int			nErrLine;			// 如果出错这里表示是那行出错
		int			nTotal;				// 总共的词数量
		int			nTotalEffective;	// 总共效字词数量
		int			nTotalChar;			// 总有效字符数
		int			nFitFactor;			// 最终返回hash表的合理程度，最佳10000，最差0
	};
	virtual int		Init(const char *cszBadWordFile, const char *cszCharSet, INIT_RST_T *pRst)
																	= 0;
	virtual int		Init(whfile *fp, const char *cszCharSet, INIT_RST_T *pRst)
																	= 0;
	virtual int		Release()										= 0;
	// 判断cszStr中是否含有非法的词
	virtual bool	IsBad(const char *cszStr)						= 0;
	// 把坏词过滤为指定的字符代替，返回true表示里面有非法词汇szDst中为过滤后的字串，返回false的话表示没有非法词汇，szDst内部不填写，上层可以直接使用cszSrc作为结果（内部可以少一次拷贝）
	virtual bool	FilterBad(const char *cszSrc, char *szDst, int nDstSize, char cX='*')
																	= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHGOODSTR_H__
