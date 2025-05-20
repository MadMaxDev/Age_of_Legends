#pragma once

#include "PGE_Config.h"

//-------------------------------------------------------------//
// ����: IPGESprite
// ˵��: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGESprite
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// ����һ��2D��ͼ
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
// Desc : ����2D����
//--------------------------------------------------------------------
DEVICE_EXPORT IPGESprite* WINAPI PGECreateSprite(IGraphicDevice* pDevice);