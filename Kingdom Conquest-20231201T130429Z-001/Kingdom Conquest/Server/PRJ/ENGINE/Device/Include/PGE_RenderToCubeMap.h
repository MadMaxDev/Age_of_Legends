#pragma once
#include "PGE_Config.h"


//-------------------------------------------------------------//
// 类名: IPGERenderToCubeMap
// 说明: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGERenderToCubeMap
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 开始渲染立方体贴图
	virtual HRESULT Begin( ICubeTexture* pCubeTex ) = 0;

	// 结束渲染
	virtual HRESULT End() = 0;

	// 渲染到立方体贴图的某个表面上
	// iFace: 是立方体表面的类型
	virtual HRESULT Face( D3DCUBEMAP_FACES iFace ) = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateRenderToCubeMap
// Desc : 创建立方体纹理渲染器
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToCubeMap* WINAPI PGECreateRenderToCubeMap(  IGraphicDevice* pDevice
																    , DWORD Size
																    , BOOL  DepthStencil
																    , D3DFORMAT DepthStencilFormat);
