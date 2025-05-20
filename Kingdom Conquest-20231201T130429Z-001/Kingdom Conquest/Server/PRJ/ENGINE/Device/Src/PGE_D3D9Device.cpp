#include "..\Include\PGE_D3D9Device.h"
#include "..\Include\PGE_D3D9DeviceCaps.h"
#include "..\Include\PGE_D3D9Surface.h"
#include "..\Include\PGE_D3D9Texture.h"
#include "..\Include\PGE_D3D9CubeTexture.h"
#include "..\Include\PGE_D3D9Effect.h"
#include "..\Include\PGE_D3D9VertexDeclaration.h"
#include "..\Include\PGE_ErrorText.h"
#include "..\Include\PGE_D3D9StateBlock.h"
#include "..\Include\PGE_D3D9VertexShader.h"
#include "..\Include\PGE_D3D9PixelShader.h"

//----------------------------------------------------------------------//
// class CD3D9Device
//----------------------------------------------------------------------//
CD3D9Device::CD3D9Device()
{
	mpD3D				 = NULL;
	mpD3DDevice			 = NULL;
	mpDeviceCaps		 = NULL;
	mpDefaultStateBlock	 = NULL;
	mpVBManager			 = NULL;
	mpIBManager			 = NULL;
	mpSurfaceManager	 = NULL;
	mpTextureManager	 = NULL;
	mpCubeTextureManager = NULL;
	mpEffectMgr			 = NULL;
	mpDeclMgr			 = NULL;
	mpStateBlockMgr		 = NULL;
	mpVertexShaderMgr	 = NULL;
	mpPixelShaderMgr	 = NULL;
	mpOccQuery			 = NULL;
	mResourceIsLost		 = FALSE;
	mRefCount			 = 1;
	mTickCount			 = 0;
	mVideoMemSize		 = 0;
	mReserveMemSize		 = 0;
	mbDeviceValid		 = FALSE;
}

CD3D9Device::~CD3D9Device()
{	
	SAFE_RELEASE(mpVBManager);
	SAFE_RELEASE(mpIBManager);
	SAFE_RELEASE(mpTextureManager);
	SAFE_RELEASE(mpCubeTextureManager);	
	SAFE_RELEASE(mpSurfaceManager);
	SAFE_RELEASE(mpEffectMgr);
	SAFE_RELEASE(mpDeclMgr);
	SAFE_RELEASE(mpStateBlockMgr);
	SAFE_RELEASE(mpVertexShaderMgr);
	SAFE_RELEASE(mpPixelShaderMgr);

	SAFE_RELEASE( mpDefaultStateBlock );
	SAFE_RELEASE( mpOccQuery );
	SAFE_DELETE( mpDeviceCaps );
	SAFE_RELEASE( mpD3DDevice );
	SAFE_RELEASE( mpD3D );
		
	LogMessage(L"Release Device...\n");
}

DWORD CD3D9Device::AddRef()
{
	InterlockedIncrement(&mRefCount);
	return mRefCount;
}

DWORD CD3D9Device::GetRefCount() const
{
	return mRefCount;
}

DWORD CD3D9Device::Release()
{
	if (InterlockedDecrement(&mRefCount) == 0L)
	{
		delete this;
		return 0;
	}
	return mRefCount;
}

BOOL CD3D9Device::Initialize(OSD3D* pD3D, const DEVICE_CONFIG* cfg)
{	
	assert(cfg);
	if (pD3D)
	{
		mpD3D = pD3D;
		mpD3D->AddRef();
	}else
	{
		mpD3D = Direct3DCreate9( D3D_SDK_VERSION );
		if (mpD3D == NULL)
		{
			DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_D3D, _T("CD3D9Device::Initialize"));
			return NULL;
		}
	}

	mGraphicConfig = *cfg;
	if (!InitDevice(cfg))
	{
		LogMessage(L"CD3D9Device::Initialize Direct3DDevice9 Failed!\n");
		return FALSE;
	}
	mReserveMemSize		= mpD3DDevice->GetAvailableTextureMem()/8;

	mStatistic.DrawPrimitiveCount = 0;
	mStatistic.DrawPrimitiveCounter = 0;

	// Initialize resource managers
	mpVBManager			 = new D3D9VertexBufferMgr;
	mpIBManager			 = new D3D9IndexBufferMgr;
	mpSurfaceManager	 = new D3D9SurfaceMgr;
	mpTextureManager	 = new D3D9TextureMgr;
	mpCubeTextureManager = new D3D9CubeTextureMgr;

	mpEffectMgr			 = new D3D9EffectMgr;
	mpVertexShaderMgr	 = new D3D9VertexShaderMgr;
	mpPixelShaderMgr	 = new D3D9PixelShaderMgr;
	mpDeclMgr			 = new D3D9VertexDeclarationMgr;
	mpStateBlockMgr		 = new D3D9StateBlockMgr;

	mpVBManager->SetDevice(this, mpD3DDevice );
	mpIBManager->SetDevice(this, mpD3DDevice );
	mpDeclMgr->SetDevice(this, mpD3DDevice);
	mpEffectMgr->SetDevice(this, mpD3DDevice);
	mpPixelShaderMgr->SetDevice(this, mpD3DDevice);
	mpVertexShaderMgr->SetDevice(this, mpD3DDevice);
	mpStateBlockMgr->SetDevice(this, mpD3DDevice);
	mpTextureManager->SetDevice( this,mpD3DDevice );
	mpCubeTextureManager->SetDevice( this,mpD3DDevice );
	mpSurfaceManager->SetDevice(this, mpD3DDevice );

	mpTextureManager->SetUserData( (void*)mpSurfaceManager );
	mpCubeTextureManager->SetUserData( (void*)mpSurfaceManager );	

	return TRUE;
}

