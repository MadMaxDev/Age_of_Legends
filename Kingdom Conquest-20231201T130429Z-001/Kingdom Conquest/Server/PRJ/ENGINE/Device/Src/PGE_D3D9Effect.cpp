#include "..\Include\PGE_D3D9Effect.h"
#include "..\Include\PGE_D3D9Texture.h"
#include "..\Include\PGE_D3D9CubeTexture.h"
#include "..\Include\PGE_ErrorText.h"
#include "..\Include\PGE_Config.h"
#include "..\Include\PGE_D3D9Device.h"

D3D9Effect::D3D9Effect()
{
	mpEffect = NULL;
}

D3D9Effect::~D3D9Effect()
{
	SAFE_RELEASE(mpEffect);
}

void D3D9Effect::OnLostDevice()
{
	if( mpEffect != NULL )
	{
		mpEffect->OnLostDevice();
	}
}

void D3D9Effect::OnResetDevice()
{
	if ( mpEffect != NULL )
	{
		mpEffect->OnResetDevice();
	}
}

D3DXHANDLE D3D9Effect::GetTechnique(UINT index)
{
	assert( mpEffect != NULL );

	return mpEffect->GetTechnique(index);
}

D3DXHANDLE D3D9Effect::GetTechniqueByName(LPCSTR pName)
{
	assert( mpEffect != NULL );

	return mpEffect->GetTechniqueByName(pName);
}

HRESULT D3D9Effect::FindNextValidTechniuqe(D3DXHANDLE handle, D3DXHANDLE* pTechnique)
{
	assert( mpEffect != NULL );
	return (mpEffect->FindNextValidTechnique(handle, pTechnique));
}

D3DXHANDLE D3D9Effect::GetParameterByName(D3DXHANDLE hParam, LPCSTR pName)
{
	assert (mpEffect != NULL);
	return mpEffect->GetParameterByName(hParam, pName);
}

D3DXHANDLE D3D9Effect::GetParameterBySemantic(D3DXHANDLE hParam, LPCSTR pSementic)
{
	assert (mpEffect != NULL);
	return mpEffect->GetParameterBySemantic(hParam, pSementic);
}

D3DXHANDLE D3D9Effect::GetParameter(D3DXHANDLE hParam, DWORD Index )
{
	return mpEffect->GetParameter( hParam, Index );
}

HRESULT D3D9Effect::BeginParameterBlock()
{
	return mpEffect->BeginParameterBlock();
}

D3DXHANDLE D3D9Effect::EndParameterBlock()
{
	return mpEffect->EndParameterBlock();
}

HRESULT D3D9Effect::ApplyParameterBlock(D3DXHANDLE hBlock)
{
	return mpEffect->ApplyParameterBlock( hBlock );
}

D3DXHANDLE D3D9Effect::GetCurrentTechnique()
{
	assert( mpEffect != NULL );
	
	return mpEffect->GetCurrentTechnique();
}

HRESULT D3D9Effect::SetTechnique(D3DXHANDLE handle)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetTechnique(handle));
}

HRESULT D3D9Effect::GetDesc(D3DXEFFECT_DESC* pDesc )
{
	return mpEffect->GetDesc( pDesc );
}

HRESULT D3D9Effect::GetParameterDesc(D3DXHANDLE hParam, D3DXPARAMETER_DESC* pDesc )
{
	return mpEffect->GetParameterDesc( hParam, pDesc );
}

HRESULT D3D9Effect::Begin(UINT *nPasses, DWORD flag)
{
	assert( mpEffect != NULL );
		
	return (mpEffect->Begin(nPasses, flag));
}

HRESULT D3D9Effect::End()
{
	assert( mpEffect != NULL );
	return (mpEffect->End());
}

HRESULT D3D9Effect::BeginPass(UINT nPass)
{
	assert( mpEffect != NULL );

	return (mpEffect->BeginPass(nPass));

}

HRESULT D3D9Effect::EndPass()
{
	assert( mpEffect != NULL );
	return (mpEffect->EndPass());
}

HRESULT D3D9Effect::CommitChanges()
{
	assert( mpEffect != NULL );
	return mpEffect->CommitChanges();
}

HRESULT D3D9Effect::SetMatrix(D3DXHANDLE hParam, const D3DXMATRIX* matrix)
{
	assert( mpEffect != NULL );
	return (mpEffect->SetMatrix(hParam, matrix));
}

HRESULT D3D9Effect::SetMatrixArray(D3DXHANDLE hParam, const D3DXMATRIX* matrixes, DWORD count)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetMatrixArray(hParam, matrixes, count));
}

HRESULT D3D9Effect::SetVector(D3DXHANDLE hParam, const D3DXVECTOR4* vec)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetVector(hParam, vec));
}

HRESULT D3D9Effect::SetVectorArray(D3DXHANDLE hParam, const D3DXVECTOR4* vecs, DWORD count)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetVectorArray(hParam, vecs, count));
}

HRESULT D3D9Effect::SetFloat(D3DXHANDLE hParam, float v)
{
	assert( mpEffect != NULL );
	return (mpEffect->SetFloat(hParam, v));
}

HRESULT D3D9Effect::SetFloatArray(D3DXHANDLE hParam, const float* pFloats, DWORD Count)
{
	assert (mpEffect != NULL);

	return (mpEffect->SetFloatArray(hParam, pFloats, Count));

}

HRESULT D3D9Effect::SetInt(D3DXHANDLE hParam, int v)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetInt(hParam, v));
}

HRESULT D3D9Effect::SetBool(D3DXHANDLE hParam, BOOL v)
{
	assert( mpEffect != NULL );

	return (mpEffect->SetBool(hParam, v));
}

HRESULT D3D9Effect::SetTexture(D3DXHANDLE hParam, IBaseTexture* pTex)
{
	assert( mpEffect != NULL );
	
	if (pTex == NULL)
	{
		mpEffect->SetTexture(hParam, NULL);
		return S_OK;
	}else
	{
		return mpEffect->SetTexture( hParam, pTex->GetOSHandle() );
	}
}



