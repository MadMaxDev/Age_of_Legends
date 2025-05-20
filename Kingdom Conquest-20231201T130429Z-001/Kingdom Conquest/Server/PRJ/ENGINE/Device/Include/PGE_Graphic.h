/********************************************************************
created:	2005/08/06
created:	6:8:2005   20:36
filename: 	l:\Bo2\Device\Include\PGE_Graphic.h
file path:	l:\Bo2\Device\Include
file base:	PGE_Graphic
file ext:	h
author:		GongJing

purpose:	图形层相关的接口声明
*********************************************************************/
#pragma  once

// 当前的版本号
#define PGE_DEVICE_VERSION 2

//----------------------------------------------------------------------//
// Include d3d9 header files
//----------------------------------------------------------------------//
#include <DXSDK/Include/d3d9.h>
#include <DXSDK/Include/d3dx9.h>
#include <DXSDK/Include/dxerr9.h>
#include <DXSDK/Include/d3dx9math.h>
#include <tchar.h>

//----------------------------------------------------------------------//
// 宏定义
//----------------------------------------------------------------------//

#ifdef _DEBUG
	#define DXTRACE_ERRA(str, hr) DXTraceA(__FILE__, __LINE__, hr, str, FALSE )
	#define DXTRACE_ERRW(str, hr) DXTraceW(__FILE__, __LINE__, hr, str, FALSE )
#else
	#define DXTRACE_ERRA(str, hr) (hr)
	#define DXTRACE_ERRW(str, hr) (hr)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)		 { if (p) { (p)->Release(); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)		 { if (p) { delete (p); p = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); p = NULL; } }
#endif

//----------------------------------------------------------------------//
// 宏定义
//----------------------------------------------------------------------//
#ifdef _DEBUG
#	define PGE_VERIFY assert
#else
#	define PGE_VERIFY
#endif

#define PGE_MAX_NUM_STREAMS				8

#define PGEGEOM_VB_DEFAULT(n)		( 1 <<  n )
#define PGEGEOM_VB_SYSTEMMEM(n)		( 1 << (n + PGE_MAX_NUM_STREAMS) )

#define PGEGEOM_IB_DEFAULT			( 1 << (PGE_MAX_NUM_STREAMS*2 ) )
#define PGEGEOM_IB_SYSTEMMEM		( 1 << (PGE_MAX_NUM_STREAMS*2 + 1 ) )

#define PGEMESH_VB_DEFAULT			  1 
#define PGEMESH_VB_SYSTEMMEM		 ( 1 << PGE_MAX_NUM_STREAMS)

#define PGEMESH_IB_DEFAULT			 PGEGEOM_IB_DEFAULT 
#define PGEMESH_IB_SYSTEMMEM		 PGEGEOM_IB_SYSTEMMEM

#if defined(_MSC_VER) 
	#define PGE_NOVTABLE __declspec(novtable)
#else
	#define PGE_NOVTABLE 
#endif

#define PGE_NOCOPY_ASSIGN(T)  T( const T& ); \
							  T& operator = (const T& );

//-------------------------------------------------------------------
// 类型定义
//-------------------------------------------------------------------
typedef IDirect3D9						OSD3D;
typedef IDirect3DDevice9				OSDevice;
typedef IDirect3DVertexBuffer9			OSVertexBuffer;
typedef IDirect3DIndexBuffer9			OSIndexBuffer;
typedef IDirect3DBaseTexture9			OSBaseTexture;
typedef IDirect3DTexture9				OSTexture;
typedef IDirect3DCubeTexture9			OSCubeTexture;
typedef IDirect3DVolume9				OSVolume;
typedef IDirect3DVertexShader9			OSVertexShader;
typedef IDirect3DPixelShader9			OSPixelShader;
typedef IDirect3DStateBlock9			OSStateBlock;
typedef IDirect3DVertexDeclaration9		OSVertexDeclaration;
typedef IDirect3DSurface9				OSSurface;