void CD3D9Device::BuildPresentParameters(const DEVICE_CONFIG* pConfig)
{	
	assert(pConfig);
	ZeroMemory(&md3dpp, sizeof(md3dpp));
	
	md3dpp.Windowed = pConfig->Windowed?TRUE:FALSE;
	md3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	md3dpp.BackBufferCount = 1;
	md3dpp.hDeviceWindow = pConfig->hWnd;
	
	md3dpp.EnableAutoDepthStencil = pConfig->EnableDepthStencilBuffer;
	if (md3dpp.EnableAutoDepthStencil)
	{
		md3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		if (pConfig->EnableDialog && !pConfig->Windowed )
			md3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		
		md3dpp.AutoDepthStencilFormat = pConfig->DepthStencilBufferFormat;
	}else
	{
		md3dpp.Flags = 0;
	}
	md3dpp.MultiSampleType = pConfig->MultisampleType;
	if (md3dpp.MultiSampleType == D3DMULTISAMPLE_NONE)
		md3dpp.MultiSampleQuality = 0;
	else
		md3dpp.MultiSampleQuality = pConfig->MultisampleQuality;

	md3dpp.PresentationInterval = pConfig->PresentInterval;
	if (pConfig->Windowed)
	{
		md3dpp.BackBufferFormat = pConfig->DisplayMode.Format;
		md3dpp.BackBufferWidth = pConfig->DisplayMode.Width;
		md3dpp.BackBufferHeight = pConfig->DisplayMode.Height;
		md3dpp.FullScreen_RefreshRateInHz = 0;
	}else
	{
		md3dpp.BackBufferFormat = pConfig->DisplayMode.Format;
		md3dpp.BackBufferWidth = pConfig->DisplayMode.Width;
		md3dpp.BackBufferHeight = pConfig->DisplayMode.Height;
		md3dpp.FullScreen_RefreshRateInHz = pConfig->DisplayMode.RefreshRate;
	}

	if (md3dpp.BackBufferWidth < 1)
		md3dpp.BackBufferWidth = 1;
	if (md3dpp.BackBufferHeight < 1)
		md3dpp.BackBufferHeight = 1;
}

BOOL CD3D9Device::InitDevice(const DEVICE_CONFIG* pConfig)
{	
	assert(pConfig);
	assert(mpD3D);

	LogMessage( L"Initialize Device...\n");

	mbDeviceValid = FALSE;

	D3DDEVTYPE d3ddevType = pConfig->DeviceType;
	
	//Build present parameters
	BuildPresentParameters(pConfig);
	
	// Get device caps
	D3DCAPS9 caps;
	if (FAILED(mHR = mpD3D->GetDeviceCaps(pConfig->AdapterIndex, d3ddevType, &caps)))
	{
		DEVICE_EXCEPT(DXGetErrorDescription9W(mHR), _T("CD3D9Device::InitDevice()"));
	}

	// create device behaviour flag 
	unsigned long behaviourFlag = 0;
	if (pConfig->EnableDebugVS)
	{
		behaviourFlag = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		if (caps.VertexShaderVersion < pConfig->VSVersion)
		{
			behaviourFlag = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}else
		{
			behaviourFlag = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
	}
	
	if( pConfig->SoftwareVS )
	{
		behaviourFlag = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	
	//if ( pConfig->EnableMultiThread )
	{
		behaviourFlag |= D3DCREATE_MULTITHREADED;
	}

	UINT AdapterIndex = pConfig->AdapterIndex;

#ifdef _DEBUG
	for ( UINT iAdapter = 0; iAdapter < mpD3D->GetAdapterCount(); ++iAdapter )
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		if ( SUCCEEDED(mpD3D->GetAdapterIdentifier( iAdapter, 0, &identifier )))
		{
			if ( strstr( identifier.Description, "PerfHUD") != 0 )
			{
				AdapterIndex = iAdapter;
				d3ddevType = D3DDEVTYPE_REF;
				break;
			}
		} 
	}
#endif


	// Create device
	if(FAILED(mHR = mpD3D->CreateDevice(AdapterIndex, 
		d3ddevType, pConfig->hWnd,
		behaviourFlag, &md3dpp, &mpD3DDevice)))
	{
		LOG_D3D9ERR(this, mHR, L"CD3D9Device::InitDevice()\n");
		DEVICE_EXCEPT(DXGetErrorDescription9W(mHR), _T("CD3D9Device::InitDevice()"));
		return FALSE;
	}

	if (mpDeviceCaps == NULL)
	{
		// create device caps
		mpDeviceCaps = new CD3D9DeviceCaps(caps);

	}else
	{
		mpDeviceCaps->SetCaps(caps);
	}

	CreateDefaultStateBlock();

	// Enable dialog box in full screen mode
	mpD3DDevice->SetDialogBoxMode(pConfig->EnableDialog);

	mbDeviceValid = TRUE;
	return TRUE;
}

HRESULT CD3D9Device::SetDefaultRenderState()
{
	if ( mpDefaultStateBlock == NULL )
	{
		DEVICE_EXCEPT(_T("Failed to get default state block"), _T("CD3D9Device::SetDefaultRenderState"));
		return E_FAIL;
	}
	return mpDefaultStateBlock->Apply();
}

void CD3D9Device::CreateDefaultStateBlock()
{
	assert( mpDefaultStateBlock == NULL );
	
	if (mpD3DDevice)
	{
		if (FAILED(mpD3DDevice->BeginStateBlock()))
		{
			DEVICE_EXCEPT(_T("Failed to begin state block"), _T("CD3D9Device::CreateDefaultStateBlock"));
		}
	//	if ( mpDeviceCaps->IsSupportWBuffer() )
	//		mpD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
	//	else
			mpD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

		mpD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		mpD3DDevice->SetRenderState( D3DRS_SHADEMODE , D3DSHADE_GOURAUD );
		mpD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		mpD3DDevice->SetRenderState( D3DRS_LASTPIXEL, FALSE );
		mpD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		mpD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		mpD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		mpD3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD);
		mpD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		mpD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		mpD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE);
		mpD3DDevice->SetRenderState( D3DRS_ALPHAREF, 0x0);
		mpD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );

		mpD3DDevice->SetRenderState( D3DRS_DEPTHBIAS, 0);

		// Note: D3D9 中的值是FALSE
		mpD3DDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );

		mpD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
		mpD3DDevice->SetRenderState( D3DRS_FOGCOLOR, 0xFF8080A0);
		// Note : D3D9 中的值是D3DFOG_NONE
		mpD3DDevice->SetRenderState( D3DRS_FOGTABLEMODE,  D3DFOG_NONE  );
		mpD3DDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

		mpD3DDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE);
		mpD3DDevice->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
		mpD3DDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		mpD3DDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
		mpD3DDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
		mpD3DDevice->SetRenderState( D3DRS_STENCILREF, 0x0);
		mpD3DDevice->SetRenderState( D3DRS_STENCILMASK, 0xFFFFFFFF);
		mpD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
		mpD3DDevice->SetRenderState( D3DRS_CLIPPING, TRUE);
		mpD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE);

		mpD3DDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		mpD3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0000000F);
		mpD3DDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, FALSE);

		// Texture stages
		mpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		mpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
		mpD3DDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2);
		mpD3DDevice->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3);

		mpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		mpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		mpD3DDevice->SetTextureStageState( 2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		mpD3DDevice->SetTextureStageState( 3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		
		// 
		mpD3DDevice->SetVertexShader( NULL );
		mpD3DDevice->SetPixelShader( NULL );

		if (FAILED(mpD3DDevice->EndStateBlock(&mpDefaultStateBlock)))
		{
			DEVICE_EXCEPT(_T("Failed to end state block"),  _T("CD3D9Device::CreateDefaultStateBlock"));
		}
	}
}


