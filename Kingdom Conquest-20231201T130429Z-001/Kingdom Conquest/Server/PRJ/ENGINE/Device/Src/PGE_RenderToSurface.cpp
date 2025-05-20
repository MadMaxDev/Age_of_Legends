#include "../include/pge_rendertosurface.h"
#include <assert.h>

class PGERenderToSurface: public IPGERenderToSurface
{
	PGE_NOCOPY_ASSIGN(PGERenderToSurface)
public:
	PGERenderToSurface(void);
	~PGERenderToSurface(void);

	BOOL Create(IGraphicDevice* pDevice, DWORD Width, DWORD Height, BOOL DepthStencil, D3DFORMAT DepthStencilFormat);

public:
	// 添加引用计数
	virtual ULONG AddRef();

	// 释放资源
	virtual ULONG Release();

	HRESULT BeginScene(ISurface* pSurface, const D3DVIEWPORT9* pViewport);
	HRESULT EndScene();
	void SetRenderTarget( ISurface* pSurface, const D3DVIEWPORT9 *pViewport );
	void RestoreRenderTarget();
private:
	IGraphicDevice* m_pDevice;
	ULONG			m_RefCount;

	ISurface*		m_pOldZBuffer;
	ISurface*		m_pOldTarget;
	OSViewport		m_OldViewport;

	ISurface*		m_pZBuffer;
};


PGERenderToSurface::PGERenderToSurface():	m_pZBuffer(NULL),
											m_pOldTarget(NULL),
											m_pOldZBuffer(NULL),
											m_pDevice(NULL)
{
}

PGERenderToSurface::~PGERenderToSurface()
{
	SAFE_RELEASE( m_pZBuffer );
	SAFE_RELEASE( m_pOldTarget );
	SAFE_RELEASE( m_pOldZBuffer );
}

ULONG PGERenderToSurface::AddRef()
{
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount);
}

ULONG PGERenderToSurface::Release()
{
	if ( InterlockedDecrement((LONG*)&m_RefCount) == 0 )
	{
		delete this;
		return 0;
	}else
	{
		return m_RefCount;
	}
}

BOOL PGERenderToSurface::Create(IGraphicDevice* pDevice, DWORD Width, DWORD Height, BOOL DepthStencil,D3DFORMAT DepthStencilFormat)
{	
	if ( pDevice == NULL )
	{
		assert(false);
		return FALSE;
	}

	m_pDevice = pDevice;

	m_RefCount = 1;

	// Create a depth stencil buffer
	if (DepthStencil)
	{
		m_pZBuffer = m_pDevice->CreateDepthStencilSurface(  Width, 
															Height,
															DepthStencilFormat,
															D3DMULTISAMPLE_NONE,
															0, 
															true);

		if (m_pZBuffer == NULL)
			return FALSE;
	}
	return TRUE;
}
void PGERenderToSurface::SetRenderTarget(ISurface* pSurface, const D3DVIEWPORT9 *pViewport )
{
	// Store old viewport 
	m_pDevice->GetViewport(&m_OldViewport);

	// Store old target and Z buffer
	m_pOldTarget = m_pDevice->GetRenderTarget();
	m_pOldZBuffer = m_pDevice->GetDepthStencilSurface();

	// Set new render target and Z buffer
	m_pDevice->SetDepthStencilSurface(m_pZBuffer);
	m_pDevice->SetRenderTarget( pSurface );

	if ( pViewport )
	{
		m_pDevice->SetViewport( pViewport );
	}else
	{
		// NOTE:不用重新去设置视口,在设RenderTarget时D3D已经设过了
		//m_pDevice->SetViewport(0, 0, m_Width, m_Height, 0.0f, 1.0f);
	}
}

void PGERenderToSurface::RestoreRenderTarget()
{
	if (m_pOldZBuffer)
	{
		m_pDevice->SetDepthStencilSurface(m_pOldZBuffer);
		m_pOldZBuffer->Release();
		m_pOldZBuffer = NULL;
	}
	if (m_pOldTarget)
	{
		m_pDevice->SetRenderTarget( m_pOldTarget );
		m_pOldTarget->Release();
		m_pOldTarget = NULL;
	}	
	// Restore viewport
	m_pDevice->SetViewport(&m_OldViewport);
}

HRESULT PGERenderToSurface::BeginScene(ISurface* pSurface, const D3DVIEWPORT9* pViewport)
{
	assert ( m_pDevice && pSurface );

	// Set render target 
	SetRenderTarget( pSurface, pViewport );

	// Begin scene
	return m_pDevice->BeginScene();
}

HRESULT PGERenderToSurface::EndScene()
{
	assert( m_pDevice );

	// End scene
	HRESULT hr = m_pDevice->EndScene();	

	// Restore render target
	RestoreRenderTarget();
	return hr;
}

//--------------------------------------------------------------------
// Name : PGECreateRenderToSurface
// Desc : 创建表面渲染器
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToSurface* WINAPI PGECreateRenderToSurface(  IGraphicDevice* pDevice
																	, DWORD Width
																	, DWORD Height
																	, BOOL DepthStencil
																	, D3DFORMAT DepthStencilFormat)
{
	assert( pDevice );
	PGERenderToSurface* pRenderToSurface = new PGERenderToSurface();
	if ( !pRenderToSurface->Create( pDevice, Width, Height, DepthStencil, DepthStencilFormat ) )
	{
		pRenderToSurface->Release();
		return NULL;
	}
	return pRenderToSurface;
}