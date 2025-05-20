/********************************************************
Copyright 1996-2006, Pixel Software Corporation, 
All rights reserved.

Module name:	Create GUID(Global Unique ID)

Purpose:		ID for Player, Item, Pet and so on.

Author:			Liuyi

Compiler:		Microsoft Visual C++ 7.1

History:		2006-1-10

Remarks:		2006-1-16	64λͳһ�����Ϊ63λ

*********************************************************/

#include "../inc/GUID.h"

// ȫ������
static WORD		g_wAreaID		= 0;
static WORD		g_wServerID		= 0;
static QWORD	g_qwCurCount	= 0;
static BYTE		g_bReserve		= 0;

// Ŀ�ģ�	��ʼ��������
// ������	����ID��ȫ�������ID���ظ�����������ID�������ڷ�����ID���ظ�������ǰ��������ֵ�������������ģ�
// ����ֵ��	��
bool	Init_GUID(WORD wAreaID,WORD wServerID,QWORD qwCurCount)
{
	g_wAreaID		= wAreaID;
	g_wServerID		= wServerID;
	g_qwCurCount	= qwCurCount;

	return true;
}

// Ŀ�ģ�	����ΩһID
// ������	��
// ����ֵ��	63λ��ΩһID
QWORD	Create_GUID()
{
	// 63		62~54		53~49		48		47~0 
	// ����		����ID		������ID	����	������ID
	QWORD qwGUID = (QWORD(g_wAreaID) << 54) | (QWORD(g_wServerID) << 49) | (QWORD(g_bReserve) << 48) | g_qwCurCount;
	g_qwCurCount++;
	return qwGUID;
}

