#pragma once

#include "PGE_Graphic.h"

class CD3D9Enumeration: public IEnumeration
{
	PGE_NOCOPY_ASSIGN(CD3D9Enumeration)
public:
	CD3D9Enumeration(OSD3D* pD3D);
	~CD3D9Enumeration(void);
	
public:
	// 添加引用计数
	virtual DWORD  AddRef();

	// 得到引用计数的值
	virtual DWORD  GetRefCount();

	// 释放资源
	virtual DWORD  Release();

	// 得到监视器的显示模式
	virtual HRESULT GetAdapterDisplayMode(D3DDISPLAYMODE* pDM) const;

protected:	

	OSD3D* mpD3D9;
	DWORD mRefCount;
};