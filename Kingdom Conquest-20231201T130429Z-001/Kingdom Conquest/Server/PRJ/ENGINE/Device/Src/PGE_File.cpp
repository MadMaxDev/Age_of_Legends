#include "../Include/PGE_File.h"
#include "../Include/PGE_Mesh.h"
#include <stdio.h>
#include <assert.h>
 
#pragma warning( disable : 4311 )

//------------------------------------------------------------------------
// Name: PGEReadFile
// Desc: 
//------------------------------------------------------------------------
void* WINAPI PGEReadFileA(LPCSTR pFileName, long& size)
{
	FILE* fp = fopen(pFileName, "rb" );
	if (fp == NULL)
	{
		return NULL;
	}
	size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void* pSrcData = new char[size];
	assert(pSrcData);

	// Read from file
	fread( pSrcData, 1, size, fp);

	fclose(fp); 
	return pSrcData;
}

void* WINAPI PGEReadFileW(LPCWSTR pFileName, long& size)
{
	FILE* fp = _wfopen(pFileName, L"rb" );
	if (fp == NULL)
	{
		return NULL;
	}
	size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void* pSrcData = new char[size];
	assert(pSrcData);

	// Read from file
	fread( pSrcData, 1, size, fp);

	fclose(fp); 
	return pSrcData;
}

//------------------------------------------------------------------------
// Name: PGELoadResource
// Desc: 
//------------------------------------------------------------------------
void* WINAPI PGELoadResourceA(HMODULE hModule, UINT nID, LPCSTR pResType, DWORD& nSize)
{
	try 
	{
		HRSRC hRes = FindResourceA( hModule, MAKEINTRESOURCEA(nID), pResType);
		if ( hRes == NULL )
		{
			return NULL;
		}

		HGLOBAL hGlobal = LoadResource( hModule, hRes );
		if ( hGlobal  == NULL )
		{
			return NULL;
		}

		void* pSrcData = LockResource( hGlobal );
		nSize		   = SizeofResource( hModule, hRes );

		return pSrcData;
	}
	catch(...)
	{
		return NULL;
	}
}

void* WINAPI PGELoadResourceW(HMODULE hModule, UINT nID, LPCWSTR pResType, DWORD& nSize)
{
	try 
	{
		HRSRC hRes = FindResourceW( hModule, MAKEINTRESOURCEW(nID), pResType);
		if ( hRes == NULL )
		{
			return NULL;
		}

		HGLOBAL hGlobal = LoadResource( hModule, hRes );
		if ( hGlobal  == NULL )
		{
			return NULL;
		}

		void* pSrcData = LockResource( hGlobal );
		nSize		   = SizeofResource( hModule, hRes );

		return pSrcData;
	}
	catch(...)
	{
		return NULL;
	}
}
//------------------------------------------------------------------------
// Name: PGECreateTextureFromFile
// Desc: 
//------------------------------------------------------------------------
ITexture* WINAPI PGECreateTextureFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName)
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, filesize, 
											D3DX_DEFAULT, D3DX_DEFAULT,
											D3DX_DEFAULT, 0, 
											D3DFMT_UNKNOWN, 
											D3DX_DEFAULT, D3DX_DEFAULT, 0,
											D3DPOOL_MANAGED, NULL );

	delete []pSrcData;
	return pTex;
}

ITexture* WINAPI PGECreateTextureFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName)
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, filesize, 
											D3DX_DEFAULT, D3DX_DEFAULT,
											D3DX_DEFAULT, 0, 
											D3DFMT_UNKNOWN, 
											D3DX_DEFAULT, D3DX_DEFAULT, 0,
											D3DPOOL_MANAGED, NULL );

	delete []pSrcData;
	return pTex;
}


//------------------------------------------------------------------------
// Name: PGECreateTextureFromResource
// Desc: 
//------------------------------------------------------------------------
ITexture* WINAPI PGECreateTextureFromResourceA(IGraphicDevice* pDevice,  HMODULE hModule, UINT nID, LPCSTR pResType)
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceA(hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, nSize, 
											D3DX_DEFAULT, D3DX_DEFAULT,
											D3DX_DEFAULT, 0, 
											D3DFMT_UNKNOWN, 
											D3DX_DEFAULT, D3DX_DEFAULT, 0,
											D3DPOOL_MANAGED, NULL );
	return pTex;
}

