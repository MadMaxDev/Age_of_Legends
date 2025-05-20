// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_gms_logic.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMS�ڸ���ģ����ָ�����������
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-09-01
// Change LOG   : 2005-09-30 ȡ����GOPLAYER����ЩҪ�ŵ��߼�����ȥд��
//              : ����ԭ����GMSPACKET_2GP_PLAYERIN/OUT����Ҫ��������Ϊ�������������ϲ��ͨ�档����GamePlayҲ�޷�֪���û����ӳɹ��ˡ��Լ��û�ʲôʱ������ˡ�

#ifndef	__PNGS_PACKET_GMS_LOGIC_H__
#define	__PNGS_PACKET_GMS_LOGIC_H__

#include "./pngs_packet.h"

namespace n_pngs
{

// ģ��汾
enum
{
	GMSGAMEPLAY_VER				= 2,
	GMSMAINSTRUCTURE_VER		= 2,
};

// ��Щcmd��������CMN::ILogic::cmd_t�����ڶ���Ϊint
enum
{
	////////////////////////////////////////////////////////////////
	// GMS�е��κ�ģ��֪ͨ��Ϸ�߼�(GP, GamePlay����д)
	////////////////////////////////////////////////////////////////
	GMSPACKET_2GP_0								= 100,
	// (����ִ��)����(���һ�㶼Ӧ����GMSMainStructure����GMSGamePlay�õ�)
	GMSPACKET_2GP_CONFIG						= GMSPACKET_2GP_0 + 1,
	// ��ҷ�����Ϸ�������ָ��(MS֪ͨ����)
	GMSPACKET_2GP_PLAYERCMD						= GMSPACKET_2GP_0 + 2,
	// GZS��������Ϸ�������ָ��(MS֪ͨ����)
	GMSPACKET_2GP_SVRCMD						= GMSPACKET_2GP_0 + 3,
	// ��ҽ���(�������)(MS֪ͨ����)
	GMSPACKET_2GP_PLAYERONLINE					= GMSPACKET_2GP_0 + 4,
	// ����뿪(����)(MS֪ͨ����)
	// ����MS���ȰѸ���ҵ�λ�ñ�����֪���ϲ������߳�������ˣ�֪ͨMS��MS���ָܻ���λ�����±����䣨�������˷�ֹһ���˶��߶�����������������ʧ�ܣ���Ϊ��Ӧ��λ����MP�л�û�������ͷţ�������
	GMSPACKET_2GP_PLAYEROFFLINE					= GMSPACKET_2GP_0 + 5,
	// ĳ��GZS������(MS֪ͨ����)
	GMSPACKET_2GP_GZSONLINE						= GMSPACKET_2GP_0 + 6,
	// ĳ��GZS�ϵ���(MS֪ͨ����)
	GMSPACKET_2GP_GZSOFFLINE					= GMSPACKET_2GP_0 + 7,
	// ĳ��GZS��ʱ�ж���
	GMSPACKET_2GP_GZSTEMPOFF					= GMSPACKET_2GP_0 + 8,
	// CAAFS���������ָ��
	GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS			= GMSPACKET_2GP_0 + 9,
	// ��ҷ���ͳ����Ϣ
	GMSPACKET_2GP_PLAYER_STAT					= GMSPACKET_2GP_0 + 10,

