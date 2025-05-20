#pragma once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"


class D3D9Effect;
typedef D3D9ResourceMgr< D3D9Effect > D3D9EffectMgr;

//////////////////////////////////////////////////////////////////////////
// class D3D9Effect
class D3D9Effect: public D3D9Resource< IEffect, D3D9EffectMgr >
{
	PGE_NOCOPY_ASSIGN(D3D9Effect)
public:
	D3D9Effect();
	~D3D9Effect();

	BOOL Create( LPD3DXBUFFER pCode )
	{
		HRESULT hr;
		if (FAILED( hr = D3DXCreateEffect( GetD3DDevice(), pCode->GetBufferPointer(), (UINT)pCode->GetBufferSize(), NULL, NULL, 0, NULL, &mpEffect, NULL )))
		{
			DXTRACE_ERRW(ERR_FAILED_TO_CREATE_EFFECT, hr );
			return FALSE;
		}
		return TRUE;
	}


	void OnLostDevice();
	void OnResetDevice();
public:  
	virtual D3DXHANDLE GetTechniqueByName(LPCSTR pName);
	virtual D3DXHANDLE GetTechnique(UINT index);
	virtual D3DXHANDLE GetCurrentTechnique();
	virtual D3DXHANDLE GetParameterByName(D3DXHANDLE hParam, LPCSTR pName);

	virtual D3DXHANDLE GetParameterBySemantic( D3DXHANDLE hParam, LPCSTR pSementic);

	virtual D3DXHANDLE GetParameter(  D3DXHANDLE hParam, DWORD Index );

	virtual HRESULT GetDesc(D3DXEFFECT_DESC* pDesc );

	virtual HRESULT GetParameterDesc( D3DXHANDLE hParam, D3DXPARAMETER_DESC* pDesc );

	virtual HRESULT    BeginParameterBlock();
	virtual D3DXHANDLE EndParameterBlock();
	virtual HRESULT ApplyParameterBlock(D3DXHANDLE hBlock);

	virtual HRESULT FindNextValidTechniuqe(D3DXHANDLE handle, D3DXHANDLE* pTechnique);
	virtual HRESULT SetTechnique(D3DXHANDLE handle);

	virtual HRESULT Begin(UINT *nPasses, DWORD flag);
	virtual HRESULT End();
	virtual HRESULT BeginPass(UINT nPass);
	virtual HRESULT EndPass();
	virtual HRESULT CommitChanges();

	virtual HRESULT SetMatrix(D3DXHANDLE hParam, const D3DXMATRIX* matrix);
	virtual HRESULT SetMatrixArray(D3DXHANDLE hParam, const D3DXMATRIX* matrixes, DWORD count);
	virtual HRESULT SetVector(D3DXHANDLE hParam, const D3DXVECTOR4* vec);
	virtual HRESULT SetVectorArray(D3DXHANDLE hParam, const D3DXVECTOR4* vecs, DWORD count);
	virtual HRESULT SetFloat(D3DXHANDLE hParam, float v);
	virtual HRESULT SetFloatArray(D3DXHANDLE hParam, const float* pFloats, DWORD Count);
	virtual HRESULT SetBool(D3DXHANDLE hParam, BOOL v);
	virtual HRESULT SetInt(D3DXHANDLE hParam, int v);

	virtual HRESULT SetTexture(D3DXHANDLE hParam, IBaseTexture* pTexture);

protected:
	OSEffect*		mpEffect;
};
