#pragma once
#include "PGE_Config.h"

//--------------------------------------------------------------------
// Name: PGEVectortoRGBA
// Desc: 
//--------------------------------------------------------------------
inline DWORD PGEVectortoRGBA( D3DXVECTOR3* v)
{
	DWORD r = (DWORD)( 127.0f * v->x + 128.0f );
	DWORD g = (DWORD)( 127.0f * v->y + 128.0f );
	DWORD b = (DWORD)( 127.0f * v->z + 128.0f );
	DWORD a = 255;

	return( (a<<24L) + (r<<16L) + (g<<8L) + (b<<0L) );
}

//--------------------------------------------------------------------
// Name: PGEGetPixelBits
// Desc: 
//--------------------------------------------------------------------
DEVICE_EXPORT DWORD WINAPI PGEGetPixelBits(D3DFORMAT fmt);

//--------------------------------------------------------------------
// Name: PGEMakeFresnelMap
// Desc: ����Fresnel��ͼ
//--------------------------------------------------------------------
DEVICE_EXPORT ITexture* WINAPI PGECreateFresnelMap(IGraphicDevice* pDevice, DWORD size);

//--------------------------------------------------------------------
// Name: PGEComputeNormalMap
// Desc: ���㷨��ͼ
//--------------------------------------------------------------------
DEVICE_EXPORT ITexture* WINAPI PGEComputeNormalMap(IGraphicDevice* pDeivce, ITexture* pHeightTex, DWORD channel, float bumpiness);

//--------------------------------------------------------------------
// Name: PGEComputeUVOffsetMap
// Desc: ���㷨��ͼ(��[U, V, 1]�ĸ�ʽ�洢)
//--------------------------------------------------------------------
DEVICE_EXPORT ITexture* WINAPI PGEComputeUVOffsetMap(IGraphicDevice* pDevice, ITexture* pHeightTex, DWORD channel);


//--------------------------------------------------------------------
// Name: PGECreateNormalizerMap
// Desc: ������һ�������ļ�����ͼ
//--------------------------------------------------------------------
DEVICE_EXPORT ICubeTexture* WINAPI PGECreateNormalizerMap(IGraphicDevice* pDevice, DWORD size);


