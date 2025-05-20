#include "..\Include\PGE_D3D9Surface.h"
#include "..\Include\PGE_D3D9Texture.h"

#include "..\Include\PGE_ErrorText.h"
#include "..\Include\PGE_Config.h"
#include "..\Include\PGE_D3D9Device.h"
#include "..\Include\PGE_Texutil.h"


BOOL D3D9Surface::CreateRenderTarget( DWORD width, DWORD height, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality, BOOL bLockable )
{	
	m_CreateType		 = CREATE_RENDERTARGET;
	m_Width				 = width;
	m_Height			 = height;
	m_Format			 = ( fmt );
	m_MultisampleType	 = (multisampleType);
	m_MultisampleQuality = multisampleQuality;
	m_IsLockable		 = bLockable ? TRUE : FALSE;

	HRESULT hr;
	if (FAILED(hr = GetD3DDevice()->CreateRenderTarget( m_Width, m_Height, m_Format, m_MultisampleType, m_MultisampleQuality, m_IsLockable, &m_pSurface, NULL )))
	{
		DXTRACE_ERRW( ERR_FAILED_TO_CREATE_RENDERTARGET, hr );
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::CreateDepthStencil( DWORD width, DWORD height, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality, BOOL bDiscard )
{
	m_CreateType		 = CREATE_DEPTHSTENCIl;

	m_Width				 = width;
	m_Height			 = height;
	m_Format			 = ( fmt );
	m_MultisampleType	 = (multisampleType);
	m_MultisampleQuality = multisampleQuality;
	m_IsDiscard			 = bDiscard ? TRUE : FALSE;
	
	HRESULT hr;
	if (FAILED(hr = GetD3DDevice()->CreateDepthStencilSurface(m_Width, m_Height, m_Format, m_MultisampleType, m_MultisampleQuality, m_IsDiscard, &m_pSurface, NULL)) )
	{
		DXTRACE_ERRW( ERR_FAILED_TO_CREATE_DEPTHSTENCIL, hr );
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch )
{
	m_CreateType		 = CREATE_OFFSCREEN_SURFACE;

	m_Width				 = width;
	m_Height			 = height;
	m_Format			 = ( fmt );
	m_IsScratched		 = bScratch ? TRUE : FALSE;

	HRESULT hr;
	if (FAILED(hr = GetD3DDevice()->CreateOffscreenPlainSurface(m_Width, m_Height, m_Format, m_IsScratched?D3DPOOL_SCRATCH:D3DPOOL_SYSTEMMEM, &m_pSurface, NULL)) )
	{
		DXTRACE_ERRW( L"Failed to create offscreen plain surface", hr );
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::GetByTexture( D3D9Texture* pTex, DWORD level )
{
	assert( pTex );
	m_CreateType = GET_BY_TEXTURE;

	m_pTexture = pTex;
	m_Level    = level;

	HRESULT hr;
	if (FAILED( hr = m_pTexture->_d3d()->GetSurfaceLevel(level, &m_pSurface)) )
	{
		DXTRACE_ERRW( ERR_FAILED_TO_GET_TEXTURELEVEL , hr);
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::GetByCubeTexture( D3D9CubeTexture* pTex, D3DCUBEMAP_FACES faceType, DWORD level )
{
	assert( pTex );
	m_CreateType   = GET_BY_CUBETEXTURE;

	m_pCubeTexture = pTex;
	m_FaceType     = (faceType);
	m_Level		   = level;

	HRESULT hr;
	if (FAILED( hr = m_pCubeTexture->_d3d()->GetCubeMapSurface(m_FaceType, m_Level, &m_pSurface)) )
	{
		DXTRACE_ERRW( ERR_FAILED_TO_GET_CUBETEXTURELEVEL, hr );
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::GetBackBuffer(DWORD iBackBuffer )
{
	m_CreateType  = GET_BACKBUFFER;
	m_iBackBuffer = iBackBuffer;
	if (FAILED(GetD3DDevice()->GetBackBuffer( 0, m_iBackBuffer, D3DBACKBUFFER_TYPE_MONO, &m_pSurface)))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL D3D9Surface::GetRenderTarget()
{
	m_CreateType = GET_RENDERTARGET;
	GetD3DDevice()->GetRenderTarget(0, &m_pSurface );
	return m_pSurface != NULL;
}

BOOL D3D9Surface::GetDepthStencil()
{
	m_CreateType = GET_DEPTHSTENCIL;
	GetD3DDevice()->GetDepthStencilSurface(&m_pSurface);
	return m_pSurface != NULL;
}

BOOL D3D9Surface::Create(IDirect3DSurface9* surface )
{
	m_pSurface   = surface;
	m_CreateType = CREATE_INNER;
	return TRUE;
}

void D3D9Surface::OnLostDevice()
{
	if (m_CreateType == CREATE_OFFSCREEN_SURFACE)
		return;

	if ( m_CreateType == CREATE_RENDERTARGET )
	{
		IDirect3DSurface9* tmpSur = NULL;
		if ( FAILED(GetD3DDevice()->CreateOffscreenPlainSurface( m_Width, m_Height, m_Format, D3DPOOL_SYSTEMMEM, &tmpSur, NULL )))
		{
			DEVICE_EXCEPT(L"Failed to create surface", L"D3D9Surface::OnLostDevice" );
		}
		HRESULT hr;
		if (FAILED(hr = GetD3DDevice()->GetRenderTargetData( m_pSurface, tmpSur )))
		{
			DXTRACE_ERRW( L"Failed to get render target data\n", hr );
		}
		m_pSurface->Release();
		m_pSurface = tmpSur;
	}else if ( m_CreateType == CREATE_DEPTHSTENCIl )
	{
		//IDirect3DSurface9* tmpSur = NULL;
		//if (FAILED(GetD3DDevice()->CreateDepthStencilSurface(m_Width, m_Height, m_Format, m_MultisampleType, m_MultisampleQuality, m_IsDiscard, &tmpSur, NULL)) )
		{
			//	GRAPHIC_EXCEPT(ERR_FAILED_TO_CREATE_SURFACE, L"D3D9Surface::OnLostDevice" );
		}
		m_pSurface->Release();
		m_pSurface = NULL;
	}else
	{
		SAFE_RELEASE(m_pSurface);
	}
}

void D3D9Surface::OnResetDevice()
{
	HRESULT hr;
	switch( m_CreateType )
	{
	case GET_BACKBUFFER:
		{
			hr = GetD3DDevice()->GetBackBuffer(0, m_iBackBuffer, D3DBACKBUFFER_TYPE_MONO, &m_pSurface );
		}break;
	case GET_RENDERTARGET:
		{
			hr = GetD3DDevice()->GetRenderTarget(0, &m_pSurface );
		}break;
	case GET_DEPTHSTENCIL:
		{
			hr = GetD3DDevice()->GetDepthStencilSurface( &m_pSurface );
		}break;
	case GET_BY_TEXTURE:
		{
			hr = m_pTexture->_d3d()->GetSurfaceLevel(m_Level, &m_pSurface);
		}break;
	case GET_BY_CUBETEXTURE:
		{
			hr = m_pCubeTexture->_d3d()->GetCubeMapSurface(m_FaceType, m_Level, &m_pSurface );
		}break;
	case CREATE_RENDERTARGET:
		{
			IDirect3DSurface9* tmpSur = NULL;
			if (FAILED(GetD3DDevice()->CreateRenderTarget( m_Width, m_Height, m_Format, m_MultisampleType, m_MultisampleQuality, m_IsLockable, &tmpSur, NULL )))
				DEVICE_EXCEPT( L"Failed to create surface", L"D3D9Surface::OnResetDevice" );

			hr = GetD3DDevice()->UpdateSurface(m_pSurface, NULL, tmpSur, NULL );
			m_pSurface->Release();
			m_pSurface = tmpSur;
		}break;
	case CREATE_DEPTHSTENCIl:
		{
			hr = GetD3DDevice()->CreateDepthStencilSurface( m_Width, m_Height, m_Format, m_MultisampleType, m_MultisampleQuality, m_IsDiscard, &m_pSurface, NULL );
		}break;
	}
}
