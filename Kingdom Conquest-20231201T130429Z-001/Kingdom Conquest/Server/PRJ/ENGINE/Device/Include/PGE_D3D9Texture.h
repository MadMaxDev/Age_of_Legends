#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"
#include "PGE_D3D9Surface.h"

class D3D9Texture;

typedef D3D9ResourceMgr< D3D9Texture > D3D9TextureMgr;

class D3D9Texture: public D3D9Resource< ITexture, D3D9TextureMgr >
{
public:
	D3D9Texture() : m_pTexture( NULL )
		, m_pSurfaces( NULL )
		, m_Level(0)
	{
	}
	~D3D9Texture()
	{
		if(m_pSurfaces!=NULL)
		{
			for ( DWORD i = 0; i < m_Level; ++i )
			{
				SAFE_RELEASE(m_pSurfaces[i]);
			}	
		}
		SAFE_DELETE_ARRAY( m_pSurfaces );
		SAFE_RELEASE( m_pTexture );
	}
	// 得到资源优先级
	virtual DWORD GetPriority() 
	{
		return m_pTexture->GetPriority();
	}

	// 设置资源优先级
	virtual void SetPriority(DWORD priority) 
	{
		m_pTexture->SetPriority( priority );
	}

	// 	
	virtual DWORD GetLevelCount()
	{
		return m_Level;
	}

	virtual BOOL GetLevelDesc( DWORD level, D3DSURFACE_DESC* desc )
	{
		return SUCCEEDED( m_pTexture->GetLevelDesc( level, (desc) ) );
	}
	virtual BOOL GenerateMipSubLevels()
	{
		DWORD LevelCount = m_pTexture->GetLevelCount();
		for (DWORD i = 0; i < LevelCount; ++i)
		{
			IDirect3DSurface9* pDstSurface = NULL;
			IDirect3DSurface9* pSrcSurface = NULL;
			if (SUCCEEDED( m_pTexture->GetSurfaceLevel(i-1, &pSrcSurface)))
			{
				if ( SUCCEEDED(m_pTexture->GetSurfaceLevel( i, &pDstSurface )) ) 
				{
					D3DXLoadSurfaceFromSurface( pDstSurface, NULL, NULL, 
						pSrcSurface, NULL, NULL,
						D3DX_FILTER_LINEAR|D3DX_FILTER_DITHER, 0);
					pDstSurface->Release();
				}
				pSrcSurface->Release();
			}
		}
		return TRUE;
	}

	//
	virtual D3DRESOURCETYPE GetResourceType() { return D3DRTYPE_TEXTURE; }

	// 锁定纹理表面
	virtual HRESULT LockRect(DWORD level, D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag) 
	{
		return ( m_pTexture->LockRect( level
									, (lr)
									, (rect)
									, (flag) ) );
	}

	// 解锁纹理表面
	virtual void UnlockRect(DWORD level) 
	{
		m_pTexture->UnlockRect(level); 
	}

	// 得到纹理表面
	virtual ISurface* GetSurfaceLevel(DWORD level) 
	{
		D3D9Surface* pSurface = m_pSurfaces[level];
		if ( pSurface == NULL )
		{
			D3D9SurfaceMgr* surfaceMgr = (D3D9SurfaceMgr*)( GetMgrUserData() );
			assert( surfaceMgr );
			pSurface = surfaceMgr->AddResource();
			if ( !pSurface->GetByTexture( this, level ) )
			{
				pSurface->Release();
				return NULL;
			}
			m_pSurfaces[level] = pSurface;
		}
		pSurface->AddRef();
		return pSurface;
	}

