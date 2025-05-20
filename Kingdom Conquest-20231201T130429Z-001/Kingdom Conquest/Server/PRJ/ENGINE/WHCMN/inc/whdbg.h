// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbg.h
// Creator      : Wei Hua (魏华)
// Comment      : 调试相关的功能
// CreationDate : 2003-05-10
// ChangeLOG    : 2006-01-04 增加了dbg_printmem打印到文件的功能

#ifndef	__WHDBG_H__
#define	__WHDBG_H__

#include "whvector.h"
#include "whtime.h"
// 这个文件应该是各个系统都有提供的啦
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 普通函数
////////////////////////////////////////////////////////////////////
// 输出到调试窗口
void	dbg_ouput_to_dbgwin(const char *szMsg);
// 弹出消息窗口
void	dbg_ouput_to_msgbox(const char *szTitle, const char *szMsg);

// 接在调试文件的后面
void	dbg_append_to_file(const char *szFile, const char *szMsg);

// 将一段内存内容打印到字串内(字串要保证足够大，如果szBuf为NULL则打印到屏幕)
// 输出格式为
// 地址1  xx xx xx xx xx .... xx   abdef...i
// 地址2  xx xx xx xx xx .... xx   abdef...i
// 具体例子和uedit中类似
// 00000000h: 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 ; XXXXXXXXXXXXXXXX
// 长度应该是77吧，不算最后的结束\r\n
const char *	dbg_printmem(const void *pMem, int nLen, char *szBuf = NULL);
// 和上面类似，不过是打印到vector中
const char *	dbg_printmem(const void *pMem, int nLen, whvector<char> &vect);
// 和上面类似，不过是打印到FILE文件中去
// 返回打印的行数
int		dbg_printmem(const void *pMem, int nLen, FILE *pout);
// 打印出16进制数组的形式
// nByteInLine表示每行的byte数
int		dbg_printmemashexarray(const void *pMem, int nLen, FILE *pout, int nByteInLine=16);

////////////////////////////////////////////////////////////////////
// 用时统计
////////////////////////////////////////////////////////////////////
class	dbgtimeuse
{
public:
	whvector<whtick_t>	m_vT;
private:
	whvector<char>		m_vStr;
	int		m_nCurIdx;
public:
	dbgtimeuse();
	void	BeginMark(int nIdx=0);
	void	Mark(int nIdx);
	void	Mark();
	char *	Disp(int nIdx0=0, int nNum=-1, char *szStr=NULL);
	int		GetDiff(int nIdx0=0, int nIdx1=-1);
};

#define	DBGTIMEUSE_DECLARE_EXTERN(var)		extern n_whcmn::dbgtimeuse	var
#define	DBGTIMEUSE_DECLARE(var)				n_whcmn::dbgtimeuse	var
#define	DBGTIMEUSE_MARKBEGIN(var, idx)		var.BeginMark(idx)
#define	DBGTIMEUSE_MARK(var)				var.Mark()
#define	DBGTIMEUSE_DISP(var)				var.Disp

DBGTIMEUSE_DECLARE_EXTERN(g_dtu);
#define	GTU_DBGTIMEUSE_MARKBEGIN(idx)		g_dtu.BeginMark(idx)
#define	GTU_DBGTIMEUSE_MARK()				g_dtu.Mark()
#define	GTU_DBGTIMEUSE_DISP					g_dtu.Disp

// 在需要统计的位置前后加宏
class	dbgtimeuse2
{
private:
	whvector<whtick_t>	m_vT;				// 其实时刻
	whvector<int>		m_vL;				// 时长
	whvector<char>		m_vStr;
public:
	dbgtimeuse2(int nTotal);
	void	Begin(int nIdx);
	void	End(int nIdx);
	char *	Disp(int nIdx0=0, int nNum=-1, char *szStr=NULL);
	int		GetDiff(int nIdx0=0, int nIdx1=-1);
};

#define	DBGTIMEUSE2_DECLARE_EXTERN(var)		extern n_whcmn::dbgtimeuse2	var
#define	DBGTIMEUSE2_DECLARE(var, num)		n_whcmn::dbgtimeuse2	var(num)
#define	DBGTIMEUSE2_BEGIN(var, idx)			var.Begin(idx)
#define	DBGTIMEUSE2_END(var, idx)			var.End(idx)
#define	DBGTIMEUSE2_DISP(var)				var.Disp
#define	DBGTIMEUSE2_GETDIFF(var)			var.GetDiff

// 模拟一次错误
void	whdbg_makebad(int nType);

}		// EOF namespace n_whcmn

#endif	// EOF __WHDBG_H__
