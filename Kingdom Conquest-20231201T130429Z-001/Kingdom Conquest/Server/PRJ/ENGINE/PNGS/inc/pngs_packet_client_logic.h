// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_client_logic.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��Client�ڸ���ģ����ָ�����������
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2006-11-28
// Change LOG   : 
//              : 

#ifndef	__PNGS_PACKET_CLIENT_LOGIC_H__
#define __PNGS_PACKET_CLIENT_LOGIC_H__

namespace n_pngs
{

// ģ��汾
enum
{
	PNGSCLIENT_VER				= 1,
};

// ��Щcmd��������CMN::ILogic::cmd_t�����ڶ���Ϊint
enum
{
	////////////////////////////////////////////
	// ���ģ�鷢��Clientģ���
	// 100~199
	////////////////////////////////////////////
	CLIENTPACKET_2CLIENT_0						= 100,
	// ����ָ��������������ģ��ͨ�������������������ָ�
	CLIENTPACKET_2CLIENT_SENDCMD2SVR			= CLIENTPACKET_2CLIENT_0 + 1,

	////////////////////////////////////////////
	// Clientģ�鷢�����ģ���
	// 100~199
	////////////////////////////////////////////
	CLIENTPACKET_2OTHER_0						= 100,
	// ���յ���ָ����ɸ�ģ�飨ģ�������ע��������ָ��Ĳ��ܴ�������ָ�
	CLIENTPACKET_2OTHER_DISPATCHCMD				= CLIENTPACKET_2OTHER_0 + 1,
};

#pragma pack(push, old_pack_num, 1)
struct	CLIENTPACKET_2CLIENT_SENDCMD2SVR_T
{
	int				nChannel;					// ͨ����
	int				nSvrIdx;					// Ŀ�������ID����PNGSClient_I::Send�е�nSvrIdx����һ�£�PNGS_SVRIDX_XXX��������pngs_def.h�У�
	size_t			nDSize;						// Ҫ����������
	void			*pData;
};
struct	CLIENTPACKET_2OTHER_DISPATCHCMD_T
{
	int				nChannel;					// ͨ����
	size_t			nDSize;						// �õ�������
	const void		*pData;
};
#pragma pack(pop, old_pack_num)


}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_CLIENT_LOGIC_H__
