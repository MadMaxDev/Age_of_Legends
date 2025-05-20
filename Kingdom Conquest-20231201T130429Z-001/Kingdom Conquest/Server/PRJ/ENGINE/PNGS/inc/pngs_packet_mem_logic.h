// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_mem_logic.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS���ڴ����ģ�������ģ��Ľ�������
//                �������Ϸ������أ����Է���Common/inc��
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2007-09-26
// Change LOG   : 

#ifndef	__PNGS_PACKET_MEM_LOGIC_H__
#define	__PNGS_PACKET_MEM_LOGIC_H__

namespace n_pngs
{

// ģ��汾
enum
{
	GSMEM_VER		= 1,
};

// MEMģ���յ���ָ��
// ���巶Χ��1400~1499
enum
{
	PNGSPACKET_2MEM_0							= 1400,
	////////////////////////////////////////////////////////////////
	// ������
	////////////////////////////////////////////////////////////////
	// (����ִ��)����(����������ָ�Ӧ�÷��ظ��Ǹ�ILogic)
	PNGSPACKET_2MEM_CONFIG						= PNGSPACKET_2MEM_0 + 1,
	// (����ִ��)��ȡָ��
	// ����ΪPNGSPACKET_2MEM_XXX_PTR_T
	PNGSPACKET_2MEM_GET_PTR						= PNGSPACKET_2MEM_0 + 12,
	// (����ִ��)����ָ��
	// ����ΪPNGSPACKET_2MEM_XXX_PTR_T
	PNGSPACKET_2MEM_SET_PTR						= PNGSPACKET_2MEM_0 + 13,
};

#pragma pack(push, old_pack_num, 1)
struct	PNGSPACKET_2MEM_CONFIG_T
{
	enum
	{
		SUBCMD_GET_MEMSTATUS					= 1,				// ����ڴ�״̬����nParam���أ�0��������ʼ����1�̳еľ��ڴ棩
	};
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ������
	int				nParam1;										// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ������
};
struct	PNGSPACKET_2MEM_XXX_PTR_T
{
	const char		*pcszKey;										// ָ�����������"::"��ͷ����������ڲ�ָ�루������ָ��ֻ��get����set��
																	// "::am"	��ʾ�Ƿ�����ָ��
																	// ���������Ǹ���ģ���Լ���Ҫ��ָ�룬һ��ģ�����ֻ��һ��ָ��
	void			*ptr;
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_MEM_LOGIC_H__