ITexture* WINAPI PGECreateTextureFromResourceW(IGraphicDevice* pDevice,  HMODULE hModule, UINT nID, LPCWSTR pResType)
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceW(hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, nSize, 
											D3DX_DEFAULT, D3DX_DEFAULT,
											D3DX_DEFAULT, 0, 
											D3DFMT_UNKNOWN, 
											D3DX_DEFAULT, D3DX_DEFAULT,0,
											D3DPOOL_MANAGED, NULL );
	return pTex;
}


//------------------------------------------------------------------------
// Name: PGECreateTextureFromFileEx
// Desc: 
//------------------------------------------------------------------------

ITexture* WINAPI PGECreateTextureFromFileExA(IGraphicDevice* pDevice, LPCSTR pFileName, 
											UINT Width, UINT Height,
											UINT MipLevels, DWORD Usage,
											D3DFORMAT Format, DWORD Filter,
											DWORD MipFilter, DWORD ColorKey,
											D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, filesize, 
											Width, Height,
											MipLevels, Usage, 
											Format, 
											Filter, MipFilter, ColorKey,
											D3DPOOL_MANAGED, pSrcInfo );

	delete []pSrcData;
	return pTex;
}

ITexture* WINAPI PGECreateTextureFromFileExW(IGraphicDevice* pDevice, LPCWSTR pFileName, 
											UINT Width, UINT Height,
											UINT MipLevels, DWORD Usage,
											D3DFORMAT Format, DWORD Filter,
											DWORD MipFilter, DWORD ColorKey,
											D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, filesize, 
											Width, Height,
											MipLevels, Usage, 
											Format, 
											Filter, MipFilter, ColorKey,
											D3DPOOL_MANAGED, pSrcInfo );

	delete []pSrcData;
	return pTex;
}

//------------------------------------------------------------------------
// Name: PGECreateTextureFromResourceEx
// Desc: 
//------------------------------------------------------------------------
ITexture* WINAPI PGECreateTextureFromResourceExA(IGraphicDevice* pDevice, 
												HMODULE hModule, UINT nID, LPCSTR pResType, 
												UINT Width, UINT Height,
												UINT MipLevels, DWORD Usage,
												D3DFORMAT Format, DWORD Filter,
												DWORD MipFilter, DWORD ColorKey,
												D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceA(hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, nSize, 
		Width, Height,
		MipLevels, Usage, 
		Format, 
		Filter, MipFilter, ColorKey,
		D3DPOOL_MANAGED, pSrcInfo );

	return pTex;
}

ITexture* WINAPI PGECreateTextureFromResourceExW(IGraphicDevice* pDevice, 
												HMODULE hModule, UINT nID, LPCWSTR pResType, 
												UINT Width, UINT Height,
												UINT MipLevels, DWORD Usage,
												D3DFORMAT Format, DWORD Filter,
												DWORD MipFilter, DWORD ColorKey,
												D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceW(hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ITexture* pTex = pDevice->CreateTexture( pSrcData, nSize, 
		Width, Height,
		MipLevels, Usage, 
		Format, 
		Filter, MipFilter, ColorKey,
		D3DPOOL_MANAGED, pSrcInfo );

	return pTex;
}


//------------------------------------------------------------------------
// Name: PGECreateCubeTextureFromFile
// Desc: 
//------------------------------------------------------------------------

ICubeTexture* WINAPI PGECreateCubeTextureFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName)
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData,
													filesize, 
													D3DX_DEFAULT, 
													D3DX_DEFAULT, 
													0, 
													D3DFMT_UNKNOWN, 
													D3DX_DEFAULT,
													D3DX_DEFAULT,
													D3DPOOL_MANAGED,
													NULL );

	delete []pSrcData;
	return pTex;
}
ICubeTexture* WINAPI PGECreateCubeTextureFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName)
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData,
													filesize, 
													D3DX_DEFAULT, 
													D3DX_DEFAULT, 
													0, 
													D3DFMT_UNKNOWN, 
													D3DX_DEFAULT,
													D3DX_DEFAULT,
													D3DPOOL_MANAGED,
													NULL );

	delete []pSrcData;
	return pTex;
}


//------------------------------------------------------------------------
// Name: PGECreateCubeTextureFromResource
// Desc: 
//------------------------------------------------------------------------
ICubeTexture* WINAPI PGECreateCubeTextureFromResourceA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType)
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData,
		nSize, 
		D3DX_DEFAULT, 
		D3DX_DEFAULT, 
		0, 
		D3DFMT_UNKNOWN, 
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DPOOL_MANAGED,
		NULL );

	return pTex;
}

