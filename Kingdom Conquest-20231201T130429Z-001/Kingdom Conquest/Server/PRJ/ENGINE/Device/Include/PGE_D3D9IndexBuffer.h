#pragma  once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class  D3D9IndexBuffer;

typedef D3D9ResourceMgr< D3D9IndexBuffer > D3D9IndexBufferMgr;

class D3D9IndexBuffer: public D3D9Resource< IIndexBuffer, D3D9IndexBufferMgr >
{
public:
	D3D9IndexBuffer() : m_pIB(NULL)
	{
	}
	~D3D9IndexBuffer()
	{
		SAFE_RELEASE( m_pIB );
	}

	// 得到资源优先级
	virtual DWORD GetPriority()
	{ 
		assert( m_pIB );
		return m_pIB->GetPriority(); 
	}

	// 设置资源优先级
	virtual void SetPriority(DWORD priority) 
	{
		assert( m_pIB );
		m_pIB->SetPriority( priority );
	}
	virtual DWORD GetSize()
	{
		return m_Desc.Size;
	}
	// 脏标记是否为真
	virtual BOOL IsDirty() 
	{
		return m_bDirty;
	}

	// 锁定缓冲
	virtual void* Lock( DWORD offset, DWORD size, DWORD flag ) 
	{
		HRESULT hr;
		void* pData = NULL;
		if ( FAILED(hr = m_pIB->Lock( offset, size, &pData, flag)) )
		{
			return NULL;
		}
		return pData;
	}

	// 解除锁定
	virtual void Unlock()
	{
		m_pIB->Unlock();
	}

	virtual void GetDesc(D3DINDEXBUFFER_DESC* desc)
	{
		assert( desc );
		*desc = m_Desc;
	}

	virtual D3DRESOURCETYPE GetResourceType()
	{
		return D3DRTYPE_INDEXBUFFER;
	}

	inline BOOL Create( DWORD size, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer9** ppIB )
	{
		HRESULT hr;
		if (FAILED(hr =  GetD3DDevice()->CreateIndexBuffer( size
			, ( usage )
			, ( format )
			,  pool 
			,  ppIB
			, NULL ) ))
		{
			GetD3DDevice()->EvictManagedResources();

			if (FAILED(hr =  GetD3DDevice()->CreateIndexBuffer( size
				, ( usage )
				, ( format )
				,  pool 
				,  ppIB
				, NULL ) ))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_IB , hr );
				return FALSE;
			}
		}
		return TRUE;
	}
	void CopyIndexBuffer( IDirect3DIndexBuffer9* pDstVB, IDirect3DIndexBuffer9* pSrcVB )
	{
		assert( pSrcVB && pDstVB );
		D3DINDEXBUFFER_DESC desc;
		pDstVB->GetDesc(&desc);

		UINT flag = 0;
		if ( desc.Pool == D3DPOOL_DEFAULT )
			flag |= D3DLOCK_DISCARD;

		void* pSrcData = NULL;
		void* pDstData = NULL;

		if ( FAILED(pDstVB->Lock( 0, 0, &pDstData, flag)) )
		{
			DEVICE_EXCEPT( ERR_FAILED_TO_LOCK_VB, L"D3D9IndexBuffer::CopyVertexBuffer" );
		}

		if ( FAILED(pSrcVB->Lock( 0, 0, &pSrcData, D3DLOCK_READONLY)) )
		{
			DEVICE_EXCEPT( ERR_FAILED_TO_LOCK_VB, L"D3D9IndexBuffer::CopyVertexBuffer" );
		}
		memcpy( pDstData, pSrcData, desc.Size );

		pDstVB->Unlock();
		pSrcVB->Unlock();
	}

	BOOL Create( DWORD count,  DWORD usage, D3DPOOL pool )
	{
		m_Desc.Format = D3DFMT_INDEX16;
		m_Desc.Pool	  = pool;
		m_Desc.Size	  = count*sizeof(WORD);
		m_Desc.Type	  = D3DRTYPE_INDEXBUFFER;
		m_Desc.Usage  = usage;
		return Create( m_Desc.Size, usage, m_Desc.Format, pool, &m_pIB );
	}

	void OnLostDevice()
	{	
		if ( m_Desc.Pool == D3DPOOL_DEFAULT )
		{	
			if ( m_Desc.Usage & D3DUSAGE_WRITEONLY )
			{
				m_pIB->Release();
				m_pIB = NULL;
				// 设置脏标记
				//SetDirtyFlag( cTrue );
			}else
			{
				// Create a temporary vertex buffer that in system memory
				IDirect3DIndexBuffer9* tempIB = NULL;
				if ( !Create( m_Desc.Size, 0, m_Desc.Format, D3DPOOL_SYSTEMMEM, &tempIB ) )
				{
					DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_IB, L"D3D9IndexBuffer::OnLostDevice" );		
				}

				// Copy data
				CopyIndexBuffer( tempIB, m_pIB );

				m_pIB->Release();
				m_pIB = tempIB;
			}
		}
	}
	void OnResetDevice()
	{
		if ( m_Desc.Pool == D3DPOOL_DEFAULT )
		{
			// Create a temporary vertex buffer that in video memory
			IDirect3DIndexBuffer9* tempIB = NULL;
			if ( !Create( m_Desc.Size, m_Desc.Usage, m_Desc.Format,  D3DPOOL_DEFAULT, &tempIB ) )
			{
				DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_IB, L"D3D9IndexBuffer::OnResetDevice" );		
			}
			if ( m_pIB )
			{
				// Copy data
				CopyIndexBuffer( tempIB, m_pIB );

				m_pIB->Release();
				m_pIB = tempIB;
			}else
			{
				m_pIB = tempIB;
			}	
		}
	}

	inline IDirect3DIndexBuffer9* _d3d() { return m_pIB; }
protected:
	IDirect3DIndexBuffer9*  m_pIB;
	D3DINDEXBUFFER_DESC		m_Desc;
	BOOL					m_bDirty;
};