HRESULT CD3D9Device::SetDialogBoxMode( BOOL bEnableDialogs )
{	
	return mpD3DDevice->SetDialogBoxMode( bEnableDialogs );
}

HRESULT CD3D9Device::SetRenderState(D3DRENDERSTATETYPE type, DWORD value)
{
	return  mpD3DDevice->SetRenderState(type, value);
}

HRESULT CD3D9Device::SetIndices(IIndexBuffer* indexBuffer)
{
	if (indexBuffer == NULL)
	{
		mpD3DDevice->SetIndices(NULL);
		return D3D_OK;
	}
	return mpD3DDevice->SetIndices(((D3D9IndexBuffer*)indexBuffer)->_d3d());
}

HRESULT CD3D9Device::SetStreamSource(DWORD streamNumber, IVertexBuffer* vertexBuffer, DWORD offset, DWORD strideStride)
{
	if (vertexBuffer == NULL)
	{
		mpD3DDevice->SetStreamSource(streamNumber, NULL, offset, strideStride);
		return D3D_OK;
	}

	return mpD3DDevice->SetStreamSource(streamNumber, ((D3D9VertexBuffer*)vertexBuffer)->_d3d(), offset, strideStride);
}

HRESULT CD3D9Device::SetVertexDeclaration( IVertexDeclaration* pDecl )
{
	if( pDecl == NULL )
	{
		mpD3DDevice->SetVertexDeclaration( NULL );
		return D3D_OK;
	}

	return mpD3DDevice->SetVertexDeclaration( ((D3D9VertexDeclaration*)pDecl)->_d3d() );
}

HRESULT CD3D9Device::SetTexture(DWORD stage, IBaseTexture* texture)
{
	if (texture == NULL)
	{
		mpD3DDevice->SetTexture(stage, NULL);
		return D3D_OK;
	}
	
	return mpD3DDevice->SetTexture( stage, texture->GetOSHandle() );
}

HRESULT CD3D9Device::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE Type, DWORD op)
{
	return mpD3DDevice->SetTextureStageState(stage, Type, op);
}

HRESULT CD3D9Device::SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE Type, DWORD value)
{
	return mpD3DDevice->SetSamplerState(stage, Type, value);
}

HRESULT CD3D9Device::SetViewport(DWORD x, DWORD y, DWORD w, DWORD h, float minZ /* = 0.0f */, float maxZ /* = 1.0f */)
{
	D3DVIEWPORT9 vp;
	vp.X = x;
	vp.Y = y;
	vp.Width = w;
	vp.Height= h;
	vp.MinZ = minZ;
	vp.MaxZ = maxZ;
	return mpD3DDevice->SetViewport(&vp);

}

HRESULT CD3D9Device::SetViewport(const OSViewport* vp) 
{
	return mpD3DDevice->SetViewport(vp);
}

HRESULT CD3D9Device::GetViewport(OSViewport *vp) const
{
	return mpD3DDevice->GetViewport(vp);
}

IDeviceCaps* CD3D9Device::GetDeviceCaps()
{
	return mpDeviceCaps;
}

HRESULT CD3D9Device::SetRenderTarget(ISurface* target)
{	
	if (target)
		return mpD3DDevice->SetRenderTarget(0, ((D3D9Surface*)target)->_d3d());
	else
		return mpD3DDevice->SetRenderTarget(0, NULL);
}

ISurface* CD3D9Device::GetRenderTarget() 
{	
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->GetRenderTarget())
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

ISurface* CD3D9Device::CreateRenderTarget(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality, BOOL bLockable /* = FALSE */)
{	
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->CreateRenderTarget(width, height, format, multisampleType, multisampleQuality, bLockable))
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

ISurface* CD3D9Device::CreateDepthStencilSurface(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality,  BOOL bDiscard /* = TRUE */)
{
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->CreateDepthStencil(width, height, format, multisampleType, multisampleQuality, bDiscard))
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

ISurface* CD3D9Device::GetDepthStencilSurface() 
{	
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->GetDepthStencil())
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

HRESULT CD3D9Device::SetDepthStencilSurface(ISurface* pSurface)
{	
	if (pSurface)
		return mpD3DDevice->SetDepthStencilSurface(((D3D9Surface*)pSurface)->_d3d());
	else
		return mpD3DDevice->SetDepthStencilSurface(NULL);
}

HRESULT CD3D9Device::SetScissorRect(const RECT* pRect)
{
	return mpD3DDevice->SetScissorRect(pRect);
}

HRESULT CD3D9Device::GetScissorRect(RECT* pRect)
{
	return mpD3DDevice->GetScissorRect(pRect);
}

void CD3D9Device::SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp)
{
	mpD3DDevice->SetGammaRamp( 0, Flags, pRamp );
}

void CD3D9Device::GetGammaRamp(D3DGAMMARAMP *pRamp)
{
	mpD3DDevice->GetGammaRamp( 0, pRamp );
}

