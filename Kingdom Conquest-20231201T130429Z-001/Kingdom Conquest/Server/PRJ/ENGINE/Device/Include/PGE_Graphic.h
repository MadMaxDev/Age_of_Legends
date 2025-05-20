/********************************************************************
created:	2005/08/06
created:	6:8:2005   20:36
filename: 	l:\Bo2\Device\Include\PGE_Graphic.h
file path:	l:\Bo2\Device\Include
file base:	PGE_Graphic
file ext:	h
author:		GongJing

purpose:	ͼ�β���صĽӿ�����
*********************************************************************/
#pragma  once

// ��ǰ�İ汾��
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
// �궨��
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
// �궨��
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
// ���Ͷ���
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
// �궨��
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
// ����: CDeviceException
// ˵��: �豸�쳣��
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
// ����: IDeviceResource
// ˵��: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IDeviceResource
{
public:
	// �õ���Դ���ȼ�
	virtual DWORD GetPriority()   = 0;

	// ������Դ���ȼ�
	virtual void SetPriority(DWORD priority) = 0;
};
//-------------------------------------------------------------------
// ����: IVertexBuffer
// ˵��: ���㻺��ӿ�
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexBuffer: public IDeviceResource
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// ��������
	virtual void* Lock(DWORD offset, DWORD size, DWORD flag) = 0;

	// �������
	virtual void  Unlock() = 0;

	// ��ȡ��С
	virtual DWORD GetSize() = 0;
};

//-------------------------------------------------------------------
// ����: IIndexBuffer
// ˵��: ��������ӿ�
//-------------------------------------------------------------------
class PGE_NOVTABLE IIndexBuffer : public IDeviceResource
{
public:	
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// ��������
	virtual void* Lock(DWORD offset, DWORD size, DWORD flag) = 0;

	// �������
	virtual void  Unlock() = 0;

	// ��ȡ��С
	virtual DWORD GetSize() = 0;
};


//-------------------------------------------------------------------
// ����: IVertexDecalration
// ˵��: ���������ӿ�
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexDeclaration
{
public:
	// ������ü���
	virtual DWORD AddRef()  = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD Release() = 0;

	virtual HRESULT GetDeclaration( D3DVERTEXELEMENT9* pElement,UINT* pNumElements ) = 0;
};

//-------------------------------------------------------------------
// ����: IVertexShader
// ˵��: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IVertexShader 
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;
};

//-------------------------------------------------------------------
// ����: IPixelShader
// ˵��: 
//-------------------------------------------------------------------
class PGE_NOVTABLE IPixelShader 
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;
};

//-------------------------------------------------------------------
// ����: IStateBlock
// ˵��: 
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
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// Ӧ��״̬��
	virtual void Apply() = 0;
};

//-------------------------------------------------------------------
// ����: IBaseTexture
// ˵��: �������
//-------------------------------------------------------------------
class PGE_NOVTABLE IBaseTexture: public IDeviceResource
{
protected:
	IBaseTexture() : mDirtyFlag(FALSE) {}
	BOOL mDirtyFlag;
public:
	inline void SetDirtyFlag(BOOL v) { mDirtyFlag=v;	  }
	inline BOOL IsDirty()			 { return mDirtyFlag; }

	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// �õ����������Դ����
	virtual D3DRESOURCETYPE GetResourceType() = 0;
 
	// ��������Mipmap
	virtual BOOL GenerateMipSubLevels() = 0;

	// �õ�Mipmap�ļ���
	virtual DWORD GetLevelCount() = 0;

	// �õ�������ı�������
	virtual BOOL GetLevelDesc(DWORD level, D3DSURFACE_DESC* desc) = 0;

	// ���������ļ�
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette) = 0;
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette) = 0;

	virtual OSBaseTexture* GetOSHandle() = 0;
};