typedef D3DVIEWPORT9					OSViewport;
typedef D3DLIGHT9						OSLight;
typedef D3DMATERIAL9					OSMaterial;
typedef D3DCAPS9						OSCaps;
typedef ID3DXEffect						OSEffect;
typedef ID3DXSprite						OSSprite;

//----------------------------------------------------------------------//
// 宏定义
//----------------------------------------------------------------------//
#if defined( _cplusplus )
#define PGE_EXTERN_C extern "C"
#else 
#define PGE_EXTERN_C
#endif

// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#   if defined( _DEVICE_EXPORTS )
#       define DEVICE_EXPORT __declspec( dllexport )
#   else
#       define DEVICE_EXPORT __declspec( dllimport )
#   endif

//-------------------------------------------------------------------
// 类名: CDeviceException
// 说明: 设备异常类
//-------------------------------------------------------------------
enum {PGE_DEVICE_ERROR = 1};

class CDeviceException
{
public:
	enum {MAX_FUNCNAME_LEN = 128, MAX_DESC_LEN = 512};
	CDeviceException(int errorId, const wchar_t* desc, const wchar_t* func)
	{
		mErrorID = errorId;
		wcsncpy( mDesc, desc, MAX_DESC_LEN );
		mDesc[MAX_DESC_LEN] = 0;
		wcsncpy(mFuncName, func, MAX_FUNCNAME_LEN );
		mFuncName[MAX_FUNCNAME_LEN] = 0;
	}

	const wchar_t* GetDesc()	 const { return mDesc;		}
	const wchar_t* GetFuncName() const { return mFuncName;  }
	int GetErrorID()			 const { return mErrorID;   }

protected:
	int	    mErrorID;
	wchar_t mDesc[MAX_DESC_LEN+1];
	wchar_t mFuncName[MAX_FUNCNAME_LEN+1];
};

//-------------------------------------------------------------------
// 类名: IDeviceResource
// 说明: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IDeviceResource
{
public:
	// 得到资源优先级
	virtual DWORD GetPriority()   = 0;

	// 设置资源优先级
	virtual void SetPriority(DWORD priority) = 0;
};
//-------------------------------------------------------------------
// 类名: IVertexBuffer
// 说明: 顶点缓冲接口
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexBuffer: public IDeviceResource
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 锁定缓冲
	virtual void* Lock(DWORD offset, DWORD size, DWORD flag) = 0;

	// 解除锁定
	virtual void  Unlock() = 0;

	// 获取大小
	virtual DWORD GetSize() = 0;
};

//-------------------------------------------------------------------
// 类名: IIndexBuffer
// 说明: 索引缓冲接口
//-------------------------------------------------------------------
class PGE_NOVTABLE IIndexBuffer : public IDeviceResource
{
public:	
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 锁定缓冲
	virtual void* Lock(DWORD offset, DWORD size, DWORD flag) = 0;

	// 解除锁定
	virtual void  Unlock() = 0;

	// 获取大小
	virtual DWORD GetSize() = 0;
};


//-------------------------------------------------------------------
// 类名: IVertexDecalration
// 说明: 顶点描述接口
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexDeclaration
{
public:
	// 添加引用计数
	virtual DWORD AddRef()  = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD Release() = 0;

	virtual HRESULT GetDeclaration( D3DVERTEXELEMENT9* pElement,UINT* pNumElements ) = 0;
};

//-------------------------------------------------------------------
// 类名: IVertexShader
// 说明: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexShader 
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;
};

//-------------------------------------------------------------------
// 类名: IPixelShader
// 说明: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IPixelShader 
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;
};

//-------------------------------------------------------------------
// 类名: IStateBlock
// 说明: 
//-------------------------------------------------------------------
struct PGE_RS_VALUE
{
	D3DRENDERSTATETYPE dwState;
	DWORD			   dwValue;
};

struct PGE_TSS_VALUE
{
	DWORD					 dwStage;
	D3DTEXTURESTAGESTATETYPE dwState;
	DWORD					 dwValue;
};

struct PGE_SS_VALUE
{
	DWORD					 dwStage;
	D3DSAMPLERSTATETYPE		 dwState;
	DWORD					 dwValue;
};

