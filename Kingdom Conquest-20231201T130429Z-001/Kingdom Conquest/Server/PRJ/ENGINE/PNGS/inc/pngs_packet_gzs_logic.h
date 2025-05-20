// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_gzs_logic.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZS�ڸ���ģ����ָ�����������
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-09-01
// Change LOG   : 2005-09-30 ȡ����GZSPACKET_2GP_PLAYERIN��GZSPACKET_2GP_PLAYEROUT��GOPLAYER����ЩҪ�ŵ��߼�����ȥд��

#ifndef	__PNGS_PACKET_GZS_LOGIC_H__
#define	__PNGS_PACKET_GZS_LOGIC_H__

#include <WHCMN/inc/whcmn_def.h>
using namespace n_whcmn;

namespace n_pngs
{

enum
{
	GZSGAMEPLAY_VER				= 2,
	GZSMAINSTRUCTURE_VER		= 2,
};

// ��Щcmd��������CMN::ILogic::cmd_t���μ���pngs_cmn.h(���ڶ���Ϊint)
enum
{
	////////////////////////////////////////////////////////////////
	// GZS�κ�ģ��֪ͨ��Ϸ�߼�(GP, GamePlay����д)
	////////////////////////////////////////////////////////////////
	// ���巶Χ��200~219
	GZSPACKET_2GP_0							= 200,
	// (����ִ��)����(���һ�㶼Ӧ����GZSMainStructure����GZSGamePlay�õ�)
	GZSPACKET_2GP_CONFIG					= GZSPACKET_2GP_0 + 1,
	// ��ҷ�����Ϸ�������ָ��(MS֪ͨ����)
	GZSPACKET_2GP_PLAYERCMD					= GZSPACKET_2GP_0 + 2,
	// ������������������Ϸ�������ָ��(MS֪ͨ����)
	GZSPACKET_2GP_SVRCMD					= GZSPACKET_2GP_0 + 3,

