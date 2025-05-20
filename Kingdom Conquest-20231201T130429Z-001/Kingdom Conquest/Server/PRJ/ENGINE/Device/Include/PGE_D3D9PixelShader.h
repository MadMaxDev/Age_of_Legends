#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class  D3D9PixelShader;

typedef D3D9ResourceMgr< D3D9PixelShader > D3D9PixelShaderMgr;

class D3D9PixelShader: public D3D9Resource< IPixelShader, D3D9PixelShaderMgr >
{
public:
	D3D9PixelShader()
	{
		m_pPS = NULL;
	}
	~D3D9PixelShader()
	{
		SAFE_RELEASE( m_pPS );
	}
	inline BOOL Create( LPD3DXBUFFER pCode )
	{
		HRESULT hr;
		if (FAILED(hr = GetD3DDevice()->CreatePixelShader( (const DWORD*)pCode->GetBufferPointer(), &m_pPS)))
		{
			DXTRACE_ERRW(L"Failed to create pixel shader", hr);
			return FALSE;
		}
		return TRUE;
	}

	inline IDirect3DPixelShader9* _d3d() { return m_pPS; }
protected:
	IDirect3DPixelShader9* m_pPS;
};