class PGE_NOVTABLE IStateBlock 
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 应用状态块
	virtual void Apply() = 0;
};

//-------------------------------------------------------------------
// 类名: IBaseTexture
// 说明: 纹理基类
//-------------------------------------------------------------------
class PGE_NOVTABLE IBaseTexture: public IDeviceResource
{
protected:
	IBaseTexture() : mDirtyFlag(FALSE) {}
	BOOL mDirtyFlag;
public:
	inline void SetDirtyFlag(BOOL v) { mDirtyFlag=v;	  }
	inline BOOL IsDirty()			 { return mDirtyFlag; }

	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 得到此纹理的资源类型
	virtual D3DRESOURCETYPE GetResourceType() = 0;
 
	// 重新生成Mipmap
	virtual BOOL GenerateMipSubLevels() = 0;

	// 得到Mipmap的级数
	virtual DWORD GetLevelCount() = 0;

	// 得到此纹理的表面描述
	virtual BOOL GetLevelDesc(DWORD level, D3DSURFACE_DESC* desc) = 0;

	// 保存纹理到文件
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette) = 0;
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette) = 0;

	virtual OSBaseTexture* GetOSHandle() = 0;
};

//-------------------------------------------------------------------
// 类名: ISurface
// 说明: 表面接口 
//-------------------------------------------------------------------
class PGE_NOVTABLE ISurface
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 锁定表面
	virtual HRESULT LockRect(D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag ) = 0;

	// 解锁表面
	virtual void UnlockRect() = 0;

	// 用表面'pSrcSurface'的数据更新自己的数据，如果大小不匹配会缩放
	virtual HRESULT Update( ISurface* pSrcSurface, const RECT* pDestRect, const RECT* pSrcRect, DWORD Filter, DWORD ColorKey ) = 0;

	// 得到表面描述
	virtual BOOL GetDesc( D3DSURFACE_DESC* desc) = 0;

	// 保存表面到文件
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) = 0;
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) = 0;

	// 从文件中加载数据
	virtual HRESULT LoadFromFileInMemory(const PALETTEENTRY *pDestPalette, const RECT *pDestRect,
										LPCVOID pSrcData, UINT SrcData,
										const RECT *pSrcRect,
										DWORD Filter,
										D3DCOLOR ColorKey,
										D3DXIMAGE_INFO *pSrcInfo ) = 0;
};

//-------------------------------------------------------------------
// 类名: ITexture
// 说明: 二维纹理接口 
//-------------------------------------------------------------------
class PGE_NOVTABLE ITexture: public IBaseTexture
{
public:
	// 锁定纹理表面
	virtual HRESULT LockRect(DWORD level, D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag) = 0;

	// 解锁纹理表面
	virtual void UnlockRect(DWORD level) = 0;

	// 得到纹理表面
	virtual ISurface* GetSurfaceLevel(DWORD level) = 0;
};


//-------------------------------------------------------------------
// 类名: ICubeTexture
// 说明: 立方本纹理接口 
//-------------------------------------------------------------------
class PGE_NOVTABLE ICubeTexture: public IBaseTexture
{
public:
	// 锁定纹理表面
	virtual HRESULT LockRect(D3DCUBEMAP_FACES faceType ,DWORD level,D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag) = 0;

	// 解锁纹理表面
	virtual void UnlockRect(D3DCUBEMAP_FACES faceType, DWORD level) = 0;

	// 得到纹理表面
	virtual ISurface* GetCubeMapSurface(D3DCUBEMAP_FACES faceType, DWORD level) = 0;

	virtual OSBaseTexture* GetOSHandle() = 0;
};


