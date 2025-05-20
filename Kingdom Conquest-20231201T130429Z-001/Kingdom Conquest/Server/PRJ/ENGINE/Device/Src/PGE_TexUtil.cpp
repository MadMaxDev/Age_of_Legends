#include "../include/PGE_TexUtil.h"
#include <assert.h>

//------------------------------------------------------------------------
// Name: PGECreateFresnelMap
// Desc: 
//------------------------------------------------------------------------
ITexture* WINAPI PGECreateFresnelMap(IGraphicDevice* pDevice, DWORD size)
{
	ITexture* pTexture = pDevice->CreateTexture(size, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);
	if (pTexture == NULL)
		return NULL;

	D3DLOCKED_RECT lr;
	if( SUCCEEDED( pTexture->LockRect( 0 , &lr , NULL , 0 ) ) )
	{
		BYTE* pAlpha = (BYTE*)lr.pBits;
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		const float n = 1.33f;
		for (DWORD i = 0; i < desc.Width; ++i)
		{
			float k  = (float)i/(desc.Width - 1);	
			float g = n * n + k * k - 1.0f;

			float f1 = (g-k)*(g-k)/(2.0f * (g+k)*(g+k));
			float f2 = 1.0f + ( (k*(g+k)-1)*(k*(g+k)-1) )/( (k*(g-k) + 1)*(k*(g-k) + 1) );
			float f = f1 * f2;

			pAlpha[0] = pAlpha[1] = pAlpha[2] = pAlpha[3] = (BYTE)(f * 255.0f);
			pAlpha += 4;
		}
		pTexture->UnlockRect(0);
	}
	return pTexture;
}

//------------------------------------------------------------------------
// Name: PGEComputeNormalMap
// Desc:
//------------------------------------------------------------------------
ITexture* WINAPI PGEComputeNormalMap(IGraphicDevice* pDeivce, ITexture* pHeightTex, DWORD channel, float bumpiness)
{
	ITexture* pBumpMap = NULL;

	float bumpFactor = bumpiness/255.0f;

	D3DSURFACE_DESC    d3dsd;
	D3DLOCKED_RECT     d3dlr;

	pHeightTex->GetLevelDesc( 0, &d3dsd );
	if (d3dsd.Format != D3DFMT_X8R8G8B8 &&
		d3dsd.Format != D3DFMT_A8R8G8B8 )
		return NULL;

	// Create the bumpmap's surface and texture objects
	pBumpMap = pDeivce->CreateTexture( d3dsd.Width, d3dsd.Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);
	if (pBumpMap == NULL)
		return NULL;

	// Fill the bits of the new texture surface with bits from
	// a private format.
	if ( SUCCEEDED( pHeightTex->LockRect( 0, &d3dlr, 0, 0 ) ))
	{
		DWORD dwSrcPitch = (DWORD)d3dlr.Pitch;
		BYTE* pSrcTopRow = (BYTE*)d3dlr.pBits;
		BYTE* pSrcCurRow = pSrcTopRow;
		BYTE* pSrcBotRow = pSrcTopRow + (dwSrcPitch * (d3dsd.Height - 1) );

		if ( SUCCEEDED( pBumpMap->LockRect( 0, &d3dlr, 0, 0 ) ))
		{
			DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
			BYTE* pDstTopRow = (BYTE*)d3dlr.pBits;
			BYTE* pDstCurRow = pDstTopRow;

			for( DWORD y=0; y<d3dsd.Height; y++ )
			{
				BYTE* pSrcB0; // addr of current pixel
				BYTE* pSrcB2; // addr of pixel above current pixel, wrapping to bottom if necessary
				BYTE* pDstT;  // addr of dest pixel;

				pSrcB0 = pSrcCurRow;
				if( y == 0 )
					pSrcB2 = pSrcBotRow;
				else
					pSrcB2 = pSrcCurRow - dwSrcPitch;

				pDstT = pDstCurRow;
				for( DWORD x=0; x<d3dsd.Width; x++ )
				{
					LONG v00; // Current pixel
					LONG v01; // Pixel to the right of current pixel, wrapping to left edge if necessary
					LONG v1M; // Pixel one line above.

					v00 = *(pSrcB0+channel);

					if( x == d3dsd.Width - 1 )
						v01 = *(pSrcCurRow);
					else
						v01 = *(pSrcB0+4+channel);
					v1M = *(pSrcB2+channel);

					LONG iDu = (v00 - v01); // The delta-u bump value
					LONG iDv = (v1M - v00); // The delta-v bump value

					D3DXVECTOR3 n(FLOAT(iDu) * bumpFactor, FLOAT(iDv)* bumpFactor, 1.0f);
					D3DXVec3Normalize(&n, &n);

					*((DWORD*)pDstT) = PGEVectortoRGBA(&n);
					pDstT += 4;

					// Move one pixel to the right (src is 32-bpp)
					pSrcB0+=4;
					pSrcB2+=4;
				}

				// Move to the next line
				pSrcCurRow += dwSrcPitch;
				pDstCurRow += dwDstPitch;
			}
			pBumpMap->UnlockRect(0);
		}
		pHeightTex->UnlockRect(0);
	}
	return pBumpMap;
}

