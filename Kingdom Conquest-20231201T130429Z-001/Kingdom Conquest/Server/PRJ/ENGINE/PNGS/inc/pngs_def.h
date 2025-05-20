// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_def.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��һЩ��������
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-07-28
// Change LOG   : 2006-08-30 ��PNGS_DFT_HELLOW_TIMEOUT��Ϊ������

#ifndef	__PNGS_DEF_H__
#define	__PNGS_DEF_H__

namespace n_pngs
{

// ������������
enum
{
	PNGS_DFT_SVRCONNECT_TIMEOUT	= 20000,							// ������֮�����ӵ�Ĭ�ϳ�ʱ(����)
	PNGS_DFT_HELLOW_TIMEOUT		= 4*60*1000,						// �ȴ�Hello���ְ���Ĭ�ϳ�ʱ(����)
	//	windows��ֻ��֧����ô��,modified by yuezhongyue 2011-10-18
#ifdef	WIN32
	GZS_MAX_NUM					= 26,								// ����GZS����������һ���0���ǲ��õ�
#else
	GZS_MAX_NUM					= 52,								// ����GZS����������һ���0���ǲ��õ�
#endif
	GZS_STRCMD_RST_LEN_MAX		= 4096,								// ��������ص��ִ�ָ��س��ȵ����ֵ
	CAAFS_CHANNEL_INOUT_NUM		= 16,								// CAAFS��ͻ���ͨѶ��Channel��In��Out���еĳ���
	GMS_MAX_CAAFS_NUM			= 4,								// ����CAAFS������
																	// ���Ҳ����������CAAFS����Group������
	GMS_DFT_PORT				= 3000,								// GMSĬ�ϱ�����Server���ӵĶ˿�
	PNGS_MAX_SOCKETTOSELECT		= 512,								// �ɹ�ѡ������socket����
	// һ��Ԥ����ָ���Ĵ�С
	PNGS_RAWBUF_SIZE			= 512*1024,
	// �汾���ĳ���
	PNGS_VER_LEN				= 32,
	// �߼����ֵĳ���
	PNGS_LOGICNAME_LEN			= 32,
	// һ��tick�������ٺ������Ϊ��big tick
	PNGS_BIG_TICK_LIMIT			= 200,
	// �ͻ��˺�CLSͨѶ��SLOTINOUTNUM
	PNGS_SLOTINOUTNUM			= 256,
	// added by yuezhongyue, web extension
	LP_MAX_CAAFS4Web_NUM		= GMS_MAX_CAAFS_NUM,			// ����CAAFS������
};
enum
{
	PNGS_SVRIDX_AUTOGZS			= 0xFF,
	PNGS_SVRIDX_ALLGZSEXCEPTME	= 0xFE,								// ���͸����е�GZS��������Դ���Ǹ�
	PNGS_SVRIDX_ALLGZS			= 0xFA,								// ���͸����е�GZS��Ҳ���Խ���Ϊ���͸������û�������GMS�ϵ��û�����
	PNGS_SVRIDX_LBA				= 29,								// ����64bitͳһIDЭ���޸ġ�
	PNGS_SVRIDX_GMS				= 30,								// ����64bitͳһIDЭ���޸ġ�
	PNGS_SVRIDX_HYDBS			= 31,								// ����64bitͳһIDЭ���޸ġ�
	PNGS_SVRIDX_GZS0			= 0,								// ����64bitͳһIDЭ���޸ġ���0�������Ǳ�������ʹ�õģ�������GZS����Ǵ�1~15�������Goָ����ʹ����������ʾ��Ҫ�뿪��Ϸ��ZOUT��
};
enum
{
	PNGS_CLIENTTAG_RESERVED		= 0,
	PNGS_CLIENTTAG_ONEGZS		= 1,								// ��ͬһ��GZS�ڵ���Ҷ����Է��������Ϊֵ
																	// ��������ڱ�Ƿ���ʱ��ͬһ��GZS�е�������ҷ�����ͬ��ָ��
};
enum
{
	PNGS_CLS_PLAYER_PROP_LOG_SEND		= 0x01,						// ����ͻ��˵����ݼ�¼����־��
	PNGS_CLS_PLAYER_PROP_LOG_RECV		= 0x02,						// �ӿͻ����յ������ݼ�¼����־��
	PNGS_CLS_PLAYER_PROP_LOG_STAT		= 0x04,						// ��������ͳ�Ʋ���ʱ�������־��
	PNGS_CLS_PLAYER_PROP_ALL	= 0xFF
};

// ����Ӧ�ó�������ͱ��
// ���磺GMS�͸���������ж������Լ��ĳ�����ʲô
enum	APPTYPE
{
	APPTYPE_NOTHING				= 0,
	APPTYPE_GMS					= 1,
	APPTYPE_CAAFS				= 2,
	APPTYPE_CLS					= 3,
	APPTYPE_GZS					= 4,
	APPTYPE_LBA					= 5,
	APPTYPE_DBA					= 6,
	APPTYPE_GMTOOL				= 11,

	// added by yuezhongyue for web extension
	APPTYPE_CLS4WEB				= 12,
	APPTYPE_CAAFS4WEB			= 13,
	APPTYPE_LP					= 14,
	APPTYPE_LBA4Web				= 15,
};

// ��Client�Ĺرղ���
enum
{
	CLOSE_PARAM_TO_CLIENT_SVRMEM		= 1,						// �������ڴ�������
	CLOSE_PARAM_TO_CLIENT_BADCMD		= 2,						// �ͻ��˷�����ָ��������������
	CLOSE_PARAM_TO_CLIENT_NOIDINCLS		= 3,						// �ͻ��˶�Ӧ����Ҷ�����CLS�в�����
	CLOSE_PARAM_TO_CLIENT_PLAYERSTATUSNOTCONNECTING	= 4,			// CLS�е�Player����connecting״̬
	CLOSE_PARAM_TO_CLIENT_ERR_EXTINFO	= 5,						// ����ĸ�����Ϣ
};

}		// EOF namespace n_pngs

// ��������
#define	PNGS_DFT_CAAFS_SELFINFO_STRING	"BO2 CAAFS2"

#define	PNGS_GSMEM_NAME_AM				"::am"

#define	PNGS_DLL_NAME_GMS_MAINSTRUCTURE	"uGMS_MAINSTRUCTURE"
#define	PNGS_DLL_NAME_GZS_MAINSTRUCTURE	"uGZS_MAINSTRUCTURE"
#define	PNGS_DLL_NAME_GMS_GAMEPLAY		"uGMS_GAMEPLAY"
#define	PNGS_DLL_NAME_GZS_GAMEPLAY		"uGZS_GAMEPLAY"
#define	PNGS_DLL_NAME_CLIENT_GAMEPLAY	"uCLIENT_GAMEPLAY"
#define	PNGS_DLL_NAME_GSMEM				"uGS_MEM"

// pngs��ָ������(1byte�㹻��)
typedef	unsigned char					pngs_cmd_t;

// CLS�û��û���ǵ��������ͣ���ʱ�����ˣ�
typedef	unsigned int					pngs_tag_t;

#endif	// EOF __PNGS_DEF_H__
