#include "..\Include\PGE_D3D9Enumeration.h"

CD3D9Enumeration::CD3D9Enumeration(OSD3D* pD3D)
{
	mpD3D9 = pD3D;
	mRefCount = 1;
	if (mpD3D9)
		mpD3D9->AddRef();
	else
		mpD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
}

CD3D9Enumeration::~CD3D9Enumeration(void)
{
	SAFE_RELEASE(mpD3D9);	
}

DWORD CD3D9Enumeration::AddRef()
{
	InterlockedIncrement((LONG*)&mRefCount);
	return mRefCount;
}

DWORD CD3D9Enumeration::GetRefCount()
{
	return mRefCount;
}

DWORD CD3D9Enumeration::Release()
{
	if (InterlockedDecrement((LONG*)&mRefCount) == 0L)
	{
		delete this;	
		return 0;
	}
	return mRefCount;
}

HRESULT CD3D9Enumeration::GetAdapterDisplayMode(D3DDISPLAYMODE* pDM) const
{
	if (mpD3D9 == NULL)
		return E_FAIL;

	return  (mpD3D9->GetAdapterDisplayMode(0, pDM));
}