ICubeTexture* WINAPI PGECreateCubeTextureFromResourceW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType)
{	
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, nSize);
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData,
		nSize, 
		D3DX_DEFAULT, 
		D3DX_DEFAULT, 
		0, 
		D3DFMT_UNKNOWN, 
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DPOOL_MANAGED,
		NULL );

	return pTex;
}


//------------------------------------------------------------------------
// Name: PGECreateCubeTextureFromFileEx
// Desc: 
//------------------------------------------------------------------------
ICubeTexture* WINAPI PGECreateCubeTextureFromFileExA(IGraphicDevice* pDevice, LPCSTR pFileName, 
													UINT Size,UINT MipLevels, DWORD Usage,
													D3DFORMAT Format, DWORD Filter,
													DWORD MipFilter, 
													D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData, filesize, 
		Size, MipLevels, Usage, 
		Format, Filter, MipFilter,
		D3DPOOL_MANAGED, pSrcInfo );

	delete []pSrcData;
	return pTex;
}

ICubeTexture* WINAPI PGECreateCubeTextureFromFileExW(IGraphicDevice* pDevice, LPCWSTR pFileName, 
													UINT Size,UINT MipLevels, DWORD Usage,
													D3DFORMAT Format, DWORD Filter,
													DWORD MipFilter, 
													D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData, filesize, 
		Size, MipLevels, Usage, 
		Format, Filter, MipFilter,
		D3DPOOL_MANAGED, pSrcInfo );

	delete []pSrcData;
	return pTex;
}

//------------------------------------------------------------------------
// Name: PGECreateCubeTextureFromResourceEx
// Desc: 
//------------------------------------------------------------------------

ICubeTexture* WINAPI PGECreateCubeTextureFromResourceExA(IGraphicDevice* pDevice,  
														HMODULE hModule, UINT nID, LPCSTR pResType, 
														UINT Size,UINT MipLevels, DWORD Usage,
														D3DFORMAT Format, DWORD Filter,
														DWORD MipFilter, 
														D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && hModule );

	DWORD filesize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, filesize);
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData, filesize, 
		Size, MipLevels, Usage, 
		Format, Filter, MipFilter,
		D3DPOOL_MANAGED, pSrcInfo );
	return pTex;
}

ICubeTexture* WINAPI PGECreateCubeTextureFromResourceExW(IGraphicDevice* pDevice,  
														HMODULE hModule, UINT nID, LPCWSTR pResType, 
														UINT Size,UINT MipLevels, DWORD Usage,
														D3DFORMAT Format, DWORD Filter,
														DWORD MipFilter, 
														D3DXIMAGE_INFO* pSrcInfo )
{	
	assert( pDevice && hModule );

	DWORD filesize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, filesize);
	if (pSrcData == NULL)
		return NULL;

	ICubeTexture* pTex = pDevice->CreateCubeTexture( pSrcData, filesize, 
		Size, MipLevels, Usage, 
		Format, Filter, MipFilter,
		D3DPOOL_MANAGED, pSrcInfo );
	return pTex;
}


//------------------------------------------------------------------------
// Name: PGELoadSurfaceFromResouce
// Desc: 
//------------------------------------------------------------------------
HRESULT WINAPI PGELoadSurfaceFromResouce(   ISurface* pDestSurface,
										 CONST PALETTEENTRY* pDestPalette,
										 CONST RECT* pDestRect,
										 HMODULE hModule, UINT nID, LPCSTR pResType, 
										 CONST RECT* pSrcRect,
										 DWORD Filter,
										 D3DCOLOR ColorKey,
										 D3DXIMAGE_INFO* pSrcInfo)
{
	assert(hModule);
	DWORD filesize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, filesize);
	if (pSrcData == NULL)
		return NULL;

	HRESULT hr = pDestSurface->LoadFromFileInMemory( pDestPalette, pDestRect, pSrcData, filesize, pSrcRect, Filter, ColorKey, pSrcInfo );
	return hr;
}
HRESULT WINAPI PGELoadSurfaceFromResouce(   ISurface* pDestSurface,
										 CONST PALETTEENTRY* pDestPalette,
										 CONST RECT* pDestRect,
										 HMODULE hModule, UINT nID, LPCWSTR pResType, 
										 CONST RECT* pSrcRect,
										 DWORD Filter,
										 D3DCOLOR ColorKey,
										 D3DXIMAGE_INFO* pSrcInfo)
{
	assert(hModule);
	DWORD filesize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, filesize);
	if (pSrcData == NULL)
		return NULL;

	HRESULT hr = pDestSurface->LoadFromFileInMemory( pDestPalette, pDestRect, pSrcData, filesize, pSrcRect, Filter, ColorKey, pSrcInfo );
	return hr;
}


