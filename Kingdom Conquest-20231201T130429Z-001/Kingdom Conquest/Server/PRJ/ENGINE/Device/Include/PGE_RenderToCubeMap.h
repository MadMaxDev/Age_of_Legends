#pragma once
#include "PGE_Config.h"


//-------------------------------------------------------------//
// ����: IPGERenderToCubeMap
// ˵��: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGERenderToCubeMap
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// ��ʼ��Ⱦ��������ͼ
	virtual HRESULT Begin( ICubeTexture* pCubeTex ) = 0;

	// ������Ⱦ
	virtual HRESULT End() = 0;

	// ��Ⱦ����������ͼ��ĳ��������
	// iFace: ����������������
	virtual HRESULT Face( D3DCUBEMAP_FACES iFace ) = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateRenderToCubeMap
// Desc : ����������������Ⱦ��
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToCubeMap* WINAPI PGECreateRenderToCubeMap(  IGraphicDevice* pDevice
																    , DWORD Size
																    , BOOL  DepthStencil
																    , D3DFORMAT DepthStencilFormat);