HRESULT CD3D9Device::GetBackBufferDesc(D3DSURFACE_DESC* pDesc)
{
	OSSurface* surface;
	HRESULT hr =  mpD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);

	surface->GetDesc(pDesc);
	surface->Release();

	return hr;
}


HRESULT CD3D9Device::Clear(DWORD count, const D3DRECT* rects, DWORD flags, DWORD clr, float z, DWORD stencil)
{
	return mpD3DDevice->Clear(count, rects, flags, clr, z, stencil);

}

HRESULT CD3D9Device::SetFVF(DWORD fvf)
{
	return mpD3DDevice->SetFVF(fvf);
}

HRESULT CD3D9Device::SetVertexShader(IVertexShader* pVS)
{
	if ( pVS )
		return mpD3DDevice->SetVertexShader( static_cast<D3D9VertexShader*>(pVS)->_d3d() );
	else
		return mpD3DDevice->SetVertexShader( NULL );
}


HRESULT CD3D9Device::SetPixelShader(IPixelShader* pPS)
{
	if ( pPS )
		return mpD3DDevice->SetPixelShader( static_cast<D3D9PixelShader*>(pPS)->_d3d() );
	else
		return mpD3DDevice->SetPixelShader( NULL );
}


HRESULT CD3D9Device::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DXMATRIX* matrix)
{
	return mpD3DDevice->SetTransform(state, matrix);
}

HRESULT CD3D9Device::SetClipPlane(DWORD index, const float* plane)
{
	return mpD3DDevice->SetClipPlane(index, plane);
}

HRESULT CD3D9Device::SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus)
{
	return mpD3DDevice->SetClipStatus(pClipStatus);
}

void CD3D9Device::SetCursorPosition( INT X, INT Y, DWORD Flags )
{
	mpD3DDevice->SetCursorPosition(X, Y, Flags);
}

HRESULT CD3D9Device::SetCursorProperties( UINT XHotSpot, UINT YHotSpot, ISurface *pCursorBitmap)
{
	OSSurface* pSurface = NULL;
	if (pCursorBitmap)
		pSurface = static_cast< D3D9Surface* >(pCursorBitmap)->_d3d();

	return mpD3DDevice->SetCursorProperties(XHotSpot, YHotSpot, pSurface);
}

HRESULT CD3D9Device::ShowCursor(BOOL bShow)
{
	return mpD3DDevice->ShowCursor(bShow);
}

HRESULT CD3D9Device::SetLight(DWORD index, OSLight* light)
{
	return mpD3DDevice->SetLight(index, light);
}

HRESULT CD3D9Device::SetMaterial(OSMaterial* mtl)
{
	return mpD3DDevice->SetMaterial(mtl);
}

HRESULT CD3D9Device::LightEnable(DWORD index, BOOL bEnable)
{
	return mpD3DDevice->LightEnable(index, bEnable);
}

BOOL CD3D9Device::TestBeforeRender()
{
	if (mpD3DDevice == NULL || !mbDeviceValid)
		return FALSE;

	if (FAILED(mHR = mpD3DDevice->TestCooperativeLevel()))
	{
		if ( D3DERR_DEVICELOST == mHR )
		{
			Sleep(50);
			return FALSE;
		}
		
		if ( D3DERR_DEVICENOTRESET == mHR )
		{
			OnLostDevice(FALSE);
				
			if ( mGraphicConfig.OnLostDeviceProc )
			{
				(*mGraphicConfig.OnLostDeviceProc)(this, FALSE);
			}

			if (md3dpp.Windowed)
			{
				D3DDISPLAYMODE dm;
				if (FAILED(mHR = mpD3D->GetAdapterDisplayMode(0, &dm)))
					DEVICE_EXCEPT(ERR_FAILED_TO_GET_ADAPTERDISPLAY_MODE, _T("CD3D9Device::TestBeforeRender()"));
				
				md3dpp.BackBufferFormat = dm.Format;
			}
			
			// Reset the device
			if (FAILED(mHR = mpD3DDevice->Reset(&md3dpp)))
			{
				LOG_D3D9ERR(this, mHR,  L"CD3D9Device::TestBeforeRender()\n");
				//DEVICE_EXCEPT(ERR_FAILED_TO_RESET_DEVICE, _T("CD3D9Device::TestBeforeRender()"));
				Sleep(50);
				return FALSE;
			}
			
			// Get device caps
			D3DCAPS9 caps;
			if (FAILED(mpD3DDevice->GetDeviceCaps(&caps)))
			{
				LOG_D3D9ERR(this, mHR,  L"CD3D9Device::TestBeforeRender()\n");
			//	DEVICE_EXCEPT(ERR_FAILED_TO_GET_DEVICECAPS,  _T("CD3D9Device::TestBeforeRender()"));
			}else
			{
				mpDeviceCaps->SetCaps(caps);
			}
						
			// Restore 
			OnResetDevice(FALSE);

			if (mGraphicConfig.OnResetDeviceProc)
			{
				(*mGraphicConfig.OnResetDeviceProc)(this, FALSE);
			}
		}
		//DEVICE_EXCEPT(ERR_FAILED_TO_RESET_DEVICE, _T("CD3D9Device::TestBeforeRender()"));
		return FALSE;
	}
	return TRUE;
}

HRESULT CD3D9Device::BeginScene()
{
	return mpD3DDevice->BeginScene();
}

HRESULT CD3D9Device::EndScene()
{
	return mpD3DDevice->EndScene();
}	

HRESULT CD3D9Device::Present(const RECT* sourceRect /* = 0 */, const RECT* destRect /* = 0 */, HWND presentTarget /* = 0 */, CONST RGNDATA *pDirtyRegion/* = NULL*/)
{
	return mpD3DDevice->Present(sourceRect, destRect, presentTarget, pDirtyRegion);
}


HRESULT CD3D9Device::DrawPrimitive(D3DPRIMITIVETYPE type, DWORD startIndex, DWORD primitiveCount)
{
	++mStatistic.DrawPrimitiveCounter;
	mStatistic.DrawPrimitiveCount += primitiveCount;

	return mpD3DDevice->DrawPrimitive(type, startIndex, primitiveCount);
}