//-------------------------------------------------------------------
// ����: ISurface
// ˵��: ����ӿ� 
//-------------------------------------------------------------------
class PGE_NOVTABLE ISurface
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// ��������
	virtual HRESULT LockRect(D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag ) = 0;

	// ��������
	virtual void UnlockRect() = 0;

	// �ñ���'pSrcSurface'�����ݸ����Լ������ݣ������С��ƥ�������
	virtual HRESULT Update( ISurface* pSrcSurface, const RECT* pDestRect, const RECT* pSrcRect, DWORD Filter, DWORD ColorKey ) = 0;

	// �õ���������
	virtual BOOL GetDesc( D3DSURFACE_DESC* desc) = 0;

	// ������浽�ļ�
	virtual HRESULT SaveToFileA(LPCSTR  pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) = 0;
	virtual HRESULT SaveToFileW(LPCWSTR pFileName, D3DXIMAGE_FILEFORMAT imgFmt, const PALETTEENTRY *pSrcPalette,const RECT* pSrcRect) = 0;

	// ���ļ��м�������
	virtual HRESULT LoadFromFileInMemory(const PALETTEENTRY *pDestPalette, const RECT *pDestRect,
										LPCVOID pSrcData, UINT SrcData,
										const RECT *pSrcRect,
										DWORD Filter,
										D3DCOLOR ColorKey,
										D3DXIMAGE_INFO *pSrcInfo ) = 0;
};

//-------------------------------------------------------------------
// ����: ITexture
// ˵��: ��ά����ӿ� 
//-------------------------------------------------------------------
class PGE_NOVTABLE ITexture: public IBaseTexture
{
public:
	// �����������
	virtual HRESULT LockRect(DWORD level, D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag) = 0;

	// �����������
	virtual void UnlockRect(DWORD level) = 0;

	// �õ��������
	virtual ISurface* GetSurfaceLevel(DWORD level) = 0;
};


//-------------------------------------------------------------------
// ����: ICubeTexture
// ˵��: ����������ӿ� 
//-------------------------------------------------------------------
class PGE_NOVTABLE ICubeTexture: public IBaseTexture
{
public:
	// �����������
	virtual HRESULT LockRect(D3DCUBEMAP_FACES faceType ,DWORD level,D3DLOCKED_RECT* lr, const RECT* rect, DWORD flag) = 0;

	// �����������
	virtual void UnlockRect(D3DCUBEMAP_FACES faceType, DWORD level) = 0;

	// �õ��������
	virtual ISurface* GetCubeMapSurface(D3DCUBEMAP_FACES faceType, DWORD level) = 0;

	virtual OSBaseTexture* GetOSHandle() = 0;
};


//-------------------------------------------------------------------
// ����: IEffect
// ˵��: Effect�ӿ�
//-------------------------------------------------------------------
class PGE_NOVTABLE IEffect
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// �������Ƶõ����ɾ��
	virtual D3DXHANDLE GetTechniqueByName(LPCSTR pName) = 0;

	// ���������õ����ɾ��
	virtual D3DXHANDLE GetTechnique(UINT index) = 0;

	// �õ���һ����Ч���ɾ��
	virtual HRESULT FindNextValidTechniuqe(D3DXHANDLE handle, D3DXHANDLE* pTechnique) = 0;

	// �õ���ǰ�ļ��ɾ��
	virtual D3DXHANDLE GetCurrentTechnique() = 0;

	// �������Ƶõ��������
	virtual D3DXHANDLE GetParameterByName(D3DXHANDLE hParam, LPCSTR pName) = 0;

	// ��������õ��������
	virtual D3DXHANDLE GetParameterBySemantic( D3DXHANDLE hParam, LPCSTR pSementic) = 0;
	
	// ���������õ��������
	virtual D3DXHANDLE GetParameter(  D3DXHANDLE hParam, DWORD Index ) = 0;

	// ���õ�ǰ�ļ���
	virtual HRESULT SetTechnique(D3DXHANDLE handle) = 0;

	// ��ȡEffect����
	virtual HRESULT GetDesc(D3DXEFFECT_DESC* pDesc ) = 0;

	virtual HRESULT GetParameterDesc( D3DXHANDLE hParam, D3DXPARAMETER_DESC* pDesc ) = 0;

	virtual HRESULT    BeginParameterBlock() = 0;
	virtual D3DXHANDLE EndParameterBlock() = 0;
	virtual HRESULT ApplyParameterBlock(D3DXHANDLE hBlock) = 0;

	// ��ʼһ������
	virtual HRESULT Begin(UINT *nPasses, DWORD flag) = 0;

	// ����һ������
	virtual HRESULT End() = 0;

	// ��ʼһ��Pass
	virtual HRESULT BeginPass(UINT nPass) = 0;

	// ����һ��Pass
	virtual HRESULT EndPass() = 0;

	// �ύ����
	virtual HRESULT CommitChanges() = 0;

	// ���ò���(Matrix)
	virtual HRESULT SetMatrix(D3DXHANDLE hParam, const D3DXMATRIX* matrix) = 0;

	// ���ò���(MatrixArray)
	virtual HRESULT SetMatrixArray(D3DXHANDLE hParam, const D3DXMATRIX* matrixes, DWORD count) = 0;

	// ���ò���(Vector)
	virtual HRESULT SetVector(D3DXHANDLE hParam, const D3DXVECTOR4* vec) = 0;

	// ���ò���(VectorArray)
	virtual HRESULT SetVectorArray(D3DXHANDLE hParam, const D3DXVECTOR4* vecs, DWORD count) = 0;

	// ���ò���(Float)
	virtual HRESULT SetFloat(D3DXHANDLE hParam, float v) = 0;

	// ���ò���(FloatArray)
	virtual HRESULT SetFloatArray(D3DXHANDLE hParam, const float* pFloats, DWORD Count) = 0;

	// ���ò���(Matrix)
	virtual HRESULT SetInt(D3DXHANDLE hParam, int v) = 0;

	// ���ò���(Matrix)
	virtual HRESULT SetBool(D3DXHANDLE hParam, BOOL v) = 0;

	// ��������
	virtual HRESULT SetTexture(D3DXHANDLE hParam, IBaseTexture* pTexture) = 0;
};	

