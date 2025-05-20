#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class   D3D9VertexBuffer;

typedef D3D9ResourceMgr< D3D9VertexBuffer > D3D9VertexBufferMgr;

class D3D9VertexBuffer: public D3D9Resource< IVertexBuffer, D3D9VertexBufferMgr >
{
public:
	D3D9VertexBuffer() : m_pVB(NULL)
	{
	}
	~D3D9VertexBuffer()
	{
		SAFE_RELEASE( m_pVB );
	}

	// 得到资源优先级
	virtual DWORD GetPriority()
	{ 
		return m_pVB->GetPriority(); 
	}

	// 设置资源优先级
	virtual void SetPriority(DWORD priority) 
	{
		m_pVB->SetPriority( priority );
	}
	// 获取大小
	virtual DWORD GetSize() 
	{
		return m_Desc.Size;
	}

	// 锁定缓冲
	virtual void* Lock( DWORD offset, DWORD size, DWORD flag ) 
	{
		HRESULT hr;
		void* pData = NULL;
		if ( FAILED(hr = m_pVB->Lock(offset, size, &pData, flag)) )
		{
			//GRAPHIC_EXCEPT( ERR_FAILED_TO_LOCK_VB, L"D3D9VertexBuffer::Lock" );
			return NULL;
		}
		return pData;
	}
	// 解除锁定
	virtual void  Unlock()
	{
		m_pVB->Unlock();
	}
	virtual void GetDesc(D3DVERTEXBUFFER_DESC* desc)
	{
		assert( desc );
		*desc = m_Desc;
	}

	inline BOOL Create( DWORD size, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer9** ppVB )
	{
		HRESULT hr;
		if (FAILED( hr = GetD3DDevice()->CreateVertexBuffer(   size
			,  usage 
			,  fvf  
			,  pool
			,  ppVB
			, NULL ) ))
		{
			GetD3DDevice()->EvictManagedResources();
			if (FAILED( hr = GetD3DDevice()->CreateVertexBuffer(size
				,  usage 
				,  fvf  
				,  pool
				,  ppVB
				, NULL ) ))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_VB, hr );	
				return FALSE;
			}
		}
		return TRUE;
	}
	void CopyVertexBuffer( IDirect3DVertexBuffer9* pDstVB, IDirect3DVertexBuffer9* pSrcVB )
	{
		assert( pSrcVB && pDstVB );

		void* pSrcData = NULL;
		void* pDstData = NULL;

		HRESULT hr;
		if ( FAILED(hr = pSrcVB->Lock( 0, 0, &pSrcData, D3DLOCK_READONLY )) )
		{
			DXTRACE_ERRW( ERR_FAILED_TO_LOCK_VB, hr);
			return;
		}

		D3DVERTEXBUFFER_DESC desc;
		pDstVB->GetDesc(&desc);

		UINT flag = 0;
		if ( desc.Pool == D3DPOOL_DEFAULT )
			flag |= D3DLOCK_DISCARD;

		if ( FAILED( hr = pDstVB->Lock( 0, 0, &pDstData, flag)) )
		{
			DXTRACE_ERRW( ERR_FAILED_TO_LOCK_VB, hr);
		}else
		{
			memcpy( pDstData, pSrcData, desc.Size );
			pDstVB->Unlock();
		}
		pSrcVB->Unlock();
	}

	BOOL Create( DWORD size, DWORD usage, DWORD fvf, D3DPOOL pool )
	{
		m_Desc.Format = D3DFMT_UNKNOWN;
		m_Desc.FVF    = fvf;
		m_Desc.Pool	  = pool;
		m_Desc.Size	  = size;
		m_Desc.Type	  = D3DRTYPE_VERTEXBUFFER;
		m_Desc.Usage  = usage;

		return Create( size,  usage,fvf,  pool, &m_pVB );
	}

	void OnLostDevice()
	{	
		if ( m_Desc.Pool == D3DPOOL_DEFAULT )
		{	
			if ( ( m_Desc.Usage & D3DUSAGE_WRITEONLY ) )
			{
				m_pVB->Release();
				m_pVB = NULL;
			}else
			{
				// Create a temporary vertex buffer that in system memory
				IDirect3DVertexBuffer9* tempVB = NULL;
				if ( !Create( m_Desc.Size, 0, m_Desc.FVF, D3DPOOL_SYSTEMMEM, &tempVB ) )
				{
					DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_VB, L"D3D9VertexBuffer::OnLostDevice" );
				}
				// Copy data
				CopyVertexBuffer( tempVB, m_pVB );

				m_pVB->Release();
				m_pVB = tempVB;
			}
		}
	}
	void OnResetDevice()
	{
		if ( m_Desc.Pool == D3DPOOL_DEFAULT )
		{		
			// Create a temporary vertex buffer that in video memory
			IDirect3DVertexBuffer9* tempVB = NULL;
			if ( !Create( m_Desc.Size, m_Desc.Usage, m_Desc.FVF, D3DPOOL_DEFAULT, &tempVB ) )
			{
				DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_VB, L"D3D9VertexBuffer::OnResetDevice" );
			}
			if ( m_pVB )
			{
				// Copy data
				CopyVertexBuffer( tempVB, m_pVB );

				m_pVB->Release();
				m_pVB = tempVB;
			}else
			{
				m_pVB = tempVB;
			}
		}
	}

	inline IDirect3DVertexBuffer9* _d3d() { return m_pVB; }
protected:
	IDirect3DVertexBuffer9*  m_pVB;
	D3DVERTEXBUFFER_DESC	 m_Desc;
};