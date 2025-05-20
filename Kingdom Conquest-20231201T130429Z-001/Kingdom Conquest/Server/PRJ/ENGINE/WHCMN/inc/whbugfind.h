// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbugfind.h
// Creator      : Wei Hua (魏华)
// Comment      : 查找程序出错的位置
//              : 注意：只能单线程用!!!!
// CreationDate : 2005-04-30

#ifndef	__WHBUGFIND_H__
#define	__WHBUGFIND_H__

#include "whvector.h"

namespace n_whcmn
{

class	whbugfind
{
private:
	struct	UNIT_T
	{
		const char	*cszFile;
		int			nLine;
		const char	*cszFunc;
	};
	static whvector<UNIT_T, false, whcmnallocationobj, 64>	ms_Units;
public:
	whbugfind(const char *cszFile, int nLine, const char *cszFunc);
	~whbugfind();
	void	SetLine(int nLine);
	// 打印调用栈
	static void	PrintToFile(const char *cszFile, const char *cszMode="wt");
};

}		// EOF namespace n_whcmn

#define	WHBUGFIND_DECLEAR0(func)		whbugfind	local_BF(__FILE__, __LINE__, #func);
#define	WHBUGFIND_DECLEAR()				whbugfind	local_BF(__FILE__, __LINE__, NULL);
#define	WHBUGFIND_SETLINE()				local_BF.SetLine(__LINE__);
#define	WHBUGFIND_PRINTTOFILE(file)		whbugfind::PrintToFile(file);

#endif	// EOF __WHBUGFIND_H__
