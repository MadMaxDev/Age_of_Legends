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

#ifndef GUID_H
#define GUID_H

#include "DMtype.h"
// Ŀ�ģ�	��ʼ��������
// ������	����ID��ȫ�������ID���ظ�����������ID�������ڷ�����ID���ظ�������ǰ��������ֵ�������������ģ�
// ����ֵ��	������ʼ���ɹ�
bool	Init_GUID(WORD wAreaID,WORD wServerID,QWORD qwCurCount);

// Ŀ�ģ�	����ΩһID
// ������	��
// ����ֵ��	63λ��ΩһID
QWORD	Create_GUID();

#endif
