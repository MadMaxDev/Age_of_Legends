#include "../include/pge_rendertocubemap.h"
#include <assert.h>

class PGERenderToCubeMap: public IPGERenderToCubeMap
{
	PGE_NOCOPY_ASSIGN(PGERenderToCubeMap)
public:
	PGERenderToCubeMap(void);
	~PGERenderToCubeMap(void);

	BOOL Create(IGraphicDevice* pDevice, DWORD Size, BOOL DepthStencil, D3DFORMAT DepthStencilFormat);
public:
	ULONG  AddRef();
	ULONG  Release();

	HRESULT Begin(ICubeTexture* pCubeTex);
	HRESULT End();

	HRESULT Face(D3DCUBEMAP_FACES iFace);
private:
	IGraphicDevice* m_pDevice;
	ISurface*		m_pOldTarget;
	ISurface*	    m_pOldZBuffer;
	ISurface*	    m_pZBuffer;
	ICubeTexture*	m_pCubeTexture;
	OSViewport		m_OldViewport;
	LONG			m_RefCount;
	BOOL			m_bBeginScene;
};


PGERenderToCubeMap::PGERenderToCubeMap(void): m_pCubeTexture(NULL),
											  m_pZBuffer( NULL ),
											  m_pOldZBuffer( NULL ),
											  m_pOldTarget(NULL),
											  m_pDevice( NULL ),
											  m_bBeginScene(FALSE)
{
}

PGERenderToCubeMap::~PGERenderToCubeMap(void)
{
	SAFE_RELEASE( m_pZBuffer );
	SAFE_RELEASE( m_pOldTarget );
	SAFE_RELEASE( m_pOldZBuffer );
}


ULONG PGERenderToCubeMap::AddRef()
{
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount);
}

ULONG PGERenderToCubeMap::Release()
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

BOOL PGERenderToCubeMap::Create(IGraphicDevice* pDevice, DWORD Size,BOOL DepthStencil, D3DFORMAT DepthStencilFormat)
{
	m_pDevice = pDevice;
	m_RefCount = 1;

	// Create a depth stencil buffer
	if (DepthStencil)
	{
		m_pZBuffer = m_pDevice->CreateDepthStencilSurface(  Size, 
															Size,
															DepthStencilFormat,
															D3DMULTISAMPLE_NONE,
															0, 
															true);

		if (m_pZBuffer == NULL)
			return FALSE;
	}

	return TRUE;
}


HRESULT PGERenderToCubeMap::Face(D3DCUBEMAP_FACES iFace)
{
	if ( m_pCubeTexture )
	{	
		ISurface* pNewTarget= m_pCubeTexture->GetCubeMapSurface(iFace, 0);
		assert(pNewTarget);

		// Set new render target and Z buffer
		m_pDevice->SetDepthStencilSurface(m_pZBuffer);
		m_pDevice->SetRenderTarget(pNewTarget);

		pNewTarget->Release();

		return S_OK;
	}else
	{
		return E_FAIL;
	}
}

HRESULT PGERenderToCubeMap::Begin(ICubeTexture* pCubeTex)
{
	if (pCubeTex == NULL)
		return E_FAIL;

	// Store old target and Z buffer
	m_pOldTarget = m_pDevice->GetRenderTarget();
	m_pOldZBuffer = m_pDevice->GetDepthStencilSurface();

	// Store old viewport 
	m_pDevice->GetViewport(&m_OldViewport);

	m_pCubeTexture = pCubeTex;
	m_bBeginScene = FALSE;

	// Begin scene
	return m_pDevice->BeginScene();
}

HRESULT PGERenderToCubeMap::End()
{
	// End scene
	m_pDevice->EndScene();

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
	m_pCubeTexture = NULL;
	m_bBeginScene = FALSE;
	return S_OK;
}


//--------------------------------------------------------------------
// Name : PGECreateRenderToCubeMap
// Desc : 创建立方体纹理渲染器
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToCubeMap* WINAPI PGECreateRenderToCubeMap(  IGraphicDevice* pDevice
																	, DWORD Size
																	, BOOL  DepthStencil
																	, D3DFORMAT DepthStencilFormat)
{
	assert( pDevice );
	PGERenderToCubeMap* pRenderToCubeMap = new PGERenderToCubeMap();
	if ( !pRenderToCubeMap->Create( pDevice, Size, DepthStencil, DepthStencilFormat ) )
	{
		pRenderToCubeMap->Release();
		return NULL;
	}
	return pRenderToCubeMap;
}