//-------------------------------------------------------------------
// 类名: IEffect
// 说明: Effect接口
//-------------------------------------------------------------------
class PGE_NOVTABLE IEffect
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 根据名称得到技巧句柄
	virtual D3DXHANDLE GetTechniqueByName(LPCSTR pName) = 0;

	// 根据索引得到技巧句柄
	virtual D3DXHANDLE GetTechnique(UINT index) = 0;

	// 得到下一个有效技巧句柄
	virtual HRESULT FindNextValidTechniuqe(D3DXHANDLE handle, D3DXHANDLE* pTechnique) = 0;

	// 得到当前的技巧句柄
	virtual D3DXHANDLE GetCurrentTechnique() = 0;

	// 根据名称得到参数句柄
	virtual D3DXHANDLE GetParameterByName(D3DXHANDLE hParam, LPCSTR pName) = 0;

	// 根据语义得到参数句柄
	virtual D3DXHANDLE GetParameterBySemantic( D3DXHANDLE hParam, LPCSTR pSementic) = 0;
	
	// 根据索引得到参数句柄
	virtual D3DXHANDLE GetParameter(  D3DXHANDLE hParam, DWORD Index ) = 0;

	// 设置当前的技巧
	virtual HRESULT SetTechnique(D3DXHANDLE handle) = 0;

	// 获取Effect描述
	virtual HRESULT GetDesc(D3DXEFFECT_DESC* pDesc ) = 0;

	virtual HRESULT GetParameterDesc( D3DXHANDLE hParam, D3DXPARAMETER_DESC* pDesc ) = 0;

	virtual HRESULT    BeginParameterBlock() = 0;
	virtual D3DXHANDLE EndParameterBlock() = 0;
	virtual HRESULT ApplyParameterBlock(D3DXHANDLE hBlock) = 0;

	// 开始一个技巧
	virtual HRESULT Begin(UINT *nPasses, DWORD flag) = 0;

	// 结束一个技巧
	virtual HRESULT End() = 0;

	// 开始一个Pass
	virtual HRESULT BeginPass(UINT nPass) = 0;

	// 结束一个Pass
	virtual HRESULT EndPass() = 0;

	// 提交更改
	virtual HRESULT CommitChanges() = 0;

	// 设置参数(Matrix)
	virtual HRESULT SetMatrix(D3DXHANDLE hParam, const D3DXMATRIX* matrix) = 0;

	// 设置参数(MatrixArray)
	virtual HRESULT SetMatrixArray(D3DXHANDLE hParam, const D3DXMATRIX* matrixes, DWORD count) = 0;

	// 设置参数(Vector)
	virtual HRESULT SetVector(D3DXHANDLE hParam, const D3DXVECTOR4* vec) = 0;

	// 设置参数(VectorArray)
	virtual HRESULT SetVectorArray(D3DXHANDLE hParam, const D3DXVECTOR4* vecs, DWORD count) = 0;

	// 设置参数(Float)
	virtual HRESULT SetFloat(D3DXHANDLE hParam, float v) = 0;

	// 设置参数(FloatArray)
	virtual HRESULT SetFloatArray(D3DXHANDLE hParam, const float* pFloats, DWORD Count) = 0;

	// 设置参数(Matrix)
	virtual HRESULT SetInt(D3DXHANDLE hParam, int v) = 0;

	// 设置参数(Matrix)
	virtual HRESULT SetBool(D3DXHANDLE hParam, BOOL v) = 0;

	// 设置纹理
	virtual HRESULT SetTexture(D3DXHANDLE hParam, IBaseTexture* pTexture) = 0;
};	

//-------------------------------------------------------------------
// 类名: IDeviceCaps
// 说明: 设备能力接口
//-------------------------------------------------------------------
class PGE_NOVTABLE IDeviceCaps
{
public:
	// 得到VertexShader的版本号
	virtual DWORD GetVertexShaderVersion() = 0;

	// 得到PixelShader的版本号
	virtual DWORD GetPixelShaderVersion() = 0;

	// 得到最大纹理宽度
	virtual DWORD GetMaxTextureWidth() = 0;

	// 得到最大纹理高度
	virtual DWORD GetMaxTextureHeight() = 0;

	// 得到最多的纹理混合层数
	virtual DWORD GetMaxTextureBlendStates() = 0;

	// 得到支持的最大的流数
	virtual DWORD GetMaxStreams() = 0;

	// 是否支持类型为(op)的纹理混合
	virtual BOOL IsSupportTextureBlendOp(D3DTEXTUREOP op) = 0;

