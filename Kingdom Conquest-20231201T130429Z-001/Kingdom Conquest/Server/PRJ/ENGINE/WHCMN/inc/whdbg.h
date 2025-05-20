// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbg.h
// Creator      : Wei Hua (κ��)
// Comment      : ������صĹ���
// CreationDate : 2003-05-10
// ChangeLOG    : 2006-01-04 ������dbg_printmem��ӡ���ļ��Ĺ���

#ifndef	__WHDBG_H__
#define	__WHDBG_H__

#include "whvector.h"
#include "whtime.h"
// ����ļ�Ӧ���Ǹ���ϵͳ�����ṩ����
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// ��ͨ����
////////////////////////////////////////////////////////////////////
// ��������Դ���
void	dbg_ouput_to_dbgwin(const char *szMsg);
// ������Ϣ����
void	dbg_ouput_to_msgbox(const char *szTitle, const char *szMsg);

// ���ڵ����ļ��ĺ���
void	dbg_append_to_file(const char *szFile, const char *szMsg);

// ��һ���ڴ����ݴ�ӡ���ִ���(�ִ�Ҫ��֤�㹻�����szBufΪNULL���ӡ����Ļ)
// �����ʽΪ
// ��ַ1  xx xx xx xx xx .... xx   abdef...i
// ��ַ2  xx xx xx xx xx .... xx   abdef...i
// �������Ӻ�uedit������
// 00000000h: 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 ; XXXXXXXXXXXXXXXX
// ����Ӧ����77�ɣ��������Ľ���\r\n
const char *	dbg_printmem(const void *pMem, int nLen, char *szBuf = NULL);
// ���������ƣ������Ǵ�ӡ��vector��
const char *	dbg_printmem(const void *pMem, int nLen, whvector<char> &vect);
// ���������ƣ������Ǵ�ӡ��FILE�ļ���ȥ
// ���ش�ӡ������
int		dbg_printmem(const void *pMem, int nLen, FILE *pout);
// ��ӡ��16�����������ʽ
// nByteInLine��ʾÿ�е�byte��
int		dbg_printmemashexarray(const void *pMem, int nLen, FILE *pout, int nByteInLine=16);

////////////////////////////////////////////////////////////////////
// ��ʱͳ��
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

// ����Ҫͳ�Ƶ�λ��ǰ��Ӻ�
class	dbgtimeuse2
{
private:
	whvector<whtick_t>	m_vT;				// ��ʵʱ��
	whvector<int>		m_vL;				// ʱ��
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

// ģ��һ�δ���
void	whdbg_makebad(int nType);

}		// EOF namespace n_whcmn

#endif	// EOF __WHDBG_H__
