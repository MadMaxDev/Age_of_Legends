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

#ifndef GUID_H
#define GUID_H

#include "DMtype.h"
// 目的：	初始化生成器
// 参数：	区域ID（全球的区域ID不重复），服务器ID（区域内服务器ID不重复），当前计数器数值（服务器传来的）
// 返回值：	真代表初始化成功
bool	Init_GUID(WORD wAreaID,WORD wServerID,QWORD qwCurCount);

// 目的：	生成惟一ID
// 参数：	无
// 返回值：	63位的惟一ID
QWORD	Create_GUID();

#endif