	// 保存纹理到文件
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette)
	{
		return D3DXSaveTextureToFileA(pFileName, imgFmt, m_pTexture, pSrcPalette);
	}
	
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette)
	{
		return D3DXSaveTextureToFileW(pFileName, imgFmt, m_pTexture, pSrcPalette);
	}
	virtual OSBaseTexture* GetOSHandle() 
	{
		return m_pTexture;
	}
	BOOL Create( DWORD width, DWORD height, DWORD level,DWORD usage, D3DFORMAT fmt, D3DPOOL pool )
	{
		m_Width  = width;
		m_Height = height;
		m_Level  = level;
		m_Usage  = (usage);
		m_Format = (fmt);
		m_Pool   = (pool);

		HRESULT hr;
		if (FAILED( hr = GetD3DDevice()->CreateTexture(   m_Width
			, m_Height
			, m_Level
			, m_Usage
			, m_Format
			, m_Pool
			, &m_pTexture
			, NULL )))
		{
			GetD3DDevice()->EvictManagedResources();
			if (FAILED( hr = GetD3DDevice()->CreateTexture(   m_Width
				, m_Height
				, m_Level
				, m_Usage
				, m_Format
				, m_Pool
				, &m_pTexture
				, NULL )))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_TEXTURE, hr );	
				return FALSE;
			}
		}
		m_Level	= m_pTexture->GetLevelCount();
		if ( m_Level < 1 )
			return FALSE;

		m_pSurfaces = new D3D9Surface*[m_Level];
		memset( m_pSurfaces, 0, sizeof(D3D9Surface*)*m_Level);
		return TRUE;
	}
	BOOL Create( const void* srcData, DWORD len,
						DWORD width, DWORD height, DWORD level,
						DWORD usages,  D3DFORMAT format,
						DWORD filter, DWORD mipFilter, DWORD ColorKey, D3DPOOL pool,
						D3DXIMAGE_INFO* pSrcInfo)
	{
		m_Width  = width;
		m_Height = height;
		m_Level  = level;
		m_Usage  = (usages);
		m_Format = (format);
		m_Pool   = (pool);

		HRESULT hr;
		// Create texture
		if ( FAILED(hr = D3DXCreateTextureFromFileInMemoryEx( GetD3DDevice()
									, srcData
									, len
									, width
									, height
									, level
									, usages
									, format
									, pool
									, filter
									, mipFilter
									, ColorKey
									, pSrcInfo
									, NULL
									, &m_pTexture )))
		{
			GetD3DDevice()->EvictManagedResources();
			if ( FAILED(hr = D3DXCreateTextureFromFileInMemoryEx( GetD3DDevice()
				, srcData
				, len
				, width
				, height
				, level
				, usages
				, format
				, pool
				, filter
				, mipFilter
				, ColorKey
				, pSrcInfo
				, NULL
				, &m_pTexture )))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_TEXTURE, hr );
				return FALSE;
			}
		}
		m_Level	= m_pTexture->GetLevelCount();
		if ( m_Level < 1 )
			return FALSE;

		m_pSurfaces = new D3D9Surface*[m_Level];
		memset( m_pSurfaces, 0, sizeof(D3D9Surface*)*m_Level);
		return TRUE;
	}

	void OnLostDevice()
	{
		if ( m_Pool == D3DPOOL_DEFAULT )
		{
			SAFE_RELEASE(m_pTexture);
			SetDirtyFlag(TRUE);
		}
	}	

	void OnResetDevice()
	{
		if ( m_Pool == D3DPOOL_DEFAULT )
		{
			HRESULT hr;
			if (FAILED( hr = GetD3DDevice()->CreateTexture(   m_Width
				, m_Height
				, m_Level
				, m_Usage
				, m_Format
				, D3DPOOL_DEFAULT
				, &m_pTexture
				, NULL )))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_TEXTURE, hr );
			//	GRAPHIC_EXCEPT( ERR_FAILED_TO_CREATE_TEXTURE, L"D3D9Texture::OnResetDevice" );
			}
		}
	}

	inline IDirect3DTexture9* _d3d() { return m_pTexture; }
protected:
	IDirect3DTexture9* m_pTexture;

	// 创建时的参数
	DWORD			   m_Width;
	DWORD			   m_Height;
	DWORD			   m_Level;
	DWORD			   m_Usage;
	D3DFORMAT		   m_Format;
	D3DPOOL			   m_Pool;
	D3D9Surface**	   m_pSurfaces;
};