//------------------------------------------------------------------------
// Name: PGELoadSurfaceFromFile
// Desc: 
//------------------------------------------------------------------------
HRESULT WINAPI PGELoadSurfaceFromResouceA(  ISurface* pDestSurface,
											CONST PALETTEENTRY* pDestPalette,
											CONST RECT* pDestRect,
											LPCSTR pSrcFile,
											CONST RECT* pSrcRect,
											DWORD Filter,
											D3DCOLOR ColorKey,
											D3DXIMAGE_INFO* pSrcInfo)
{
	assert(pDestSurface);
	long filesize = 0;
	void* pSrcData = PGEReadFileA( pSrcFile, filesize );
	if (pSrcData == NULL)
		return E_FAIL;

	HRESULT hr = pDestSurface->LoadFromFileInMemory( pDestPalette, pDestRect, pSrcData, filesize, pSrcRect, Filter, ColorKey, pSrcInfo );

	delete []pSrcData;
	return hr;
}
HRESULT WINAPI PGELoadSurfaceFromResouceW(  ISurface* pDestSurface,
											CONST PALETTEENTRY* pDestPalette,
											CONST RECT* pDestRect,
											LPCWSTR pSrcFile,
											CONST RECT* pSrcRect,
											DWORD Filter,
											D3DCOLOR ColorKey,
											D3DXIMAGE_INFO* pSrcInfo)
{
	assert(pDestSurface);
	long filesize = 0;
	void* pSrcData = PGEReadFileW( pSrcFile, filesize );
	if (pSrcData == NULL)
		return E_FAIL;

	HRESULT hr = pDestSurface->LoadFromFileInMemory( pDestPalette, pDestRect, pSrcData, filesize, pSrcRect, Filter, ColorKey, pSrcInfo );

	delete []pSrcData;
	return hr;
}
//----------------------------------------------------------------------//
// Name: PGECompileEffect
// Desc: 
//----------------------------------------------------------------------//
HRESULT WINAPI PGECompileEffect(void* srcData, DWORD srcDataLen, DWORD Flags, LPD3DXBUFFER* ppCode, LPD3DXBUFFER *ppErrors )
{
	// 编译Effect
	LPD3DXBUFFER errors = NULL;
	ID3DXEffectCompiler* compiler = NULL;
	LPD3DXBUFFER pCode = NULL;

	HRESULT hr = S_OK;
	if (FAILED(hr = D3DXCreateEffectCompiler((const char*)srcData, srcDataLen, NULL, NULL, Flags, &compiler, &errors)))
	{	
		DXTRACE_ERRA((LPCSTR)errors->GetBufferPointer(), hr);
	}else
	{
		SAFE_RELEASE(errors);
		if (FAILED(hr = compiler->CompileEffect(Flags, &pCode, &errors)))
		{	
			assert(errors);
			DXTRACE_ERRA((LPCSTR)errors->GetBufferPointer(), hr);
		}
	}	

	SAFE_RELEASE(compiler);
	if (ppErrors) 
	{
		*ppErrors = errors;
	}else
	{
		SAFE_RELEASE(errors);
	}

	if (ppCode)
	{
		*ppCode = pCode;
	}else
	{
		SAFE_RELEASE( pCode );        
	}
	return hr;
}

//------------------------------------------------------------------------
// Name: PGECreateEffectFromFile
// Desc: 
//------------------------------------------------------------------------

IEffect* WINAPI PGECreateEffectFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName, DWORD shaderFlags, LPD3DXBUFFER* ppErrors)
{
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	IEffect* pEffect = NULL;
	HRESULT hr;
	if (SUCCEEDED(hr = PGECompileEffect( pSrcData, filesize, shaderFlags, &pEffectBuf, &pErrors)))
	{
		pEffect = pDevice->CreateEffect( pEffectBuf );
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}
	SAFE_RELEASE(pEffectBuf);

	if ( ppErrors )
		*ppErrors = pErrors;
	else
		SAFE_RELEASE(pErrors);

	delete []pSrcData;
	return pEffect;
}

