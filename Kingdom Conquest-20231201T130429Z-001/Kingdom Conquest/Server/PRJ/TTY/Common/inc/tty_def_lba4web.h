#ifndef __tty_def_lba4web_H__
#define __tty_def_lba4web_H__

namespace n_pngs
{
	enum
	{
		P_LBA4Web_CMD_BEGIN				= 10000,
		// 校验帐号密码
		P_LBA4Web_LOGIN_VERIFY_REQ		= P_LBA4Web_CMD_BEGIN + 1,
		P_LBA4Web_LOGIN_VERIFY_RPL		= P_LBA4Web_CMD_BEGIN + 2,
		// 登录
		P_LBA4Web_LOGIN_REQ				= P_LBA4Web_CMD_BEGIN + 3,
		P_LBA4Web_LOGIN_RPL				= P_LBA4Web_CMD_BEGIN + 4,
		// 登出
		P_LBA4Web_LOGOUT_REQ			= P_LBA4Web_CMD_BEGIN + 5,
		P_LBA4Web_LOGOUT_RPL			= P_LBA4Web_CMD_BEGIN + 6,
	};

#pragma pack(1)
	// 基类,其他都是派生的
	struct P_LBA4Web_CMD_T 
	{
		int			nCmd;
	};
	struct P_LBA4Web_LOGIN_VERIFY_REQ_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		char		szAccount[TTY_ACCOUNTNAME_LEN];
		int			nTermType;
		char		szCltIP[WHNET_MAXIPSTRLEN];
		int			nPassLen;
		char		szPass[1];
	};
	struct P_LBA4Web_LOGIN_VERIFY_RPL_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_rst_t	nRst;
	};
	struct P_LBA4Web_LOGIN_REQ_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_id_t	nAccountID;
		char		szAccount[TTY_ACCOUNTNAME_LEN];
	};
	//////////////////////////////////////////////////////////////////////////
	struct P_LBA4Web_LOGIN_RPL_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_rst_t	nRst;
	};
	// 当要用到这个指令的时候,说明已经登录成功了
	struct P_LBA4Web_LOGOUT_REQ_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_id_t	nAccountID;
		char		szAccount[TTY_ACCOUNTNAME_LEN];
	};
	// 这个没意义,暂时登出不会返回结果给LP
	struct P_LBA4Web_LOGOUT_RPL_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_rst_t	nRst;
	};
#pragma pack()
}

#endif
