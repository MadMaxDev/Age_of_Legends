#pragma once

#include "PGE_Config.h"

//-------------------------------------------------------------//
// 类名: IPGESprite
// 说明: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGESprite
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 绘制一个2D贴图
	virtual HRESULT Draw(
		  ITexture* pTexture
		, bool _UsingAlpha
		, bool _UsingColor
		, const RECT* pSrcRect
		, const D3DXVECTOR3* pPosition
		, const D3DXVECTOR2* pScaling
		, DWORD Color) = 0;

	virtual HRESULT Render() = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateSprite
// Desc : 创建2D精灵
//--------------------------------------------------------------------
DEVICE_EXPORT IPGESprite* WINAPI PGECreateSprite(IGraphicDevice* pDevice);