IEffect* WINAPI PGECreateEffectFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName, DWORD shaderFlags, LPD3DXBUFFER* ppErrors)
{
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	IEffect* pEffect = NULL;
	HRESULT hr;
	if (SUCCEEDED(hr = PGECompileEffect( pSrcData, filesize, shaderFlags, &pEffectBuf, &pErrors)))
	{
		pEffect = pDevice->CreateEffect( pEffectBuf );
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}
	SAFE_RELEASE(pEffectBuf);

	if ( ppErrors )
		*ppErrors = pErrors;
	else
		SAFE_RELEASE(pErrors);

	delete []pSrcData;
	return pEffect;
}

//------------------------------------------------------------------------
// Name: PGECreateEffectFromFileBin
// Desc: 
//------------------------------------------------------------------------

IEffect* WINAPI PGECreateEffectFromFileBinA(IGraphicDevice* pDevice, LPCSTR pFileName)
{
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileA( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(filesize, &pCode)) )
	{
		assert(false);
		delete []pSrcData;
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, filesize);
	delete []pSrcData;

	IEffect* pEffect = pDevice->CreateEffect( pCode );
	pCode->Release();
	return pEffect;
}

IEffect* WINAPI PGECreateEffectFromFileBinW(IGraphicDevice* pDevice, LPCWSTR pFileName)
{
	assert( pDevice && pFileName );

	long filesize = 0;
	void* pSrcData = PGEReadFileW( pFileName, filesize );
	if (pSrcData == NULL)
		return NULL;

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(filesize, &pCode)) )
	{
		assert(false);
		delete []pSrcData;
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, filesize);
	delete []pSrcData;

	IEffect* pEffect = pDevice->CreateEffect( pCode );
	pCode->Release();
	return pEffect;
}

//--------------------------------------------------------------------
// Name: PGECreateEffectFromResource
// Desc: 从资源中创建Effect
//-------------------------------------------------------------------
IEffect* WINAPI PGECreateEffectFromResourceA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, DWORD shaderFlags, LPD3DXBUFFER* ppErrors )
{
	assert( pDevice && hModule );

	if ( ppErrors )
		*ppErrors = NULL;

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, nSize);
	if ( pSrcData == NULL  )
	{
		char str[512];
		sprintf(str, "Load resource( id = %d Type = %s Module = 0x%x ) failed!", nID, pResType, (DWORD)hModule );
		OutputDebugStringA( str );
		return NULL;
	}

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	IEffect* pEffect = NULL;
	HRESULT hr;
	if (SUCCEEDED(hr = PGECompileEffect( pSrcData, nSize, shaderFlags, &pEffectBuf, &pErrors)))
	{
		pEffect = pDevice->CreateEffect( pEffectBuf );
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}
	SAFE_RELEASE(pEffectBuf);

	if ( ppErrors )
		*ppErrors = pErrors;
	else
		SAFE_RELEASE(pErrors);

	return pEffect;
}

IEffect* WINAPI PGECreateEffectFromResourceW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, DWORD shaderFlags, LPD3DXBUFFER* ppErrors )
{
	assert( pDevice && hModule );

	if ( ppErrors )
		*ppErrors = NULL;

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, nSize);
	if ( pSrcData == NULL  )
	{
		wchar_t str[512];
		swprintf(str, L"Load resource( id = %d Type = %s Module = 0x%x ) failed!", nID, pResType, (DWORD)hModule );
		OutputDebugStringW( str );

		return NULL;
	}

	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pEffectBuf = NULL;
	IEffect* pEffect = NULL;
	HRESULT hr;
	if (SUCCEEDED(hr = PGECompileEffect( pSrcData, nSize, shaderFlags, &pEffectBuf, &pErrors)))
	{
		pEffect = pDevice->CreateEffect( pEffectBuf );
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}
	SAFE_RELEASE(pEffectBuf);

	if ( ppErrors )
		*ppErrors = pErrors;
	else
		SAFE_RELEASE(pErrors);

	return pEffect;
}


//--------------------------------------------------------------------
// Name: PGECreateEffectFromResourceBinA
// Desc: 从资源中创建Effect
//-------------------------------------------------------------------
IEffect* WINAPI PGECreateEffectFromResourceBinA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType)
{
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, nSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(nSize, &pCode)) )
	{
		assert(false);
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, nSize);

	IEffect* pEffect = pDevice->CreateEffect( pCode );
	pCode->Release();

	return pEffect;
}
IEffect* WINAPI PGECreateEffectFromResourceBinW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType)
{
	assert( pDevice && hModule );

	DWORD nSize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, nSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(nSize, &pCode)) )
	{
		assert(false);
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, nSize);

	IEffect* pEffect = pDevice->CreateEffect( pCode );
	pCode->Release();

	return pEffect;
}

