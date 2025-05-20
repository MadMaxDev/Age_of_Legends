#pragma once 

#include "PGE_Graphic.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"
#include "PGE_D3D9Surface.h"

class D3D9CubeTexture;

typedef D3D9ResourceMgr< D3D9CubeTexture > D3D9CubeTextureMgr;


class D3D9CubeTexture : public D3D9Resource< ICubeTexture, D3D9CubeTextureMgr >
{
public:
	D3D9CubeTexture() : m_pCubeTex ( NULL )
		, m_pSurfaces( NULL )
		, m_Level(0)
	{
	}
	~D3D9CubeTexture()
	{
		if(m_pSurfaces!=NULL)
		{
			DWORD n = m_Level*6;
			for ( DWORD i = 0; i < n; ++i )
			{
				SAFE_RELEASE( m_pSurfaces[i] );
			}
		}

		SAFE_DELETE_ARRAY( m_pSurfaces );	
		SAFE_RELEASE( m_pCubeTex );
	}
	// 得到资源优先级
	virtual DWORD GetPriority() 
	{
		return m_pCubeTex->GetPriority();
	}

	// 设置资源优先级
	virtual void SetPriority(DWORD priority) 
	{
		m_pCubeTex->SetPriority( priority );
	}

	//
	virtual D3DRESOURCETYPE GetResourceType() { return D3DRTYPE_CUBETEXTURE; }

