#pragma  once

#include "PGE_Graphic.h"
#include "PGE_Config.h"
#include "PGE_D3D9VertexBuffer.h"
#include "PGE_D3D9IndexBuffer.h"
#include "PGE_D3D9VertexDeclaration.h"
#include "PGE_D3D9PixelShader.h"
#include "PGE_D3D9VertexShader.h"
#include "PGE_D3D9Effect.h"
#include "PGE_D3D9StateBlock.h"
#include "PGE_D3D9Surface.h"
#include "PGE_D3D9Texture.h"
#include "PGE_D3D9CubeTexture.h"

class CD3D9DeviceCaps;

class CD3D9Device: public IGraphicDevice
{
	PGE_NOCOPY_ASSIGN(CD3D9Device)
	public:
		CD3D9Device(void);
		virtual ~CD3D9Device(void);
	public:
		// 添加引用计数
		virtual DWORD AddRef() ;

		// 得到引用计数的值
		virtual DWORD GetRefCount() const ;

		// 释放资源
		virtual DWORD Release();

		// 得到当前的设备配置
		virtual const DEVICE_CONFIG& GetDeviceConfig() const { return mGraphicConfig; }

		// 设置默认的渲染状态
		virtual HRESULT SetDefaultRenderState();

		// 设置对话框的弹出模式
		virtual HRESULT SetDialogBoxMode(BOOL bEnable);

		// 设置渲染状态
		virtual HRESULT SetRenderState(D3DRENDERSTATETYPE type, DWORD value) ;