HRESULT CD3D9Device::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD baseVertexIndex, DWORD minIndex, DWORD numVertices, DWORD startIndex, DWORD count)
{	
	++mStatistic.DrawPrimitiveCounter;
	mStatistic.DrawPrimitiveCount += count;

	return mpD3DDevice->DrawIndexedPrimitive(type, baseVertexIndex, minIndex, numVertices, startIndex, count);
}



IDirect3DVertexBuffer9* CD3D9Device::GetDynVB( DWORD nSize )
{
	DWORD Level   = 0;
	DWORD BufSize = 1 << (MIN_DYNIB_LEVEL);
	while ( BufSize < nSize )
	{
		Level ++;
		BufSize <<= 1;
	}
	if ( Level < NUM_DYNVB_LEVEL )
	{
		DynVB& vb = m_pDynVBs[Level];
		int index = vb.Index++;
		if (vb.Index >= 2)
			vb.Index = 0;
		if (vb.pVB[index] == NULL )
		{
			if ( FAILED( mpD3DDevice->CreateVertexBuffer( BufSize, D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &(vb.pVB[index]), NULL ) ))
				return NULL;
		}
		vb.LastAccess[index] = GetTickCount();
		return vb.pVB[index];
	}else
	{
		return NULL;
	}
}

IDirect3DIndexBuffer9* CD3D9Device::GetDynIB( DWORD nCount )
{
	DWORD Level    = 0;
	DWORD BufCount = 1 << MIN_DYNIB_LEVEL;
	while ( BufCount < nCount )
	{
		Level ++;
		BufCount <<= 1;
	}
	if ( Level < NUM_DYNIB_LEVEL )
	{
		DynIB& ib = m_pDynIBs[Level];
		int index = ib.Index++;
		if (ib.Index >= 2)
			ib.Index = 0;
		if ( ib.pIB[index] == NULL )
		{
			if ( FAILED( mpD3DDevice->CreateIndexBuffer( BufCount*sizeof(WORD), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &(ib.pIB[index]), NULL ) ))
				return NULL;
		}	
		ib.LastAccess[index] = GetTickCount();
		return ib.pIB[index];
	}else
	{
		return NULL;
	}
}


HRESULT CD3D9Device::DrawPrimitiveUP(D3DPRIMITIVETYPE  type, DWORD primitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	DWORD streamSize = 0;
	switch ( type )
	{
		case D3DPT_POINTLIST	  : streamSize = VertexStreamZeroStride * primitiveCount;        break;
		case D3DPT_LINELIST		  : streamSize = VertexStreamZeroStride * primitiveCount * 2;    break;
		case D3DPT_LINESTRIP	  : streamSize = VertexStreamZeroStride * ( primitiveCount + 1); break;
		case D3DPT_TRIANGLELIST  : streamSize = VertexStreamZeroStride * primitiveCount * 3;    break;
		case D3DPT_TRIANGLESTRIP : 
		case D3DPT_TRIANGLEFAN   : streamSize = VertexStreamZeroStride * ( primitiveCount + 2 );break;
		default: return E_FAIL;
	}
	IDirect3DVertexBuffer9* pVB = GetDynVB( streamSize );
	if ( pVB == NULL )
		return E_OUTOFMEMORY;

	HRESULT hr;
	void* dst = NULL;
	if ( FAILED(hr = pVB->Lock( 0, streamSize, &dst, D3DLOCK_DISCARD)) )
		return (int)hr;

	memcpy( dst, pVertexStreamZeroData, streamSize );
	pVB->Unlock();

	mpD3DDevice->SetStreamSource( 0, pVB, 0, VertexStreamZeroStride );
	return  mpD3DDevice->DrawPrimitive( type, 0, primitiveCount ) ;
}

HRESULT CD3D9Device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type,
							UINT MinVertexIndex,
							UINT NumVertices,
							UINT PrimitiveCount,
							const void *pIndexData,
							D3DFORMAT IndexDataFormat,
							CONST void* pVertexStreamZeroData,
							UINT bytesPerVertex)
{
	DWORD indexCount = 0;
	switch ( type )
	{
	case D3DPT_POINTLIST	  : indexCount = PrimitiveCount;	 break;
	case D3DPT_LINELIST	  : indexCount = PrimitiveCount * 2; break;
	case D3DPT_LINESTRIP	  : indexCount = PrimitiveCount + 1; break;
	case D3DPT_TRIANGLELIST  : indexCount = PrimitiveCount * 3; break;
	case D3DPT_TRIANGLESTRIP : 
	case D3DPT_TRIANGLEFAN   : indexCount = PrimitiveCount + 2; break;
	default: return E_FAIL;
	}

	DWORD streamSize = bytesPerVertex*NumVertices;
	DWORD indexSize  = indexCount*sizeof(WORD);

	IDirect3DVertexBuffer9* pVB = GetDynVB( streamSize );
	IDirect3DIndexBuffer9* pIB  = GetDynIB( indexCount );
	if ( pVB == NULL || pIB == NULL )
		return E_OUTOFMEMORY;

	HRESULT hr;
	void* dst = NULL;
	if ( FAILED(hr = pVB->Lock( 0, streamSize, &dst, D3DLOCK_DISCARD)) )
		return (int)hr;
	memcpy( dst, pVertexStreamZeroData, streamSize );
	pVB->Unlock();

	if (FAILED( hr = pIB->Lock( 0, indexSize, &dst, D3DLOCK_DISCARD) ))
		return (int)hr;
	memcpy( dst, pIndexData, indexSize );
	pIB->Unlock();

	mpD3DDevice->SetStreamSource( 0, pVB, 0, bytesPerVertex );
	mpD3DDevice->SetIndices( pIB );

	return  mpD3DDevice->DrawIndexedPrimitive(  type
													, MinVertexIndex
													, 0
													, NumVertices
													, 0
													, PrimitiveCount ) ;
}