//--------------------------------------------------------------------
// Name: PGECreateVertexShaderFromResource
// Desc: 从资源中创建VertexShader
//-------------------------------------------------------------------
IVertexShader* WINAPI PGECreateVertexShaderFromResourceA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, LPCSTR funcName, LPCSTR pProfile, DWORD shaderFlags, LPD3DXBUFFER* ppErrors, LPD3DXCONSTANTTABLE* ppConstantTable )
{
	assert( pDevice && hModule );

	if ( ppErrors )
		*ppErrors = NULL;

	if ( ppConstantTable )
		*ppConstantTable = NULL;

	DWORD nResSize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, nResSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode = NULL;
	LPD3DXBUFFER pErrors = NULL;
	IVertexShader* pVS = NULL;
	HRESULT hr;
	if ( SUCCEEDED( hr = D3DXCompileShader( (LPCSTR)pSrcData, nResSize, 
		NULL, NULL,
		funcName, pProfile, 
		shaderFlags, &pCode, 
		&pErrors, ppConstantTable)))
	{
		pVS = pDevice->CreateVertexShader( pCode );
		pCode->Release();
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}

	if ( ppErrors )
		*ppErrors = pErrors;

	return pVS;
}
IVertexShader* WINAPI PGECreateVertexShaderFromResourceW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, LPCSTR funcName, LPCSTR pProfile, DWORD shaderFlags, LPD3DXBUFFER* ppErrors, LPD3DXCONSTANTTABLE* ppConstantTable )
{
	assert( pDevice && hModule );

	if ( ppErrors )
		*ppErrors = NULL;

	if ( ppConstantTable )
		*ppConstantTable = NULL;

	DWORD nResSize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, nResSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode = NULL;
	LPD3DXBUFFER pErrors = NULL;
	IVertexShader* pVS = NULL;
	HRESULT hr;
	if ( SUCCEEDED( hr = D3DXCompileShader( (LPCSTR)pSrcData, nResSize, 
		NULL, NULL,
		funcName, pProfile, 
		shaderFlags, &pCode, 
		&pErrors, ppConstantTable)))
	{
		pVS = pDevice->CreateVertexShader( pCode );
		pCode->Release();
	}else
	{
		assert(pErrors);
		DXTRACE_ERRA((LPCSTR)pErrors->GetBufferPointer(), hr);
	}

	if ( ppErrors )
		*ppErrors = pErrors;

	return pVS;
}
IVertexShader* WINAPI PGECreateVertexShaderFromResourceBinA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType )
{
	assert( pDevice && hModule );
	DWORD nResSize = 0;
	void* pSrcData = PGELoadResourceA( hModule, nID, pResType, nResSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(nResSize, &pCode)) )
	{
		assert(false);
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, nResSize);
	IVertexShader* pVS = pDevice->CreateVertexShader( pCode );
	pCode->Release();

	return pVS;
}
IVertexShader* WINAPI PGECreateVertexShaderFromResourceBinW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType )
{
	assert( pDevice && hModule );
	DWORD nResSize = 0;
	void* pSrcData = PGELoadResourceW( hModule, nID, pResType, nResSize);
	if ( pSrcData == NULL  )
	{
		return NULL;
	}

	LPD3DXBUFFER pCode;
	if ( FAILED(D3DXCreateBuffer(nResSize, &pCode)) )
	{
		assert(false);
		return NULL;
	}
	memcpy(pCode->GetBufferPointer(), pSrcData, nResSize);
	IVertexShader* pVS = pDevice->CreateVertexShader( pCode );
	pCode->Release();

	return pVS;
}

