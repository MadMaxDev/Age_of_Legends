#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class  D3D9VertexDeclaration;

typedef D3D9ResourceMgr< D3D9VertexDeclaration > D3D9VertexDeclarationMgr;

class D3D9VertexDeclaration: public D3D9Resource< IVertexDeclaration, D3D9VertexDeclarationMgr >
{
public:
	D3D9VertexDeclaration()
	{
		m_pVD = NULL;
	}
	~D3D9VertexDeclaration()
	{
		SAFE_RELEASE( m_pVD );
	}
	inline BOOL Create( const D3DVERTEXELEMENT9* pElements )
	{
		HRESULT hr;
		if (FAILED(hr = GetD3DDevice()->CreateVertexDeclaration( ( pElements ), &m_pVD )))
		{
			DXTRACE_ERRW(ERR_FAILED_TO_CREATE_VD, hr);
			return FALSE;
		}
		return TRUE;
	}

	// 得到声明的元素
	virtual HRESULT GetDeclaration( D3DVERTEXELEMENT9* pElement,UINT* pNumElements ) 
	{
		return ( m_pVD->GetDeclaration( (pElement), pNumElements ) );
	}

	inline IDirect3DVertexDeclaration9* _d3d() { return m_pVD; }
protected:
	IDirect3DVertexDeclaration9* m_pVD;
};
