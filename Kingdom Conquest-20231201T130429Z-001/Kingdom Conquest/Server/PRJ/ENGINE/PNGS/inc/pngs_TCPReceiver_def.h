// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_TCPReceiver_def.h
// Creator      : Wei Hua (κ��)
// Comment      : TCPָ�����ģ��������ģ�齻����ָ���
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2007-02-25
// Change LOG   : 2007-06-07 �޸�TCPRECEIVER_VER�汾����Ϊ���������tcpmsger���ڲ�����ʽ����

#ifndef	__PNGS_TCPRECEIVER_DEF_H__
#define __PNGS_TCPRECEIVER_DEF_H__

namespace n_pngs
{

enum
{
	TCPRECEIVER_VER				= 2,
};

////////////////////////////////////////////////////////////////////
// ָ���
// ע��TR��TCPReceiver����д��CD��CmdDealer����д
////////////////////////////////////////////////////////////////////
// ��CmdDealer��
enum
{
	PNGS_TR2CD_0				= 300,
	PNGS_TR2CD_NOTIFY			= PNGS_TR2CD_0 + 1,					// ֪ͨһЩ����
	PNGS_TR2CD_CMD				= PNGS_TR2CD_0 + 4,					// ��Ҫ�����ָ��
};
// ��CmdDealer����
enum
{
	PNGS_CD2TR_0				= 300,
	PNGS_CD2TR_CONFIG			= PNGS_CD2TR_0 + 1,					// ���ã���������CmdDealer��ָ�룩
	PNGS_CD2TR_DISCONNECT		= PNGS_CD2TR_0 + 2,					// �Ͽ�ĳ�����ӣ���������������ConnecterID��
	PNGS_CD2TR_SETEXT			= PNGS_CD2TR_0 + 3,					// ����ĳ�����ӵĸ��Ӳ���
	PNGS_CD2TR_CMD				= PNGS_CD2TR_0 + 4,					// ָ�����Ľ���������Ƿ����͵�ָ�
	PNGS_CD2TR_FLUSH			= PNGS_CD2TR_0 + 5,					// ��tr���Ѿ��յ�����Ϣ�ȶ�����һ��(�����ϲ�buf������,�����instant��,��Ҫ����ִ��)
																	// for web,added by yuezhongyue 2011-12-29,��������,DBS4Web�ڼ���ͼƬ����ʱm_queueCmdIn������
};

////////////////////////////////////////////////////////////////////
// ���ݽṹ����
////////////////////////////////////////////////////////////////////
#pragma pack(push, old_pack_num, 1)
// ��CmdDealer��
struct	PNGS_TR2CD_NOTIFY_T
{
	enum
	{
		SUBCMD_DISCONNECT		= 0x01,								// �����Ӷ���
																	// nParam1ΪnCntrID��nParam2Ϊ���Ӳ���������ͨ������ҵ��ϲ��Ӧ�Ķ�����
		SUBCMD_CONNECT			= 0x02,								// ��������,nParam1ΪnCntrID,added by yuezhongyue
	};
	unsigned char	nSubCmd;
	int				nParam1;										// ����ID
	int				nParam2;										// ���Ӹ��ӵ���������
};
struct	PNGS_TR2CD_CMD_T
{
	int				nConnecterID;									// ��Ӧ����ָ���������ID
	unsigned int	IP;												// ��Դ��ַIP
	size_t			nDSize;											// �����ĳ���
	void			*pData;											// ָ��Ŀ�ͷ
};
// ��CmdDealer����
struct	PNGS_CD2TR_CONFIG_T
{
	enum
	{
		SUBCMD_SET_CMDDEALER	= 1,								// ����ָ�����
																	// pRstAccepter������Ҫ�Ĵ�����ָ��
		SUBCMD_REGISTERCMDDEAL	= 2,								// ���ö��ض�ָ��Ĵ�����������һ��ָ��������Ԫ����
																	// nParam����ת��Ϊpngs_cmd_t��ָ��
																	// pRstAccepter������Ҫ�Ĵ�����ָ��
	};
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ�����ݣ����ȿ��ܳ���sizeof(int)��
};
struct	PNGS_CD2TR_SETEXT_T
{
	int				nConnecterID;									// Ҫ���ø������ݵ�������ID
	int				nExt;											// ��������
};
struct	PNGS_CD2TR_CMD_T
{
	int				nConnecterID;									// ��Ӧ����ָ���������ID�����nConnecterIDΪ-8888��㲥�����е����ӣ�
	size_t			nDSize;											// �����ĳ���
	void			*pData;											// ����Ŀ�ͷ
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

// ��������
// TCPReceiver��ģ�����֣�CmdDealer��Ҫ�ҵ��������Ȼ����Լ���ָ���������
#define	PNGS_DLL_NAME_TCPRECEIVER	"uTCPReceiver"

#endif	// EOF __PNGS_TCPRECEIVER_DEF_H__
