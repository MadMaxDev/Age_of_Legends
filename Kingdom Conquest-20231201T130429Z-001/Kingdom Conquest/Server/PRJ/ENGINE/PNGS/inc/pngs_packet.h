// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS���������������
//                PNGS��Pixel Network Game Structure����д
//                �������GMS��GZSʵ���ϴ���������ǵ�MainStructure
//                PNGS_SVRIDX_XXX������pngs_def.h��
//                !!!!ע�⣺������CLIENT_CLS_DATA����CLS_CLIENT_DATA��data�����ʼ��һ����pngs_cmd_t��ͷ��һ��ָ���룬������Ͳ��ܸı䣬������PNLGSClient���ڲ��ּ�ָ��ʱ���������ˡ�
// CreationDate : 2005-08-01
// Change LOG   : 2005-09-30 ȡ���߼������PlayerIn��Out(��ȡ����GZS_GMS_PLAYER_IN_OVER��GZS_GMS_PLAYER_OUT_OVER��GZS_GMS_PLAYER_WANTOUT)��
//              : ��Щ��Ӧ�÷ŵ�GamePlay����ȥд����ҳɹ�����GMS֮��GMS������е�GZSͬ����ҵ����ӡ�
//              : �޸���GMS_CAAFS_CTRL_T::SUBCMD_EXIT�����ã��μ���Ӧ��ע�ͣ�
//              : 2007-02-05 ��ΪҪ����ͬһ��CAAFS/CLS���ڵĸ����������͵ķֲ�����������������汾XXX_VER
//              : 2007-03-16 GZS���û����ݶ��뷢�������⣬��windows�º�linux�²�ͬ��ԭ����windows�����ڵĲ��ܴ����Ͳ����ˣ�����linux�ܴա������޸���λ��������ͣ�ʹ��ǡ���ܴ�����
//              : 2007-10-08 ������GMS�ĸ���HELLO����������ԭ��������ID���ڱ�������������������İ汾Ҳ��˸ı䡣
//				: 2008-05-07 GMS��GZS��GamePlay�����Э��仯�����Եİ汾�޸ġ����ܻ�����

#ifndef	__PNGS_PACKET_H__
#define	__PNGS_PACKET_H__

#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHCMN/inc/whcmn_def.h>
#include "./pngs_def.h"

using namespace n_whcmn;

