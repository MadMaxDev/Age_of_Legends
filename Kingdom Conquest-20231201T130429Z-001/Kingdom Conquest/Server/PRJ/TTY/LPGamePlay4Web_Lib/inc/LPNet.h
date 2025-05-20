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
	// 发送指令给用户
	//////////////////////////////////////////////////////////////////////////
	virtual int		SendCmdToClient(int nClientID, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToMultiClient(int* aClientID, int nClientNum, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToClientWithTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToClientWithTag64(whuint64 nTag, const void* pCmd, size_t nSize) = 0;
	virtual int		SendCmdToAllClient(const void* pCmd, size_t nSize) = 0;
	virtual int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal) = 0;
	virtual int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 向数据库发送异步指令
	//////////////////////////////////////////////////////////////////////////
	virtual	int		SendCmdToDB(const void* pData, size_t nSize) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 发送消息到全局服务器
	//////////////////////////////////////////////////////////////////////////
	virtual	int		SendCmdToGS(const void* pData, size_t nSize)	= 0;

	//////////////////////////////////////////////////////////////////////////
	// 踢用户下线
	//////////////////////////////////////////////////////////////////////////
	virtual	int		KickPlayerOffline(int nClientID, pngs_cmd_t nKickSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 辅助功能
	//////////////////////////////////////////////////////////////////////////
	virtual	PlayerData*		GetPlayerData(tty_id_t nAccountID)	= 0;
	virtual	unsigned int	GetPlayerAppID(unsigned int nClientID)		= 0;
	virtual	void	SendPlayerCharAtbToClient(PlayerData* pPlayer, bool bSimple=true)		= 0;
	virtual	void	RefreshPlayerCharAtbFromDB(tty_id_t nAccountID, bool bSimple=true, bool bCheckOnline=false)		= 0;
	virtual	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, unsigned int nNum, tty_id_t*	pHeroIDs, unsigned int nReason)		= 0;
	// 会用到m_vectrawbuf
	virtual	int		AddMail(tty_id_t nSenderID, const char*	cszSenderName, tty_id_t nReceiverID, bool bNeedRst, unsigned char nType, unsigned char nFlag, bool bSendLimit, int nTextLen, const char* cszText, int nExtDataLen, void* pExtData, int pAddedExtData[PNGSPACKET_2DB4WEB_EXT_NUM])	= 0;
	// 钻石充值
	virtual	int		RechargeDiamond(tty_id_t nAccountID, unsigned int nAddedDiamond, bool bFromGS=false, const int* pExt=NULL)	= 0;
	// 外部通知自己
	virtual	int		NotifyMeRefreshLevelRankOfPlayerCard(bool bLoadLevelRankOK)				= 0;
	// 防止注入式攻击
	virtual	int		AntiSQLInjectionAttack(int nMaxLen, char* szStr)	= 0;
	// 名字是否合法(目前是不允许有空格)
	virtual bool	IsCharNameValid(const char* cszName)				= 0;
	virtual	bool	IsAccountNameValid(const char* cszName)				= 0;
	// 记录金钱花销日志
	// 1.money_use_type_buy_item,nExtData0是excel_id,nExtData1是num
	// 2.money_use_type_accelerate_build,nExtData0是加速的时间,nExtData1是excel_id,nExtData2是auto_id
	// 3.money_use_type_accelerate_goldore,nExtData0是加速的时间,nExtData1是excel_id,nExtData2是auto_id
	// 4.money_use_type_accelerate_research,nExtData0是加速的时间,nExtData1是excel_id,nExtData2是auto_id
	// 5.money_use_type_accelerate_march,nExtData0是加速的时间
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

	// 全服聊天世界频道广播通知
	virtual	void	AddNotifyInWorldChat(unsigned int nTextLen, const char* cszText, tty_id_t nCharID=0)	= 0;

	// 获取大区序号
	virtual	unsigned char		GetSvrGroupIndex()	= 0;
};
extern CLPNet*	g_pLPNet;
}

#endif
