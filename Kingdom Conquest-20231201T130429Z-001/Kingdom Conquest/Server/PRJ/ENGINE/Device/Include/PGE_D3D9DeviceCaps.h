#pragma once

#include "PGE_Graphic.h"
#include "PGE_SysResourceMgr.h"

class CD3D9DeviceCaps: public IDeviceCaps
{
	PGE_NOCOPY_ASSIGN(CD3D9DeviceCaps)
public:
	CD3D9DeviceCaps(const OSCaps& caps);
	~CD3D9DeviceCaps();
	
	virtual DWORD GetVertexShaderVersion();
	virtual DWORD GetPixelShaderVersion();
	virtual DWORD GetMaxTextureWidth();
	virtual DWORD GetMaxTextureHeight();
	virtual DWORD GetMaxTextureBlendStates();
	virtual DWORD GetMaxStreams();
	
	virtual BOOL IsSupportPureDevice();
	virtual BOOL IsSupportTextureBlendOp(D3DTEXTUREOP op);
	virtual BOOL IsSupportSrcBlendMode(D3DBLEND mode);
	virtual BOOL IsSupportDestBlendMode(D3DBLEND mode);
	
	virtual BOOL IsSupportCubeMap();
	virtual BOOL IsSupportCubeMapPower2();
	
	virtual BOOL IsSupportDither();
	virtual BOOL IsSupportDepthBias();
	virtual BOOL IsSupportSlopeDepthBias();
	virtual BOOL IsSupportBumpEnvMap();
	virtual BOOL IsSupportBumpEnvMapLuminance();
	virtual BOOL IsSupportWBuffer();
	
	void SetCaps(const OSCaps& caps) { mCaps = caps; }
	const OSCaps& GetCaps() const { return mCaps; }
	
protected:
	OSCaps mCaps;
};