// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbugfind.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 查找程序出错的位置
//              : 注意：只能单线程用!!!!
// CreationDate : 2005-04-30

#include "../inc/whbugfind.h"
#include "../inc/whtime.h"
#include "../inc/whstring.h"

using namespace n_whcmn;

whvector<whbugfind::UNIT_T, false, whcmnallocationobj, 64>	whbugfind::ms_Units;

whbugfind::whbugfind(const char *cszFile, int nLine, const char *cszFunc)
{
	UNIT_T	*pUnit	= ms_Units.push_back();
	pUnit->cszFile	= cszFile;
	pUnit->nLine	= nLine;
	pUnit->cszFunc	= cszFunc;
}
whbugfind::~whbugfind()
{
	ms_Units.pop_back();
}
void	whbugfind::SetLine(int nLine)
{
	ms_Units.getlast().nLine	= nLine;
}
// 打印调用栈
void	whbugfind::PrintToFile(const char *cszFile, const char *cszMode)
{
	size_t	nSize	= ms_Units.size();
	FILE	*fp		= fopen(cszFile, cszMode);
	if( !fp )
	{
		return;
	}
	fprintf(fp, "**** Call Stack Dump begin [%s] ****%s", wh_gettimestr(), WHLINEEND);
	for(size_t i=0;i<nSize;i++)
	{
		const char	*cszFunc	= "[NULL]";
		if( ms_Units[i].cszFunc )
		{
			cszFunc	= ms_Units[i].cszFunc;
		}
		fprintf(fp, "%s : %d  %s%s"
			, ms_Units[i].cszFile
			, ms_Units[i].nLine
			, cszFunc
			, WHLINEEND);
	}
	fprintf(fp, "**** Call Stack Dump END ****%s%s%s", WHLINEEND, WHLINEEND, WHLINEEND);
	fclose(fp);
}