HRESULT CD3D9Device::DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE type, UINT primitiveCount, const void* streamData, UINT  bytesPerVertex, UINT  vertexCount )
{
	UINT streamSize = bytesPerVertex*vertexCount;
	IDirect3DVertexBuffer9* pVB = GetDynVB( streamSize );
	if ( pVB == NULL )
		return E_OUTOFMEMORY;

	HRESULT hr;
	void* dst = NULL;
	if ( FAILED(hr = pVB->Lock( 0, streamSize, &dst, D3DLOCK_DISCARD)) )
		return (int)hr;
	memcpy( dst, streamData, streamSize );
	pVB->Unlock();

	mpD3DDevice->SetStreamSource( 0, pVB, 0, bytesPerVertex );

	return ( mpD3DDevice->DrawIndexedPrimitive(type
			, 0
			, 0
			, vertexCount
			, 0
			, primitiveCount ) );
}

void CD3D9Device::Reset(const DEVICE_CONFIG* cfg)
{
	if (mpD3DDevice == NULL || cfg == NULL)
	{
		assert(FALSE);
		return;
	}
	mbDeviceValid = FALSE;
	if (cfg != &mGraphicConfig)
	{
		mGraphicConfig = *cfg;
	}

	//BOOL bReCreate = FALSE;
	//if ( cfg->hWnd != md3dpp.hDeviceWindow )
	//{	
	//	// Release resources 
	//	OnLostDevice(TRUE);

	//	if ( mGraphicConfig.OnLostDeviceProc )
	//	{
	//		(*mGraphicConfig.OnLostDeviceProc)( this, TRUE );
	//	}
	//	// Must recreate device
	//	SAFE_RELEASE(mpD3DDevice);
	//	InitDevice((DEVICE_CONFIG*)cfg);

	//	bReCreate = TRUE;
	//}else
	{
		// Release resources 
		OnLostDevice(FALSE);

		if ( mGraphicConfig.OnLostDeviceProc )
		{
			(*mGraphicConfig.OnLostDeviceProc)( this, FALSE );
		}

		// Rebuild present parameters
		BuildPresentParameters(cfg);
		
		// Reset direct3d9 device
		if (FAILED( mHR = mpD3DDevice->Reset(&md3dpp)))
		{		
			LOG_D3D9ERR(this, mHR,  L"CD3D9Device::Reset()\n");
			return;
		}else
		{
			// Enable dialog box in full screen mode
			mpD3DDevice->SetDialogBoxMode((cfg)->EnableDialog);
			
			// Get device caps
			D3DCAPS9 caps;
			if (FAILED(mpD3DDevice->GetDeviceCaps(&caps)))
			{
				LOG_D3D9ERR(this, mHR,  L"CD3D9Device::Reset()\n");
			//	DEVICE_EXCEPT(ERR_FAILED_TO_GET_DEVICECAPS,  _T("CD3D9Device::Reset()\n"));
			}else
			{
				(mpDeviceCaps)->SetCaps(caps);
			}
			// Enumerate supported texture formats
			//EnumerateSupportedFormats((DEVICE_CONFIG*)cfg);
		}
	}	
	mbDeviceValid = TRUE;

	// Reset resources 
	OnResetDevice(FALSE);

	if (mGraphicConfig.OnResetDeviceProc)
	{
		(*mGraphicConfig.OnResetDeviceProc)(this, FALSE);
	}
}

void CD3D9Device::OnLostDevice(BOOL bRecreate)
{	
	if (mResourceIsLost)
		return;

	int i;
	for (i = 0; i < 8; ++i)
	{
		mpD3DDevice->SetTexture(0, NULL);
	}
	for (i = 0; i < NUM_DYNIB_LEVEL; ++i)
	{
		m_pDynIBs[i].clear();
	}
	for (i = 0; i < NUM_DYNVB_LEVEL; ++i)
	{
		m_pDynVBs[i].clear();
	}
	mpD3DDevice->SetVertexShader(NULL);
	mpD3DDevice->SetPixelShader(NULL);

	mpStateBlockMgr->OnLostDevice();
	mpEffectMgr->OnLostDevice();
	mpDeclMgr->OnLostDevice();
	mpVertexShaderMgr->OnLostDevice();
	mpPixelShaderMgr->OnLostDevice();

	// NOTE: 注意表面要在纹理的前面释放
	// 因为有可能这个表面是从一个纹理中得到的,
	// 如果先释放纹理,其保存的数据有可能失效
	mpVBManager->OnLostDevice();
	mpIBManager->OnLostDevice();

	mpSurfaceManager->OnLostDevice();
	mpTextureManager->OnLostDevice();
	mpCubeTextureManager->OnLostDevice();

	SAFE_RELEASE(mpDefaultStateBlock);
	SAFE_RELEASE(mpOccQuery);

	mResourceIsLost = TRUE;
}

void CD3D9Device::OnResetDevice(BOOL bRecreate)
{
	if ( !mResourceIsLost )
		return;

	// NOTE: 注意其重置设备的顺序
	// 首先重置所有非管理的资源,因为其不需要进行显存的踢除, 
	// 这样可以增加显存的使用率
	mpStateBlockMgr->OnResetDevice();
	mpDeclMgr->OnResetDevice();
	mpVertexShaderMgr->OnResetDevice();
	mpPixelShaderMgr->OnResetDevice();	
	mpEffectMgr->OnResetDevice();

	// NOTE: 注意表面要在纹理的后面重置
	mpVBManager->OnResetDevice();
	mpIBManager->OnResetDevice();
	mpTextureManager->OnResetDevice();
	mpCubeTextureManager->OnResetDevice();
	mpSurfaceManager->OnResetDevice();

	if ( mpDefaultStateBlock == NULL )
	{
		// 如果是重新创建,'defatul state block'已经创建了
		CreateDefaultStateBlock();
	}
	mResourceIsLost = FALSE;
}


HRESULT CD3D9Device::SetVertexShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 )
{
	return mpD3DDevice->SetVertexShaderConstantF( _Index , _pData , _CntVec4 );
}

HRESULT CD3D9Device::SetPixelShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 )
{
	return mpD3DDevice->SetPixelShaderConstantF( _Index , _pData , _CntVec4 );
}

IVertexBuffer* CD3D9Device::CreateVertexBuffer(DWORD size, DWORD fvf, DWORD usage, D3DPOOL pool)
{
	D3D9VertexBuffer* pVB = mpVBManager->AddResource();
	if (!pVB->Create( size, usage, fvf,  pool ))
	{
		pVB->Release();
		return NULL;
	}
	return pVB;
}