namespace n_pngs
{

// д������������ʾ��n_pngs��������ռ��п���ʹ��
using n_whnet::port_t;
using n_whnet::CNL2SlotMan;

// �������Ͷ���
enum
{
	NETWORKTYPE_UNKNOWN		= 0,								// δ֪���ͣ������û��ƥ��ľ�Ĭ���������
	NETWORKTYPE_C_Netcom	= 1,								// �й���ͨ
	NETWORKTYPE_C_Telcom	= 2,								// �й�����
	NETWORKTYPE_C_Edu		= 3,								// �й�������
	NETWORKTYPE_C_Unicom	= 4,								// �й���ͨ
	NETWORKTYPE_C_TOTAL,										// ��������
};

// �����������ĵ�ǰ�汾���汾��ƥ���ʱ���ܻ�����
enum
{
	GMS_VER		= 10,
	CAAFS_VER	= 8,
	CLS_VER		= 8,
	GZS_VER		= 10,
	GMTOOL_VER	= 0,
};

////////////////////////////////////////////////////////////////////
// GMS��CAAFS��CLS��GZS��������ļ�����Ҫ�����������
// ��Щָ������Ͷ���pngs_cmd_t����ο�pngs_def.h
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// ָ���
////////////////////////////////////////////////////////////////////
enum
{
	////////////////////////////////////////////////////////////////
	// CAAFS->GMS
	////////////////////////////////////////////////////////////////
	// CAAFS����GMS������Լ�����Ϣ
	CAAFS_GMS_HELLO						= 1,
	// CAAFS��GMS��һЩ���ƹ���(�磺GMS�������ִ�ָ��ķ��صȵ�)
	CAAFS_GMS_CTRL						= 2,
	// ֪ͨGMS������뿪ʼ����CLS��(��ҵ�һЩ��Ϣ����֪ͨ��CLS)
	CAAFS_GMS_CLIENTWANTCLS				= 3,
	// CAAFS������һЩԤ�������磺�û�����CAAFS��ʱ��ÿ���˿�����һ��PRELOGIN��У��һ���������Ϣ���û����ܷ���̫����������Ϣ��һ������ֻ�ܷ���һ�Σ�
	CAAFS_GMS_CLIENT_CMD				= 4,
	////////////////////////////////////////////////////////////////
	// GMS->CAAFS
	////////////////////////////////////////////////////////////////
	// GMS����CAAFS���Թ���
	GMS_CAAFS_HI						= 11,
	// GMS��CAAFS��һЩ���ƹ���(�磺��ֹ/�����û���¼����̬�޸İ汾�ȵ�)
	GMS_CAAFS_CTRL						= 12,
	// GMS����CAAFS��GMS����Ϣ�仯��(�磺���������仯�ȵ�)
	GMS_CAAFS_GMSINFO					= 13,
	// GMS����CAAFS��������ҿ�ʼ����CLS��
	// ֻҪ��Ӧ��CLS������CAAFSһ�����յ�������ص�(���CLS���ˣ�GMS������CLS�û���ʱ��Ҳ����CAAFS�����û����ӽ������Ϣ)
	GMS_CAAFS_CLIENTCANGOTOCLS			= 14,
	// GMS����CAAFS�������������к��ʺ�
	GMS_CAAFS_KEEP_QUEUE_POS			= 15,
	////////////////////////////////////////////////////////////////
	// CLS->GMS
	////////////////////////////////////////////////////////////////
	// CLS����GMS������Լ��ĳ�ʼ��Ϣ
	CLS_GMS_HELLO						= 21,
	CLS_GMS_HELLO1						= 22,
	// CLS֪ͨGMS�Լ���ĳ��GZS���ӳɹ�(����GMS�Ϳ��Ա�����CLS���Լ���������)
	CLS_GMS_CONNECTEDTOGZS				= 23,
	// CLS֪ͨGMS�ͻ��˿��������Լ���
	CLS_GMS_CLIENTCANGOTOCLS			= 24,
	// CLS֪ͨGMS�ͻ������ӽ��
	CLS_GMS_CLIENTTOCLS_RST				= 25,
	// CLS֪ͨGMS�ͻ��˶���(gms�յ������Ϣ��ʱ���û�Ӧ���Ѿ���CLSɾ����)
	CLS_GMS_CLIENT_DROP					= 26,
	// һЩ���ƹ�������֪ͨ�Լ�Ҫ����ͣ���ˣ�
	CLS_GMS_CTRL						= 27,
	////////////////////////////////////////////////////////////////
	// GMS->CLS
	////////////////////////////////////////////////////////////////
	// GMS����CLS���Թ���
	GMS_CLS_HI							= 31,
	// GMS����CLS��GZS�仯����Ϣ(CLS�յ���ͻ�ȥ��GZS)
	GMS_CLS_GZSINFO						= 33,
	// GMS����CLS��Ҫ�пͻ�����������
	GMS_CLS_CLIENTWANTCLS				= 34,
	// ֪ͨCLSָ����Һ�����Ҫ���Ǹ�GZSͨѶ
	GMS_CLS_ROUTECLIENTDATATOGZS		= 35,
	// GMS����CLS���û�ID�޸�Ϊ��ģ����Ӧ�����û������̳о��û��õģ�
	GMS_CLS_PLAYER_CHANGEID				= 36,
	////////////////////////////////////////////////////////////////
	// GZS->GMS
	////////////////////////////////////////////////////////////////
	// CZS����GMS������Լ��ĳ�ʼ��Ϣ(�磺����1��)
	GZS_GMS_HELLO						= 41,
	// GZS����GMS�Լ��Ŀ��õ�ַ(���ֻ���յ�GMS��Hi֮������н��)
	GZS_GMS_MYINFO						= 42,
	// ��Ϸ�߼���ص�ָ��(��GamePlay����Ҫ�õ���ָ��)
	GZS_GMS_GAMECMD						= 43,
	// ����GMS���û����ߡ����Ӧ������GZS��CLS����������²Żᷢ���ġ�
	GZS_GMS_KICKPALYERBYID				= 44,
	// һЩ���ƹ�������֪ͨ�Լ�Ҫ����ͣ���ˣ�
	GZS_GMS_CTRL						= 45,
	////////////////////////////////////////////////////////////////
	// GMS->GZS
	////////////////////////////////////////////////////////////////
	// GMS����GZS���Թ���
	GMS_GZS_HI							= 51,
	// GMS��GZS��һЩ���ƹ���(�磺�߳�ĳ���û�������ĳЩ��Ϣ��)
	GMS_GZS_CTRL						= 52,
	// ��Ϸ�߼���ص�ָ��(��GamePlay����Ҫ�õ���ָ��)
	GMS_GZS_GAMECMD						= 53,
	// ֪ͨ���������(ֻ�����Ӳ����)
	GMS_GZS_PLAYER_ONLINE				= 54,
	// ֪ͨ���������(ֻ�����Ӳ����)
	GMS_GZS_PLAYER_OFFLINE				= 55,
	// ֪ͨGZS���û�ID�ı�
	GMS_GZS_PLAYER_CHANGEID				= 56,
	// 
	////////////////////////////////////////////////////////////////
	// CLS->GZS
	////////////////////////////////////////////////////////////////
	// CLS����GZS������Լ��ĳ�ʼ��Ϣ
	CLS_GZS_HELLO						= 61,
	////////////////////////////////////////////////////////////////
	// GZS->CLS
	////////////////////////////////////////////////////////////////
	// GZS����CLSS���Թ���
	GZS_CLS_HI							= 71,
	////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS
	////////////////////////////////////////////////////////////////
	// ���Ϳͻ��˵�һЩ��Ϣ���磺�汾��������Կ��PubForServer�����
	// CLIENT_CAAFS_HELLO					= 81, 2005-08-16 ��Ϊֱ��ͨ�����Ӳ�����
	////////////////////////////////////////////////////////////////
	// CAAFS->CLIENT
	////////////////////////////////////////////////////////////////
	// ���ҽ��ܣ��������ǵ���II��CAAFS�����������ܷ�ʽΪ0��֮��ģ����������ͻ��������ӣ�ֻ�пͻ����жϿ������Ӳ�����һ������HELLO��
	// �ͻ����ж�info����������ߵĻ��Ϳ��Լ���������
	CAAFS_CLIENT_CAAFSINFO				= 91,
	// ֪ͨ�û�Ŀǰ�Ŷӵ�״��
	CAAFS_CLIENT_QUEUEINFO				= 92,
	// ֪ͨ�û����Կ�ʼ����CLS��(�����û�CLS�ĵ�ַ���Լ���������)
	CAAFS_CLIENT_GOTOCLS				= 93,
	////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS
	////////////////////////////////////////////////////////////////
	CLIENT_CAAFS_2GMS					= 98,
	//��¼ǰ��׼������CAAFS����׼����½���ʺ�
	CLIENT_CAAFS_BEFORE_LOGIN				= 99,
	////////////////////////////////////////////////////////////////
	// CLIENT->CLS
	////////////////////////////////////////////////////////////////
	// ���Ϳͻ��˵�һЩ��Ϣ���磺���롢������Կ��PubForServer�����
	// CLIENT_CLS_HELLO					= 101,
	// �û�����
	CLIENT_CLS_DATA						= 102,
	// �û�ͳ����Ϣ
	// �ṹCLIENT_CLS_DATA_T
	CLIENT_CLS_STAT						= 103,
	////////////////////////////////////////////////////////////////
	// CLS->CLIENT
	////////////////////////////////////////////////////////////////
	// CLS���߿ͻ������У���Ƿ�ɹ������н�����Կ��PubForClient��������ɹ��Ϳ��Կ�ʼ������
	//CLS_CLIENT_HI						= 111,
	// �û�����
	CLS_CLIENT_DATA						= 112,
	// �û����ݴ������Ҫ�Ƚ⿪�ٴ����ϲ㣩
	CLS_CLIENT_DATA_PACK				= 113,
	// ���û�����һЩ֪ͨ
	CLS_CLIENT_NOTIFY					= 114,
	////////////////////////////////////////////////////////////////
	// CLS���͸��������Ĺ���ָ��
	////////////////////////////////////////////////////////////////
	// �ͻ��˷���������
	CLS_SVR_CLIENT_DATA					= 201,
	// �ͻ��˷�����ͳ����Ϣ
	// �ṹCLS_SVR_CLIENT_DATA_T
	CLS_SVR_CLIENT_STAT					= 202,
	////////////////////////////////////////////////////////////////
	// CLS�յ��ķ����������Ĺ���ָ��
	////////////////////////////////////////////////////////////////
	// ��������CLS��һЩ���ƹ���(�磺�߳�ĳ���û�������ĳЩ��Ϣ��)
	SVR_CLS_CTRL						= 210,
	// ������ұ�־
	SVR_CLS_SET_TAG_TO_CLIENT			= 211,
	// ����������ԣ������Ƿ�¼���Ƿ��������־�ȵȣ�
	SVR_CLS_SET_PROP_TO_CLIENT			= 212,
	// **** ע�⣺�����XXX_PACK��XXXָ���ָ��ṹ ****
	// ������һ��ҵ�����
	SVR_CLS_CLIENT_DATA					= 213,
	SVR_CLS_CLIENT_DATA_PACK			= 214,
	// ���������ҵ�����
	SVR_CLS_MULTICLIENT_DATA			= 215,
	SVR_CLS_MULTICLIENT_DATA_PACK		= 216,
	// ����ĳ����־�������������
	SVR_CLS_TAGGED_CLIENT_DATA			= 217,
	SVR_CLS_TAGGED_CLIENT_DATA_PACK		= 218,
	// �����������
	SVR_CLS_ALL_CLIENT_DATA				= 219,
	SVR_CLS_ALL_CLIENT_DATA_PACK		= 220,
	// ����64λ�ķ��ͱ�־
	SVR_CLS_SET_TAG64_TO_CLIENT			= 221,
	// ����ĳ��64λ��־�������������
	SVR_CLS_TAGGED64_CLIENT_DATA		= 222,
	SVR_CLS_TAGGED64_CLIENT_DATA_PACK	= 223,