	////////////////////////////////////////////////////////////////
	// GZS�κ�ģ��֪ͨMAINSTRUCTURE(MSΪMAINSTRUCTURE����д)
	////////////////////////////////////////////////////////////////
	// ���巶Χ��220~239����20��
	GZSPACKET_2MS_0							= 220,
	// (����ִ��)���ã�Ҳ���������гɹ���ִ����Ӧ�����ã�
	GZSPACKET_2MS_CONFIG					= GZSPACKET_2MS_0 + 1,
	// ����ҷ�������
	// ��: һ�����
	GZSPACKET_2MS_CMD2ONEPLAYER				= GZSPACKET_2MS_0 + 2,
	// ��: һ�����
	GZSPACKET_2MS_CMD2MULTIPLAYER			= GZSPACKET_2MS_0 + 3,
	// ��: ����ĳ�ֱ�ʶ�����
	GZSPACKET_2MS_CMD2PLAYERWITHTAG			= GZSPACKET_2MS_0 + 4,
	// ��: ȫ�����
	GZSPACKET_2MS_CMD2ALLPLAYER				= GZSPACKET_2MS_0 + 5,
	// ������ҵı�ʶ
	GZSPACKET_2MS_SETPLAYERTAG				= GZSPACKET_2MS_0 + 6,
	// �߳����
	GZSPACKET_2MS_KICKPLAYER				= GZSPACKET_2MS_0 + 7,
	// ����ĳ��GZS������(����GMS)��ָ��
	GZSPACKET_2MS_SVRCMD					= GZSPACKET_2MS_0 + 8,
	// ָ�����Ĳ�������ͣ�������������ݣ�ֱ����һ������ָ����ݳ���Ϊ0��������ķ��ͷ�ʽ��channel�������ķ���ָ������ģ�
	GZSPACKET_2MS_CMD_PACK_ADD				= GZSPACKET_2MS_0 + 9,
	// �����ʼ��Ϊ���߼��Ǳ߸Ķ���С��
	GZSPACKET_2MS_CMD_PACK_BEGIN			= GZSPACKET_2MS_0 + 10,
	// ������ҵ�64λ��ʶ
	GZSPACKET_2MS_SETPLAYERTAG64			= GZSPACKET_2MS_0 + 11,
	// �� ����ĳ��64λ��ʶ����ҷ�����Ϣ
	GZSPACKET_2MS_CMD2PLAYERWITHTAG64		= GZSPACKET_2MS_0 + 12,
};

// ���нṹ�Ķ��嶼����n_pngs::CMN::ILogic::CMDIN_TMPL_T�ĵ�ģ�����
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	GZSPACKET_2GP_CONFIG_T
{
	enum
	{
		SUBCMD_PLAYERNUMMAX			= 1,							// ��������Player����
																	// nParam��������
		SUBCMD_SVRIDX				= 2,							// ����SvrIdx
																	// nParam����SvrIdx
	};
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ�����ݣ����ȿ��ܳ���sizeof(int)��
};
struct	GZSPACKET_2GP_PLAYERCMD_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2GP_SVRCMD_T
{
	unsigned char	nFromSvrIdx;									// ��ʾ�����Ǹ���������PNGS_SVRIDX_GMS��ʾGMS��1~GZS_MAX_NUM��ʾGZS
	size_t			nDSize;
	void			*pData;
};
////////////////////////////////////////////////////////////////////
// ��Щָ��ͽṹ��GMS�е���һ����
struct	GZSPACKET_2MS_CONFIG_T
{
	enum
	{
		SUBCMD_GET_SVRIDX				= 1,						// Ҫ��ȡGZS_MS�ķ�������Ҫ����Ϊ���GZSGamePlayҲ��Ҫ
																	// ���صĽ�������nParam��
		SUBCMD_GET_CONNECTEDTOGMSPTR	= 2,						// ����Ƿ��GMS�����ϵ�bool������ָ��
																	// nParam������Ӧ��ָ��
	};
	int				nSubCmd;
	int				nParam;											// ����subcmd�Ĳ�ͬ���ͳ�Ϊ��ͬ�����ݣ����ȿ��ܳ���sizeof(int)��
};
struct	GZSPACKET_2MS_CMD2ONEPLAYER_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD2MULTIPLAYER_T
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
		return	sizeof(GZSPACKET_2MS_CMD2MULTIPLAYER_T) + nCNum*sizeof(int) + nDSize;
	}
};
struct	GZSPACKET_2MS_CMD2PLAYERWITHTAG_T
{
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��Ŷ�Ӧ��ֵ
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD2ALLPLAYER_T
{
	unsigned char	nChannel;
	unsigned char	nSvrIdx;										// ��Ӧ�ķ�������ţ�����ΪPNGS_SVRIDX_XXX��pngs_def.h�У�
	size_t			nDSize;
	void			*pData;

	GZSPACKET_2MS_CMD2ALLPLAYER_T()
		: nSvrIdx(PNGS_SVRIDX_ALLGZS)
	{}
};
struct	GZSPACKET_2MS_SETPLAYERTAG_T
{
	int				nClientID;										// ���ID
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��Ŷ�Ӧ��ֵ
};
struct	GZSPACKET_2MS_KICKPLAYER_T
{
	enum
	{
		CMD_KICK_END	= 0,										// �߳��û���������
		CMD_KICK_DELAY	= 1,										// �߳��û�������Ҫ��ָ���ӳٺ�������
	};
	int				nClientID;
	unsigned char	nKickCmd;
	GZSPACKET_2MS_KICKPLAYER_T()
		: nClientID(0)
		, nKickCmd(CMD_KICK_END)
	{
	}
};
struct	GZSPACKET_2MS_SVRCMD_T
{
	int				nSvrIdx;										// Ŀ��GZS����ţ�������PNGS_SVRIDX_XXX
	size_t			nDSize;
	void			*pData;
};
struct GZSPACKET_2MS_CMD_PACK_ADD_T
{
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD_PACK_BEGIN_T
{
	int				nSendCmd;										// ��Ӧ��Sendָ������GZSPACKET_2MS_CMD2XXX
																	// �����GZSPACKET_2MS_0���ʾû�д��
};
struct	GZSPACKET_2MS_SETPLAYERTAG64_T
{
	int				nClientID;										// ���ID
	whuint64		nTag;											// ��Ӧ�ı��
	bool			bDel;
};
struct	GZSPACKET_2MS_CMD2PLAYERWITHTAG64_T
{
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;
	whuint64		nTag;											// ��Ӧ�ı��
	size_t			nDSize;
	void			*pData;
};

////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_GZS_LOGIC_H__
