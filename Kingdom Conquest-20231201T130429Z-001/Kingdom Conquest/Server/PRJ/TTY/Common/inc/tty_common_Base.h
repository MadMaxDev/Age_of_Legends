// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : ��
// File         : tty_common_Base.h
// Creator      : Wei Hua (κ��)
// Comment      : ����tty_common_xxx.h����Ҫ������ͷ�ļ���������һЩ��������
// CreationDate : 2006-02-24
// Change LOG   : 2006-06-08 ��ȷָ����βΪ4�ֽڶ���
//              : 2008-06-06 �޸��û����ݰ汾�����������崵�

#ifndef	__TTY_COMMON_BASE_H__
#define	__TTY_COMMON_BASE_H__

#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whdataobj.h>
#include <GameUtilityLib/inc/DMtype.h>

using namespace n_whcmn;
typedef	whallocationobjwithonemem<>	TTY_AOONE_T;

// ������Ҫ�����ݿ�ͬ�������ݵİ汾���Ϳ�֧�ֵ���Ͱ汾��
enum
{
	TTY_DATA_VER	= 11,
	TTY_DATA_VERMIN	= 11,
};

// ��Ҫ�����б��еĽṹ����Ҫ������ṹ�̳�
// ����д��Ȼ���ܻ����һ�����鷳�����粻��ֱ���õȺŻ�memcpy�Խṹ��ֵ���и�ֵ����Ϊ���������nStatus���ı䣩��������Ϊ���������һ��ר�Ŵ淽Status��list����������������򵥵�������
#pragma pack(push, old_pack_num, 4)
struct	TTY_LISTUNIT_BASE_T
{
private:
	// ȡֵΪWHDATAOBJ_LISTTYPE_UNIT_STATUS_XXX��������whdataobj_def.h��
	unsigned char	nStatus;
public:
	TTY_LISTUNIT_BASE_T()
		: nStatus(0)
	{
	}
	inline unsigned char	GetStatus() const
	{
		return	nStatus;
	}
	inline unsigned char	GetStatusLow() const	// ���������Ҫ����Ϊ&�����ȼ��ȱȽ�Ҫ�ף����������������װһ�¾Ͳ���д��ô��������
	{
		return	nStatus & 0x0F;
	}
	inline void	SetStatus(unsigned char nVal)
	{
		nStatus	= nVal;
	}
	inline void	SetStatusBit(unsigned char nBitVal)
	{
		nStatus	|= nBitVal;
	}
	inline void	ClearStatusBit(unsigned char nBitVal)
	{
		nBitVal	&= nStatus;
		if( nBitVal != 0 )
		{
			nStatus	^= nBitVal;
		}
	}
};
#pragma pack(pop, old_pack_num)

// ��������һ�·��������
#define	TTY_RESET_AOMGR(pObj, pAM)	pObj->SetBase(pObj->_pBase, pAM)

#endif