//-------------------------------------------------------------------
// ����: IDeviceCaps
// ˵��: �豸�����ӿ�
//-------------------------------------------------------------------
class PGE_NOVTABLE IDeviceCaps
{
public:
	// �õ�VertexShader�İ汾��
	virtual DWORD GetVertexShaderVersion() = 0;

	// �õ�PixelShader�İ汾��
	virtual DWORD GetPixelShaderVersion() = 0;

	// �õ����������
	virtual DWORD GetMaxTextureWidth() = 0;

	// �õ��������߶�
	virtual DWORD GetMaxTextureHeight() = 0;

	// �õ����������ϲ���
	virtual DWORD GetMaxTextureBlendStates() = 0;

	// �õ�֧�ֵ���������
	virtual DWORD GetMaxStreams() = 0;

	// �Ƿ�֧������Ϊ(op)��������
	virtual BOOL IsSupportTextureBlendOp(D3DTEXTUREOP op) = 0;

	// �Ƿ�֧������Ϊ(mode)��AlhpaԴ���
	virtual BOOL IsSupportSrcBlendMode(D3DBLEND mode) = 0;

	// �Ƿ�֧������Ϊ(mode)��AlhpaĿ�Ļ��
	virtual BOOL IsSupportDestBlendMode(D3DBLEND mode) = 0;

	// �Ƿ�֧������������
	virtual BOOL IsSupportCubeMap() = 0;

	// �Ƿ�֧�ִ�СΪ2���ݵ�����������
	virtual BOOL IsSupportCubeMapPower2() = 0;

	// �Ƿ�֧��WBuffer
	virtual BOOL IsSupportWBuffer() = 0;

	// �Ƿ�֧�ֶ���
	virtual BOOL IsSupportDither() = 0;

	// �Ƿ�֧�����ֵƫ��
	virtual BOOL IsSupportDepthBias() = 0;

	// �Ƿ�֧�ַ�Χ���ֵƫ��
	virtual BOOL IsSupportSlopeDepthBias() = 0;

	// �Ƿ�֧�ְ�͹ӳ��
	virtual BOOL IsSupportBumpEnvMap() = 0;

	// �Ƿ�֧�ֹ��հ�͹ӳ��
	virtual BOOL IsSupportBumpEnvMapLuminance() = 0;
};

//-------------------------------------------------------------------
// ����: IEnumeration
// ˵��: ö���豸��
//-------------------------------------------------------------------
class PGE_NOVTABLE IEnumeration
{
public:
	// ������ü���
	virtual DWORD  AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD  GetRefCount() = 0;

	// �ͷ���Դ
	virtual DWORD  Release() = 0;

	// �õ�����������ʾģʽ
	virtual HRESULT GetAdapterDisplayMode(D3DDISPLAYMODE* pDM) const = 0;
};

// ���û�ʵ�֣�֪ͨ�ַ�����Ϣ�Ļص��ӿ�
struct PGE_NOVTABLE PG_Report_I
{
	// ����		:֪ͨ�ַ�����Ϣ
	// ����		:��Ϣ
	// ����ֵ	:��
	virtual void Notify( const wchar_t* _szMsg ) = 0;
};

