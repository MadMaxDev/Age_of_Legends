#ifndef __LPNet_H__
#define __LPNet_H__

#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_cmn.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"

namespace n_pngs
{
class CLPNet
{
public:
	//////////////////////////////////////////////////////////////////////////
	// ����ָ����û�
	//////////////////////////////////////////////////////////////////////////
	virtual int		SendCmdToClient(int nClientID, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToMultiClient(int* aClientID, int nClientNum, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToClientWithTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToClientWithTag64(whuint64 nTag, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToAllClient(const void* pCmd, size_t nSize) = 0;
	virtual int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal) = 0;
	virtual int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel) = 0;

	//////////////////////////////////////////////////////////////////////////
	// �����ݿⷢ���첽ָ��
	//////////////////////////////////////////////////////////////////////////
	virtual	int		SendCmdToDB(const void* pData, size_t nSize) = 0;

	//////////////////////////////////////////////////////////////////////////
	// ������Ϣ��ȫ�ַ�����
	//////////////////////////////////////////////////////////////////////////
	virtual	int		SendCmdToGS(const void* pData, size_t nSize)	= 0;

	//////////////////////////////////////////////////////////////////////////
	// ���û�����
	//////////////////////////////////////////////////////////////////////////
	virtual	int		KickPlayerOffline(int nClientID, pngs_cmd_t nKickSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID) = 0;

	//////////////////////////////////////////////////////////////////////////
	// ��������
	//////////////////////////////////////////////////////////////////////////
	virtual	PlayerData*		GetPlayerData(tty_id_t nAccountID)	= 0;
	virtual	unsigned int	GetPlayerAppID(unsigned int nClientID)		= 0;
	virtual	void	SendPlayerCharAtbToClient(PlayerData* pPlayer, bool bSimple=true)		= 0;
	virtual	void	RefreshPlayerCharAtbFromDB(tty_id_t nAccountID, bool bSimple=true, bool bCheckOnline=false)		= 0;
	virtual	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, unsigned int nNum, tty_id_t*	pHeroIDs, unsigned int nReason)		= 0;
	// ���õ�m_vectrawbuf
	virtual	int		AddMail(tty_id_t nSenderID, const char*	cszSenderName, tty_id_t nReceiverID, bool bNeedRst, unsigned char nType, unsigned char nFlag, bool bSendLimit, int nTextLen, const char* cszText, int nExtDataLen, void* pExtData, int pAddedExtData[PNGSPACKET_2DB4WEB_EXT_NUM])	= 0;
	// ��ʯ��ֵ
	virtual	int		RechargeDiamond(tty_id_t nAccountID, unsigned int nAddedDiamond, bool bFromGS=false, const int* pExt=NULL)	= 0;
	// �ⲿ֪ͨ�Լ�
	virtual	int		NotifyMeRefreshLevelRankOfPlayerCard(bool bLoadLevelRankOK)				= 0;
	// ��ֹע��ʽ����
	virtual	int		AntiSQLInjectionAttack(int nMaxLen, char* szStr)	= 0;
	// �����Ƿ�Ϸ�(Ŀǰ�ǲ������пո�)
	virtual bool	IsCharNameValid(const char* cszName)				= 0;
	virtual	bool	IsAccountNameValid(const char* cszName)				= 0;
	// ��¼��Ǯ������־
	// 1.money_use_type_buy_item,nExtData0��excel_id,nExtData1��num
	// 2.money_use_type_accelerate_build,nExtData0�Ǽ��ٵ�ʱ��,nExtData1��excel_id,nExtData2��auto_id
	// 3.money_use_type_accelerate_goldore,nExtData0�Ǽ��ٵ�ʱ��,nExtData1��excel_id,nExtData2��auto_id
	// 4.money_use_type_accelerate_research,nExtData0�Ǽ��ٵ�ʱ��,nExtData1��excel_id,nExtData2��auto_id
	// 5.money_use_type_accelerate_march,nExtData0�Ǽ��ٵ�ʱ��
	// 6.money_use_type_lottery
	// 7.money_use_type_change_char_name
	// 8.money_use_type_change_hero_name
	virtual void	AddMoneyCostLog(tty_id_t nAccountID, unsigned int nUseType, unsigned int nMoneyType, unsigned int nMoney
		, unsigned int nExtData0=0, unsigned int nExtData1=0, unsigned int nExtData2=0, unsigned int nExtData3=0)	= 0;
	// gold_produce_xxx
	virtual void	AddGoldProduceLog(tty_id_t nAccountID, unsigned int nProduceType, unsigned int nGold)	= 0;
	// gold_cost_xxx
	virtual void	AddGoldCostLog(tty_id_t nAccountID, unsigned int nCostType, unsigned int nGold)	= 0;

	virtual	void	GetPlayerCardFromDB(tty_id_t nAccountID)		= 0;

	// ȫ����������Ƶ���㲥֪ͨ
	virtual	void	AddNotifyInWorldChat(unsigned int nTextLen, const char* cszText, tty_id_t nCharID=0)	= 0;

	// ��ȡ�������
	virtual	unsigned char		GetSvrGroupIndex()	= 0;
};
extern CLPNet*	g_pLPNet;
}

#endif
