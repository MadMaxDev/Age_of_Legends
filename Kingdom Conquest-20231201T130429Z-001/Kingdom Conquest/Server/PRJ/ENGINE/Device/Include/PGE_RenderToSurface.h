#pragma once
#include "PGE_Config.h"


//-------------------------------------------------------------//
// 类名: IPGERenderToSurface
// 说明: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGERenderToSurface
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 开始渲染场景
	// pSurface: 渲染目标表面
	// pViewport: 场景表面视口(如果为空，则视口为整个表面)
	virtual HRESULT BeginScene( ISurface* pSurface, const D3DVIEWPORT9 *pViewport) = 0;

	// 结束渲染场景
	virtual HRESULT EndScene() = 0;

	// 设置新的rt
	virtual void SetRenderTarget( ISurface* pSurface, const D3DVIEWPORT9 *pViewport ) = 0;

	// 恢复原来的rt
	virtual void RestoreRenderTarget() = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateRenderToSurface
// Desc : 创建表面渲染器
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToSurface* WINAPI PGECreateRenderToSurface(  IGraphicDevice* pDevice
																	, DWORD Width
																	, DWORD Height
																	, BOOL DepthStencil
																	, D3DFORMAT DepthStencilFormat);