class IGraphicDevice;
//----------------------------------------------------------------------//
// �豸�ش�ʱ�Ļص�����
//----------------------------------------------------------------------//
typedef void (CALLBACK *LPCALLBACKRESETDEVICE)(IGraphicDevice* pDevice, BOOL bIsRecreated);

//-------------------------------------------------------------------
// ����: DEVICE_CONFIG
// ˵��: �����豸ʱ����������
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
	DWORD				  AdapterIndex;					// ������������
	D3DFORMAT			  AdapterFormat;				// �������ı����ʽ
	D3DDEVTYPE			  DeviceType;					// �豸����
	D3DDISPLAYMODE		  DisplayMode;					// ��ʾģʽ
	D3DMULTISAMPLE_TYPE   MultisampleType;				// ���������ʽ
	DWORD				  MultisampleQuality;			// ��������
	D3DFORMAT			  DepthStencilBufferFormat;		// ��Ȼ���ı����ʽ
	DWORD				  PresentInterval;				// 
	BOOL			      EnableDepthStencilBuffer;		// �Ƿ�ʹ����Ȼ���
	BOOL				  Windowed;						// �Ƿ�Ϊ����ģʽ
	BOOL				  EnableDebugVS;				// �Ƿ��������Vertex Shader
	BOOL				  EnableDialog;					// ���������Ի���
	BOOL				  EnableMultiThread;			// �Ƿ�������߳�
	BOOL				  SoftwareVS;					// �Ƿ�ǿ��ʹ��������㴦��
	HWND				  hWnd;							// �����豸ʱ�Ĵ��ھ��
	DWORD				  VSVersion;					// Vertex shader�İ汾
	mutable PG_Report_I*		  pLogStream;			// �豸��־��
	mutable LPCALLBACKRESETDEVICE OnResetDeviceProc;	// �豸�����õĻص�����
	mutable LPCALLBACKRESETDEVICE OnLostDeviceProc;		// �豸��ʧʱ�Ļص�����
};

//-------------------------------------------------------------------
// ����: VIDEO_MEMROY_USAGE
// ˵��: 
//-------------------------------------------------------------------
struct VIDEO_MEMROY_USAGE
{
	UINT VertexBufferMemSize;			// Vertex buffer���Դ��С
	UINT IndexBufferMemSize;			// Index buffer���Դ��С
	UINT TextureMemSize;				// 2D������Դ��С
	UINT CubeTextureMemSize;			// ������������Դ��С
	UINT SurfaceMemSize;				// RenderTarget��DepthStencilBuffer�Ĵ�С
};

//-------------------------------------------------------------------
// ����: DEVICE_STATISTIC
// ˵��: 
//-------------------------------------------------------------------
struct DEVICE_STATICSTIC
{
	UINT DrawPrimitiveCount;		// ���˶��ٸ�ͼԪ
	UINT DrawPrimitiveCounter;		// ���˶��ٴλ�ͼԪ���ƺ���
};

typedef IDirect3DQuery9 IQuery;
//-------------------------------------------------------------------
// ����: IGraphicDevice
// ˵��: ͼ���豸�ӿ� 
//-------------------------------------------------------------------
class PGE_NOVTABLE IGraphicDevice
{
public:
	// ������ü���
	virtual DWORD AddRef() = 0;

	// �õ����ü�����ֵ
	virtual DWORD GetRefCount() const = 0;

	// �ͷ���Դ
	virtual DWORD Release() = 0;

	// �õ���ǰ���豸����
	virtual const DEVICE_CONFIG& GetDeviceConfig() const = 0;

	// ����Ĭ�ϵ���Ⱦ״̬
	virtual HRESULT SetDefaultRenderState() = 0;

	// ���öԻ���ĵ���ģʽ
	virtual HRESULT SetDialogBoxMode(BOOL bEnable) = 0;

	// ������Ⱦ״̬
	virtual HRESULT SetRenderState(D3DRENDERSTATETYPE type, DWORD value) = 0;

