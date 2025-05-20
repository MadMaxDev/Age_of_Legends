// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : lHttpGet.h
// Creator      : ������
// Comment      : �򵥵�http GET����ķ�װ,���Դ���Դ��ĳ���㴫��(����������֧�ֶϵ�����)
// CreationDate : 2006.07-28
// ChangeLOG    : 2006.11.23 �����������ִ����Ժ�״̬û��ת����nothing��û�йر��׽��ֵ�bug, ����tick�жԿ���״̬�ļ��
//				  2007.01.16 �ı�ص�����������������һ��void *���ڰ�httpget����һ������ʱ�����Է��㴫�������Ǹ���Ķ����ָ��
//								get��������һ�����������ûص��ý��յ���void *

#ifndef		LANE_LHTTPGET_H
#define		LANE_LHTTPGET_H

#include <string>
#include "../inc/lSocket.h"

namespace n_whnet
{
	
// ��Ҫʹ�����Լ�ע��Ļص�����,�������˹涨�ĳߴ�������Ժ���Զ������������,
// ��һ�������ǽ��ܵ������ݵĻ���,�ڶ����ǽ��ܵ��ĳߴ�,�������������������Ƿ����.
typedef		int (* pfn_onget)	( const char * buf, size_t buflen, bool over, void * expptr );

class	httpget
{
public:
	
	// ��ǰhttpget����������״̬,����ͨ������getcurstate()��ȡ��ǰ��״̬
	enum	state_t
	{
		nothing,		// ʲôҲû��
		connecting,		// ���ڽ�������
		prochead,		// �����յ���httpͷ
		procdata,		// ������ܵ�������
		memerr = -1		// �ڴ治��
	};

	// tick()������get()�������ܷ��صĴ���
	enum	err_t
	{
		complete = 1,				// get������ɣ���ȷ���
		noerr	 = 0,				// û�д�������get�������ڴ���
		nomemerr = -1,				// �ڴ治�㣬����
		connerr  = -2,				// ���Ӵ��󣬴���
		sockerr	 = -3,				// socket���ִ��󣬴���
		httprequesterr	 = -4,		// http������󣬴���
		httpservererr   = -5,		// http���������󣬴���
		httpunknownerr = -6,		// httpδ֪���󣬴���
		unknownerr = -1024,			// δ֪���󣬴���
	};

	// httpЭ��ķ���ֵ����get()����tick()���ش���󣬿���ͨ��gethttpret()ȷ��http�ķ���ֵ
	enum	httpret
	{
		// �ɹ�
		http_OK							= 200,	// OK
		http_create						= 201,	//
		http_accepted					= 202,	//
		http_noconent					= 203,	//

		// �������
		http_badrequest					= 400,	//
		http_unauthorized				= 401,	//
		http_forbidden					= 403,	// 
		http_notfound					= 404,	// 

		// ����˴���
		http_internal_server_error		= 500,
		http_not_implemented			= 501,	//
		http_bad_gateway				= 502,	//
		http_service_unavailable		= 503,	// 
	};

	
	// ���캯��,�����������Ӻ�����
	// ����:��������,���û�о���ΪNULL��
	//		������ip����������ȷ��
	//		�ص�������ָ�롣
	//		ÿ�β����ص�Ҫ��Ļ���ߴ磬�������ܵ�callbacksize�ߴ�������Ժ�����pcallbackfnָ��ĺ���
	// ע�⣺�����ص�ʱ�����ݵĽ��ܵ������ݵĳߴ����С��callbacksize��һ��ֻ���ڵ�һ�λص������һ�λص�ʱ�Ų�����
	httpget( const char * host, const char * ip, pfn_onget pcallbackfn, size_t callbacksize, unsigned short int port = 80 );

	// �����������ͷ���Դ���Ͽ�����
	~httpget();

	// ����������������һ��get����resourceΪ������ļ���Դ��startposΪ�������Դ�Ŀ�ʼλ�ã���������֧�ֶϵ������� timeoutΪconnect���ӳ�ʱ��ʱ��
	int	get( const char * resource, size_t startpos, void * exp,int timeout);

	// ��ȡ����get����һ���ᴫ�͵����ݴ�С
	unsigned int	gettotalsize()
	{
		return m_uTotalTransSize;	
	}
	
	// ��ȡ�Ѿ���������ݵĴ�С
	unsigned int	gettransedsize()
	{
		return m_uGetedSize;	
	}

	// ֹͣ����
	int	stop();


	// ��ȡhttpЭ��ķ���ֵ������Ϊhttpret
	int	gethttpret()
	{
		return m_uHttpRet;	
	}

	// ��ȡ��ǰ��״̬����������Ϊstate_t
	state_t	getcurstate()
	{
		return	m_stat;
	}

	// tick�����������������tick����á�����ֵΪerr_t���͡�
	// �������complete��˵��get��������ݴ�����ϣ�����noerr��˵���������ڼ�����û�д�������
	// ��������ֵ��˵����������Ӧ����ֹ���á�
	int tick();

	// �ͷ���Դ���Ͽ�����
	void release();

	bool free()
	{
		return	(nothing == m_stat);
	}

private:
	// ������Ϣ
	std::string				m_sHost;
	std::string				m_sIP;
	unsigned short int		m_uPort;
	std::string				m_sRequest;
	size_t					m_uCurPos;
	pfn_onget				m_pCallbackFun;
	size_t					m_uCallbackSize;


	// ͨ��ʹ��
	size_t					m_uTotalTransSize;
	lane::Socket			m_TcpSock;
	//unsigned int			m_uHostIP;
	lane::Select			m_select;

	char					* m_sBuf;
	//char			* m_sBufWrPtr;
	//char			* m_sBufRdPtr;
	int						m_uRecvLenTmp; // ÿ���յ������ݰ��Ĵ�С,��ʱ����
	size_t					m_uGetedSize;		

	unsigned int			m_uHttpRet;

	void					* m_pExp;
//////////////////////////////////////////////////////////////////////////

	state_t			m_stat;

	typedef	int (httpget:: *pfn_proc)();

	pfn_proc	m_pfProc;
	//	pfn_proc	m_pfProcData;

	int prochttphead();

	int	prochttpheadline( const char * line, unsigned int charnum );

	int checkhttperr();

	int procgetdata();

};	// end class

};	// end namespace n_whnet

#endif		// LANE_LHTTPGET_H
