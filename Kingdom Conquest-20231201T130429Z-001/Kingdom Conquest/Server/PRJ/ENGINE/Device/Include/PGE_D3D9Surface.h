#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class D3D9Surface;
class D3D9Texture;
class D3D9CubeTexture;

typedef D3D9ResourceMgr< D3D9Surface > D3D9SurfaceMgr;

class D3D9Surface: public D3D9Resource< ISurface, D3D9SurfaceMgr >
{
public:
	typedef D3D9Resource< ISurface, D3D9SurfaceMgr > base_t;

	D3D9Surface() : m_pSurface( NULL )
		, m_CreateType(NO_OWNER)
	{
	}
	~D3D9Surface()
	{
		SAFE_RELEASE(m_pSurface);
	}
	// 锁定表面
	virtual HRESULT LockRect( D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag )
	{
		return ( m_pSurface->LockRect( (lr)
									, (rect)
									, (flag ) ) );
	}

	// 解锁表面
	virtual void UnlockRect()
	{
		( m_pSurface->UnlockRect() );
	}

	// 得到表面描述
	virtual BOOL GetDesc( D3DSURFACE_DESC* desc) 
	{
		if (FAILED( m_pSurface->GetDesc( (desc)) ))
			return FALSE;
		return TRUE;
	}

	// 
	virtual HRESULT StretchRect( const D3DRECT* dstRect, const RECT* srcRect, ISurface* srcSurface, DWORD filter )
	{
		if( srcSurface == NULL )
			return E_FAIL;

		return ( D3DXLoadSurfaceFromSurface( m_pSurface,
											NULL, 
											(const RECT*)dstRect,
											((D3D9Surface*)srcSurface)->_d3d(), 
											NULL,
											(const RECT*)srcRect,  
											filter,
											0 ) );
	}

	// 用表面'pSrcSurface'的数据更新自己的数据，如果大小不匹配会缩放
	virtual HRESULT Update( ISurface* pSrcSurface, const RECT* pDestRect, const RECT* pSrcRect, DWORD Filter, DWORD ColorKey ) 
	{
		if ( m_pSurface == NULL )
		{
			return E_FAIL;
		}	

		HRESULT hr = D3DXLoadSurfaceFromSurface(  m_pSurface
												, NULL
												, pDestRect
												, static_cast<D3D9Surface*>(pSrcSurface)->m_pSurface
												, NULL
												, pSrcRect
												, Filter
												, ColorKey);
		return hr;
	}
	// 保存表面到文件
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) 
	{
		if (m_pSurface == NULL)
		{
			return E_FAIL;
		}
		return D3DXSaveSurfaceToFileA( pFileName, imgFmt, m_pSurface, pSrcPalette, pSrcRect );
	}
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) 
	{
		if (m_pSurface == NULL)
		{
			return E_FAIL;
		}
		return D3DXSaveSurfaceToFileW( pFileName, imgFmt, m_pSurface, pSrcPalette, pSrcRect );
	}

	// 从文件中加载数据
	virtual HRESULT LoadFromFileInMemory(const PALETTEENTRY *pDestPalette, const RECT *pDestRect,
		LPCVOID pSrcData, UINT SrcData,
		const RECT *pSrcRect,
		DWORD Filter,
		D3DCOLOR ColorKey,
		D3DXIMAGE_INFO *pSrcInfo )
	{
		if (m_pSurface == NULL)
		{
			return E_FAIL;
		}
		return D3DXLoadSurfaceFromFileInMemory( m_pSurface, pDestPalette, pDestRect, 
												pSrcData, SrcData, pSrcRect, 
												Filter, ColorKey, pSrcInfo );
	}

	//
	BOOL CreateRenderTarget( DWORD width, DWORD height, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality, BOOL bLockable );
	BOOL CreateDepthStencil( DWORD width, DWORD height, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE multisampleType, DWORD multisampleQuality, BOOL bDiscard );
	BOOL CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch );
	BOOL GetByTexture( D3D9Texture* pTex, DWORD level );
	BOOL GetByCubeTexture( D3D9CubeTexture* pTex, D3DCUBEMAP_FACES faceType, DWORD level );
	BOOL GetRenderTarget();
	BOOL GetDepthStencil();
	BOOL GetBackBuffer( DWORD iBackBuffer );
	BOOL Create( IDirect3DSurface9* surface );

	void OnLostDevice();
	void OnResetDevice();

	inline IDirect3DSurface9* _d3d() { return m_pSurface; }
protected:
	enum CREATE_TYPE
	{
		GET_BY_TEXTURE		= 0,
		GET_BY_CUBETEXTURE,
		GET_RENDERTARGET,
		GET_DEPTHSTENCIL,
		GET_BACKBUFFER,
		CREATE_OFFSCREEN_SURFACE,
		CREATE_RENDERTARGET,
		CREATE_DEPTHSTENCIl,
		CREATE_INNER,
		NO_OWNER,
	};

	IDirect3DSurface9* m_pSurface;

	CREATE_TYPE m_CreateType;

	// 用于重新创建的参数
	union 
	{
		struct 
		{
			D3D9Texture*		m_pTexture;
			UINT				m_Level;
		};
		struct 
		{
			D3D9CubeTexture*	m_pCubeTexture;
			UINT				m_Level;
			D3DCUBEMAP_FACES	m_FaceType;
		};
		struct 
		{	
			DWORD				  m_iBackBuffer;
		};
		struct 
		{
			DWORD			    m_Width;
			DWORD			    m_Height;
			D3DFORMAT		    m_Format;
			D3DMULTISAMPLE_TYPE m_MultisampleType;
			DWORD			    m_MultisampleQuality;
			BOOL			    m_IsDiscard   : 8;	// 创建DepthStencilSurface的附加属性
			BOOL				m_IsLockable  : 8;	// 创建RenderTarget时的附加属性		
			BOOL				m_IsScratched : 16;	// 创建OffscreenPlainSurface的附加属性
		};	

	};
};