//------------------------------------------------------------------------
// Name: PGEComputeUVOffsetMap
// Desc:
//------------------------------------------------------------------------
ITexture* WINAPI PGEComputeUVOffsetMap(IGraphicDevice* pDevice, ITexture* pHeightTex, DWORD channel)
{
	ITexture* pBumpMap = NULL;

	D3DSURFACE_DESC    d3dsd;
	D3DLOCKED_RECT     d3dlr;

	pHeightTex->GetLevelDesc( 0, &d3dsd );
	if (d3dsd.Format != D3DFMT_X8R8G8B8 &&
		d3dsd.Format != D3DFMT_A8R8G8B8 )
		return NULL;

	// Create the bumpmap's surface and texture objects
	pBumpMap = pDevice->CreateTexture( d3dsd.Width, d3dsd.Height, 1, 0, D3DFMT_V8U8, D3DPOOL_MANAGED);
	if (pBumpMap == NULL)
		return NULL;

	// Fill the bits of the new texture surface with bits from
	// a private format.
	if (SUCCEEDED( pHeightTex->LockRect( 0, &d3dlr, 0, 0 ) ))
	{
		DWORD dwSrcPitch = (DWORD)d3dlr.Pitch;
		BYTE* pSrcTopRow = (BYTE*)d3dlr.pBits;
		BYTE* pSrcCurRow = pSrcTopRow;
		BYTE* pSrcBotRow = pSrcTopRow + (dwSrcPitch * (d3dsd.Height - 1) );

		if (SUCCEEDED(pBumpMap->LockRect( 0, &d3dlr, 0, 0 )))
		{
			DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
			BYTE* pDstTopRow = (BYTE*)d3dlr.pBits;
			BYTE* pDstCurRow = pDstTopRow;
			//BYTE* pDstBotRow = pDstTopRow + (dwDstPitch * (d3dsd.Height - 1) );

			for( DWORD y=0; y<d3dsd.Height; y++ )
			{
				BYTE* pSrcB0; // addr of current pixel
				BYTE* pSrcB1; // addr of pixel below current pixel, wrapping to top if necessary
				BYTE* pSrcB2; // addr of pixel above current pixel, wrapping to bottom if necessary
				BYTE* pDstT;  // addr of dest pixel;

				pSrcB0 = pSrcCurRow;

				if( y == d3dsd.Height - 1)
					pSrcB1 = pSrcTopRow;
				else
					pSrcB1 = pSrcCurRow + dwSrcPitch;

				if( y == 0 )
					pSrcB2 = pSrcBotRow;
				else
					pSrcB2 = pSrcCurRow - dwSrcPitch;

				pDstT = pDstCurRow;

				for( DWORD x=0; x<d3dsd.Width; x++ )
				{
					LONG v00; // Current pixel
					LONG v01; // Pixel to the right of current pixel, wrapping to left edge if necessary
					LONG vM1; // Pixel to the left of current pixel, wrapping to right edge if necessary
					LONG v10; // Pixel one line below.
					LONG v1M; // Pixel one line above.

					v00 = *(pSrcB0+channel);

					if( x == d3dsd.Width - 1 )
						v01 = *(pSrcCurRow+channel);
					else
						v01 = *(pSrcB0+4+channel);

					if( x == 0 )
						vM1 = *(pSrcCurRow + (4 * (d3dsd.Width - 1)) + channel);
					else
						vM1 = *(pSrcB0-4+channel);
					v10 = *(pSrcB1+channel);
					v1M = *(pSrcB2+channel);

					LONG iDu = (vM1-v01); // The delta-u bump value
					LONG iDv = (v1M-v10); // The delta-v bump value

					*pDstT++ = (BYTE)(iDu/2);
					*pDstT++ = (BYTE)(iDv/2);

					// Move one pixel to the right (src is 32-bpp)
					pSrcB0+=4;
					pSrcB1+=4;
					pSrcB2+=4;
				}

				// Move to the next line
				pSrcCurRow += dwSrcPitch;
				pDstCurRow += dwDstPitch;
			}

			pBumpMap->UnlockRect(0);
		}
		pHeightTex->UnlockRect(0);
	}

	return pBumpMap;
}