	////////////////////////////////////////////////////////////////
	// GM�����������������ָ��
	////////////////////////////////////////////////////////////////
	GMTOOL_SVR_HELLO					= 241,
	GMTOOL_SVR_CMD						= 242,
	////////////////////////////////////////////////////////////////
	// ��������GM���߷�����ָ��
	////////////////////////////////////////////////////////////////
	SVR_GMTOOL_HI						= 251,
	SVR_GMTOOL_CMD_RST					= 252,

	////////////////////////////////////////////////////////////////
	// CLS_SVR_CLIENT_STAT����ָ��
	////////////////////////////////////////////////////////////////
	// �ͻ��˻�������ʱ��
	// ����Ϊһ������
	CLIENT_STAT_STAT_SUBCMD_ONTIME		= 1,
};
////////////////////////////////////////////////////////////////////
// ���ݽṹ����
////////////////////////////////////////////////////////////////////
#pragma pack(push, old_pack_num, 1)
// �������е�ָ��ṹ������nCmd��ͷ
////////////////////////////////////////////////////////////////////
// CAAFS->GMS
////////////////////////////////////////////////////////////////////
struct	CAAFS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGroupID;										// CAAFS��ź���Ӧ��CLS��Ӧ
	int				nCAAFSVer;										// CAAFS�汾
	bool			bReConnect;										// ��ʾ������������µ�
};
struct	CAAFS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_STRCMD			= 0,								// �ִ�ָ��
		SUBCMD_STRRST			= 1,								// �ִ�ָ��ķ���
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nExt;											// ���Ӳ���ԭ������
	int				nParam;											// һ��Ĭ����һ��int����������չ������ʱ�����nSubCmd����
};
struct	CAAFS_GMS_CLIENTWANTCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;								// �û���CAAFS�е�ID
	unsigned int	IP;												// �û�IP
	unsigned int	nPassword;										// �û���������CLS������
	unsigned char	nNetworkType;									// �û�����������
};
struct	CAAFS_GMS_CLIENT_CMD_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;								// �û���CAAFS�е�ID
	unsigned int	IP;
	// ������������ָ������ݣ�����PRELOGIN�ĺ�����LOGIN�������ơ����������ǵײ�Ͳ�д���ݵľ���ṹ�ˣ�
};
////////////////////////////////////////////////////////////////////
// GMS->CAAFS
////////////////////////////////////////////////////////////////////
struct	GMS_CAAFS_HI_T
{
	enum
	{
		RST_OK					= 0,								// ���ӳɹ�
		RST_ERR_BAD_VER			= 1,								// �汾��ƥ��
		RST_ERR_BAD_GROUPID		= 2,								// GroupID����
		RST_ERR_DUP_GROUPID		= 3,								// ���GroupID�Ѿ���CAAFS������
		RST_ERR_MEMERR			= 4,								// �ڴ�������
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGMSVer;										// GMS�汾
};
struct	GMS_CAAFS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// �˳�
		SUBCMD_STRCMD			= 1,								// �ִ�ָ��
		SUBCMD_STRRST			= 2,								// �ִ�ָ��ķ���
		SUBCMD_CLIENTCMDNUMSUB1	= 3,								// ���û���ָ�������1��nExt���û���nClientIDInCAAFS
																	// ���÷���
		SUBCMD_CLIENTQUEUETRANS	= 4,								// ת���û����Ŷӣ��磺����vip1�ȣ���nExt���û���nClientIDInCAAFS��nParam��Ŀ����С�
																	// ���÷���
		SUBCMD_CLIENTQUEUEPOSRESUME = 5,	
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nCAAFSIdx;										// �����Ҫ���ϲ㷢��MS���õģ�����ָ�������Ǹ�CAAFS
	int				nExt;											// ���Ӳ���ԭ������(һ����������λָ����ߵģ��磺GM�����)
	int				nParam;											// һ��Ĭ����һ��int����������չ������ʱ�����nSubCmd����
};
struct	GMS_CAAFS_GMSINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned short	nGMSAvailPlayer;								// GMS��Ϊ���CAAFS���ܳ���������Ϸ������
																	// GMS���ÿ��CAAFS����һ����������(�������CAAFS��ʱ�������Ͳ����ͻ��)
};
struct	GMS_CAAFS_CLIENTCANGOTOCLS_T
{
	enum
	{
		RST_OK					= 0,								// �ɹ�����������CLS��
		RST_ERR_MEM				= 1,								// ʧ�ܣ��޷������ڴ�
		RST_ERR_CLS				= 2,								// ʧ�ܣ�û�п�ȥ��CLS���ߺ�CLSͨѶ������û�з�Ӧ
		RST_ERR_NOCLSCANACCEPT	= 3,								// ʧ�ܣ�û���ܹ������û���CLS��
		RST_ERR_OTHERKNOWN		= 99,								// ʧ�ܣ�����δ֪����
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// �Ƿ����cango
	port_t			nPort;											// CLS�Ķ˿�
	unsigned int	IP;												// CLS��IP
	int				nClientIDInCAAFS;								// �û���CAAFS�е�ID
	int				nClientID;										// �û���GMS�е�ID��������ǽ���������Ϸ�����е�ͳһ����ID
																	// ���ID������CLS��ʱ������ȷ���Լ���Ӧ��Player����
};
struct	GMS_CAAFS_KEEP_QUEUE_POS_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;
	int				nCAAFSIdx;
	char			szAccount[64];
};
////////////////////////////////////////////////////////////////////
// CLS->GMS
////////////////////////////////////////////////////////////////////
struct	CLS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGroupID;										// CLS��ź���Ӧ��CAAFS��Ӧ
	int				nCLSVer;										// CLS�汾
	int				nOldID;											// �ɵ�CLSID�������0���ʾ��������
	unsigned char	nNetworkType;									// �������ͣ�NETWORKTYPE_XXX��
	bool			bCanAcceptPlayerWhenCLSConnectingGZS;			// ��CLS����GZS�Ĺ����������û�����
};
struct	CLS_GMS_HELLO1_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// ��Client�Ķ˿�
	unsigned int	IP;												// ��Client��IP
};
struct	CLS_GMS_CONNECTEDTOGZS_T
{
	enum
	{
		RST_OK		= 0,											// �ɹ�
		RST_FAIL	= 1,											// ����ʧ�ܣ����������ӹ�����GZS�����ˣ�
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// ���ӵĽ��
	unsigned char	nSvrIdx;										// �����ϵ�GZS�����
};
struct	CLS_GMS_CLIENTCANGOTOCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// �û���GMS�е�ID(��ʵҲ����CLS�е�ID)
};
struct	CLS_GMS_CLIENTTOCLS_RST_T
{
	enum
	{
		RST_OK					= 0,								// ���ӳɹ�
		RST_ERR_NOTCONNECT		= 1,								// �ͻ��˸���û��������(��ʱ��)
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// �Ƿ�ɹ�
	int				nClientID;										// �û���GMS�е�ID(��ʵҲ����CLS�е�ID)
};
struct	CLS_GMS_CLIENT_DROP_T
{
	enum
	{
		REMOVEREASON_NOTHING	= 0,
		REMOVEREASON_SENDERR	= 1,								// ��ͻ��˷���������Send���ش��󣨿����ǻ������ˣ�
		REMOVEREASON_CLIENTHACK	= 2,								// �ͻ��˷�����ָ����hack������
		REMOVEREASON_KICKED		= 3,								// �ͻ��˱���������ָ���߳�
		REMOVEREASON_NOTEXIST	= 4,								// �ͻ��˶�Ӧ��ID������
		REMOVEREASON_END		= 5,								// �������������û�����
																	// �����������Ҫ���ͽ����ĳ���ͻ��ˣ�������Ҫ�߳�ĳ���ͻ���ʱ�����ֶ�Ӧ��IDʵ���ϲ����ڡ�����CLS����GMS֮�󣬸���GMSĳЩID�Ѿ��������ˣ���GMSɾ��һ�£�
		REMOVEREASON_AS_DROP	= 6,
		// ǰ���Ӧ�ò��ᳬ��100�ɣ��Ǻ�(�������Щ������ΪClose��ɵ�)
		REMOVEREASON_SLOTCLOSE_REASON_NOTHING		= 100 + CNL2SlotMan::SLOTCLOSE_REASON_NOTHING,
		REMOVEREASON_SLOTCLOSE_REASON_INITIATIVE	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_INITIATIVE,
		REMOVEREASON_SLOTCLOSE_REASON_PASSIVE		= 100 + CNL2SlotMan::SLOTCLOSE_REASON_PASSIVE,
		REMOVEREASON_SLOTCLOSE_REASON_CLOSETimeout	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_CLOSETimeout,
		REMOVEREASON_SLOTCLOSE_REASON_DROP			= 100 + CNL2SlotMan::SLOTCLOSE_REASON_DROP,
		REMOVEREASON_SLOTCLOSE_REASON_CONNECTTimeout= 100 + CNL2SlotMan::SLOTCLOSE_REASON_CONNECTTimeout,
		REMOVEREASON_SLOTCLOSE_REASON_ACCEPTTimeout	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_ACCEPTTimeout,
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRemoveReason;									// �����RemovePlayer֮ǰ�����������˵������һ��ԭ��رյ�
	int				nClientID;										// �û���GMS�е�ID(��ʵҲ����CLS�е�ID)
};
struct	CLS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// �˳�(����GMS�Լ��������˳���)
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
////////////////////////////////////////////////////////////////////
// GMS->CLS
////////////////////////////////////////////////////////////////////
struct	GMS_CLS_HI_T
{
	enum
	{
		RST_OK					= 0x00,								// ���ӳɹ�
		RST_ERR_BAD_VER			= 0x01,								// �汾����
		RST_ERR_BAD_GROUPID		= 0x02,								// GroupID����
		RST_ERR_TOOMANY_CLS		= 0x03,								// ���GroupID��CLS����������Ҫ��
		RST_ERR_MEMERR			= 0x04,								// �ڴ�������
		RST_ERR_UNKNOWN			= 0xFF,								// δ֪����
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nCLSID;											// ��CLS��GMS�е�ID
	int				nGMSMaxPlayer;									// GMS���֧�ֵ��û�����(���ǣ����CLS�е�Player����Ĵ�С)
	int				nCLSMaxConnection;								// GMS�涨��CLS�����Գ��ص��û���������
	int				nGMSVer;										// GMS�汾
};
struct	GMS_CLS_GZSINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// GZS���������(1~GZS_MAX_NUM-1)
	port_t			nPort;											// ��������Ķ˿�
	unsigned int	IP;												// ���������IP
};
struct	GMS_CLS_CLIENTWANTCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// �û���GMS�е�ID(��ʵҲ������CLS�е�ID)
	unsigned int	nPassword;										// ��������(�����CAAFS����GMS��Ȼ��GMS��ת��������)
};
struct	GMS_CLS_ROUTECLIENTDATATOGZS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// �û���GMS�е�ID(��ʵҲ������CLS�е�ID)
	unsigned char	nSvrIdx;										// ��ҪͨѶ��GZS�����(���Ϊ0���ʾ���뿪GZS���ص�GMS�Ļ���)
};
struct	GMS_CLS_PLAYER_CHANGEID_T
{
	pngs_cmd_t		nCmd;
	int				nFromID;
	int				nToID;
	unsigned char	nSvrIdx;										// ��Ҫ����һ��CLS�����û�����Ӧ��GZS����
};
////////////////////////////////////////////////////////////////////
// GZS->GMS
////////////////////////////////////////////////////////////////////
struct	GZS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// GZS���������(1~GZS_MAX_NUM-1)��Ҳ������AuthID
	unsigned char	nForceKickOldGZS;								// ����оɵ�nSvrIdx�����һ���ģ����߳��ɵ�GZS (���һ��ֻ���������߳����������е�GZS�ˣ���Ϊ��TCP���Բ������ԭ���ĵ�����ʱ��������GZS�����ڵ�����)
																	// ��ʵTCP����ֺܳ�ʱ�䲻���ߵ���������Ի���Ҫ��ǿ���߳����μ�GZS_MAINSTRUCTURE::CFGINFO_T::nForceKickOldGZS˵��
	int				nGZSVer;										// GZS�汾
	bool			bReConnect;										// ��ʾ������������µ�
};
struct	GZS_GMS_MYINFO_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// ��������Ķ˿�
	unsigned int	IP;												// ���������IP
};
struct	GZS_GMS_GAMECMD_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nToSvrIdx;										// Ŀ�����ĸ���������ȡֵΪPNGS_SVRIDX_XXX
	char			data[1];										// ������ָ��+���ݲ���
};
struct	GZS_GMS_KICKPALYERBYID_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;
};
struct	GZS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// �˳�(����GMS�Լ��������˳���)
		SUBCMD_STRCMD			= 1,								// �ִ�ָ��
																	// nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_STRRST			= 2,								// �ִ�ָ��ķ���
		SUBCMD_CLSDOWN			= 3,								// ĳ��CLS��ˣ��͸�GZS�����ˣ�
																	// nParam����CLSID
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};