IIndexBuffer* CD3D9Device::CreateIndexBuffer(DWORD count, DWORD usage, D3DPOOL pool)
{
	D3D9IndexBuffer* pIB = mpIBManager->AddResource();
	if ( !pIB->Create( count, usage, pool ) )
	{
		pIB->Release();
		return NULL;
	}
	return pIB;
}

ITexture* CD3D9Device::CreateTexture(DWORD width, DWORD height, DWORD level,DWORD usages, D3DFORMAT fmt, D3DPOOL pool )
{
	D3D9Texture* pTex = mpTextureManager->AddResource();
	if ( !pTex->Create( width, height, level, usages, fmt, pool ) )
	{
		pTex->Release();
		return NULL;
	}
	return pTex;
}

ITexture* CD3D9Device::CreateTexture(void* srcData, DWORD len, DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt , DWORD filter, DWORD mipFilter, DWORD colorKey, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo)
{
	D3D9Texture* pTex = mpTextureManager->AddResource();
	if ( !pTex->Create(srcData, len, width, height, level, usages, fmt, filter, mipFilter, colorKey, pool, pSrcInfo ) )
	{
		pTex->Release();
		return NULL;
	}
	return pTex;
}

ICubeTexture* CD3D9Device::CreateCubeTexture(DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool)
{
	D3D9CubeTexture* pTex = mpCubeTextureManager->AddResource();
	if ( !pTex->Create( size, level, usages, fmt, pool ) )
	{
		pTex->Release();
		return NULL;
	}
	return pTex;
}

ICubeTexture* CD3D9Device::CreateCubeTexture(void* srcData, DWORD len, DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, DWORD filter, DWORD mipFilter, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo)
{
	D3D9CubeTexture* pTex = mpCubeTextureManager->AddResource();
	if ( !pTex->Create(srcData, len, size, level, usages, fmt, filter, mipFilter, pool , pSrcInfo ) )
	{
		pTex->Release();
		return NULL;
	}
	return pTex;
}

IEffect* CD3D9Device::CreateEffect(LPD3DXBUFFER pCode)
{
	if ( pCode == NULL )
	{
		assert( FALSE );
		return NULL;
	}

	D3D9Effect* pEffect = mpEffectMgr->AddResource();
	if ( !pEffect->Create(pCode) )
	{
		pEffect->Release();
		return NULL;
	}
	return pEffect;
}

IVertexDeclaration*	CD3D9Device::CreateVertexDeclaration(const D3DVERTEXELEMENT9* elements)
{
	D3D9VertexDeclaration* pVD = mpDeclMgr->AddResource();
	if ( !pVD->Create( elements ) )
	{
		pVD->Release();
		return NULL;
	}
	return pVD;
}

IVertexShader* CD3D9Device::CreateVertexShader(LPD3DXBUFFER pCode)
{
	D3D9VertexShader* pVS = mpVertexShaderMgr->AddResource();
	if ( !pVS->Create( pCode ) )
	{
		pVS->Release();
		return NULL;
	}
	return pVS;
}

IPixelShader* CD3D9Device::CreatePixelShader(LPD3DXBUFFER pCode)
{
	D3D9PixelShader* pPS = mpPixelShaderMgr->AddResource();
	if ( !pPS->Create( pCode ) )
	{
		pPS->Release();
		return NULL;
	}
	return pPS;
}

IStateBlock* CD3D9Device::CreateStateBlock(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount)
{
	D3D9StateBlock* pSB = mpStateBlockMgr->AddResource();
	if ( !pSB->Create( pRSValues, dwRSCount, pTSSValues, dwTSSCount, pSSValues, dwSSCount ) )
	{
		pSB->Release();
		return NULL;
	}
	return pSB;
}

ISurface* CD3D9Device::CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch)
{
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->CreateOffscreenPlainSurface(width, height, fmt, bScratch))
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

UINT CD3D9Device::GetVideoMemorySize()
{
	//if (mVideoMemSize == 0)
	//{
	//	HRESULT hr;
	//	IDxDiagProvider*  pDxDiagProvider = NULL ;
	//	IDxDiagContainer* pDxDiagRoot = NULL ;
	//	// CoCreate a IDxDiagProvider*
	//	hr = CoCreateInstance( CLSID_DxDiagProvider,
	//		NULL,
	//		CLSCTX_INPROC_SERVER,
	//		IID_IDxDiagProvider,
	//		(LPVOID*) &pDxDiagProvider);

	//	if( FAILED(hr) ) // if FAILED(hr) then DirectX 9 is not installed
	//		return 0;

	//	// Fill out a DXDIAG_INIT_PARAMS struct and pass it to IDxDiagContainer::Initialize
	//	// Passing in TRUE for bAllowWHQLChecks, allows dxdiag to check if drivers are
	//	// digital signed as logo'd by WHQL which may connect via internet to update
	//	// WHQL certificates.
	//	DXDIAG_INIT_PARAMS dxDiagInitParam;
	//	ZeroMemory( &dxDiagInitParam, sizeof(DXDIAG_INIT_PARAMS) );

	//	dxDiagInitParam.dwSize                  = sizeof(DXDIAG_INIT_PARAMS);
	//	dxDiagInitParam.dwDxDiagHeaderVersion   = DXDIAG_DX9_SDK_VERSION;
	//	dxDiagInitParam.bAllowWHQLChecks        = TRUE;
	//	dxDiagInitParam.pReserved               = NULL;

	//	hr = pDxDiagProvider->Initialize( &dxDiagInitParam );
	//	if( FAILED(hr) )
	//		return 0;

	//	hr = pDxDiagProvider->GetRootContainer( &pDxDiagRoot );
	//	if( FAILED(hr) )
	//		return 0;		

	//	VARIANT var;
	//	IDxDiagContainer* pChildContainer = NULL;
	//	VariantInit( &var );

	//	IDxDiagContainer *pContainer = 0;
	//	if (FAILED(pDxDiagRoot->GetChildContainer(L"DxDiag_DisplayDevices", &pContainer)))
	//		return 0;
	//	
	//	if (FAILED(pContainer->GetChildContainer(L"0", &pChildContainer)))
	//		return 0;

	//	if (FAILED(pChildContainer->GetProp( L"szDisplayMemoryEnglish", &var )))
	//		return 0;

	//	WCHAR wszPropValue[256];
 //       // Switch off the type.  There's 4 different types:
	//	switch( var.vt )
	//	{
	//	case VT_UI4:
	//		StringCchPrintfW( wszPropValue, 256, L"%d", var.ulVal );
	//		break;
	//	case VT_I4:
	//		StringCchPrintfW( wszPropValue, 256, L"%d", var.lVal );
	//		break;
	//	case VT_BOOL:
	//		StringCchCopyW( wszPropValue, 256, (var.boolVal) ? L"true" : L"false" );
	//		break;
	//	case VT_BSTR:
	//		StringCchCopyW( wszPropValue, 256, var.bstrVal );
	//		break;
	//	}

	//	// Clear the variant (this is needed to free BSTR memory)
	//	VariantClear( &var );

	//	// 把宽字符转化成整型
	//	mVideoMemSize  = wcstol( wszPropValue, NULL, 10);
	//	mVideoMemSize *= 1024*1024;
	//}
	//return mVideoMemSize;
	return 0;
}

