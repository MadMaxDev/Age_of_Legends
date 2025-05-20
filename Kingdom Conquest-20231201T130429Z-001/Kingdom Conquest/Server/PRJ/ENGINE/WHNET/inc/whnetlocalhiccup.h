// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup.h
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP�ľ������ڵ�Ӧ�ó�����Ȩ����
//              : hiccup�Ǵ��õ���˼��ֻ������뵽����ʾ���ô����:P
// CreationDate : 2006-06-29
// ChangeLog    : 

#ifndef	__WHNETLOCALHICCUP_H__
#define	__WHNETLOCALHICCUP_H__

#include "whnetcmn.h"

namespace n_whnet
{

class	whnetlocalhiccup_client
{
public:
	static whnetlocalhiccup_client *	Create();
public:
	// ��ʼ������
	struct	INFO_T
	{
		char	szDst[256];				// Ŀ���ַ���飨Ŀ���ַ���ö��ŷָ���
		char	szPass[64];				// ���͵�У������
		int		(*pCallBack)(const char *pszCmd, void *pParam);
										// �ϲ���Ļص����������ʵ���ʱ���֪ͨ�ϲ㣨����У��ɹ���ʧ�ܣ���Ϣ��ʾ�ȵȣ�
										// ����pszCmd��ָ���Ŀǰ�ķ��ض����У�
										// Succeed		���Ӳ������֤�ɹ���ֻ���յ�������ܼ���������
										// Fail			�����еĵ�ַ����ʧ��
										// Msg <msgstr>	��ӡ��һ����ʾ��Ϣ
										// ����pParam���ϲ㴫�ĸ��Ӳ��������ϲ㶨�岢����ʹ�á�
		void	*pCBParam;				// ����pCallBack��Ҫʹ�õ�pParam����
		INFO_T();
	};
public:
	virtual	~whnetlocalhiccup_client()	{};
	virtual	int	Init(INFO_T *pInfo)		= 0;
	virtual	int	Release()				= 0;
	virtual	int	Tick()					= 0;
	virtual	SOCKET GetSocket() const	= 0;

	// ����һ���߳����������
	static int	DoItInThread(INFO_T *pInfo);
	static int	StopTheThread();
};

class	whnetlocalhiccup_server
{
public:
	static whnetlocalhiccup_server *	Create();
public:
	// ��ʼ������
	struct	INFO_T
	{
		char	szBindAddr[WHNET_MAXADDRSTRLEN];
		int		nMaxConnection;			// ������������
		int		nPassTimeOut;			// �ȴ��û���������ĳ�ʱ
		char	szPass[64];				// ��Ҫ�յ���У������
		INFO_T();
	};
public:
	virtual	~whnetlocalhiccup_server()	{};
	virtual	int	Init(INFO_T *pInfo)		= 0;
	virtual	int	Release()				= 0;
	virtual	int	Tick()					= 0;
	virtual	SOCKET GetSocket() const	= 0;
};

}		// EOF namespace n_whnet

#endif
