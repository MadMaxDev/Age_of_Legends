#pragma once

#include <vector>
#include <assert.h>

#define INVALID_HANDLE ( 0xffffffff )

template< class BaseResource, class Manager >
class D3D9Resource: public BaseResource
{
public:
	// ������ü���
	virtual DWORD AddRef()
	{
		return InterlockedIncrement((volatile LONG*)&m_RefCount);
	}

	// �õ����ü�����ֵ
	virtual DWORD GetRefCount() 
	{
		return m_RefCount;
	}

	// �ͷ���Դ
	virtual DWORD Release() 
	{
		if ( InterlockedDecrement( (volatile LONG*)&m_RefCount ) == 0 )
		{
			if (m_pManager != NULL)
			{
				m_pManager->DeleteResource( m_Handle );
			}else
			{
				delete this;
			}
			return 0;
		}
		return m_RefCount;
	}

	void OnLostDevice() {}
	void OnResetDevice() {}

	inline void SetHandle( DWORD handle )		{ m_Handle = handle;	 }
	inline void SetManager( Manager* pManager ) { m_pManager = pManager; }

	inline IDirect3DDevice9* GetD3DDevice() 
	{
		assert(m_pManager);
		return m_pManager->GetD3DDevice();
	}

	inline void* GetMgrUserData() 
	{
		assert( m_pManager );
		return m_pManager->GetUserData();
	}

	inline IGraphicDevice* GetDevice() 
	{	
		assert( m_pManager );
		return m_pManager->GetDevice();
	}
protected:
	D3D9Resource() : m_RefCount(1)
		, m_Handle(INVALID_HANDLE)
		, m_pManager( NULL )
	{
	}
	virtual ~D3D9Resource()
	{
	}

	Manager* m_pManager;
	DWORD m_RefCount;
	DWORD m_Handle;
};

//////////////////////////////////////////////////////////////////////////
// class D3D9ResourceMgr
template < typename DATA >
class D3D9ResourceMgr
{
public:
	D3D9ResourceMgr() : m_RefCount(1)
		,   m_pD3DDevice( NULL )
		,   m_pUserData( NULL )
		,   m_pDevice(NULL)
	{
		 InitializeCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
	}
	virtual ~D3D9ResourceMgr()
	{
		for (size_t i = 0; i < m_Data.size(); ++i)
		{
			m_Data[i]->SetManager(NULL);
		}
		for (size_t i = 0; i < m_FreeData.size(); ++i)
		{
			delete m_FreeData[i];
		}
		DeleteCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
	}
	void Lock()
    {
        EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
    }
    void Unlock()
    {
        LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
    }

	inline void SetDevice( IGraphicDevice* pDevice, IDirect3DDevice9* pD3DDevice )	
	{
		m_pDevice    = pDevice;
		m_pD3DDevice = pD3DDevice;
	}
	inline IGraphicDevice* GetDevice()					 { return m_pDevice;		  }
	inline IDirect3DDevice9* GetD3DDevice()				 { return m_pD3DDevice;	      }

	inline void SetUserData( void* pData ) { m_pUserData = pData; }
	inline void* GetUserData()			   { return m_pUserData;  }

	// ������ü���
	inline DWORD AddRef()
	{
		return InterlockedIncrement((volatile LONG*)&m_RefCount);
	}

	// �ͷ���Դ
	inline DWORD Release() 
	{
		if ( InterlockedDecrement( (volatile LONG*)&m_RefCount ) == 0 )
		{
			delete this;
			return 0;
		}
		return m_RefCount;
	}

	inline DATA* AddResource()
	{
		DATA* pData = NULL;
		// ��֤�̰߳�ȫ
		Lock();
		if (m_FreeData.empty())
		{	
			pData = new DATA;	
		}else
		{
			void* ptr = (void*)m_FreeData.back();
			pData = ::new(ptr) DATA();
			m_FreeData.pop_back();
		}
		pData->SetManager( this );
		pData->SetHandle( (DWORD)m_Data.size() );
		m_Data.push_back( pData );
		Unlock();
		return  pData;
	}

	inline void DeleteResource( DWORD handle )
	{	
		// ��֤�̰߳�ȫ
		Lock();

		DATA* pData = m_Data[handle];	

		m_Data[handle] = m_Data.back();
		m_Data[handle]->SetHandle( handle );	
		m_Data.pop_back();
		m_FreeData.push_back(pData);

		// �ͷ��ڴ�
		pData->~DATA();

		Unlock();
	}	

	inline void OnLostDevice()
	{	
		// ��֤�̰߳�ȫ
		Lock();
		for (size_t i = 0; i < m_Data.size(); ++i )
		{
			m_Data[i]->OnLostDevice();
		}
		Unlock();
	}

	inline void OnResetDevice()
	{	
		// ��֤�̰߳�ȫ
		Lock();
		for (size_t i = 0; i < m_Data.size(); ++i )
		{
			m_Data[i]->OnResetDevice();
		}
		Unlock();
	}
protected:	
	volatile CRITICAL_SECTION m_cs;

	IGraphicDevice*			m_pDevice;
	IDirect3DDevice9*		m_pD3DDevice;
	void*					m_pUserData;
	DWORD					m_RefCount;
	std::vector< DATA* >	m_Data;	
	std::vector< DATA* >	m_FreeData;
};
