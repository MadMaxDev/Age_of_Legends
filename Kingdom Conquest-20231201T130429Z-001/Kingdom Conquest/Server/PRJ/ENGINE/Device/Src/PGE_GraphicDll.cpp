#include "..\Include\PGE_D3D9Device.h"
#include "..\Include\PGE_D3D9Surface.h"
#include "..\Include\PGE_D3D9Texture.h"
#include "..\Include\PGE_D3D9CubeTexture.h"
#include "..\Include\PGE_D3D9Enumeration.h"
#include "..\Include\PGE_ErrorText.h"

//-------------------------------------------------------------------
// 全局函数的实现
//-------------------------------------------------------------------
DEVICE_EXPORT IGraphicDevice* WINAPI PGECreateDevice(const DEVICE_CONFIG* pConfig, int version )
{
	if (version != PGE_DEVICE_VERSION )
		return NULL;

	CD3D9Device* pDevice = new CD3D9Device();
	if (pDevice->Initialize(NULL, pConfig))
	{
		return pDevice;
	}else
	{
		pDevice->Release();
		return NULL;
	}
}


DEVICE_EXPORT IEnumeration* WINAPI PGECreateEnumeration(int version)
{
	if (version != PGE_DEVICE_VERSION )
		return NULL;
	else
		return  new CD3D9Enumeration(NULL);
}

DEVICE_EXPORT void WINAPI PGEDeviceCleanUp()
{
}