	// 是否支持类型为(mode)的Alhpa源混合
	virtual BOOL IsSupportSrcBlendMode(D3DBLEND mode) = 0;

	// 是否支持类型为(mode)的Alhpa目的混合
	virtual BOOL IsSupportDestBlendMode(D3DBLEND mode) = 0;

	// 是否支持立文体纹理
	virtual BOOL IsSupportCubeMap() = 0;

	// 是否支持大小为2的幂的立文体纹理
	virtual BOOL IsSupportCubeMapPower2() = 0;

	// 是否支持WBuffer
	virtual BOOL IsSupportWBuffer() = 0;

	// 是否支持抖动
	virtual BOOL IsSupportDither() = 0;

	// 是否支持深度值偏移
	virtual BOOL IsSupportDepthBias() = 0;

	// 是否支持范围深度值偏移
	virtual BOOL IsSupportSlopeDepthBias() = 0;

	// 是否支持凹凸映射
	virtual BOOL IsSupportBumpEnvMap() = 0;

	// 是否支持光照凹凸映射
	virtual BOOL IsSupportBumpEnvMapLuminance() = 0;
};

//-------------------------------------------------------------------
// 类名: IEnumeration
// 说明: 枚举设备类
//-------------------------------------------------------------------
class PGE_NOVTABLE IEnumeration
{
public:
	// 添加引用计数
	virtual DWORD  AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD  GetRefCount() = 0;

	// 释放资源
	virtual DWORD  Release() = 0;

	// 得到监视器的显示模式
	virtual HRESULT GetAdapterDisplayMode(D3DDISPLAYMODE* pDM) const = 0;
};

// （用户实现）通知字符串信息的回调接口
struct PGE_NOVTABLE PG_Report_I
{
	// 功能		:通知字符串信息
	// 参数		:信息
	// 返回值	:无
	virtual void Notify( const wchar_t* _szMsg ) = 0;
};

class IGraphicDevice;
//----------------------------------------------------------------------//
// 设备重创时的回调函数
//----------------------------------------------------------------------//
typedef void (CALLBACK *LPCALLBACKRESETDEVICE)(IGraphicDevice* pDevice, BOOL bIsRecreated);

//-------------------------------------------------------------------
// 类名: DEVICE_CONFIG
// 说明: 创建设备时的所需配置
//-------------------------------------------------------------------
struct DEVICE_CONFIG
{
	DEVICE_CONFIG()
	{
		AdapterIndex			 = 0;
		AdapterFormat			 = D3DFMT_UNKNOWN;	
		DeviceType				 = D3DDEVTYPE_HAL;
		MultisampleType			 = D3DMULTISAMPLE_NONE;
		MultisampleQuality		 = 0;
		DepthStencilBufferFormat = D3DFMT_D16;
		PresentInterval			 = D3DPRESENT_INTERVAL_IMMEDIATE;
		EnableDepthStencilBuffer = TRUE;
		Windowed				 = TRUE;
		EnableDialog			 = TRUE;
		EnableDebugVS			 = FALSE;
		EnableMultiThread		 = FALSE;
		hWnd					 = NULL;
		OnResetDeviceProc		 = NULL;
		OnLostDeviceProc		 = NULL;
		pLogStream				 = NULL;
		VSVersion				 = D3DVS_VERSION(1,1);
		SoftwareVS				 = FALSE;
	}
	DWORD				  AdapterIndex;					// 适配器的索引
	D3DFORMAT			  AdapterFormat;				// 适配器的表面格式
	D3DDEVTYPE			  DeviceType;					// 设备类型
	D3DDISPLAYMODE		  DisplayMode;					// 显示模式
	D3DMULTISAMPLE_TYPE   MultisampleType;				// 表面采样格式
	DWORD				  MultisampleQuality;			// 采样质量
	D3DFORMAT			  DepthStencilBufferFormat;		// 深度缓冲的表面格式
	DWORD				  PresentInterval;				// 
	BOOL			      EnableDepthStencilBuffer;		// 是否使用深度缓冲
	BOOL				  Windowed;						// 是否为窗口模式
	BOOL				  EnableDebugVS;				// 是否允许调试Vertex Shader
	BOOL				  EnableDialog;					// 否允许弹出对话框
	BOOL				  EnableMultiThread;			// 是否允许多线程
	BOOL				  SoftwareVS;					// 是否强制使用软件顶点处理
	HWND				  hWnd;							// 创建设备时的窗口句柄
	DWORD				  VSVersion;					// Vertex shader的版本
	mutable PG_Report_I*		  pLogStream;			// 设备日志流
	mutable LPCALLBACKRESETDEVICE OnResetDeviceProc;	// 设备丢重置的回调函数
	mutable LPCALLBACKRESETDEVICE OnLostDeviceProc;		// 设备丢失时的回调函数
};