	////////////////////////////////////////////////////////////////
	// GMS�е��κ�ģ��֪ͨMAINSTRUCTURE(MSΪMAINSTRUCTURE����д)
	////////////////////////////////////////////////////////////////
	GMSPACKET_2MS_0								= 120,
	// (����ִ��)����
	GMSPACKET_2MS_CONFIG						= GMSPACKET_2MS_0 + 1,
	// ����ҷ�������
	// ��: һ�����
	GMSPACKET_2MS_CMD2ONEPLAYER					= GMSPACKET_2MS_0 + 2,
	// ��: һ�����
	GMSPACKET_2MS_CMD2MULTIPLAYER				= GMSPACKET_2MS_0 + 3,
	// ��: ����ĳ�ֱ�ʶ�����
	GMSPACKET_2MS_CMD2PLAYERWITHTAG				= GMSPACKET_2MS_0 + 4,
	// ��: ȫ�����
	GMSPACKET_2MS_CMD2ALLPLAYER					= GMSPACKET_2MS_0 + 5,
	// ������ҵı�ʶ
	GMSPACKET_2MS_SETPLAYERTAG					= GMSPACKET_2MS_0 + 6,
	// ������ҹ�����GZS(�������Ҫ���͸�GZS��ָ��Ĭ�϶��������GZS)
	GMSPACKET_2MS_SETCLIENTGZSROUTE				= GMSPACKET_2MS_0 + 7,
	// �߳����
	GMSPACKET_2MS_KICKPLAYER					= GMSPACKET_2MS_0 + 8,
	// ����ĳ��GZS��������ָ��
	GMSPACKET_2MS_GZS_SVRCMD					= GMSPACKET_2MS_0 + 9,
	// ����MSĳ��λ�õ�����������ͷ��ˣ�MS���Իָ���������λ��
	GMSPACKET_2MS_UNRESERVEPLAYER				= GMSPACKET_2MS_0 + 10,
	// ����ָ���������ڵ�CLS
	GMSPACKET_2MS_CMD2CLS						= GMSPACKET_2MS_0 + 11,
	// ����ָ�����Ӧ��GZS��MainStructure
	GMSPACKET_2MS_RAWCMD2GZSMS					= GMSPACKET_2MS_0 + 12,
	// ��ĳ���û���ID����Ϊ��һ��ID��������GZS��CLS��֪ͨ��
	GMSPACKET_2MS_CHANGECLIENTID				= GMSPACKET_2MS_0 + 13,
	// һЩ����ָ��
	GMSPACKET_2MS_CTRL							= GMSPACKET_2MS_0 + 19,
	// ͨ��MS����CAAFS�Ŀ���ָ��
	// ������GMS_CAAFS_CTRL_T ,GMS_CAAFS_KEEP_QUEUE_POS
	GMSPACKET_2MS_CAAFSCTRL						= GMSPACKET_2MS_0 + 20,
	GMSPACKET_2MS_CAAFSCTRL_QUEUE				= GMSPACKET_2MS_0 + 21,
	
};

// ���нṹ�Ķ��嶼����n_pngs::CMN::ILogic::CMDIN_TMPL_T�ĵ�ģ�����
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	GMSPACKET_2GP_CONFIG_T
{
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ�����ݣ����ȿ��ܳ���sizeof(int)��
};
struct	GMSPACKET_2GP_PLAYERCMD_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2GP_SVRCMD_T
{
	unsigned char	nSvrIdx;										// ˵��ָ����Դ���ĸ�GZS������
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2GP_PLAYERONLINE_T
{
	int				nClientID;
	unsigned int	IP;
	unsigned char	nNetworkType;									// ��Ҷ�Ӧ���������ͣ�NETWORKTYPE_XXX��
};
struct	GMSPACKET_2GP_PLAYEROFFLINE_T
{
	int				nClientID;
	unsigned char	nReason;										// �û����ߵ�ԭ��CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
};
struct	GMSPACKET_2GP_GZSONLINE_T
{
	unsigned char	nSvrIdx;
	bool			bReconnect;										// �Ƿ�����������
};
struct	GMSPACKET_2GP_GZSOFFLINE_T
{
	enum
	{
		MODE_REALLYOFF	= 0,										// ����������
		MODE_STOPPING	= 1,										// ����ֹͣ�����У����н������Ӧ�ñ��ܾ���
	};
	unsigned char	nSvrIdx;
	unsigned char	nMode;