//------------------------------------------------------------------------
// Name: PGEMakeNormalizerCubeMap
// Desc:
//------------------------------------------------------------------------
ICubeTexture* WINAPI PGECreateNormalizerMap(IGraphicDevice* pDevice, DWORD dwSize)
{
	ICubeTexture* pCubeTexture = pDevice->CreateCubeTexture(dwSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);
	if (pCubeTexture == NULL)
		return NULL;

	D3DSURFACE_DESC desc;
	pCubeTexture->GetLevelDesc(0, &desc);

	D3DXVECTOR3 n;
	float sc, tc;
	D3DLOCKED_RECT lr;
	for (DWORD i = 0; i < 6; ++i)
	{
		D3DCUBEMAP_FACES face = (D3DCUBEMAP_FACES)i;
		if (SUCCEEDED(pCubeTexture->LockRect(face, 0, &lr, NULL, 0)))
		{
			BYTE* pRow = (BYTE*)lr.pBits;
			for (DWORD h = 0; h < desc.Height; ++h)
			{
				BYTE* pCol = pRow;
				for (DWORD w = 0; w < desc.Width; ++w)
				{
					sc = ((float)w + 0.5f)/(float)(dwSize);
					tc = ((float)h + 0.5f)/(float)(dwSize);
					sc = sc * 2.0f - 1.0f;
					tc = tc * 2.0f - 1.0f;

					switch (i)
					{
					case D3DCUBEMAP_FACE_POSITIVE_X: n = D3DXVECTOR3(1.0f, -tc, -sc);	break;
					case D3DCUBEMAP_FACE_NEGATIVE_X: n = D3DXVECTOR3(-1.0f, -tc, sc);	break;
					case D3DCUBEMAP_FACE_POSITIVE_Y: n = D3DXVECTOR3(sc, 1.0f, tc);		break;
					case D3DCUBEMAP_FACE_NEGATIVE_Y: n = D3DXVECTOR3(sc, -1.0f, -tc);	break;
					case D3DCUBEMAP_FACE_POSITIVE_Z: n = D3DXVECTOR3(sc, -tc, 1.0f);	break;
					case D3DCUBEMAP_FACE_NEGATIVE_Z: n = D3DXVECTOR3(-sc, -tc, -1.0f);	break;
					}
					D3DXVec3Normalize(&n, &n);

					// NOTE: RGBA的内存布局	
					pCol[0] = BYTE((n.z * 0.5f + 0.5f) * 255.0f);	
					pCol[1] = BYTE((n.y * 0.5f + 0.5f) * 255.0f);
					pCol[2] = BYTE((n.x * 0.5f + 0.5f) * 255.0f);
					pCol[3] = 255;

					pCol += 4;
				}
				pRow += lr.Pitch;
			}
			pCubeTexture->UnlockRect(face, 0);
		}
	}

	return pCubeTexture;
}


DEVICE_EXPORT DWORD WINAPI PGEGetPixelBits(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_UNKNOWN:  return 0;	
	case D3DFMT_R8G8B8:	  return 24;
	case D3DFMT_A8R8G8B8: return 32;
	case D3DFMT_X8R8G8B8: return 32;
	case D3DFMT_R5G6B5:   return 16;
	case D3DFMT_X1R5G5B5: return 16;
	case D3DFMT_A1R5G5B5: return 16;
	case D3DFMT_A4R4G4B4: return 16;
	case D3DFMT_R3G3B2:	  return 16;
	case D3DFMT_A8:		  return 8;
	case D3DFMT_A8R3G3B2: return 16;
	case D3DFMT_X4R4G4B4: return 16;
	case D3DFMT_A2B10G10R10: return 32;
	case D3DFMT_A8B8G8R8:	 return 32;
	case D3DFMT_X8B8G8R8:	 return 32;
	case D3DFMT_G16R16:		 return 32;
	case D3DFMT_A2R10G10B10: return 32;
	case D3DFMT_A16B16G16R16: return 64;
	case D3DFMT_A8P8:		  return 16;
	case D3DFMT_P8:			  return 8;
	case D3DFMT_L8:			  return 8;
	case D3DFMT_A8L8:		  return 16;
	case D3DFMT_A4L4:		  return 8;
	case D3DFMT_V8U8:		  return 16;
	case D3DFMT_L6V5U5:		  return 16;
	case D3DFMT_X8L8V8U8:	  return 32;
	case D3DFMT_Q8W8V8U8:	  return 32;
	case D3DFMT_V16U16:		  return 32;
	case D3DFMT_A2W10V10U10:  return 32;
	case D3DFMT_UYVY:		  return 0; // Unknown
	case D3DFMT_R8G8_B8G8:	  return 32;
	case D3DFMT_YUY2:		  return 0; // Unknown
	case D3DFMT_G8R8_G8B8:	  return 32;
	case D3DFMT_DXT1:		  return 4;
	case D3DFMT_DXT2:		  return 8;
	case D3DFMT_DXT3:		  return 8;
	case D3DFMT_DXT4:		  return 8;
	case D3DFMT_DXT5:		  return 8;

	case D3DFMT_D16_LOCKABLE : return 16;
	case D3DFMT_D32          : return 32;
	case D3DFMT_D15S1        : return 16;
	case D3DFMT_D24S8		 : return 32;
	case D3DFMT_D24X8		 : return 32;
	case D3DFMT_D24X4S4		 : return 32;
	case D3DFMT_D16			 : return 16;
	case D3DFMT_Q16W16V16U16 : return 64;
	case D3DFMT_MULTI2_ARGB8 : return 8;
	case D3DFMT_R16F		 : return 16;
	case D3DFMT_G16R16F		 : return 32;
	case D3DFMT_A16B16G16R16F: return 64;
	case D3DFMT_R32F		 : return 32;
	case D3DFMT_G32R32F		 : return 64;
	case D3DFMT_A32B32G32R32F: return 128
								   ;
	case D3DFMT_CxV8U8		 : return 32;// Unsure
	}
	return 0;
}
