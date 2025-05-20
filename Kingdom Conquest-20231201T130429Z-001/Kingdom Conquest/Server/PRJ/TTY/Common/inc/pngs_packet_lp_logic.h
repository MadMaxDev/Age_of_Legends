#ifndef __pngs_packet_lp_logic_H__
#define __pngs_packet_lp_logic_H__

#include "WHCMN/inc/whcmn_def.h"

using namespace n_whcmn;

namespace n_pngs
{
enum
{
	MAIL_FLAG_NORMAL		= 1,		// ��ͨ�ʼ�
	MAIL_FLAG_FIGHT			= 2,		// ������ϢΪս��
	MAIL_FLAG_REWARD		= 3,		// ����������Ϊ����
	MAIL_FLAG_COMBAT		= 4,		// �µ�ս��ģʽ(���˸���)
};
enum
{
	MAIL_TYPE_PRIVATE		= 0x01,		// ˽���ʼ�
	MAIL_TYPE_ALLIANCE		= 0x02,		// �����ʼ�
	MAIL_TYPE_SYSTEM		= 0x04,		// ϵͳ�ʼ�
	MAIL_TYPE_ALLIANCE_EVENT	= 0x08,		// �����¼�
	MAIL_TYPE_GOLDORE_EVENT		= 0x10,		// ����¼�,��������͵���߻��߽�ˮ��
};
enum
{
	mail_reward_type_none				= 0,			// �޽���
	mail_reward_type_item				= 1,			// ����
	mail_reward_type_diamond			= 2,			// ��ʯ
	mail_reward_type_crystal			= 3,			// ˮ��
	mail_reward_type_gold				= 4,			// �ƽ�
};
// cmd�����Ͷ���int
// �ܶ�ָ����web�в���Ҫ��,��Ԥ��
enum
{
	//////////////////////////////////////////////////////////////////////////
	// LP�е��κ�ģ��֪ͨ��Ϸ�߼�(GP,GamePlay����д)
	//////////////////////////////////////////////////////////////////////////
	LPPACKET_2GP_0									= 100,
	// (����ִ��)����(���һ�㶼Ӧ����LPMainStructure����LPGamePlay�õ�)(����)
	LPPACKET_2GP_CONFIG								= LPPACKET_2GP_0 + 1,
	// ��ҷ�����Ϸ�������ָ��(MS֪ͨ����)
	LPPACKET_2GP_PLAYERCMD							= LPPACKET_2GP_0 + 2,
	// ��ҽ���(�������)(MS֪ͨ����)
	LPPACKET_2GP_PLAYERONLINE						= LPPACKET_2GP_0 + 3,
	// ����뿪(����)(MS֪ͨ����)
	LPPACKET_2GP_PLAYEROFFLINE						= LPPACKET_2GP_0 + 4,
	// CAAFS4Web���������ָ��
	LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web			= LPPACKET_2GP_0 + 5,
	// ��ҷ�����ͳ����Ϣ
	LPPACKET_2GP_PLAYER_STAT						= LPPACKET_2GP_0 + 6,

	//////////////////////////////////////////////////////////////////////////
	// LP���κ�ģ��֪ͨMainStructure(MSΪMainStructure����д)
	//////////////////////////////////////////////////////////////////////////
	LPPACKET_2MS_0									= 120,
	// (����ִ��)����
	LPPACKET_2MS_CONFIG								= LPPACKET_2MS_0 + 1,
	// ����ҷ�������
	// to:һ�����
	LPPACKET_2MS_CMD2ONEPLAYER						= LPPACKET_2MS_0 + 2,
	// to:һ�����
	LPPACKET_2MS_CMD2MULTIPLAYER					= LPPACKET_2MS_0 + 3,
	// to:����ĳ�ֱ�ʶ�����
	LPPACKET_2MS_CMD2PLAYERWITHTAG					= LPPACKET_2MS_0 + 4,
	// to:ȫ�����
	LPPACKET_2MS_CMD2ALLPLAYER						= LPPACKET_2MS_0 + 5,
	// ������ҵı�ʶ
	LPPACKET_2MS_SETPLAYERTAG						= LPPACKET_2MS_0 + 6,
	// �߳����
	LPPACKET_2MS_KICKPLAYER							= LPPACKET_2MS_0 + 8,
	// ����MSĳ��λ�õ�����������ͷ���,MS���Իָ���������λ��
	LPPACKET_2MS_UNRESERVEPLAYER					= LPPACKET_2MS_0 + 9,
	// ����ָ���������ڵ�CLS4Web
	LPPACKET_2MS_CMD2CLS4Web						= LPPACKET_2MS_0 + 10,
	// ��ĳ���û���ID����Ϊ��һ��ID(������CLS4Web��֪ͨ)
	LPPACKET_2MS_CHANGECLIENTID						= LPPACKET_2MS_0 + 11,
	// ������ĳ��64λ��ʶ����ҷ���ָ��
	LPPACKET_2MS_CMD2PLAYERWITHTAG64				= LPPACKET_2MS_0 + 12,
	// ������ҵ�64λ��ʶ
	LPPACKET_2MS_SETPLAYERTAG64						= LPPACKET_2MS_0 + 13,
	// һЩ����ָ��
	LPPACKET_2MS_CTRL								= LPPACKET_2MS_0 + 14,
	// ͨ��MS����CAAFS4Web�Ŀ���ָ��
	// ����:LP_CAAFS4Web_CTRL_T
	LPPACKET_2MS_CAAFS4WebCTRL						= LPPACKET_2MS_0 + 15,
};

#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// 2GP
//////////////////////////////////////////////////////////////////////////
struct LPPACKET_2GP_PLAYERCMD_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T
{
	int					nCAAFS4WebIdx;										// ��ԴCAAFS4Web�����(�����ж��CAAFS4Web)
	int					nClientIDInCAAFS4Web;
	unsigned int		IP;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2GP_PLAYERONLINE_T 
{
	int					nClientID;
	unsigned int		IP;
	int					nTermType;											// �ն�����
};
struct LPPACKET_2GP_PLAYEROFFLINE_T 
{
	int					nClientID;
	unsigned char		nReason;											// �û�����ԭ��CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX
};
//////////////////////////////////////////////////////////////////////////
// 2MS
//////////////////////////////////////////////////////////////////////////
// ������ָ��
struct LPPACKET_2MS_CONFIG_T 
{
	enum
	{
		SUBCMD_REG_SVRCMD				= 1,								// ע����Ҫ�����svrcmd
		SUBCMD_SET_PLAYERNUM			= 2,								// �����߼����������(�������instant��!!!)
	};
	int					nSubCmd;
	int					nParam;
};
struct LPPACKET_2MS_CMD2CLS4Web_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_KICKPLAYER_T 
{
	int					nClientID;
	unsigned char		nKickSubCmd;										// SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_XXX
};
struct LPPACKET_2MS_CMD2ONEPLAYER_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2MULTIPLAYER_T 
{
	unsigned short		nClientNum;
	int*				paClientID;
	int					nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2PLAYERWITHTAG_T 
{
	unsigned char		nTagIdx;
	short				nTagVal;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2ALLPLAYER_T 
{
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_SETPLAYERTAG_T 
{
	int					nClientID;
	unsigned char		nTagIdx;
	short				nTagVal;
};
struct LPPACKET_2MS_CMD2PLAYERWITHTAG64_T 
{
	whuint64			nTag;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_SETPLAYERTAG64_T 
{
	int					nClientID;
	whuint64			nTag;
	bool				bDel;
};
#pragma pack()
}

#endif