		// 设置纹理舞台
		virtual HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE Type, DWORD op) ;

		// 设置纹理采样舞台
		virtual HRESULT SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE Type, DWORD value) ;

		// 设置纹理
		virtual HRESULT SetTexture(DWORD stage, IBaseTexture* texture) ;

		// 设置视口
		virtual HRESULT SetViewport(DWORD x, DWORD y, DWORD w, DWORD h, float minZ , float maxZ);
		virtual HRESULT SetViewport(const OSViewport* pViewport );

		// 得到视口
		virtual HRESULT GetViewport(OSViewport *vp) const;

		// 设置顶点流
		virtual HRESULT SetStreamSource(DWORD sreamNumber, IVertexBuffer* vertexBuffer, DWORD offset, DWORD strideSize) ;

		// 设置索引缓冲
		virtual HRESULT SetIndices(IIndexBuffer* indexBuffer) ;

		// 设置顶点声明
		virtual	HRESULT SetVertexDeclaration( IVertexDeclaration* pDecl ) ;

		// 设置顶点管线的常数寄存器
		virtual	HRESULT SetVertexShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) ;

		// 设置象数管线的常数寄存器
		virtual	HRESULT SetPixelShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) ;

		// 设置当的顶点格式(FVF)	
		virtual HRESULT SetFVF(DWORD fvf) ;

		// 设置VertexShader
		virtual HRESULT SetVertexShader(IVertexShader* pVS);

		// 设置PixelShader
		virtual HRESULT SetPixelShader(IPixelShader* pPS);

		// 开启光照
		virtual HRESULT LightEnable(DWORD index, BOOL bEnable) ;

		// 设置光源
		virtual HRESULT SetLight(DWORD index, OSLight* light) ;

		// 没置材质
		virtual HRESULT SetMaterial(OSMaterial* mtl) ;

		// 设置剪裁面
		virtual HRESULT SetClipPlane(DWORD index, const float* plane) ;

		// 设置剪裁面的状态
		virtual HRESULT SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus) ;

		// 设置鼠标的位置
		virtual void SetCursorPosition( INT X, INT Y, DWORD Flags ) ;

		// 设置鼠标的属性
		virtual HRESULT SetCursorProperties( UINT XHotSpot, UINT YHotSpot, ISurface *pCursorBitmap) ;

		// 显示鼠标
		virtual HRESULT ShowCursor(BOOL bShow) ;

		// 设置变换阵
		virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, const D3DXMATRIX* matrix) ;

		// 清除后备缓冲
		virtual HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, DWORD clr, float z, DWORD stencil) ;

		// 测试设备能否正常渲染
		virtual BOOL TestBeforeRender() ;

		// 开始渲染场景
		virtual HRESULT BeginScene() ;	

		// 结束渲染场景
		virtual HRESULT EndScene() ;

		// 把后备缓冲刷到当前屏幕缓冲
		virtual HRESULT Present( const RECT* sourceRect, const RECT* destRect	, HWND presentTarget, CONST RGNDATA *pDirtyRegion  ) ;

		// 图元绘制
		virtual HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE  type, DWORD primitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) ;
		virtual HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
			UINT MinVertexIndex,
			UINT NumVertices,
			UINT PrimitiveCount,
			const void *pIndexData,
			D3DFORMAT IndexDataFormat,
			CONST void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride) ;
		virtual HRESULT DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE type, UINT primitiveCount, const void* streamData, UINT  bytesPerVertex, UINT  vertexCount );

		virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE  type, DWORD startIndex, DWORD primitiveCount) ;
		virtual HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE  type, DWORD baseVertexIndex, DWORD minIndex, DWORD numVertices, DWORD startIndex, DWORD count) ;
		
		// 创建VertexBuffer
		virtual IVertexBuffer*		 CreateVertexBuffer(DWORD size, DWORD fvf,  DWORD usage, D3DPOOL pool ) ;

		// 创建IndexBuffer
		virtual IIndexBuffer*		 CreateIndexBuffer(DWORD count, DWORD usage,  D3DPOOL pool) ;

		// 创建VertexDeclaration
		virtual	IVertexDeclaration*  CreateVertexDeclaration(const D3DVERTEXELEMENT9* elements);

		// 创建VertexShader
		virtual IVertexShader*		 CreateVertexShader(LPD3DXBUFFER pCode);

		// 创建PixelShader
		virtual IPixelShader*		 CreatePixelShader(LPD3DXBUFFER pCode);

		// 创建StateBlock
		virtual IStateBlock*		 CreateStateBlock(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount);

		// 创建纹理
		virtual ITexture*			 CreateTexture(DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool) ;

		// 从文件数据中创建纹理
		virtual ITexture*			 CreateTexture(void* srcData, DWORD len, DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, DWORD filter, DWORD mipFilter, DWORD ColorKey, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) ;

		// 创建立方体纹理
		virtual ICubeTexture*		 CreateCubeTexture(DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool ) ;

		// 从文件数据中创建立方体纹理
		virtual ICubeTexture*		 CreateCubeTexture(void* srcData, DWORD len, DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt , DWORD filter, DWORD mipFilter, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) ;

		// 从文件数据中创建Effect接口
		virtual IEffect*			 CreateEffect(LPD3DXBUFFER pCode);

		// 创建离屏表面
		virtual ISurface*			 CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch);

		// 得到设备能力
		virtual IDeviceCaps* GetDeviceCaps() ;

		// 得到后备缓冲的格式描述
		virtual HRESULT GetBackBufferDesc(D3DSURFACE_DESC* pDesc) ;

		// 创建RenderTarget
		virtual ISurface* CreateRenderTarget(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType , DWORD multisampleQuality,  BOOL bLockable) ;

		// 创建Depth Stencil Surface
		virtual ISurface* CreateDepthStencilSurface(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType, DWORD multisampleQuality,  BOOL bDiscard ) ;

		// 设置RenderTarget
		virtual HRESULT SetRenderTarget(ISurface* target) ;

		// 得到当前的RenderTarget
		virtual ISurface* GetRenderTarget() ;

		// 设置Depth Stencil Surface
		virtual HRESULT SetDepthStencilSurface(ISurface* pSurface);

		// 得到当前的Depth Stencil Surface
		virtual ISurface* GetDepthStencilSurface();

		// 设置剪切区域
		virtual HRESULT SetScissorRect(const RECT* pRect);

		// 获取剪切区域
		virtual HRESULT GetScissorRect(RECT* pRect);

		// 设置Gamma修正值
		virtual void SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp);

		// 得到Gamma修正值
		virtual void GetGammaRamp(D3DGAMMARAMP *pRamp);

		// 重置设备
		virtual void Reset(const DEVICE_CONFIG* cfg);

		// 得到显存的大小(包括AGP显存和局部显存)
		virtual UINT GetVideoMemorySize();

		// 程序能用多少显存
		virtual void ReserveVideoMemory(UINT Memsize) { mReserveMemSize = Memsize; }

		// 得到当前显存的使用状态
		virtual void GetVideoMemoryUsage(VIDEO_MEMROY_USAGE* pMemUsage);

		// 得到当前显存的使用总量
		virtual UINT GetVideoMemroyUsage();

		// 回收被管理的显存资源
		virtual void EvictManagedResources(UINT cacheTime);

		// 开始统计信息
		virtual void BeginStat();

		// 得到统计信息
		virtual void GetStatistic(DEVICE_STATICSTIC* pStatistic);

		// 得到D3D的设备接口
		virtual OSDevice* GetD3DDevice()  { return mpD3DDevice; }

		//
		virtual HRESULT StretchRect( ISurface * pSourceSurface,
			const RECT * pSourceRect,
			ISurface * pDestSurface,
			const RECT * pDestRect,
			D3DTEXTUREFILTERTYPE Filter);

		virtual ISurface* GetBackBuffer();
		//
		virtual HRESULT GetRenderTargetData( ISurface* pRenderTarget, ISurface* pDestSurface );

		// 检测设备格式是否可用
		virtual HRESULT CheckDeviceFormat( DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt );

		// 获取遮挡查询接口
		virtual IQuery* GetOcclusionQuery();

		// 内部函数
		void OnLostDevice(BOOL bRecreate);
		void OnResetDevice(BOOL bRecreate);
		BOOL Initialize(OSD3D* pD3D, const DEVICE_CONFIG* cfg);	
		void LogMessage(const wchar_t* format, ...);

		D3D9SurfaceMgr* GetSurfaceMgr() const { return mpSurfaceManager; }
	protected:
		void CreateDefaultStateBlock();
		void CreateDynVBAndIB();
		BOOL InitDevice(const DEVICE_CONFIG* pConfig);
		void BuildPresentParameters(const DEVICE_CONFIG* pConfig);
		void EvictDynamicVBAndIB(DWORD cacheTime);
		
		IDirect3DVertexBuffer9* GetDynVB( DWORD nSize );
		IDirect3DIndexBuffer9*  GetDynIB( DWORD nSize );
		struct DynVB
		{
			DynVB()
			{
				Index = 0;
				pVB[0] = NULL;
				pVB[1] = NULL;
			}
			~DynVB()
			{
				SAFE_RELEASE(pVB[0]);
				SAFE_RELEASE(pVB[1]);
			}
			void clear()
			{
				SAFE_RELEASE(pVB[0]);
				SAFE_RELEASE(pVB[1]);
				Index = 0;
			}
			IDirect3DVertexBuffer9* pVB[2];
			int						Index;
			DWORD					LastAccess[2];
		};
		struct DynIB
		{
			DynIB()
			{
				Index = 0;
				pIB[0] = NULL;
				pIB[1] = NULL;
			}
			~DynIB()
			{
				SAFE_RELEASE(pIB[0]);
				SAFE_RELEASE(pIB[1]);
			}
			void clear()
			{
				SAFE_RELEASE(pIB[0]);
				SAFE_RELEASE(pIB[1]);
				Index = 0;
			}
			IDirect3DIndexBuffer9*  pIB[2];
			int						Index;
			DWORD					LastAccess[2];
		};
		enum {MIN_DYNVB_LEVEL = 4, MAX_DYNVB_LEVEL = 24,
			  NUM_DYNVB_LEVEL = MAX_DYNVB_LEVEL - MIN_DYNVB_LEVEL,
			  MIN_DYNIB_LEVEL = 4, MAX_DYNIB_LEVEL = 24,
			  NUM_DYNIB_LEVEL = MAX_DYNIB_LEVEL - MIN_DYNIB_LEVEL};

		D3DPRESENT_PARAMETERS	    md3dpp;
		DEVICE_CONFIG				mGraphicConfig;

		// 系统设备
		OSD3D*						mpD3D;	
		OSDevice*					mpD3DDevice;
		CD3D9DeviceCaps*			mpDeviceCaps;
		OSStateBlock*				mpDefaultStateBlock;

		// 内部管理器
		D3D9VertexBufferMgr			*mpVBManager;	
		D3D9IndexBufferMgr			*mpIBManager;
		D3D9SurfaceMgr				*mpSurfaceManager;
		D3D9TextureMgr				*mpTextureManager;
		D3D9CubeTextureMgr			*mpCubeTextureManager;
		D3D9EffectMgr				*mpEffectMgr;
		D3D9VertexDeclarationMgr	*mpDeclMgr;
		D3D9StateBlockMgr			*mpStateBlockMgr;
		D3D9VertexShaderMgr			*mpVertexShaderMgr;
		D3D9PixelShaderMgr			*mpPixelShaderMgr;

		DynVB						m_pDynVBs[NUM_DYNVB_LEVEL];
		DynIB						m_pDynIBs[NUM_DYNIB_LEVEL];
	
		std::vector< IDeviceResource* > mManagedRes;
		UINT						mVideoMemSize;
		UINT						mReserveMemSize;
		IQuery*						mpOccQuery;


		DEVICE_STATICSTIC			mStatistic;
		DWORD						mTickCount;
		BOOL						mResourceIsLost;
		LONG						mRefCount;
		BOOL						mbDeviceValid;
		HRESULT						mHR;		// 临时变量
};	