	// ����������̨
	virtual HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE Type, DWORD op) = 0;

	// �������������̨
	virtual HRESULT SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE Type, DWORD value) = 0;

	// ��������
	virtual HRESULT SetTexture(DWORD stage, IBaseTexture* texture) = 0;

	// �����ӿ�	
	virtual HRESULT SetViewport(DWORD x, DWORD y, DWORD w, DWORD h, float minZ , float maxZ) = 0;
	virtual HRESULT SetViewport(const OSViewport* pViewport ) = 0;

	// �õ��ӿ�
	virtual HRESULT GetViewport(OSViewport *vp) const = 0;

	// ���ö�����
	virtual HRESULT SetStreamSource(DWORD sreamNumber, IVertexBuffer* vertexBuffer, DWORD offset, DWORD strideSize) = 0;

	// ������������
	virtual HRESULT SetIndices(IIndexBuffer* indexBuffer) = 0;

	// ���ö�������
	virtual	HRESULT SetVertexDeclaration( IVertexDeclaration* pDecl ) = 0;

	// ���ö�����ߵĳ����Ĵ���
	virtual	HRESULT SetVertexShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) = 0;

	// �����������ߵĳ����Ĵ���
	virtual	HRESULT SetPixelShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) = 0;

	// ���õ��Ķ����ʽ(FVF)	
	virtual HRESULT SetFVF(DWORD fvf) = 0;

	// ����VertexShader
	virtual HRESULT SetVertexShader(IVertexShader* pVS) = 0;

	// ����PixelShader
	virtual HRESULT SetPixelShader(IPixelShader* pPS) = 0;

	// ��������
	virtual HRESULT LightEnable(DWORD index, BOOL bEnable) = 0;

	// ���ù�Դ
	virtual HRESULT SetLight(DWORD index, OSLight* light) = 0;

	// û�ò���
	virtual HRESULT SetMaterial(OSMaterial* mtl) = 0;

	// ���ü�����
	virtual HRESULT SetClipPlane(DWORD index, const float* plane) = 0;

	// ���ü������״̬
	virtual HRESULT SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus) = 0;

	// ��������λ��
	virtual void SetCursorPosition( INT X, INT Y, DWORD Flags ) = 0;

	// ������������
	virtual HRESULT SetCursorProperties( UINT XHotSpot, UINT YHotSpot, ISurface *pCursorBitmap) = 0;

	// ��ʾ���
	virtual HRESULT ShowCursor(BOOL bShow) = 0;

	// ���ñ任��
	virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, const D3DXMATRIX* matrix) = 0;

	// ����󱸻���
	virtual HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, DWORD clr, float z, DWORD stencil) = 0;

	// �����豸�ܷ�������Ⱦ
	virtual BOOL TestBeforeRender() = 0;

	// ��ʼ��Ⱦ����
	virtual HRESULT BeginScene() = 0;	

	// ������Ⱦ����
	virtual HRESULT EndScene() = 0;

	// �Ѻ󱸻���ˢ����ǰ��Ļ����
	virtual HRESULT Present( const RECT* sourceRect, const RECT* destRect	, HWND presentTarget, CONST RGNDATA *pDirtyRegion  ) = 0;

	// ͼԪ����
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

	// ����VertexBuffer
	virtual IVertexBuffer*		 CreateVertexBuffer(DWORD size, DWORD fvf,  DWORD usage, D3DPOOL pool ) = 0;

	// ����IndexBuffer
	virtual IIndexBuffer*		 CreateIndexBuffer(DWORD count, DWORD usage, D3DPOOL pool) = 0;

	// ����VertexDeclaration
	virtual	IVertexDeclaration*  CreateVertexDeclaration(const D3DVERTEXELEMENT9* elements) = 0;

	// ����VertexShader
	virtual IVertexShader*		 CreateVertexShader(LPD3DXBUFFER pCode) = 0;

	// ����PixelShader
	virtual IPixelShader*		 CreatePixelShader(LPD3DXBUFFER pCode) = 0;

	// ����StateBlock
	virtual IStateBlock*		 CreateStateBlock(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount) = 0;

	// ��������
	virtual ITexture*			 CreateTexture(DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool) = 0;

	// ���ļ������д�������
	virtual ITexture*			 CreateTexture(void* srcData, DWORD len, DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, DWORD filter, DWORD mipFilter, DWORD ColorKey, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) = 0;

	// ��������������
	virtual ICubeTexture*		 CreateCubeTexture(DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool ) = 0;

	// ���ļ������д�������������
	virtual ICubeTexture*		 CreateCubeTexture(void* srcData, DWORD len, DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt , DWORD filter, DWORD mipFilter, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) = 0;

	// ���ļ������д���Effect�ӿ�
	virtual IEffect*			 CreateEffect( LPD3DXBUFFER pCode ) = 0;

	// ������������
	virtual ISurface*			 CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch) = 0;

	// �õ��豸����
	virtual IDeviceCaps* GetDeviceCaps() = 0;

	// �õ��󱸻���ĸ�ʽ����
	virtual HRESULT GetBackBufferDesc(D3DSURFACE_DESC* pDesc) = 0;

	// ����RenderTarget
	virtual ISurface* CreateRenderTarget(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType , DWORD multisampleQuality,  BOOL bLockable) = 0;

	// ����Depth Stencil Surface
	virtual ISurface* CreateDepthStencilSurface(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType, DWORD multisampleQuality,  BOOL bDiscard ) = 0;

	// ����RenderTarget
	virtual HRESULT SetRenderTarget(ISurface* target) = 0;

	// �õ���ǰ��RenderTarget
	virtual ISurface* GetRenderTarget() = 0;

	// ����Depth Stencil Surface
	virtual HRESULT SetDepthStencilSurface(ISurface* pSurface) = 0;

	// �õ���ǰ��Depth Stencil Surface
	virtual ISurface* GetDepthStencilSurface() = 0;

	// ���ü�������
	virtual HRESULT SetScissorRect(const RECT* pRect) = 0;

	// ��ȡ��������
	virtual HRESULT GetScissorRect(RECT* pRect) = 0;

	// ����Gamma����ֵ
	virtual void SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp) = 0;

	// �õ�Gamma����ֵ
	virtual void GetGammaRamp(D3DGAMMARAMP *pRamp) = 0;

	// �õ��Դ�Ĵ�С(����AGP�Դ�;ֲ��Դ�)
	virtual UINT GetVideoMemorySize() = 0;

	// �������ö����Դ�, ���������ʹ�õ��Դ��Сָ����Сʱ
	// ����EvictManagedResources�����'Managed'���Դ���Դ
	// ֱ����ǰ�Դ�ʹ�ô�СС��ָ���Ĵ�С.
	virtual void ReserveVideoMemory(UINT Memsize) = 0;

	// �õ���ǰ�Դ��ʹ��״̬
	virtual void GetVideoMemoryUsage(VIDEO_MEMROY_USAGE* pMemUsage) = 0;

	// �õ���ǰ�Դ��ʹ������
	virtual UINT GetVideoMemroyUsage() = 0;

	// ���ձ�������Դ���Դ
	// ����: cacheTimeΪ��Դ�Ļ���ʱ��
	virtual void EvictManagedResources(UINT cacheTime) = 0;

	// ��ʼͳ����Ϣ(��:���˶��������ε�)
	virtual void BeginStat() = 0;

	// �õ�ͳ����Ϣ
	virtual void GetStatistic(DEVICE_STATICSTIC* pStatistic)= 0;

	// �����豸
	virtual void Reset(const DEVICE_CONFIG* cfg) = 0;

	// �õ�D3D���豸���
	virtual OSDevice* GetD3DDevice() = 0;

	virtual HRESULT StretchRect( ISurface * pSourceSurface,
		const RECT * pSourceRect,
		ISurface * pDestSurface,
		const RECT * pDestRect,
		D3DTEXTUREFILTERTYPE Filter) = 0;

	virtual ISurface* GetBackBuffer() = 0;

	//
	virtual HRESULT GetRenderTargetData( ISurface* pRenderTarget, ISurface* pDestSurface ) = 0;

	// ����豸��ʽ�Ƿ����
	virtual HRESULT CheckDeviceFormat( DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt ) = 0;

	// ��ȡ�ڵ���ѯ�ӿ�
	virtual IQuery* GetOcclusionQuery() = 0;

};

//-------------------------------------------------------------------
// ȫ�ֺ���
//-------------------------------------------------------------------
// �����豸
PGE_EXTERN_C DEVICE_EXPORT IGraphicDevice* WINAPI PGECreateDevice(const DEVICE_CONFIG* pConfig, int version);

// �����豸ö����
PGE_EXTERN_C DEVICE_EXPORT IEnumeration*   WINAPI PGECreateEnumeration(int version);

// ���ȫ�ֲ���
PGE_EXTERN_C DEVICE_EXPORT void WINAPI PGEDeviceCleanUp();