//-------------------------------------------------------------------
// 类名: VIDEO_MEMROY_USAGE
// 说明: 
//-------------------------------------------------------------------
struct VIDEO_MEMROY_USAGE
{
	UINT VertexBufferMemSize;			// Vertex buffer的显存大小
	UINT IndexBufferMemSize;			// Index buffer的显存大小
	UINT TextureMemSize;				// 2D纹理的显存大小
	UINT CubeTextureMemSize;			// 立方本纹理的显存大小
	UINT SurfaceMemSize;				// RenderTarget和DepthStencilBuffer的大小
};

//-------------------------------------------------------------------
// 类名: DEVICE_STATISTIC
// 说明: 
//-------------------------------------------------------------------
struct DEVICE_STATICSTIC
{
	UINT DrawPrimitiveCount;		// 画了多少个图元
	UINT DrawPrimitiveCounter;		// 调了多少次绘图元绘制函数
};

typedef IDirect3DQuery9 IQuery;
//-------------------------------------------------------------------
// 类名: IGraphicDevice
// 说明: 图形设备接口 
//-------------------------------------------------------------------
class PGE_NOVTABLE IGraphicDevice
{
public:
	// 添加引用计数
	virtual DWORD AddRef() = 0;

	// 得到引用计数的值
	virtual DWORD GetRefCount() const = 0;

	// 释放资源
	virtual DWORD Release() = 0;

	// 得到当前的设备配置
	virtual const DEVICE_CONFIG& GetDeviceConfig() const = 0;

	// 设置默认的渲染状态
	virtual HRESULT SetDefaultRenderState() = 0;

	// 设置对话框的弹出模式
	virtual HRESULT SetDialogBoxMode(BOOL bEnable) = 0;

	// 设置渲染状态
	virtual HRESULT SetRenderState(D3DRENDERSTATETYPE type, DWORD value) = 0;

