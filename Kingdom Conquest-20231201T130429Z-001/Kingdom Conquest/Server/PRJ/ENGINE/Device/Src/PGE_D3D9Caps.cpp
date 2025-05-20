#include "..\Include\PGE_D3D9DeviceCaps.h"

BOOL IsSupportBlendMode(DWORD caps, D3DBLEND mode)
{
	switch (mode)
	{
	case D3DBLEND_ZERO: return (caps & D3DPBLENDCAPS_ZERO)?TRUE:FALSE;
	case D3DBLEND_ONE: return (caps & D3DPBLENDCAPS_ONE)?TRUE:FALSE;
	case D3DBLEND_SRCCOLOR : return (caps & D3DPBLENDCAPS_SRCCOLOR)?TRUE:FALSE;
	case D3DBLEND_INVSRCCOLOR : return (caps & D3DPBLENDCAPS_INVSRCCOLOR)?TRUE:FALSE;
	case D3DBLEND_SRCALPHA: return (caps & D3DPBLENDCAPS_SRCALPHA)?TRUE:FALSE;
	case D3DBLEND_INVSRCALPHA : return (caps & D3DPBLENDCAPS_INVSRCALPHA)?TRUE:FALSE;
	case D3DBLEND_DESTALPHA : return (caps & D3DPBLENDCAPS_DESTALPHA)?TRUE:FALSE;
	case D3DBLEND_INVDESTALPHA: return (caps & D3DPBLENDCAPS_INVDESTALPHA)?TRUE:FALSE;
	case D3DBLEND_DESTCOLOR: return (caps & D3DPBLENDCAPS_DESTCOLOR)?TRUE:FALSE;
	case D3DBLEND_INVDESTCOLOR: return (caps & D3DPBLENDCAPS_INVDESTCOLOR)?TRUE:FALSE;
	case D3DBLEND_SRCALPHASAT: return (caps & D3DPBLENDCAPS_SRCALPHASAT)?TRUE:FALSE;
	case D3DBLEND_BOTHSRCALPHA : return (caps & D3DPBLENDCAPS_BOTHSRCALPHA)?TRUE:FALSE;
	case D3DBLEND_BOTHINVSRCALPHA : return (caps & D3DPBLENDCAPS_BOTHINVSRCALPHA)?TRUE:FALSE;
	case D3DBLEND_BLENDFACTOR: return (caps & D3DPBLENDCAPS_BLENDFACTOR)?TRUE:FALSE;
	case D3DBLEND_INVBLENDFACTOR : return (caps & D3DPBLENDCAPS_BLENDFACTOR)?TRUE:FALSE;
	default: return FALSE;
	}
}

CD3D9DeviceCaps::CD3D9DeviceCaps(const OSCaps& caps) : mCaps(caps)
{
}

CD3D9DeviceCaps::~CD3D9DeviceCaps()
{
}

DWORD CD3D9DeviceCaps::GetVertexShaderVersion()
{
	return mCaps.VertexShaderVersion;
}

DWORD CD3D9DeviceCaps::GetPixelShaderVersion()
{
	return mCaps.PixelShaderVersion;
}

DWORD CD3D9DeviceCaps::GetMaxTextureWidth()
{
	return mCaps.MaxTextureWidth;
}
DWORD CD3D9DeviceCaps::GetMaxTextureHeight()
{
	return mCaps.MaxTextureHeight;
}

DWORD CD3D9DeviceCaps::GetMaxTextureBlendStates()
{
	return mCaps.MaxTextureBlendStages;
}

DWORD CD3D9DeviceCaps::GetMaxStreams()
{
	return mCaps.MaxStreams;
}

