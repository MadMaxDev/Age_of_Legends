// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whpipe.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�PIPE����
// CreationDate : 2005-06-27
// ChangeLOG    : 2006-08-07 ��whpipe_win_server::WaitSelect�е�ConnectNamedPipe�Ĳ���ol�Ӿֲ�����������Ա������������vs2003����ĳ����п��ܳ���ֵĴ��󣬱���˵Disconnect��ʱ������Run-Time Check Failure #0 - The value of ESP was not properly saved ����

#include "../inc/whstreambuffer.h"
#include "../inc/whqueue.h"
#include "../inc/whvector.h"
#include "../inc/whpipe.h"
#include "../inc/whtime.h"

namespace	n_whcmn
{

#ifdef	WIN32				//	{

#include "../inc/wh_platform.h"

// �ж�handle�Ƿ��б�signal�ˣ����handleһ��Ҫ����Ҫmanualreset��
inline bool	IsHandleSignaled(HANDLE h)
{
	if( WaitForSingleObject(h, 0) == WAIT_OBJECT_0 )
	{
		return	true;
	}
	return		false;
}

////////////////////////////////////////////////////////////////////
// ��ṹ����
////////////////////////////////////////////////////////////////////

// whpipe_win����ֱ��ʹ�ã���Ϊ��ֻ��һ���ֹ���
class	whpipe_win			: public whpipe
{
protected:
	enum
	{
		STATUS_ERROR		= -1,
		STATUS_NOTHING		= 0,
		STATUS_WAITCLIENT0	= 1,							// ��ʼ����ϣ����ǻ�û�п�ʼ�ȴ��ͻ�������
		STATUS_WAITCLIENT	= 2,							// �ȴ��ͻ�������
		STATUS_WORKING		= 3,							// �Ϳͻ��˵Ľ�����
	};
	int		m_nStatus;
	HANDLE	m_hPipe;
	HANDLE	m_hReadEvent;
	HANDLE	m_hWriteEvent;
	bool	m_bIsReading;
	bool	m_bIsWriting;
	whsmpqueue			m_bufRead;
	whsmpqueue			m_bufWrite;
	whvector<char>		m_vectFromPipe;
	DWORD				m_nReadSize;						// �ϴεĶ�ָ����������ĳ���
	DWORD				m_nWrittenSize;						// �ϴε�дָ������д��ĳ���
	OVERLAPPED			m_olread, m_olwrite;
	OVERLAPPED			m_olaccept;
public:
	whpipe_win();
	virtual ~whpipe_win();
	virtual int		Write(const void *pData, int nSize);
	virtual int		Read(void *pData, int *pnSize);
protected:
	int		Init(int nQueueSize);
	int		Release();
	// ���õ�������״̬
	void	SetWorkingStatus();
	// ��������ʱ�����ݶ��к���Ϣ�Ĵ���
	int		OnWork(int nMS);
private:
	virtual int		OnWaitError(DWORD nErr)	= 0;
};

// server��Զֻ���ȵ��������˲����յ�������Ȼ����ܷ��Ͷ���
class	whpipe_win_server	: public whpipe_win
{
public:
	whpipe_win_server();
	virtual ~whpipe_win_server();
	virtual	void	Disconnect();
	virtual int		WaitSelect(int nMS);
public:
	int	StartServer(const char *cszPipeName, int nIOBufSize);
private:
	virtual int		OnWaitError(DWORD nErr);
};
class	whpipe_win_client	: public whpipe_win
{
public:
	whpipe_win_client();
	virtual ~whpipe_win_client();
	virtual	void	Disconnect();
	virtual int		WaitSelect(int nMS);
	virtual bool	IsFailed()
	{
		return	m_nStatus != STATUS_WORKING;
	}
public:
	int	StartClient(const char *cszPipeName, int nTimeOut, int nIOBufSize);
private:
	virtual int		OnWaitError(DWORD nErr);
};

////////////////////////////////////////////////////////////////////
// ʵ�ֲ���
////////////////////////////////////////////////////////////////////
// whpipe_win
////////////////////////////////////////////////////////////////////
whpipe_win::whpipe_win()
: m_nStatus(STATUS_NOTHING)
, m_hPipe(INVALID_HANDLE_VALUE)
, m_hReadEvent(NULL)
, m_hWriteEvent(NULL)
, m_bIsReading(false)
, m_bIsWriting(false)
, m_nReadSize(0)
, m_nWrittenSize(0)
{
}
whpipe_win::~whpipe_win()
{
	Release();
}
int		whpipe_win::Write(const void *pData, int nSize)
{
	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}
	return	m_bufWrite.In(pData, nSize);
}
int		whpipe_win::Read(void *pData, int *pnSize)
{
	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}
	return	m_bufRead.Out(pData, (size_t *)pnSize);
}
int		whpipe_win::Init(int nQueueSize)
{
	if( m_bufRead.Init(nQueueSize)<0 )
	{
		return	-1;
	}
	if( m_bufWrite.Init(nQueueSize)<0 )
	{
		return	-2;
	}
	m_hReadEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWriteEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_vectFromPipe.resize(nQueueSize);
	return	0;
}
int		whpipe_win::Release()
{
	m_bufRead.Release();
	m_bufWrite.Release();
	if( m_hReadEvent )
	{
		CloseHandle(m_hReadEvent);
		m_hReadEvent	= NULL;
	}
	if( m_hWriteEvent )
	{
		CloseHandle(m_hWriteEvent);
		m_hWriteEvent	= NULL;
	}
	return	0;
}
void	whpipe_win::SetWorkingStatus()
{
	m_nStatus		= STATUS_WORKING;
	m_bIsReading	= false;
	m_bIsWriting	= false;
	ResetEvent(m_hReadEvent);
	ResetEvent(m_hWriteEvent);
	m_bufRead.Clean();
	m_bufWrite.Clean();
}
int		whpipe_win::OnWork(int nMS)
{
	if( !m_bIsReading )
	{
		// ������ָ��
		memset(&m_olread, 0, sizeof(m_olread));
		m_olread.hEvent	= m_hReadEvent;
		m_nReadSize	= 0;
		ReadFile(m_hPipe, m_vectFromPipe.getbuf(), m_vectFromPipe.size(), &m_nReadSize, &m_olread);
		m_bIsReading	= true;
	}
	if( !m_bIsWriting )
	{
		if( m_bufWrite.GetUnitNum()>0 )
		{
			// ����дָ��
			// ������ָ��
			memset(&m_olwrite, 0, sizeof(m_olwrite));
			m_olwrite.hEvent		= m_hWriteEvent;
			m_nWrittenSize	= 0;
			size_t	nSize	= 0;
			void	*pData	= m_bufWrite.OutPeek(&nSize);	
			WriteFile(m_hPipe, pData, nSize, &m_nWrittenSize, &m_olwrite);
			m_bIsWriting	= true;
		}
	}

	// 
	HANDLE	hs[2]	= {m_hReadEvent, m_hWriteEvent};
	DWORD	nRst	= WaitForMultipleObjects(2, hs, FALSE, nMS);
	switch( nRst )
	{
		case	WAIT_TIMEOUT:
		break;
		case	WAIT_OBJECT_0 + 0:
		{
			// ������
			// ��ý������ʼ����
			if( GetOverlappedResult(m_hPipe, &m_olread, &m_nReadSize, FALSE) )
			{
				// ���������
				if( m_nReadSize > 0 )
				{
					m_bufRead.In(m_vectFromPipe.getbuf(), m_nReadSize);
				}
			}
			else
			{
				OnWaitError(0);
			}
			// ����¼�
			ResetEvent(m_hReadEvent);
			// ������־
			m_bIsReading	= false;
		}
		break;
		case	WAIT_OBJECT_0 + 1:
		{
			// д����
			// ��ý������ʼ����
			if( GetOverlappedResult(m_hPipe, &m_olwrite, &m_nWrittenSize, FALSE) )
			{
				// ֱ�ӽ����ݴӵȴ�������ɾ��
				if( m_nWrittenSize>0 )
				{
					m_bufWrite.FreeN(1);
				}
			}
			else
			{
				OnWaitError(0);
			}
			// ����¼�
			ResetEvent(m_hWriteEvent);
			// ������־
			m_bIsWriting	= false;
		}
		break;
		default:
			// �����ˣ�Ӧ�ö���
			OnWaitError(nRst);
		break;
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// whpipe_win_server
////////////////////////////////////////////////////////////////////
whpipe_win_server::whpipe_win_server()
{
}
whpipe_win_server::~whpipe_win_server()
{
	Disconnect();
	if( m_hPipe != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hPipe);
		m_hPipe	= INVALID_HANDLE_VALUE;
	}
}
void	whpipe_win_server::Disconnect()
{
	if( m_hPipe != INVALID_HANDLE_VALUE )
	{
		if( m_nStatus != STATUS_WAITCLIENT0 )
		{
			DisconnectNamedPipe(m_hPipe);
			m_nStatus	= STATUS_WAITCLIENT0;
		}
	}
}
int		whpipe_win_server::WaitSelect(int nMS)
{
	// �ж��Լ���ʲô״̬
	switch( m_nStatus )
	{
		case	STATUS_WAITCLIENT0:
		{
			// ���������ȴ��ͻ�������
			memset(&m_olaccept, 0, sizeof(m_olaccept));
			m_olaccept.hEvent	= m_hReadEvent;
			if( ConnectNamedPipe(m_hPipe, &m_olaccept) )
			{
				// ֱ�Ӿ͵õ���һ�������client
				SetWorkingStatus();
			}
			else
			{
				switch(GetLastError())
				{
					case	ERROR_IO_PENDING:
						// ����͵ȴ����뼴��
						m_nStatus	= STATUS_WAITCLIENT;
					break;
					default:
						// �����ˣ����ܹܵ�broken��
						Disconnect();
						// ����״̬����������ȴ�״̬��
					break;
				}
			}
		}
		break;
		case	STATUS_WAITCLIENT:
		{
			switch(WaitForSingleObject(m_hReadEvent, nMS))
			{
				case	WAIT_TIMEOUT:
				break;
				case	WAIT_OBJECT_0:
					// ���ӽ���
					ResetEvent(m_hReadEvent);
					SetWorkingStatus();
				break;
				default:
					// ������
				break;
			}
		}
		break;
		case	STATUS_WORKING:
		{
			whpipe_win::OnWork(nMS);
		}
		break;
		default:
			wh_sleep(nMS);
		break;
	}
	return	0;
}
int		whpipe_win_server::StartServer(const char *cszPipeName, int nIOBufSize)
{
	assert( m_hPipe == INVALID_HANDLE_VALUE );

	m_hPipe	= CreateNamedPipe(
			cszPipeName
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE
			, 1
			, nIOBufSize
			, nIOBufSize
			, 0
			, NULL);
	if( m_hPipe == INVALID_HANDLE_VALUE )
	{
		return	-1;
	}

	// ��ʼ��
	if( Init(nIOBufSize*4)<0 )
	{
		return	-2;
	}

	// ���õȴ�
	m_nStatus	= STATUS_WAITCLIENT0;

	return	0;
}
int		whpipe_win_server::OnWaitError(DWORD nErr)
{
	// �Ͽ����ӣ����µȴ�
	Disconnect();
	m_nStatus	= STATUS_WAITCLIENT0;
	return	0;
}

////////////////////////////////////////////////////////////////////
// whpipe_win_client
////////////////////////////////////////////////////////////////////
whpipe_win_client::whpipe_win_client()
{
}
whpipe_win_client::~whpipe_win_client()
{
	Disconnect();
}
void	whpipe_win_client::Disconnect()
{
	if( m_hPipe != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hPipe);
		m_hPipe	= INVALID_HANDLE_VALUE;
	}
}
int		whpipe_win_client::WaitSelect(int nMS)
{
	// �ܵ������һ���ǹ���״̬
	switch( m_nStatus )
	{
		case	STATUS_WORKING:
			whpipe_win::OnWork(nMS);
		break;
		default:
			wh_sleep(nMS);
		break;
	}
	return	0;
}
int		whpipe_win_client::StartClient(const char *cszPipeName, int nTimeOut, int nIOBufSize)
{
	assert( m_hPipe == INVALID_HANDLE_VALUE );
	whtick_t	t	= wh_gettickcount();
again:
	// ����һֱ�ȵ���ʱΪֹ
	m_hPipe	= CreateFile(
			cszPipeName
			, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE
			, 0
			, NULL
			, OPEN_EXISTING
			, FILE_FLAG_OVERLAPPED
			, NULL);

	if( m_hPipe == INVALID_HANDLE_VALUE )
	{
		DWORD	nErr	= GetLastError();
		switch(nErr)
		{
			case	ERROR_PIPE_BUSY:
				if( wh_tickcount_diff(wh_gettickcount(), t) >= nTimeOut )
				{
					// �޷�����
					return	-1;
				}
				Sleep(100);
				goto	again;
			break;
			default:
				return	-2;
			break;
		}
	}

	DWORD	nMode	= PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(m_hPipe, &nMode, NULL, NULL);

	// ��ʼ��������
	if( Init(nIOBufSize*4)<0 )
	{
		return	-3;
	}

	// ����Ϊ����״̬
	SetWorkingStatus();

	return	0;
}
int		whpipe_win_client::OnWaitError(DWORD nErr)
{
	// �Ͽ�����
	Disconnect();
	m_nStatus	= STATUS_ERROR;
	return	0;
}

#endif	// EOF WIN32		//	}


// ����pipe�Ⱥ�����
whpipe *	whpipe_create(const char *cszPipeName, int nIOBufSize)
{
#ifdef	WIN32
	whpipe_win_server	*pSvr	= new whpipe_win_server;
	if( pSvr )
	{
		if( pSvr->StartServer(cszPipeName, nIOBufSize)<0 )
		{
			delete	pSvr;
			pSvr	= NULL;
		}
	}
	return	pSvr;
#else
	return	NULL;
#endif
}
// ����һ���Ѿ������õ�pipe
whpipe *	whpipe_open(const char *cszPipeName, int nTimeOut, int nIOBufSize)
{
#ifdef	WIN32
	whpipe_win_client	*pCli	= new whpipe_win_client;
	if( pCli )
	{
		if( pCli->StartClient(cszPipeName, nTimeOut, nIOBufSize)<0 )
		{
			delete	pCli;
			pCli	= NULL;
		}
	}
	return	pCli;
#else
	return	NULL;
#endif
}

}		// EOF namespace n_whcmn
