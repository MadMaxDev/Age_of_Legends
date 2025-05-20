#pragma once
#include "PGE_Config.h"


//-------------------------------------------------------------//
// ����: IPGERenderToSurface
// ˵��: 
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGERenderToSurface
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// ��ʼ��Ⱦ����
	// pSurface: ��ȾĿ�����
	// pViewport: ���������ӿ�(���Ϊ�գ����ӿ�Ϊ��������)
	virtual HRESULT BeginScene( ISurface* pSurface, const D3DVIEWPORT9 *pViewport) = 0;

	// ������Ⱦ����
	virtual HRESULT EndScene() = 0;

	// �����µ�rt
	virtual void SetRenderTarget( ISurface* pSurface, const D3DVIEWPORT9 *pViewport ) = 0;

	// �ָ�ԭ����rt
	virtual void RestoreRenderTarget() = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateRenderToSurface
// Desc : ����������Ⱦ��
//--------------------------------------------------------------------
DEVICE_EXPORT IPGERenderToSurface* WINAPI PGECreateRenderToSurface(  IGraphicDevice* pDevice
																	, DWORD Width
																	, DWORD Height
																	, BOOL DepthStencil
																	, D3DFORMAT DepthStencilFormat);