BOOL CD3D9DeviceCaps::IsSupportPureDevice()
{
	if (mCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
		return TRUE;

	return FALSE;
}

BOOL CD3D9DeviceCaps::IsSupportTextureBlendOp(D3DTEXTUREOP op)
{
	switch (op)
	{
	case	D3DTOP_SELECTARG1:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
	case	D3DTOP_SELECTARG2:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
		
	case	D3DTOP_MODULATE:  return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
	case	D3DTOP_MODULATE2X:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
	case	D3DTOP_MODULATE4X:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
		
	case	D3DTOP_ADD:	     return (mCaps.TextureOpCaps&D3DTEXOPCAPS_ADD)?TRUE:FALSE;
	case	D3DTOP_ADDSIGNED:   return (mCaps.TextureOpCaps&D3DTEXOPCAPS_ADDSIGNED)?TRUE:FALSE;
	case	D3DTOP_ADDSIGNED2X :return (mCaps.TextureOpCaps&D3DTEXOPCAPS_ADDSIGNED2X)?TRUE:FALSE;
		
	case	D3DTOP_SUBTRACT:	return (mCaps.TextureOpCaps&D3DTEXOPCAPS_SUBTRACT)?TRUE:FALSE;
	case	D3DTOP_ADDSMOOTH:	return (mCaps.TextureOpCaps&D3DTEXOPCAPS_ADDSMOOTH)?TRUE:FALSE;
		
	case	D3DTOP_BLENDDIFFUSEALPHA:  return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDDIFFUSEALPHA)?TRUE:FALSE;
	case	D3DTOP_BLENDTEXTUREALPHA:  return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHA)?TRUE:FALSE; 
	case	D3DTOP_BLENDFACTORALPHA:   return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDFACTORALPHA)?TRUE:FALSE; 
	case	D3DTOP_BLENDTEXTUREALPHAPM:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDTEXTUREALPHAPM)?TRUE:FALSE; 
	case	D3DTOP_BLENDCURRENTALPHA:  return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BLENDCURRENTALPHA)?TRUE:FALSE; 
		
	case	D3DTOP_PREMODULATE:           return (mCaps.TextureOpCaps&D3DTEXOPCAPS_PREMODULATE)?TRUE:FALSE; 
	case	D3DTOP_MODULATEALPHA_ADDCOLOR:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)?TRUE:FALSE; 
	case	D3DTOP_MODULATECOLOR_ADDALPHA:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)?TRUE:FALSE; 
		
	case	D3DTOP_MODULATEINVALPHA_ADDCOLOR:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR)?TRUE:FALSE; 
	case	D3DTOP_MODULATEINVCOLOR_ADDALPHA:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA)?TRUE:FALSE; 
		
	case	D3DTOP_BUMPENVMAP:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BUMPENVMAP)?TRUE:FALSE; 
	case	D3DTOP_BUMPENVMAPLUMINANCE:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)?TRUE:FALSE; 
		
	case	D3DTOP_DOTPRODUCT3:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_DOTPRODUCT3)?TRUE:FALSE; 
	case	D3DTOP_MULTIPLYADD:return (mCaps.TextureOpCaps&D3DTEXOPCAPS_MULTIPLYADD)?TRUE:FALSE; 
	case	D3DTOP_LERP:		return (mCaps.TextureOpCaps&D3DTEXOPCAPS_LERP)?TRUE:FALSE; 
	default: return FALSE;
	}		
}

BOOL CD3D9DeviceCaps::IsSupportSrcBlendMode(D3DBLEND mode)
{
	return IsSupportBlendMode(mCaps.SrcBlendCaps, mode);
}

BOOL CD3D9DeviceCaps::IsSupportDestBlendMode(D3DBLEND mode)
{
	return IsSupportBlendMode(mCaps.DestBlendCaps, mode);
}

BOOL CD3D9DeviceCaps::IsSupportBumpEnvMap()
{
	if ( 0 == (mCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportBumpEnvMapLuminance()
{
	if ( 0 == (mCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportCubeMap()
{
	if (0 == (mCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportCubeMapPower2()
{
	if (0 == (mCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportWBuffer()
{
	if (0 == (mCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportDither()
{
	if (0 == (mCaps.RasterCaps & D3DPRASTERCAPS_DITHER))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportDepthBias()
{
	if (0 == (mCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS))
		return FALSE;
	else
		return TRUE;
}

BOOL CD3D9DeviceCaps::IsSupportSlopeDepthBias()
{
	if (0 == (mCaps.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
		return FALSE;
	else
		return TRUE;
}