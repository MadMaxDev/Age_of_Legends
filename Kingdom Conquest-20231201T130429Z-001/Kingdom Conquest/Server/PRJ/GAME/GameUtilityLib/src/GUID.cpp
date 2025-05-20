/********************************************************
Copyright 1996-2006, Pixel Software Corporation, 
All rights reserved.

Module name:	Create GUID(Global Unique ID)

Purpose:		ID for Player, Item, Pet and so on.

Author:			Liuyi

Compiler:		Microsoft Visual C++ 7.1

History:		2006-1-10

Remarks:		2006-1-16	64位统一编码变为63位

*********************************************************/

#include "../inc/GUID.h"

// 全局数据
static WORD		g_wAreaID		= 0;
static WORD		g_wServerID		= 0;
static QWORD	g_qwCurCount	= 0;
static BYTE		g_bReserve		= 0;

// 目的：	初始化生成器
// 参数：	区域ID（全球的区域ID不重复），服务器ID（区域内服务器ID不重复），当前计数器数值（服务器传来的）
// 返回值：	无
bool	Init_GUID(WORD wAreaID,WORD wServerID,QWORD qwCurCount)
{
	g_wAreaID		= wAreaID;
	g_wServerID		= wServerID;
	g_qwCurCount	= qwCurCount;

	return true;
}

// 目的：	生成惟一ID
// 参数：	无
// 返回值：	63位的惟一ID
QWORD	Create_GUID()
{
	// 63		62~54		53~49		48		47~0 
	// 不用		区域ID		服务器ID	保留	递增的ID
	QWORD qwGUID = (QWORD(g_wAreaID) << 54) | (QWORD(g_wServerID) << 49) | (QWORD(g_bReserve) << 48) | g_qwCurCount;
	g_qwCurCount++;
	return qwGUID;
}