	GMSPACKET_2GP_GZSOFFLINE_T()
	{
		WHMEMSET0THIS();
	}
};
struct	GMSPACKET_2GP_GZSTEMPOFF_T
{
	unsigned char	nSvrIdx;
	bool			bOff;											// Ϊtrue��˵���Ƕ��ˣ������˵���ָ���
};
struct	GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS_T
{
	int				nCAAFSIdx;										// ��ԴCAAFS����ţ���Ϊ�����ж��CAAFS��
	int				nClientIDInCAAFS;
	unsigned int	IP;
	size_t			nDSize;
	void			*pData;
};

////////////////////////////////////////////////////////////////////
struct	GMSPACKET_2MS_CONFIG_T
{
	enum
	{
		SUBCMD_REG_SVRCMD			= 1,							// ע����Ҫ�����SVRCMD�����յ�GMSPACKET_2MS_GZS_SVRCMDʱ�������ݰ�һЩָ����뻺�壩
																	// nParam������Ӧ��GAMDCMD
		SUBCMD_SET_PLAYERNUM		= 2,							// �����߼���������������������instant�ģ�
																	// nParam������Ӧ�������
	};
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ�����ݣ����ȿ��ܳ���sizeof(int)��
};
struct	GMSPACKET_2MS_CMD2ONEPLAYER_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CMD2MULTIPLAYER_T
{
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	unsigned short	nClientNum;										// ClientID������
	int				*panClientID;									// nClient�����ַ
	int				nDSize;
	void			*pData;
	// (���̲߳���Ҫ)�����û����������ݼ�����ܳ���
	// �ṹ�����ȸ���panClientID����ʵ���ݣ�Ȼ�������û����ݲ���
	static int	MT_GetTotalSize(int nCNum, int nDSize)
	{
		return	sizeof(GMSPACKET_2MS_CMD2MULTIPLAYER_T) + nCNum*sizeof(int) + nDSize;
	}
};
struct	GMSPACKET_2MS_CMD2PLAYERWITHTAG_T
{
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��Ŷ�Ӧ��ֵ
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CMD2ALLPLAYER_T
{
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_SETPLAYERTAG_T
{
	int				nClientID;										// ���ID
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��Ŷ�Ӧ��ֵ
};
struct	GMSPACKET_2MS_SETCLIENTGZSROUTE_T
{
	int				nClientID;										// ���ID
	unsigned char	nSvrIdx;										// ������Ŀ��GZS�����
};
struct	GMSPACKET_2MS_KICKPLAYER_T
{
	int				nClientID;
	pngs_cmd_t		nKickSubCmd;									// ȡֵΪ��SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_XXX�����磺�Ƿ����CLS�����û�DROP�Ľ��
	GMSPACKET_2MS_KICKPLAYER_T()
		: nClientID(0)
		, nKickSubCmd(SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID)
	{}
};
struct	GMSPACKET_2MS_GZS_SVRCMD_T
{
	unsigned char	nSvrIdx;										// Ŀ��GZS�����
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_UNRESERVEPLAYER_T
{
	int				nClientID;										// ��Ҿ�����ID�����������õ�����ţ������Ӧ���󻹴������൱��Kick
};
struct	GMSPACKET_2MS_CMD2CLS_T
{
	int				nClientID;										// �������ID����������ҵ���Ӧ��CLS��
	size_t			nDSize;
	void			*pData;											// ����CLS��ָ��
};
struct	GMSPACKET_2MS_RAWCMD2GZSMS_T
{
	unsigned char	nSvrIdx;										// Ŀ��GZS�����
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CHANGECLIENTID_T
{
	int				nFromID;
	int				nToID;
	unsigned char	nSvrIdx;										// ��ҪCLS�и���ҹ�����gzs���
};
struct	GMSPACKET_2MS_CTRL_T
{
	enum
	{
		SUBCMD_CLOSE_CONNECTER	= 0x01,								// �ر�ĳ�����ӵ�socket������nParam���Ƕ�Ӧ��connecterID
		SUBCMD_CLOSEALLCAAFS	= 0x02,								// �ر����е�CAAFS
		SUBCMD_SETCLSMAXPLAYER	= 0x03,								// ����ĳ��CLS������û�����������nParam��CLSID��nParam1���û�����
		SUBCMD_PLAYER_STAT_ON	= 0x04,								// ����ҵ�ͳ�ƿ�ʼ��nParam����nClientID��nParam1�Ƿ��ͼ������Ϊ��λ��
		SUBCMD_PLAYER_STAT_OFF	= 0x05,								// ����ҵ�ͳ�ƽ�����nParam����nClientID
	};
	unsigned char	nSubCmd;
	int				nParam;
	int				nParam1;
};
////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_GMS_LOGIC_H__
