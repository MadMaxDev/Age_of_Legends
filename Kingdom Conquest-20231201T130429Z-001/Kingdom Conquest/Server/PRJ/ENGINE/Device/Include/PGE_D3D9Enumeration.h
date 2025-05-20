#pragma once

#include "PGE_Graphic.h"

class CD3D9Enumeration: public IEnumeration
{
	PGE_NOCOPY_ASSIGN(CD3D9Enumeration)
public:
	CD3D9Enumeration(OSD3D* pD3D);
	~CD3D9Enumeration(void);
	
public:
	// ������ü���
	virtual DWORD  AddRef();

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount();

	// �ͷ���Դ
	virtual DWORD  Release();

	// �õ�����������ʾģʽ
	virtual HRESULT GetAdapterDisplayMode(D3DDISPLAYMODE* pDM) const;

protected:	

	OSD3D* mpD3D9;
	DWORD mRefCount;
};