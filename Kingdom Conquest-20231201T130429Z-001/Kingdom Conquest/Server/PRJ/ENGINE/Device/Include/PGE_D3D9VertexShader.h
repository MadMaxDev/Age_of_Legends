#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class  D3D9VertexShader;

typedef D3D9ResourceMgr< D3D9VertexShader > D3D9VertexShaderMgr;

class D3D9VertexShader: public D3D9Resource< IVertexShader, D3D9VertexShaderMgr >
{
public:
	D3D9VertexShader()
	{
		m_pVS = NULL;
	}
	~D3D9VertexShader()
	{
		SAFE_RELEASE( m_pVS );
	}
	inline BOOL Create( LPD3DXBUFFER pCode )
	{
		HRESULT hr;
		if (FAILED(hr = GetD3DDevice()->CreateVertexShader( (const DWORD*)pCode->GetBufferPointer(), &m_pVS)))
		{
			DXTRACE_ERRW(L"Failed to create vertex shader", hr);
			return FALSE;
		}
		return TRUE;
	}

	inline IDirect3DVertexShader9* _d3d() { return m_pVS; }
protected:
	IDirect3DVertexShader9* m_pVS;
};