DEVICE_EXPORT BOOL PGEParsePMFFileInMemory( PMFMeshData& meshData, void* pFileData, DWORD DataLength )
{
	//-------------------------------------------------------------//
	const DWORD	PMF_MAGIC = MAKEFOURCC('P', 'M', 'F', '\0');
	const DWORD	PMF_VERSION = 200;
	//-------------------------------------------------------------//
	assert( pFileData );

	PMF_MESH_HEAD FileHead;

	BYTE* p = (BYTE*)pFileData;

	// Read file head size
	DWORD FileHeadSize = *( (DWORD*)p );	
	FileHeadSize = __min(FileHeadSize, sizeof(FileHead));
	p += sizeof(DWORD);

	// Read file head	
	memset(&FileHead, 0, sizeof(FileHead) );
	memcpy(&FileHead, p, FileHeadSize );
	p += FileHeadSize;

	if ( FileHead.Magic != PMF_MAGIC || FileHead.Version != PMF_VERSION )
	{
		return FALSE;
	}

	DWORD NumVertices = FileHead.NumVertices;
	meshData.NumVertices = FileHead.NumVertices;
	meshData.NumFaces	 = FileHead.NumFaces;
	meshData.NumTCs		 = FileHead.NumTCs;
	meshData.AttrTableSize = FileHead.AttrTableSize;

	// Read attribute table
	if ( meshData.AttrTableSize > 0 )
	{
		meshData.pAttrTable = (PGE_ATTRIBUTE_RANGE*)p;
		p += sizeof(PGE_ATTRIBUTE_RANGE) * meshData.AttrTableSize;
	}

	if ( FileHead.bPosition )
	{
		meshData.pPositions = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bBlendWeight  )
	{
		meshData.pBlendWeights = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bBlendIndex )
	{
		meshData.pBlendIndices = (WORD*)p;
		p += sizeof(WORD) * NumVertices * 4;
	}

	if ( FileHead.bNormal )
	{
		meshData.pNormals = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bTangent  )
	{
		meshData.pTangents = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bWeight )
	{
		meshData.pWeights = (float*)p;
		p += sizeof(FLOAT) * NumVertices;
	}

	if ( FileHead.bDiffuses )
	{
		meshData.pDiffuses = (DWORD*)p;
		p += sizeof(DWORD) * NumVertices;
	}

	if ( FileHead.bSpecular )
	{
		meshData.pSpeculars = (DWORD*)p;
		p += sizeof(DWORD) * NumVertices;
	}

	if ( FileHead.NumTCs > 0 )
	{
		meshData.pTC0s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 1  )
	{
		meshData.pTC1s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 2  )
	{
		meshData.pTC2s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 3  ) 
	{
		meshData.pTC3s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}

	// Read face indices
	meshData.pFaceIndices = (WORD*)p;
	p += sizeof(WORD) * 3 * meshData.NumFaces;

	assert( p - (BYTE*)pFileData == DataLength );
	return TRUE;
}