	// 重新生成Mipmap
	virtual BOOL GenerateMipSubLevels() 
	{
		DWORD LevelCount = m_pCubeTex->GetLevelCount();
		for (DWORD i = 0; i < LevelCount; ++i)
		{
			for (int iFace = 0; iFace < 6; ++iFace)
			{
				IDirect3DSurface9* pDstSurface = NULL;
				IDirect3DSurface9* pSrcSurface = NULL;
				if (SUCCEEDED( m_pCubeTex->GetCubeMapSurface( (D3DCUBEMAP_FACES)iFace, i-1, &pSrcSurface)))
				{
					if ( SUCCEEDED( m_pCubeTex->GetCubeMapSurface( (D3DCUBEMAP_FACES)iFace, i, &pSrcSurface))) 
					{
						D3DXLoadSurfaceFromSurface( pDstSurface, NULL, NULL,
							pSrcSurface, NULL, NULL,
							D3DX_FILTER_LINEAR|D3DX_FILTER_DITHER, 0);
						pDstSurface->Release();
					}
					pSrcSurface->Release();
				}
			}
		}
		return TRUE;
	}
	// 保存纹理到文件
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette)
	{
		return D3DXSaveTextureToFileA(pFileName, imgFmt, m_pCubeTex, pSrcPalette);
	}

	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette)
	{
		return D3DXSaveTextureToFileW(pFileName, imgFmt, m_pCubeTex, pSrcPalette);
	}
	virtual OSBaseTexture* GetOSHandle() 
	{
		return m_pCubeTex;
	}

	// 得到Mipmap的级数
	virtual DWORD GetLevelCount()
	{
		return m_pCubeTex->GetLevelCount();
	}

	// 得到此纹理的表面描述
	virtual BOOL GetLevelDesc(DWORD level, D3DSURFACE_DESC* desc) 
	{
		return SUCCEEDED( m_pCubeTex->GetLevelDesc(level, (desc) ) );
	}

	// 锁定纹理表面
	virtual HRESULT LockRect(D3DCUBEMAP_FACES faceType ,DWORD level, D3DLOCKED_RECT* lr, const RECT* rect,DWORD flag)
	{
		return ( m_pCubeTex->LockRect(  (faceType)
			, level
			, (lr)
			, (rect)
			, (flag) ) );
	}

	// 解锁纹理表面
	virtual void UnlockRect(D3DCUBEMAP_FACES faceType, DWORD level)
	{
		m_pCubeTex->UnlockRect(  faceType, level );
	}

	// 得到纹理表面
	virtual ISurface* GetCubeMapSurface(D3DCUBEMAP_FACES faceType, DWORD level) 
	{
		int iFace = (int)faceType;
		D3D9Surface* pSurface = m_pSurfaces[level*6+iFace];
		if ( pSurface == NULL )
		{	
			D3D9SurfaceMgr* surfaceMgr = (D3D9SurfaceMgr*)( GetMgrUserData() );
			assert( surfaceMgr );
			pSurface = surfaceMgr->AddResource();
			if ( !pSurface->GetByCubeTexture( this, faceType, level ) )
			{
				pSurface->Release();
				return NULL;
			}
			m_pSurfaces[level*6+iFace] = pSurface;
		}
		pSurface->AddRef();
		return pSurface;
	}

	BOOL Create( DWORD size, DWORD level,DWORD usage, D3DFORMAT fmt, D3DPOOL pool )
	{
		m_Size	 = size;
		m_Level  = level;
		m_Usage  = (usage);
		m_Format = (fmt);
		m_Pool   = (pool);

		HRESULT hr;
		if (FAILED( hr = GetD3DDevice()->CreateCubeTexture(   m_Size
			, m_Level
			, m_Usage
			, m_Format
			, m_Pool
			, &m_pCubeTex
			, NULL )))
		{
			DXTRACE_ERRW( ERR_FAILED_TO_CREATE_CUBETEXTURE , hr );
			return FALSE;
		}
		m_Level	= m_pCubeTex->GetLevelCount();
		if ( m_Level < 1 )
			return FALSE;

		m_pSurfaces = new D3D9Surface*[m_Level*6];
		memset( m_pSurfaces, 0, sizeof(D3D9Surface*)*m_Level*6);

		return TRUE;
	}

	BOOL Create(  const void* srcData, DWORD len,
						DWORD size, DWORD level,
						DWORD usages,  D3DFORMAT format,
						DWORD filter, DWORD mipFilter,
						D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo)
	{
		m_Size	 = size;
		m_Level  = level;
		m_Usage  = (usages);
		m_Format = (format);
		m_Pool   = (pool);

		// Create texture from file
		HRESULT hr;
		if ( FAILED( hr = D3DXCreateCubeTextureFromFileInMemoryEx( GetD3DDevice()
			, srcData
			, len
			, size
			, level
			, usages
			, format
			, pool
			, filter
			, mipFilter
			, 0
			, pSrcInfo
			, NULL
			, &m_pCubeTex)))
		{
			DXTRACE_ERRW(ERR_FAILED_TO_CREATE_CUBETEXTURE, hr);
			return FALSE;
		}
		m_Level	= m_pCubeTex->GetLevelCount();
		if ( m_Level < 1 )
			return FALSE;

		m_pSurfaces = new D3D9Surface*[m_Level*6];
		memset( m_pSurfaces, 0, sizeof(D3D9Surface*)*m_Level*6);

		return TRUE;
	}

	void OnLostDevice()
	{
		if ( m_Pool == D3DPOOL_DEFAULT )
		{
			SAFE_RELEASE(m_pCubeTex);
			SetDirtyFlag(TRUE);
		}
	}	

	void OnResetDevice()
	{
		if ( m_Pool == D3DPOOL_DEFAULT )
		{
			HRESULT hr;
			if (FAILED( hr =  GetD3DDevice()->CreateCubeTexture(  m_Size
				, m_Level
				, m_Usage
				, m_Format
				, D3DPOOL_DEFAULT
				, &m_pCubeTex
				, NULL )))
			{
				DXTRACE_ERRW( ERR_FAILED_TO_CREATE_CUBETEXTURE , hr );
				DEVICE_EXCEPT( ERR_FAILED_TO_CREATE_CUBETEXTURE, L"D3D9CubeTexture::OnResetDevice" );
			}
		}
	}
	inline IDirect3DCubeTexture9* _d3d() { return m_pCubeTex; }
protected:	
	IDirect3DCubeTexture9* m_pCubeTex;
	DWORD				   m_Size;
	DWORD				   m_Level;
	D3DFORMAT			   m_Format;
	DWORD				   m_Usage;
	D3DPOOL				   m_Pool;
	D3D9Surface**		   m_pSurfaces;
};