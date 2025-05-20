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
		// ������ü���
		virtual DWORD AddRef() ;

		// �õ����ü�����ֵ
		virtual DWORD GetRefCount() const ;

		// �ͷ���Դ
		virtual DWORD Release();

		// �õ���ǰ���豸����
		virtual const DEVICE_CONFIG& GetDeviceConfig() const { return mGraphicConfig; }

		// ����Ĭ�ϵ���Ⱦ״̬
		virtual HRESULT SetDefaultRenderState();

		// ���öԻ���ĵ���ģʽ
		virtual HRESULT SetDialogBoxMode(BOOL bEnable);

		// ������Ⱦ״̬
		virtual HRESULT SetRenderState(D3DRENDERSTATETYPE type, DWORD value) ;

		// ����������̨
		virtual HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE Type, DWORD op) ;

		// �������������̨
		virtual HRESULT SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE Type, DWORD value) ;

		// ��������
		virtual HRESULT SetTexture(DWORD stage, IBaseTexture* texture) ;

		// �����ӿ�
		virtual HRESULT SetViewport(DWORD x, DWORD y, DWORD w, DWORD h, float minZ , float maxZ);
		virtual HRESULT SetViewport(const OSViewport* pViewport );

		// �õ��ӿ�
		virtual HRESULT GetViewport(OSViewport *vp) const;

		// ���ö�����
		virtual HRESULT SetStreamSource(DWORD sreamNumber, IVertexBuffer* vertexBuffer, DWORD offset, DWORD strideSize) ;

		// ������������
		virtual HRESULT SetIndices(IIndexBuffer* indexBuffer) ;

		// ���ö�������
		virtual	HRESULT SetVertexDeclaration( IVertexDeclaration* pDecl ) ;

		// ���ö�����ߵĳ����Ĵ���
		virtual	HRESULT SetVertexShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) ;

		// �����������ߵĳ����Ĵ���
		virtual	HRESULT SetPixelShaderConstantF( DWORD _Index , const float* _pData , DWORD _CntVec4 ) ;

		// ���õ��Ķ����ʽ(FVF)	
		virtual HRESULT SetFVF(DWORD fvf) ;

		// ����VertexShader
		virtual HRESULT SetVertexShader(IVertexShader* pVS);

		// ����PixelShader
		virtual HRESULT SetPixelShader(IPixelShader* pPS);

		// ��������
		virtual HRESULT LightEnable(DWORD index, BOOL bEnable) ;

		// ���ù�Դ
		virtual HRESULT SetLight(DWORD index, OSLight* light) ;

		// û�ò���
		virtual HRESULT SetMaterial(OSMaterial* mtl) ;

		// ���ü�����
		virtual HRESULT SetClipPlane(DWORD index, const float* plane) ;

		// ���ü������״̬
		virtual HRESULT SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus) ;

		// ��������λ��
		virtual void SetCursorPosition( INT X, INT Y, DWORD Flags ) ;

		// ������������
		virtual HRESULT SetCursorProperties( UINT XHotSpot, UINT YHotSpot, ISurface *pCursorBitmap) ;

		// ��ʾ���
		virtual HRESULT ShowCursor(BOOL bShow) ;

		// ���ñ任��
		virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, const D3DXMATRIX* matrix) ;

		// ����󱸻���
		virtual HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, DWORD clr, float z, DWORD stencil) ;

		// �����豸�ܷ�������Ⱦ
		virtual BOOL TestBeforeRender() ;

		// ��ʼ��Ⱦ����
		virtual HRESULT BeginScene() ;	

		// ������Ⱦ����
		virtual HRESULT EndScene() ;

		// �Ѻ󱸻���ˢ����ǰ��Ļ����
		virtual HRESULT Present( const RECT* sourceRect, const RECT* destRect	, HWND presentTarget, CONST RGNDATA *pDirtyRegion  ) ;

		// ͼԪ����
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
		
		// ����VertexBuffer
		virtual IVertexBuffer*		 CreateVertexBuffer(DWORD size, DWORD fvf,  DWORD usage, D3DPOOL pool ) ;

		// ����IndexBuffer
		virtual IIndexBuffer*		 CreateIndexBuffer(DWORD count, DWORD usage,  D3DPOOL pool) ;

		// ����VertexDeclaration
		virtual	IVertexDeclaration*  CreateVertexDeclaration(const D3DVERTEXELEMENT9* elements);

		// ����VertexShader
		virtual IVertexShader*		 CreateVertexShader(LPD3DXBUFFER pCode);

		// ����PixelShader
		virtual IPixelShader*		 CreatePixelShader(LPD3DXBUFFER pCode);

		// ����StateBlock
		virtual IStateBlock*		 CreateStateBlock(const PGE_RS_VALUE* pRSValues, DWORD dwRSCount, const PGE_TSS_VALUE* pTSSValues, DWORD dwTSSCount, PGE_SS_VALUE* pSSValues, DWORD dwSSCount);

		// ��������
		virtual ITexture*			 CreateTexture(DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool) ;

		// ���ļ������д�������
		virtual ITexture*			 CreateTexture(void* srcData, DWORD len, DWORD width, DWORD height, DWORD level, DWORD usages, D3DFORMAT fmt, DWORD filter, DWORD mipFilter, DWORD ColorKey, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) ;

		// ��������������
		virtual ICubeTexture*		 CreateCubeTexture(DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt, D3DPOOL pool ) ;

		// ���ļ������д�������������
		virtual ICubeTexture*		 CreateCubeTexture(void* srcData, DWORD len, DWORD size, DWORD level, DWORD usages, D3DFORMAT fmt , DWORD filter, DWORD mipFilter, D3DPOOL pool, D3DXIMAGE_INFO* pSrcInfo) ;

		// ���ļ������д���Effect�ӿ�
		virtual IEffect*			 CreateEffect(LPD3DXBUFFER pCode);

		// ������������
		virtual ISurface*			 CreateOffscreenPlainSurface( DWORD width, DWORD height, D3DFORMAT fmt, BOOL bScratch);

		// �õ��豸����
		virtual IDeviceCaps* GetDeviceCaps() ;

		// �õ��󱸻���ĸ�ʽ����
		virtual HRESULT GetBackBufferDesc(D3DSURFACE_DESC* pDesc) ;

		// ����RenderTarget
		virtual ISurface* CreateRenderTarget(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType , DWORD multisampleQuality,  BOOL bLockable) ;

		// ����Depth Stencil Surface
		virtual ISurface* CreateDepthStencilSurface(DWORD width, DWORD height, D3DFORMAT format, D3DMULTISAMPLE_TYPE  multisampleType, DWORD multisampleQuality,  BOOL bDiscard ) ;

		// ����RenderTarget
		virtual HRESULT SetRenderTarget(ISurface* target) ;

		// �õ���ǰ��RenderTarget
		virtual ISurface* GetRenderTarget() ;

		// ����Depth Stencil Surface
		virtual HRESULT SetDepthStencilSurface(ISurface* pSurface);

		// �õ���ǰ��Depth Stencil Surface
		virtual ISurface* GetDepthStencilSurface();

		// ���ü�������
		virtual HRESULT SetScissorRect(const RECT* pRect);

		// ��ȡ��������
		virtual HRESULT GetScissorRect(RECT* pRect);

		// ����Gamma����ֵ
		virtual void SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp);

		// �õ�Gamma����ֵ
		virtual void GetGammaRamp(D3DGAMMARAMP *pRamp);

		// �����豸
		virtual void Reset(const DEVICE_CONFIG* cfg);

		// �õ��Դ�Ĵ�С(����AGP�Դ�;ֲ��Դ�)
		virtual UINT GetVideoMemorySize();

		// �������ö����Դ�
		virtual void ReserveVideoMemory(UINT Memsize) { mReserveMemSize = Memsize; }

		// �õ���ǰ�Դ��ʹ��״̬
		virtual void GetVideoMemoryUsage(VIDEO_MEMROY_USAGE* pMemUsage);

		// �õ���ǰ�Դ��ʹ������
		virtual UINT GetVideoMemroyUsage();

		// ���ձ�������Դ���Դ
		virtual void EvictManagedResources(UINT cacheTime);

		// ��ʼͳ����Ϣ
		virtual void BeginStat();

		// �õ�ͳ����Ϣ
		virtual void GetStatistic(DEVICE_STATICSTIC* pStatistic);

		// �õ�D3D���豸�ӿ�
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

		// ����豸��ʽ�Ƿ����
		virtual HRESULT CheckDeviceFormat( DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt );

		// ��ȡ�ڵ���ѯ�ӿ�
		virtual IQuery* GetOcclusionQuery();

		// �ڲ�����
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

		// ϵͳ�豸
		OSD3D*						mpD3D;	
		OSDevice*					mpD3DDevice;
		CD3D9DeviceCaps*			mpDeviceCaps;
		OSStateBlock*				mpDefaultStateBlock;

		// �ڲ�������
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
		HRESULT						mHR;		// ��ʱ����
};	