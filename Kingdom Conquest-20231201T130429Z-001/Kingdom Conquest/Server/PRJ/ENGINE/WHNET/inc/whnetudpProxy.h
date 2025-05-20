// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpProxy.h
// Creator      : Wei Hua (κ��)
// Comment      : UDP�˿�ת������
//              : �������յ��İ���ת����ָ��λ�ã�һ������ֻ�󶨼���һ���˿ڽ���ת��������ͨ��console����UDP���ƶ˿ڽ��ж�����������ӳ�����
//              : һ��û�г��ֹ��ĵ�ַ���͵������˿ھͻᴴ��һ��FORWARD����
// CreationDate : 2006-12-19
// ChangeLog    : 

#ifndef	__WHNETUDPPROXY_H__
#define	__WHNETUDPPROXY_H__

#include "./whnetcmn.h"

namespace n_whnet
{

// �ӿڶ���
class	whnetudpProxy
{
public:
	static whnetudpProxy *	Create();
protected:
	virtual	~whnetudpProxy()										{};
public:
	struct	INFO_T
	{
		char		szLocalAddr[WHNET_MAXADDRSTRLEN];				// ����������ַ
		char		szDestAddr[WHNET_MAXADDRSTRLEN];				// FORWARDĿ���ַ
		char		szCtrlAddr[WHNET_MAXADDRSTRLEN];				// ���ڽ��տ��ư��ĵ�ַ
		int			nMaxFORWARD;									// ������֧��FORWARD��Դ�ĸ���
		int			nTQChunkSize;									// ʱ����еķ�����С
		// ��֮���Ӧ�ö�������ʱ������ͨ��ResetInfo��
		int			nPacketSize;									// ����������󳤶�
		int			nDropTimeOut;									// ���һ��FORWARD�û���Դ��ָ��ʱ����û��������û���ձ��������FORWARD��ʧЧ�����룩
		int			nUpLostRate;									// ���ж����ʣ����û�������Ŀ��Ķ����ʣ����ٷ��ʣ�0~99����10��ʾ10%��
		int			nDownLostRate;									// ���ж����ʣ���Ŀ�귢�ظ��û��Ķ����ʣ����ٷ��ʣ�0~99����10��ʾ10%��
		int			nUpDelay0, nUpDelay1;							// �����ӳٷ�Χ�����룩
		int			nDownDelay0, nDownDelay1;						// �����ӳٷ�Χ�����룩

		INFO_T()
			: nMaxFORWARD(64)
			, nTQChunkSize(512)
			, nPacketSize(4096)
			, nDropTimeOut(20000)
			, nUpLostRate(0)
			, nDownLostRate(0)
			, nUpDelay0(0), nUpDelay1(0)
			, nDownDelay0(0), nDownDelay1(0)
		{
			// ��������ַ���������õ�
			szLocalAddr[0]	= 0;
			szDestAddr[0]	= 0;
			szCtrlAddr[0]	= 0;
		};
	};
public:
	// ��������
	virtual	void	SelfDestroy()									= 0;
	// ��ʼ��
	virtual	int		Init(const INFO_T *pInfo)						= 0;
	// ��������ĳЩ�������糬ʱ�������ʡ��ӳ٣�
	virtual	int		ResetInfo(const INFO_T *pInfo)					= 0;
	// �ս�
	virtual	int		Release()										= 0;
	// һ�ι�����nMS�Ǹ��ڲ�select��ʱ�䣩
	virtual	int		Tick(int nMS)									= 0;
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETUDPPROXY_H__