//--------------------------------------------------------------------
// Name: PGECreateModelFromPMFModelData
// Desc:
//-------------------------------------------------------------------
DEVICE_EXPORT IPGEMesh*  WINAPI PGECreateMeshFromPMFMeshData( IGraphicDevice* pDevice, DWORD dwOptions, const PMFMeshData& meshData, IPGESkinInfo** ppSkinInfo )
{
	if ( meshData.NumFaces == 0 || meshData.NumVertices == 0 || meshData.pFaceIndices == NULL )
		return NULL;

	DWORD dwNumVertices = meshData.NumVertices;
	IPGESkinInfo* pSkinInfo = NULL;
	DWORD* pNewBlendIndices = NULL;

	D3DVERTEXELEMENT9 decls[MAXD3DDECLLENGTH+1];
	DWORD dwNumElements = 0;
	WORD Offset = 0;
	if ( meshData.pPositions )
	{
		D3DVERTEXELEMENT9 pos_decl = {0, Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0};
		decls[dwNumElements++] = pos_decl;	
		Offset += sizeof(D3DXVECTOR3);
	}

	if ( meshData.pBlendWeights ) 
	{
		if ( ppSkinInfo )
		{
			pSkinInfo = PGECreateSkinInfo();
			*ppSkinInfo = pSkinInfo;
		}
		D3DVERTEXELEMENT9 blendweight_decl = {0, Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0};
		decls[dwNumElements++] = blendweight_decl;	
		Offset += sizeof(D3DXVECTOR3);
	}

	if ( meshData.pBlendIndices )
	{
		D3DVERTEXELEMENT9 blendIndices_decl = {0, Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0};
		decls[dwNumElements++] = blendIndices_decl;
		Offset += sizeof(DWORD);
	}

	if ( meshData.pNormals )
	{
		D3DVERTEXELEMENT9 normal_decl = {0, Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0};
		Offset += sizeof(D3DXVECTOR3);
		decls[dwNumElements++] = normal_decl;
	}

	if ( meshData.pTangents )
	{
		D3DVERTEXELEMENT9 tangent_decl = {0, Offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0};
		Offset += sizeof(D3DXVECTOR3);
		decls[dwNumElements++] = tangent_decl;
	}

	if ( meshData.pWeights )
	{
		D3DVERTEXELEMENT9 weigth_decl = {0, Offset, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE,  0};
		Offset += sizeof(float);
		decls[dwNumElements++] = weigth_decl;
	}

	if ( meshData.pTC0s )
	{
		D3DVERTEXELEMENT9 tc0_decl = {0, Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0};
		Offset += sizeof(D3DXVECTOR2);
		decls[dwNumElements++] = tc0_decl;
	}

	if ( meshData.pTC1s )
	{
		D3DVERTEXELEMENT9 tc0_decl = {0, Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1};
		Offset += sizeof(D3DXVECTOR2);
		decls[dwNumElements++] = tc0_decl;
	}

	if ( meshData.pTC2s )
	{
		D3DVERTEXELEMENT9 tc0_decl = {0, Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2};
		Offset += sizeof(D3DXVECTOR2);
		decls[dwNumElements++] = tc0_decl;
	}

	if ( meshData.pTC3s )
	{
		D3DVERTEXELEMENT9 tc0_decl = {0, Offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3};
		Offset += sizeof(D3DXVECTOR2);
		decls[dwNumElements++] = tc0_decl;
	}

	D3DVERTEXELEMENT9 decl_end = D3DDECL_END();
	decls[dwNumElements++] = decl_end;

	IPGEMesh* pMesh = PGECreateMesh( pDevice,  meshData.NumVertices, meshData.NumFaces, decls, dwOptions);
	if ( pMesh == NULL )
		return NULL;

	void* pVerts = pMesh->GetVertexBuffer()->Lock(0, 0, 0);
	DWORD dwVertexStride = pMesh->GetNumBytesPerVertex();

	Offset = 0;
	if ( pSkinInfo )
	{
		pNewBlendIndices = new DWORD[meshData.NumVertices];

		// Remap blend indices
		pSkinInfo->RemapBlendIndices( meshData.pBlendIndices, (BYTE*)pNewBlendIndices, meshData.NumVertices );
	}

	// Fill vertex buffer
	if ( meshData.pPositions )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR3* pDest = (D3DXVECTOR3*)( (BYTE*)pVerts + i * dwVertexStride ) ;
			*pDest = meshData.pPositions[i];
		}	
		Offset = sizeof(D3DXVECTOR3);
	}
	if ( meshData.pBlendWeights )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR3* pDest = (D3DXVECTOR3*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pBlendWeights[i];
		}
		Offset += sizeof(D3DXVECTOR3);
	}
	if ( meshData.pBlendIndices )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			DWORD* pDest = (DWORD*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = pNewBlendIndices[i];
		}
		Offset += sizeof(DWORD);
	}
	if ( meshData.pNormals )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR3* pDest = (D3DXVECTOR3*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pNormals[i];
		}
		Offset += sizeof(D3DXVECTOR3);
	}	
	if ( meshData.pWeights )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i )
		{
			float* pDest = (float*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pWeights[i];
		}
		Offset += sizeof(float);
	}
	if ( meshData.pTC0s )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR2* pDest = (D3DXVECTOR2*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pTC0s[i];
		}
		Offset += sizeof(D3DXVECTOR2);
	} 
	if ( meshData.pTC1s )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR2* pDest = (D3DXVECTOR2*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pTC1s[i];
		}
		Offset += sizeof(D3DXVECTOR2);
	}
	if ( meshData.pTC2s )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR2* pDest = (D3DXVECTOR2*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pTC2s[i];
		}
		Offset += sizeof(D3DXVECTOR2);
	}
	if ( meshData.pTC3s )
	{
		for ( DWORD i = 0; i < dwNumVertices; ++i)
		{
			D3DXVECTOR2* pDest = (D3DXVECTOR2*)( (BYTE*)pVerts + i * dwVertexStride + Offset );
			*pDest = meshData.pTC3s[i];
		}
		Offset += sizeof(D3DXVECTOR2);
	}
	pMesh->GetVertexBuffer()->Unlock();

	// Fill index buffer
	void* pIndices = pMesh->GetIndexBuffer()->Lock(0, 0, 0);
	memcpy( pIndices, meshData.pFaceIndices, sizeof(WORD) * meshData.NumFaces * 3);
	pMesh->GetIndexBuffer()->Unlock();

	// Set Attribute table
	pMesh->SetAttributeTable( meshData.pAttrTable, meshData.AttrTableSize);

	// Delete temporary memory
	if ( pNewBlendIndices )
		delete []pNewBlendIndices;

	return pMesh;
}
