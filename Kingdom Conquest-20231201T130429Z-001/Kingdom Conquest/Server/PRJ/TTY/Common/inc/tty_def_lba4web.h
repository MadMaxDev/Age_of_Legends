#ifndef __tty_def_lba4web_H__
#define __tty_def_lba4web_H__

namespace n_pngs
{
	enum
	{
		P_LBA4Web_CMD_BEGIN				= 10000,
		// У���ʺ�����
		P_LBA4Web_LOGIN_VERIFY_REQ		= P_LBA4Web_CMD_BEGIN + 1,
		P_LBA4Web_LOGIN_VERIFY_RPL		= P_LBA4Web_CMD_BEGIN + 2,
		// ��¼
		P_LBA4Web_LOGIN_REQ				= P_LBA4Web_CMD_BEGIN + 3,
		P_LBA4Web_LOGIN_RPL				= P_LBA4Web_CMD_BEGIN + 4,
		// �ǳ�
		P_LBA4Web_LOGOUT_REQ			= P_LBA4Web_CMD_BEGIN + 5,
		P_LBA4Web_LOGOUT_RPL			= P_LBA4Web_CMD_BEGIN + 6,
	};

#pragma pack(1)
	// ����,��������������
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
	// ��Ҫ�õ����ָ���ʱ��,˵���Ѿ���¼�ɹ���
	struct P_LBA4Web_LOGOUT_REQ_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_id_t	nAccountID;
		char		szAccount[TTY_ACCOUNTNAME_LEN];
	};
	// ���û����,��ʱ�ǳ����᷵�ؽ����LP
	struct P_LBA4Web_LOGOUT_RPL_T : public P_LBA4Web_CMD_T
	{
		int			nTransactionID;
		tty_rst_t	nRst;
	};
#pragma pack()
}

#endif