void CD3D9Device::GetVideoMemoryUsage(VIDEO_MEMROY_USAGE* pMemUsage)
{
	assert( pMemUsage );

	//pMemUsage->VertexBufferMemSize = mpVBManager->GetMemoryUsage();
	//pMemUsage->IndexBufferMemSize = mpIBManager->GetMemoryUsage();
	//pMemUsage->CubeTextureMemSize = mpCubeTextureManager->GetMemoryUsage();
	//pMemUsage->TextureMemSize = mpTextureManager->GetMemoryUsage();
	//pMemUsage->SurfaceMemSize = mpSurfaceManager->GetMemoryUsage();
}


UINT CD3D9Device::GetVideoMemroyUsage()
{
	return 0;
}

void CD3D9Device::EvictDynamicVBAndIB(DWORD cacheTime)
{
	DWORD curTime = GetTickCount();
	int i;
	for ( i = 0; i < NUM_DYNVB_LEVEL; ++i)
	{
		DynVB& vb = m_pDynVBs[i];
		for (int n = 0; n < 2; ++n)
		{
			if (vb.pVB[n] != NULL && curTime - vb.LastAccess[n] > cacheTime)
			{
				vb.pVB[n]->Release();
				vb.pVB[n] = NULL;
			}
		}
	}
	for ( i = 0; i < NUM_DYNIB_LEVEL; ++i)
	{
		DynIB& ib = m_pDynIBs[i];
		for (int n = 0; n < 2; ++n)
		{
			if (ib.pIB[n] != NULL && curTime - ib.LastAccess[n] > cacheTime)
			{
				ib.pIB[n]->Release();
				ib.pIB[n] = NULL;
			}
		}
	}
}

void CD3D9Device::EvictManagedResources(UINT cacheTime)
{
   	EvictDynamicVBAndIB(cacheTime);
  	if (cacheTime == 0)
	{
		OnLostDevice(FALSE);
		mpD3DDevice->EvictManagedResources();
		OnResetDevice(FALSE);
	}
}

void CD3D9Device::BeginStat()
{
	mStatistic.DrawPrimitiveCount = 0;
	mStatistic.DrawPrimitiveCounter = 0;
}

void CD3D9Device::GetStatistic(DEVICE_STATICSTIC* pStatistic)
{
	assert(pStatistic);

	*pStatistic = mStatistic;
}

HRESULT CD3D9Device::CheckDeviceFormat( DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt )
{
	D3DSURFACE_DESC desc;
	GetBackBufferDesc( &desc );
	return mpD3D->CheckDeviceFormat(  mGraphicConfig.AdapterIndex
									, mGraphicConfig.DeviceType
									, desc.Format
									, usage
									, rtype
									, checkFmt );
}

IQuery* CD3D9Device::GetOcclusionQuery()
{
	if ( mpOccQuery == NULL )
	{
		mpD3DDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &mpOccQuery );
	}
	return mpOccQuery;
}


HRESULT CD3D9Device::StretchRect( ISurface * pSourceSurface, const RECT * pSourceRect, ISurface * pDestSurface, const RECT * pDestRect, D3DTEXTUREFILTERTYPE Filter )
{
	return mpD3DDevice->StretchRect( ((D3D9Surface*)pSourceSurface)->_d3d(), pSourceRect, ((D3D9Surface*)pDestSurface)->_d3d(), pDestRect, Filter );
}

ISurface* CD3D9Device::GetBackBuffer( )
{
	D3D9Surface* pSurface = mpSurfaceManager->AddResource();
	if (!pSurface->GetBackBuffer(0))
	{
		pSurface->Release();
		return NULL;
	}
	return pSurface;
}

HRESULT CD3D9Device::GetRenderTargetData(ISurface* pRenderTarget, ISurface* pDestSurface )
{
	if( pRenderTarget && pDestSurface )
	{
		return mpD3DDevice->GetRenderTargetData( ((D3D9Surface*)pRenderTarget)->_d3d(), ((D3D9Surface*)pDestSurface)->_d3d() );
	}else
	{
		return E_FAIL;
	}
}

void CD3D9Device::LogMessage(const wchar_t* format, ...)
{
	if ( mGraphicConfig.pLogStream == NULL )
		return;

	const int MAX_BUF_SIZE = 1024;
	wchar_t wszBuffer[MAX_BUF_SIZE];

	// Write time into log
	struct tm *newtime;
	time_t ltime;
	time( &ltime );
	newtime = localtime( &ltime );

	swprintf(wszBuffer, L"%d:%d:%d: ", newtime->tm_hour, newtime->tm_min, newtime->tm_sec );

	int len  = wcslen(wszBuffer);

	va_list list;
	va_start( list, format );
	vswprintf( wszBuffer + len, format, list);
	va_end( list );

	mGraphicConfig.pLogStream->Notify( wszBuffer );
}