	// 设置纹理舞台
	virtual HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE Type, DWORD op) = 0;

	// 设置纹理采样舞台
	virtual HRESULT SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE Type, DWORD value) = 0;

	// 设置纹理
	virtual HRESULT SetTexture(DWORD stage, IBaseTexture* texture) = 0;

	// 设置视口	
	virtual HRESULT SetViewport(DWORD x, DWORD y, DWORD w, DWORD h, float minZ , float maxZ) = 0;
	virtual HRESULT SetViewport(const OSViewport* pViewport ) = 0;

	// 得到视口
	virtual HRESULT GetViewport(OSViewport *vp) const = 0;

	// 设置顶点流
	virtual HRESULT SetStreamSource(DWORD sreamNumber, IVertexBuffer* vertexBuffer, DWORD offset, DWORD strideSize) = 0;

	// 设置索引缓冲
	virtual HRESULT SetIndices(IIndexBuffer* indexBuffer) = 0;

	// 设置顶点声明
	virtual	HRESULT SetVertexDeclaration( IVertexDeclaration* pDecl ) = 0;

	// 设置顶点管线的常数寄存器
	virtual	HRESULT SetVertexShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) = 0;

	// 设置象数管线的常数寄存器
	virtual	HRESULT SetPixelShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) = 0;

	// 设置当的顶点格式(FVF)	
	virtual HRESULT SetFVF(DWORD fvf) = 0;

	// 设置VertexShader
	virtual HRESULT SetVertexShader(IVertexShader* pVS) = 0;

	// 设置PixelShader
	virtual HRESULT SetPixelShader(IPixelShader* pPS) = 0;

	// 开启光照
	virtual HRESULT LightEnable(DWORD index, BOOL bEnable) = 0;

	// 设置光源
	virtual HRESULT SetLight(DWORD index, OSLight* light) = 0;

	// 没置材质
	virtual HRESULT SetMaterial(OSMaterial* mtl) = 0;

	// 设置剪裁面
	virtual HRESULT SetClipPlane(DWORD index, const float* plane) = 0;

	// 设置剪裁面的状态
	virtual HRESULT SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus) = 0;

	// 设置鼠标的位置
	virtual void SetCursorPosition( INT X, INT Y, DWORD Flags ) = 0;

	// 设置鼠标的属性
	virtual HRESULT SetCursorProperties( UINT XHotSpot, UINT YHotSpot, ISurface *pCursorBitmap) = 0;

	// 显示鼠标
	virtual HRESULT ShowCursor(BOOL bShow) = 0;

	// 设置变换阵
	virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, const D3DXMATRIX* matrix) = 0;

	// 清除后备缓冲
	virtual HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, DWORD clr, float z, DWORD stencil) = 0;

	// 测试设备能否正常渲染
	virtual BOOL TestBeforeRender() = 0;

	// 开始渲染场景
	virtual HRESULT BeginScene() = 0;	

	// 结束渲染场景
	virtual HRESULT EndScene() = 0;

	// 把后备缓冲刷到当前屏幕缓冲
	virtual HRESULT Present( const RECT* sourceRect, const RECT* destRect	, HWND presentTarget, CONST RGNDATA *pDirtyRegion  ) = 0;

	// 图元绘制
	virtual HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE  type, DWORD primitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) = 0;
	virtual HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
										UINT MinVertexIndex,
										UINT NumVertices,
										UINT PrimitiveCount,
										const void *pIndexData,
										D3DFORMAT IndexDataFormat,
										CONST void* pVertexStreamZeroData,
										UINT VertexStreamZeroStride) = 0;
	virtual HRESULT DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE type, UINT primitiveCount, const void* streamData, UINT  bytesPerVertex, UINT  vertexCount ) = 0;

	virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE  type, DWORD startIndex, DWORD primitiveCount) = 0;
	virtual HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE  type, DWORD baseVertexIndex, DWORD minIndex, DWORD numVertices, DWORD startIndex, DWORD count) = 0;

	// 创建VertexBuffer
	virtual IVertexBuffer*		 CreateVertexBuffer(DWORD size, DWORD fvf,  DWORD usage, D3DPOOL pool ) = 0;

	// 创建IndexBuffer
	virtual IIndexBuffer*		 CreateIndexBuffer(DWORD count, DWORD usage, D3DPOOL pool) = 0;

	// 创建VertexDeclaration
	virtual	IVertexDeclaration*  CreateVertexDeclaration(const D3DVERTEXELEMENT9* elements) = 0;

	// 创建VertexShader
	virtual IVertexShader*		 CreateVertexShader(LPD3DXBUFFER pCode) = 0;

	// 创建PixelShader
	virtual IPixelShader*		 CreatePixelShader(LPD3DXBUFFER pCode) = 0;

	// 创建StateBlock
	virtual IStateBlock*		 CreateStateBlock(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount) = 0;

	// 创建纹理
	virtual ITexture*			 CreateTexture(DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool) = 0;

	// 从文件数据中创建纹理
	virtual ITexture*			 CreateTexture(void* srcData, DWORD len, DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, DWORD filter, DWORD mipFilter, DWORD ColorKey, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) = 0;

	// 创建立方体纹理
	virtual ICubeTexture*		 CreateCubeTexture(DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool ) = 0;

	// 从文件数据中创建立方体纹理
	virtual ICubeTexture*		 CreateCubeTexture(void* srcData, DWORD len, DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt , DWORD filter, DWORD mipFilter, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) = 0;

	// 从文件数据中创建Effect接口
	virtual IEffect*			 CreateEffect( LPD3DXBUFFER pCode ) = 0;

	// 创建离屏表面
	virtual ISurface*			 CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch) = 0;

	// 得到设备能力
	virtual IDeviceCaps* GetDeviceCaps() = 0;

	// 得到后备缓冲的格式描述
	virtual HRESULT GetBackBufferDesc(D3DSURFACE_DESC* pDesc) = 0;

	// 创建RenderTarget
	virtual ISurface* CreateRenderTarget(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType , DWORD multisampleQuality,  BOOL bLockable) = 0;

	// 创建Depth Stencil Surface
	virtual ISurface* CreateDepthStencilSurface(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType, DWORD multisampleQuality,  BOOL bDiscard ) = 0;

	// 设置RenderTarget
	virtual HRESULT SetRenderTarget(ISurface* target) = 0;

	// 得到当前的RenderTarget
	virtual ISurface* GetRenderTarget() = 0;

	// 设置Depth Stencil Surface
	virtual HRESULT SetDepthStencilSurface(ISurface* pSurface) = 0;

	// 得到当前的Depth Stencil Surface
	virtual ISurface* GetDepthStencilSurface() = 0;

	// 设置剪切区域
	virtual HRESULT SetScissorRect(const RECT* pRect) = 0;

	// 获取剪切区域
	virtual HRESULT GetScissorRect(RECT* pRect) = 0;

	// 设置Gamma修正值
	virtual void SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp) = 0;

	// 得到Gamma修正值
	virtual void GetGammaRamp(D3DGAMMARAMP *pRamp) = 0;

	// 得到显存的大小(包括AGP显存和局部显存)
	virtual UINT GetVideoMemorySize() = 0;

	// 程序能用多少显存, 如果当程序使用的显存大小指定大小时
	// 调用EvictManagedResources会回收'Managed'的显存资源
	// 直到当前显存使用大小小于指定的大小.
	virtual void ReserveVideoMemory(UINT Memsize) = 0;

	// 得到当前显存的使用状态
	virtual void GetVideoMemoryUsage(VIDEO_MEMROY_USAGE* pMemUsage) = 0;

	// 得到当前显存的使用总量
	virtual UINT GetVideoMemroyUsage() = 0;

	// 回收被管理的显存资源
	// 参数: cacheTime为资源的缓冲时间
	virtual void EvictManagedResources(UINT cacheTime) = 0;

	// 开始统计信息(如:画了多少三角形等)
	virtual void BeginStat() = 0;

	// 得到统计信息
	virtual void GetStatistic(DEVICE_STATICSTIC* pStatistic)= 0;

	// 重置设备
	virtual void Reset(const DEVICE_CONFIG* cfg) = 0;

	// 得到D3D的设备结口
	virtual OSDevice* GetD3DDevice() = 0;

	virtual HRESULT StretchRect( ISurface * pSourceSurface,
		const RECT * pSourceRect,
		ISurface * pDestSurface,
		const RECT * pDestRect,
		D3DTEXTUREFILTERTYPE Filter) = 0;

	virtual ISurface* GetBackBuffer() = 0;

	//
	virtual HRESULT GetRenderTargetData( ISurface* pRenderTarget, ISurface* pDestSurface ) = 0;

	// 检测设备格式是否可用
	virtual HRESULT CheckDeviceFormat( DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt ) = 0;

	// 获取遮挡查询接口
	virtual IQuery* GetOcclusionQuery() = 0;

};

//-------------------------------------------------------------------
// 全局函数
//-------------------------------------------------------------------
// 创建设备
PGE_EXTERN_C DEVICE_EXPORT IGraphicDevice* WINAPI PGECreateDevice(const DEVICE_CONFIG* pConfig, int version);

// 创建设备枚举器
PGE_EXTERN_C DEVICE_EXPORT IEnumeration*   WINAPI PGECreateEnumeration(int version);

// 清空全局部量
PGE_EXTERN_C DEVICE_EXPORT void WINAPI PGEDeviceCleanUp();

