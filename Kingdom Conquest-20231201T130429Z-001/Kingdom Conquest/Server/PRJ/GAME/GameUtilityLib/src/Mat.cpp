/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.

	Module name: mat

	Purpose: һЩ��ѧ����

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 2004-11-27 

	Remarks: 2006-4-19 Erica - �����Bit��־λ�Ĵ���

*********************************************************/


#include   "../inc/Mat.h"


// ����Ĩȥ��ҪSet��Bits
const BYTE aBitMask[8] = {0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};
const BYTE aDBitMask[4] = {0xFC,0xF3,0xCF,0x3F};
const BYTE aQBitMask[2] = {0xF0,0x0F};

BYTE Int2QBit( int iData )
{
	if ( iData < 0 )
	{
		iData = 0;
	}
	else if ( iData > QBIT_MAX )
	{
		iData = QBIT_MAX;
	}
	return (BYTE)iData;
}

BYTE Int2Byte( int iData )
{
	if ( iData < 0 )
	{
		iData = 0;
	}
	else if ( iData > BYTE_MAX )
	{
		iData = BYTE_MAX;
	}
	return (BYTE)iData;
}

WORD Int2Word( int iData )
{
	if ( iData < 0 )
	{
		iData = 0;
	}
	else if ( iData > WORD_MAX )
	{
		iData = WORD_MAX;
	}
	return (WORD)iData;
}

int SQWord2Int( SQWORD qwData )
{
	if ( qwData < INT_MIN )
	{
		qwData = INT_MIN;
	}
	else if ( qwData > INT_MAX )
	{
		qwData = INT_MAX;
	}
	return (int)qwData;
}

DWORD SQWord2DWord( SQWORD qwData )
{
	if ( qwData < 0 )
	{
		qwData = 0;
	}
	else if ( qwData > DWORD_MAX )
	{
		qwData = DWORD_MAX;
	}
	return (DWORD)qwData;
}

bool SetBitToByte( int idx, BYTE & byNewData, BYTE byOldData )
{
	idx &= 0x07;

	if ( byNewData )
	{// ����ԭ����
		byNewData <<= idx; // �Ƶ���ȷλ��
		byNewData |= byOldData & aBitMask[idx];
	}
	else
	{// ���ԭ����
		byNewData = byOldData & aBitMask[idx];
	}
	return (byNewData != byOldData);
}

bool SetDBitToByte( int idx, BYTE & byNewData, BYTE byOldData )
{
	idx &= 0x03;

	if ( byNewData )
	{// ����ԭ����
		byNewData <<= idx*2; // �Ƶ���ȷλ��
		byNewData |= byOldData & aDBitMask[idx];
	}
	else
	{// ���ԭ����
		byNewData = byOldData & aDBitMask[idx];
	}
	return (byNewData != byOldData);
}

bool SetQBitToByte( int idx, BYTE & byNewData, BYTE byOldData )
{
	idx &= 0x01;

	if ( byNewData )
	{// ����ԭ����
		byNewData <<= idx*4; // �Ƶ���ȷλ��
		byNewData |= byOldData & aQBitMask[idx];
	}
	else
	{// ���ԭ����
		byNewData = byOldData & aQBitMask[idx];
	}
	return (byNewData != byOldData);
}

bool ChgQBitToByte( int idx, int & iNewData, BYTE byOldData )
{
	idx &= 0x01;

	iNewData += ( byOldData>>idx*4 ) & 0x0F; // ����ԭ����

	BYTE byNewData = Int2QBit(iNewData);
	
	if ( byNewData )
	{// ����ԭ����
		byNewData <<= idx*4; // �Ƶ���ȷλ��
		byNewData |= byOldData & aQBitMask[idx];
	}
	else
	{// ���ԭ����
		byNewData = byOldData & aQBitMask[idx];
	}
	return (byNewData != byOldData);
}

// ȫ�ֱ�������RandF�����õ�
int g_iRandF = 7654321;
