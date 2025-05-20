#pragma once

#include <assert.h>
#include <time.h>
#include <stdio.h>

// include standard header files
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <set>

#include "PGE_Graphic.h"

#define DEVICE_EXCEPT( desc, func ) { assert(false); throw (CDeviceException(PGE_DEVICE_ERROR, desc, func)); }
#define LOG_D3D9ERR(dev, hr, func ) dev->LogMessage(L"Func:%s, Desc: %s\n", func, DXGetErrorDescription9W(hr))

#pragma warning(disable : 4251)


