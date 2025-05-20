#pragma once

#include "Include/PGE_Graphic.h"

#include "Include/PGE_File.h"
#include "Include/PGE_Math.h"
#include "Include/PGE_TexUtil.h"

#include "Include/PGE_Mesh.h"
#include "Include/PGE_Sprite.h"

#include "Include/PGE_RenderToSurface.h"
#include "Include/PGE_RenderToCubeMap.h"

#include "Include/PGE_Aabb.h"
#include "Include/PGE_Camera.h"
#include "Include/PGE_OBB.h"

#ifdef _DEBUG
	#pragma comment(lib,"PGE_Device_d.lib") 
	#pragma message("Automatically linking with PGE_Device_d.dll")
#else
	#pragma comment(lib,"PGE_Device.lib") 
	#pragma message("Automatically linking with PGE_Device.dll")
#endif
