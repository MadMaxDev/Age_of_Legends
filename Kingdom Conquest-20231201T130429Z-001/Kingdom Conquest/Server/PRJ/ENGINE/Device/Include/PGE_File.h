#pragma once 

#include "PGE_Config.h"

class IPGEMesh;
class IPGESkinInfo;
struct PGE_ATTRIBUTE_RANGE;

//-------------------------------------------------------------//
#pragma pack(push, 4)

struct PMF_MESH_HEAD
{
	DWORD Magic;
	DWORD Version;
	DWORD bPosition		: 1;
	DWORD bBlendWeight	: 1;
	DWORD bBlendIndex	: 1;	
	DWORD bWeight		: 1;	
	DWORD bNormal		: 1;	
	DWORD bTangent		: 1;
	DWORD bDiffuses		: 1;
	DWORD bSpecular		: 1;
	DWORD bUnused		: 24;
	DWORD NumVertices;
	DWORD NumFaces;
	DWORD NumTCs;
	DWORD AttrTableSize;
};

struct PMFMeshData
{
	PMFMeshData()
	{
		pAttrTable		 = NULL;
		pPositions		 = NULL;
		pBlendWeights	 = NULL;
		pBlendIndices	 = NULL;
		pWeights		 = NULL;
		pNormals		 = NULL;
		pTangents		 = NULL;
		pDiffuses		 = NULL;
		pSpeculars		 = NULL;
		pTC0s			 = NULL;
		pTC1s			 = NULL;
		pTC2s			 = NULL;
		pTC3s			 = NULL;
		pFaceIndices	 = NULL;
		NumFaces		 = 0;
		NumVertices		 = 0;
		AttrTableSize	 = 0;
		NumTCs			 = 0;
	}
	DWORD				 AttrTableSize;	
	DWORD				 NumFaces;
	DWORD				 NumVertices;
	DWORD				 NumTCs;

	PGE_ATTRIBUTE_RANGE* pAttrTable;
	D3DXVECTOR3*		 pPositions;
	D3DXVECTOR3*		 pBlendWeights;
	WORD*				 pBlendIndices;
	FLOAT*				 pWeights;
	D3DXVECTOR3*		 pNormals;
	D3DXVECTOR3*		 pTangents;
	D3DCOLOR*			 pDiffuses;
	D3DCOLOR*			 pSpeculars;
	D3DXVECTOR2*		 pTC0s;
	D3DXVECTOR2*		 pTC1s;
	D3DXVECTOR2*		 pTC2s;
	D3DXVECTOR2*		 pTC3s;
	WORD*				 pFaceIndices;
};

#pragma pack(pop)


//-------------------------------------------------------------//


//--------------------------------------------------------------------
// Name: PGECreateTextureFromFile
// Desc: 从文件中创建纹理
//-------------------------------------------------------------------
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName);
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName);

DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromResourceA( IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType );
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromResourceW( IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType );

//--------------------------------------------------------------------
// Name: PGECreateTextureFromFileEx
// Desc: 从文件中创建纹理
//-------------------------------------------------------------------
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromFileExA(IGraphicDevice* pDevice, LPCSTR pFileName, 
															UINT Width, UINT Height,
															UINT MipLevels, DWORD Usage,
															D3DFORMAT Format, DWORD Filter,
															DWORD MipFilter, DWORD ColorKey, D3DXIMAGE_INFO* pSrcInfo );
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromFileExW(IGraphicDevice* pDevice, LPCWSTR pFileName, 
														   UINT Width, UINT Height,
														   UINT MipLevels, DWORD Usage,
														   D3DFORMAT Format, DWORD Filter,
														   DWORD MipFilter, DWORD ColorKey, D3DXIMAGE_INFO* pSrcInfo );

DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromResourceExA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, 
																UINT Width, UINT Height,
																UINT MipLevels, DWORD Usage,
																D3DFORMAT Format, DWORD Filter,
																DWORD MipFilter, DWORD ColorKey, D3DXIMAGE_INFO* pSrcInfo );
DEVICE_EXPORT ITexture* WINAPI PGECreateTextureFromResourceExW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, 
															  UINT Width, UINT Height,
															  UINT MipLevels, DWORD Usage,
															  D3DFORMAT Format, DWORD Filter,
															  DWORD MipFilter, DWORD ColorKey, D3DXIMAGE_INFO* pSrcInfo );
//--------------------------------------------------------------------
// Name: PGECreateCubeTextureFromFile
// Desc: 从文件中创建立方体纹理
//-------------------------------------------------------------------
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName);
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName);

DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromResourceA( IGraphicDevice* pDevice,  HMODULE hModule, UINT nID, LPCSTR pResType );
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromResourceW( IGraphicDevice* pDevice,  HMODULE hModule, UINT nID, LPCWSTR pResType );

//--------------------------------------------------------------------
// Name: PGECreateCubeTextureFromFileEx
// Desc: 从文件中创建立方体纹理
//-------------------------------------------------------------------
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromFileExA(IGraphicDevice* pDevice, LPCSTR pFileName, 
																	UINT Size,UINT MipLevels, DWORD Usage,
																	D3DFORMAT Format, DWORD Filter,
																	DWORD MipFilter, D3DXIMAGE_INFO* pSrcInfo );

DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromFileExW(IGraphicDevice* pDevice, LPCWSTR pFileName, 
																  UINT Size,UINT MipLevels, DWORD Usage,
																  D3DFORMAT Format, DWORD Filter,
																  DWORD MipFilter, D3DXIMAGE_INFO* pSrcInfo );

DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromResourceExA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, 
																		UINT Size,UINT MipLevels, DWORD Usage,
																		D3DFORMAT Format, DWORD Filter,
																		DWORD MipFilter, D3DXIMAGE_INFO* pSrcInfo );
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateCubeTextureFromResourceExW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, 
																	  UINT Size,UINT MipLevels, DWORD Usage,
																	  D3DFORMAT Format, DWORD Filter,
																	  DWORD MipFilter, D3DXIMAGE_INFO* pSrcInfo );

//--------------------------------------------------------------------
// Name: PGELoadSurfaceFromFile
// Desc: 从文件中加载表面
//-------------------------------------------------------------------
DEVICE_EXPORT HRESULT WINAPI PGELoadSurfaceFromFileA(   ISurface* pDestSurface,
														CONST PALETTEENTRY* pDestPalette,
														CONST RECT* pDestRect,
														LPCSTR pSrcFile,
														CONST RECT* pSrcRect,
														DWORD Filter,
														D3DCOLOR ColorKey,
														D3DXIMAGE_INFO* pSrcInfo);
DEVICE_EXPORT HRESULT WINAPI PGELoadSurfaceFromFileW(   ISurface* pDestSurface,
														CONST PALETTEENTRY* pDestPalette,
														CONST RECT* pDestRect,
														LPCWSTR pSrcFile,
														CONST RECT* pSrcRect,
														DWORD Filter,
														D3DCOLOR ColorKey,
														D3DXIMAGE_INFO* pSrcInfo);
DEVICE_EXPORT HRESULT WINAPI PGELoadSurfaceFromResourceA(   ISurface* pDestSurface,
															CONST PALETTEENTRY* pDestPalette,
															CONST RECT* pDestRect,
															HMODULE hModule,
															UINT nID,
															LPCSTR pResType,
															CONST RECT* pSrcRect,
															DWORD Filter,
															D3DCOLOR ColorKey,
															D3DXIMAGE_INFO* pSrcInfo);
DEVICE_EXPORT HRESULT WINAPI PGELoadSurfaceFromResourceW(   ISurface* pDestSurface,
															CONST PALETTEENTRY* pDestPalette,
															CONST RECT* pDestRect,
															HMODULE hModule,
															UINT nID,
															LPCWSTR pResType,
															CONST RECT* pSrcRect,
															DWORD Filter,
															D3DCOLOR ColorKey,
															D3DXIMAGE_INFO* pSrcInfo);
//----------------------------------------------------------------------//
// Name: PGECompileEffect
// Desc: 
//----------------------------------------------------------------------//
DEVICE_EXPORT HRESULT WINAPI PGECompileEffect(void* srcData, DWORD srcDataLen, DWORD Flags, LPD3DXBUFFER* ppCode, LPD3DXBUFFER *ppErrors );

//--------------------------------------------------------------------
// Name: PGECreateEffectFromFile
// Desc: 从文件中创建Effect
//-------------------------------------------------------------------
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromFileA(IGraphicDevice* pDevice, LPCSTR pFileName, DWORD shaderFlags,  LPD3DXBUFFER* pErrors);
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromFileW(IGraphicDevice* pDevice, LPCWSTR pFileName, DWORD shaderFlags,  LPD3DXBUFFER* pErrors);

DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromFileBinA(IGraphicDevice* pDevice, LPCSTR pFileName);
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromFileBinW(IGraphicDevice* pDevice, LPCWSTR pFileName);

//--------------------------------------------------------------------
// Name: PGECreateEffectFromResource
// Desc: 从资源中创建Effect
//-------------------------------------------------------------------
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromResourceA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, DWORD shaderFlags, LPD3DXBUFFER* pErrors );
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromResourceW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, DWORD shaderFlags, LPD3DXBUFFER* pErrors );

DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromResourceBinA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType);
DEVICE_EXPORT IEffect* WINAPI PGECreateEffectFromResourceBinW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType);

//--------------------------------------------------------------------
// Name: PGECreateVertexShaderFromResource
// Desc: 从资源中创建VertexShader
//-------------------------------------------------------------------
DEVICE_EXPORT IVertexShader* WINAPI PGECreateVertexShaderFromResourceA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType, LPCSTR funcName, LPCSTR pProfile, DWORD shaderFlags, LPD3DXBUFFER* pErrors, LPD3DXCONSTANTTABLE* pConstantTable);
DEVICE_EXPORT IVertexShader* WINAPI PGECreateVertexShaderFromResourceW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType, LPCSTR funcName, LPCSTR pProfile, DWORD shaderFlags, LPD3DXBUFFER* pErrors, LPD3DXCONSTANTTABLE* pConstantTable);

DEVICE_EXPORT IVertexShader* WINAPI PGECreateVertexShaderFromResourceBinA(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCSTR pResType );
DEVICE_EXPORT IVertexShader* WINAPI PGECreateVertexShaderFromResourceBinW(IGraphicDevice* pDevice, HMODULE hModule, UINT nID, LPCWSTR pResType );


//--------------------------------------------------------------------
// Name: PGECreateModelFromPMFModelData
// Desc:
//-------------------------------------------------------------------
DEVICE_EXPORT BOOL PGEParsePMFFileInMemory( PMFMeshData& meshData, void* pFileData , DWORD DataLength );
DEVICE_EXPORT IPGEMesh*  WINAPI PGECreateMeshFromPMFMeshData( IGraphicDevice* pDevice, DWORD dwOptions, const PMFMeshData& modelData, IPGESkinInfo** ppSkinInfo );

#ifdef _UNICODE
	#define PGECreateTextureFromFile			PGECreateTextureFromFileW
	#define PGECreateTextureFromFileEx			PGECreateTextureFromFileExW

	#define PGECreateTextureFromResource		PGECreateTextureFromResourceW
	#define PGECreateTextureFromResourceEx		PGECreateTextureFromResourceExW

	#define PGECreateCubeTextureFromFile		PGECreateCubeTextureFromFileW
	#define PGECreateCubeTextureFromResource	PGECreateCubeTextureFromResourceW

	#define PGECreateCubeTextureFromFileEx	    PGECreateCubeTextureFromFileExW
	#define PGECreateCubeTextureFromResourceEx  PGECreateCubeTextureFromResourceExW

	#define PGELoadSurfaceFromFile				PGELoadSurfaceFromFileW
	#define PGELoadSurfaceFromResource			PGELoadSurfaceFromResourceW

	#define PGECreateEffectFromFile				PGECreateEffectFromFileW
	#define PGECreateEffectFromFileBin			PGECreateEffectFromFileBinW

	#define PGECreateEffectFromResource			PGECreateEffectFromResourceW
	#define PGECreateEffectFromResourceBin		PGECreateEffectFromResourceBinW

	#define PGECreateVertexShaderFromResource	 PGECreateVertexShaderFromResourceW
	#define PGECreateVertexShaderFromResourceBin PGECreateVertexShaderFromResourceBinW
#else
	#define PGECreateTextureFromFile			PGECreateTextureFromFileA
	#define PGECreateTextureFromFileEx			PGECreateTextureFromFileExA

	#define PGECreateTextureFromResource		PGECreateTextureFromResourceA
	#define PGECreateTextureFromResourceEx		PGECreateTextureFromResourceExA

	#define PGECreateCubeTextureFromFile		PGECreateCubeTextureFromFileA
	#define PGECreateCubeTextureFromResource	PGECreateCubeTextureFromResourceA

	#define PGECreateCubeTextureFromFileEx	    PGECreateCubeTextureFromFileExA
	#define PGECreateCubeTextureFromResourceEx  PGECreateCubeTextureFromResourceExA

	#define PGELoadSurfaceFromFile				PGELoadSurfaceFromFileA
	#define PGELoadSurfaceFromResource			PGELoadSurfaceFromResourceA

	#define PGECreateEffectFromFile				PGECreateEffectFromFileW
	#define PGECreateEffectFromFileBin			PGECreateEffectFromFileBinA

	#define PGECreateEffectFromResource			PGECreateEffectFromResourceA
	#define PGECreateEffectFromResourceBin		PGECreateEffectFromResourceBinA

	#define PGECreateVertexShaderFromResource	 PGECreateVertexShaderFromResourceA
	#define PGECreateVertexShaderFromResourceBin PGECreateVertexShaderFromResourceBinA
#endif


