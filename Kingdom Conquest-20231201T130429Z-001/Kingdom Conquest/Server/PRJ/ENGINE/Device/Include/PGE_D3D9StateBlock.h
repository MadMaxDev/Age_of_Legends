#pragma  once

#include "PGE_Config.h"
#include "PGE_SysResourceMgr.h"
#include "PGE_ErrorText.h"

class D3D9StateBlock;

typedef D3D9ResourceMgr< D3D9StateBlock > D3D9StateBlockMgr;

class D3D9StateBlock: public D3D9Resource< IStateBlock, D3D9StateBlockMgr >
{
	PGE_NOCOPY_ASSIGN(D3D9StateBlock)
public:
	D3D9StateBlock()
	{
		mpStateBlock = NULL;
		mpRSValues = NULL;
		mpTSSValues = NULL;
		mpSSValues = NULL;
		mTSSCount = 0;
		mRSCount = 0;
		mSSCount = 0;
	}
	~D3D9StateBlock()
	{
		SAFE_DELETE_ARRAY(mpRSValues);
		SAFE_DELETE_ARRAY(mpTSSValues);
		SAFE_DELETE_ARRAY(mpSSValues);
		SAFE_RELEASE(mpStateBlock);
	}

	BOOL Create(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount)
	{
		if ( pRSValues )
		{
			mpRSValues = new PGE_RS_VALUE[dwRSCount];	
			mRSCount = dwRSCount;
			memcpy( mpRSValues, pRSValues, dwRSCount*sizeof(PGE_RS_VALUE));
		}
		if ( pTSSValues )
		{
			mpTSSValues = new PGE_TSS_VALUE[dwTSSCount];
			mTSSCount =dwTSSCount;
			memcpy( mpTSSValues, pTSSValues, dwTSSCount*sizeof(PGE_TSS_VALUE));
		}
		if ( pSSValues )
		{
			mpSSValues = new PGE_SS_VALUE[dwSSCount];
			mSSCount = dwSSCount;
			memcpy( mpSSValues, pSSValues, dwSSCount * sizeof(PGE_SS_VALUE) );
		}
		OnResetDevice();
		return mpStateBlock != NULL;
	}

	// Ó¦ÓÃ×´Ì¬¿é
	void Apply()
	{
		assert(mpStateBlock);
		mpStateBlock->Apply();
	}

	void OnLostDevice()
	{
		SAFE_RELEASE(mpStateBlock);
	}

	void OnResetDevice()
	{
		assert(mpStateBlock == NULL);
		OSDevice* pD3DDevice = GetD3DDevice();

		if (SUCCEEDED(pD3DDevice->BeginStateBlock()))
		{
			DWORD i;
			for ( i = 0; i < mRSCount; ++i)
			{
				pD3DDevice->SetRenderState(mpRSValues[i].dwState, mpRSValues[i].dwValue);
			}
			for ( i = 0; i < mTSSCount; ++i)
			{
				pD3DDevice->SetTextureStageState(mpTSSValues[i].dwStage, mpTSSValues[i].dwState, mpTSSValues[i].dwValue);
			}
			for ( i = 0; i < mSSCount; ++i)
			{
				pD3DDevice->SetSamplerState(mpSSValues[i].dwStage, mpSSValues[i].dwState, mpSSValues[i].dwValue);
			}
			pD3DDevice->EndStateBlock(&mpStateBlock);
		}
	}
private:
	PGE_RS_VALUE*		mpRSValues;
	PGE_TSS_VALUE*		mpTSSValues;
	PGE_SS_VALUE*		mpSSValues;
	DWORD				mRSCount;
	DWORD				mTSSCount;
	DWORD				mSSCount;
	OSStateBlock*		mpStateBlock;
};