////////////////////////////////////////////////////////////////////
// GMS->GZS
////////////////////////////////////////////////////////////////////
struct	GMS_GZS_HI_T
{
	enum
	{
		RST_OK		= 0,
		RST_IDXDUP	= 2,											// ˵���Լ���Idx�ͱ����ظ���
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGMSMaxPlayer;									// GMS���֧�ֵ��û�����(���ǣ����GZS�е�Player����Ĵ�С)
	int				nGMSMaxCLS;										// GMS���֧�ֵ�CLS����(���ǣ����GZS�е�CLS����Ĵ�С)
	int				nGMSVer;										// GMS�汾
};
struct	GMS_GZS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// �˳�(�����GMS���������������ģ����൱�ڷ������Զ��˳��������GZS���������������ģ����Զ��رպ�GZS������)
		SUBCMD_STRCMD			= 1,								// �ִ�ָ��
																	// nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_STRRST			= 2,								// �ִ�ָ��ķ���
		SUBCMD_CLSDOWN			= 3,								// ĳ��CLS���
																	// nParam����CLSID
		SUBCMD_CHECKPLAYEROFFLINE		= 6,						// ����Ƿ����û��Ѿ������ˣ�GZS������ID�Ƿ���ڣ���������ھ�ֱ��ɾ����Ӧ���û���
																	// nParam���û�ID�������������int����
		SUBCMD_ENABLE_CLIENT_RECV		= 10,						// �����Ƿ���տͻ�������������
		SUBCMD_DISENABLE_CLIENT_RECV	= 11,						// ����Ƿ���տͻ�������������
																	// ���������Ĳ�������nParam����Ӧ�ڼ���bit
																	// �����ȶ�GSDB/GMSGamePlay:0��GZSGamePlay:1
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
struct	GMS_GZS_GAMECMD_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nFromSvrIdx;									// ������ָ�������Ǹ���������ȡֵΪPNGS_SVRIDX_XXX
	char			data[1];										// ������ָ��+���ݲ���
};
struct	GMS_GZS_PLAYER_ONLINE_T
{
	pngs_cmd_t		nCmd;
	int				nCLSID;											// ��Ҷ�Ӧ��CLS��ID(���û�������ôGZS���޷�ֱ�Ӹ�Player������Ϣ��)
	int				anClientID[1];									// �û���GMS�е�ID(��ʵҲ����GZS�е�ID)�������Ƕ��ID����Ϊ��������GZS����GMS֮��GMSһ�η��Ͷ����GZS��
};
struct	GMS_GZS_PLAYER_OFFLINE_T
{
	pngs_cmd_t		nCmd;
	int				anClientID[1];									// �û���GMS�е�ID(��ʵҲ����GZS�е�ID)�������Ƕ��ID��
};
struct	GMS_GZS_PLAYER_CHANGEID_T
{
	pngs_cmd_t		nCmd;
	int				nFromID;
	int				nToID;
};
////////////////////////////////////////////////////////////////////
// CLS->GZS
////////////////////////////////////////////////////////////////////
struct	CLS_GZS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nCLSID;											// �Լ���GMS�е�ID
	int				nCLSVer;										// CLS�汾
};
////////////////////////////////////////////////////////////////////
// GZS->CLS
////////////////////////////////////////////////////////////////////
struct	GZS_CLS_HI_T
{
	enum
	{
		RST_OK				= 0,
		RST_ERR_BAD_VER		= 1,
		RST_ERR_MEMERR		= 2,
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGZSVer;										// GZS�汾
};
////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// CAAFS->CLIENT
////////////////////////////////////////////////////////////////////
struct	CAAFS_CLIENT_CAAFSINFO_T
{
	enum
	{
		SELF_NOTIFY_REFUSEALL			= 0x00000001,				// �ܾ����еĵ�¼
	};
	pngs_cmd_t		nCmd;
	unsigned char	nVerCmpMode;									// �汾�ȽϷ�ʽ
	char			szVer[PNGS_VER_LEN];							// �汾
	unsigned short	nQueueSize;										// ǰ���Ŷӵ�����(����һ�������Ͳ�Ҫ���û������ˣ���������ǰ���Ŷ�����̫��)
	unsigned int	nSelfNotify;									// 32bit��ʾ32����˼
	char			info[1];										// �����ִ�
};
struct	CAAFS_CLIENT_QUEUEINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nVIPChannel;									// ��Ӧ��VIPͨ�����������0�����ڿͻ�����ʾͨ���ţ�
	unsigned short	nQueueSize;										// ǰ���Ŷӵ�����
};
struct	CAAFS_CLIENT_GOTOCLS_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// CLS�˿�
	unsigned int	IP;												// CLS��ַIP
	unsigned int	nPassword;										// ��������
	int				nClientID;										// ���ID
};
////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS
////////////////////////////////////////////////////////////////////
struct	CLIENT_CAAFS_2GMS_T
{
	pngs_cmd_t		nCmd;
	char			data[1];										// ������loginһ�������ݰ�������������߼����ݣ�����Ͳ�д�ˡ�
};
////////////////////////////////////////////////////////////////////
// CLIENT->CLS
////////////////////////////////////////////////////////////////////
// ���ӵ�ͬʱ�ͻ��˸�����һЩ��Ϣ���磺�û�ID�����룩
struct	CLIENT_CLS_CONNECT_EXTINFO_T								// ע�⣺�������һ��ָ�ֻ�������������Ĳ���
{
	int				nClientID;										// ���ID
	unsigned int	nPassword;										// �û���������CLS������
};
// �û�����
struct	CLIENT_CLS_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// ��������ָ��һ���Ǹ�˭�ġ�ȡֵ��PNGS_SVRIDX_XXX
																	// PNGS_SVRIDX_AUTOGZS��ʾ����ǰָ�������GZS
																	// PNGS_SVRIDX_GMS��ʾ��GMS
																	// �����ı�ʾ������˳���GZS
	char			data[1];
};
////////////////////////////////////////////////////////////////////
// CLS->CLIENT
////////////////////////////////////////////////////////////////////
// �û�����
struct	CLS_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	char			data[1];
};
struct	CLS_CLIENT_DATA_PACK_T
{
	pngs_cmd_t		nCmd;
	char			data[1];
};
struct	CLS_CLIENT_NOTIFY_T
{
	enum
	{
		SUBCMD_GZSDROP	= 0x01,			// �û����ڵ�GZS崻��ˣ����û��ȴ�������
		SUBCMD_STATON	= 0x02,			// ����stat
		SUBCMD_STATOFF	= 0x03,			// �ر�stat
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	char			data[1];
};
////////////////////////////////////////////////////////////////////
// CLS����������͵Ĺ���ָ��
////////////////////////////////////////////////////////////////////
struct	CLS_SVR_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;
	int				nClientID;
	char			data[1];										// ����
};
////////////////////////////////////////////////////////////////////
// CLS�յ��ķ����������Ĺ���ָ��
////////////////////////////////////////////////////////////////////
struct	SVR_CLS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// �˳�
																	// nParam������
		SUBCMD_STRCMD			= 1,								// �ִ�ָ��
																	// nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_STRRST			= 2,								// �ִ�ָ��ķ���
																	// nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_KICKPALYERBYID	= 3,								// ����ID�߳�ĳ���û�
																	// nParamΪPlayerID
		SUBCMD_DROPGZS			= 4,								// ��GZS����(�����GMS����GZS���ߺ�֪ͨCLS�ģ������Ҫ��������ֹGZS��GMS�������������)
																	// nParamΪSvrIdx
		SUBCMD_KICKPLAYEROFGZS	= 5,								// ��CLS�߳����к�ĳ��GZS��ص����
																	// nParamΪSvrIdx
		SUBCMD_CHECKPLAYEROFFLINE		= 6,						// ����Ƿ����û��Ѿ������ˣ����ֻ��GMS�ᷢ����CLS������ID�Ƿ���ڣ���������ھ�������GMS�����û�������Ϣ��
																	// nParam���û�ID�������������int����
		SUBCMD_KICKPALYERBYID_AS_DROP	= 7,						// ��SUBCMD_KICKPALYERBYID����һ������������GMS�����û���DROP�Ľ��
		SUBCMD_KICKPALYERBYID_AS_END	= 8,						// ��SUBCMD_KICKPALYERBYID����һ������������GMS�����û��������˳��Ľ���������kick�û������ٱ�������Ϸ���ӳ�һ��ʱ���ˣ�
		SUBCMD_SET_NAME			= 9,								// �����û��ִ�������Ҫ���ڼ�¼��־����nParamΪ�û�ID����������ִ�
		SUBCMD_PLAYER_STAT_OFF	= 10,								// �ر�ͳ�ƣ� nParamΪClientID
		SUBCMD_PLAYER_STAT_ON	= 11,								// ����ͳ�ƣ� nParamΪClientID�����滹��һ��������ʾ���ͼ��
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
struct	SVR_CLS_SET_TAG_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��Ŷ�Ӧ��ֵ��ע�⣺nTagVal����Ϊ0��
	int				nClientID;										// ���ID
};
struct	SVR_CLS_SET_PROP_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nProp;											// ���ԣ�����ΪPNGS_CLS_PLAYER_PROP_LOG_XXX����pngs_def.h�У�
	int				nClientID;										// ���ID
};
struct	SVR_CLS_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	int				nClientID;										// ���ID
	char			data[1];										// ����
};
struct	SVR_CLS_MULTICLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	unsigned short	nClientNum;										// �������ClientID������
	int				anClientID[1];									// nClient���鿪ͷ
	// ��һЩ������֮�󣬺��滹�и��ӵ����ݾ�����ʵ�����ݲ���
	int	GetDSize(int nTotalSize)
	{
		return	nTotalSize - wh_offsetof(SVR_CLS_MULTICLIENT_DATA_T, anClientID) - nClientNum*sizeof(int);
	}
	void *	GetDataPtr()
	{
		return	anClientID + nClientNum;
	}
	static int	GetTotalSize(int nCNum, int nDSize)
	{
		return	wh_offsetof(SVR_CLS_MULTICLIENT_DATA_T, anClientID) + nCNum*sizeof(int) + nDSize;
	}
};
struct	SVR_CLS_TAGGED_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	unsigned char	nTagIdx;										// nTag������ֵ
	short			nTagVal;										// ��������ֵ(���е�Ĭ��ֵ����0)
	char			data[1];										// ����
};
struct	SVR_CLS_ALL_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	unsigned char	nSvrIdx;										// ��Ӧ�ķ�������ţ�����ΪPNGS_SVRIDX_XXX��	2007-10-26�ӣ�Ϊ�˽�������ĳЩGZS���û����͹㲥����
	char			data[1];										// ����
};
struct	SVR_CLS_SET_TAG64_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// ���ID
	whuint64		nTag;
	bool			bDel;											// ���Ϊ�����ʾɾ����ҵ�������
};
struct	SVR_CLS_TAGGED64_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// ȡֵ������PNGS_SVRIDX_XXX������1~15��ֵ
	unsigned char	nChannel;										// ��Ӧ��ͨ��
	whuint64		nTag;
	char			data[1];										// ����
};

struct	GMTOOL_SVR_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGMTOOLVer;										// GMTOOL�İ汾
};
struct	GMTOOL_SVR_CMD_T
{
	pngs_cmd_t		nCmd;
	int				nExt;											// ��Ҫԭ�����ص�����
	char			data[1];										// ����
};
struct	SVR_GMTOOL_HI_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrType;										// ����������APPTYPE_XXX��������pngs_def.h�У�
	int				nVer;											// �������汾
};
struct	SVR_GMTOOL_CMD_RST_T
{
	pngs_cmd_t		nCmd;
	int				nExt;											// ԭ�����ص�����
	char			data[1];										// ����
};
////////////////////////////////////////////////////////////////////
// �ͻ�������CAAFSʱ�������ĸ�������
////////////////////////////////////////////////////////////////////
struct	CLIENT_CONNECT_CAAFS_EXTINFO_T
{
	struct	CMN_T
	{
		unsigned char	nNetworkType;								// NETWORKTYPE_XXX
		CMN_T()
			: nNetworkType(NETWORKTYPE_UNKNOWN)
		{
		}
	}cmn;
	char	junk[32-sizeof(CMN_T)];
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_H__
