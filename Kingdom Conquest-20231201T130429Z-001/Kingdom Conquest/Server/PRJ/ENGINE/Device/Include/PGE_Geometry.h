#include "PGE_Graphic.h"

//-------------------------------------------------------------//
// 类名: IPGEGeometry
// 说明: 通用的几何体接口
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGEGeometry
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 得到流的数量(有可能为多流)
	virtual DWORD GetNumStreams() = 0;

	// 得到顶点的格式声明
	virtual IVertexDeclaration* GetDeclaration() = 0;

	// 得到顶点缓冲
	virtual IVertexBuffer* GetVertexBuffer(DWORD NumStream) = 0;

	// 得到每个顶点的字节数
	virtual DWORD GetNumBytesPerVertex(DWORD NumStream) = 0;

	// 得到索引缓冲(可能为空，如果图元类型图元列表那么应该会有索引缓冲)
	virtual IIndexBuffer* GetIndexBuffer() = 0;

	// 得到顶点数
	virtual DWORD GetNumVertices() = 0;

	// 得到图元数
	virtual DWORD GetNumPrimitives() = 0;

	// 得到图元的类型
	virtual D3DPRIMITIVETYPE GetPrimitiveType() = 0;

	// 绘制几何体图元
	virtual HRESULT Draw( IGraphicDevice* pDevice ) = 0;
};

//--------------------------------------------------------------------
// Name: PGECreateGeometry
// Desc: 创建几何体接口
//		dwNumVertices :  顶点数
//		dwNumPrimitives: 图元数
//		PrimitiveType:   图元类型 
//		pDecls:			 顶点声明
//		dwOptions:		 几何体缓冲池选项, 如果这个值为0,那么所有的顶点缓冲,
//						 索引缓冲都是D3DPOOL_MANAGED类型的, 如果要改变，可以通过以下宏:
//						 PGE_GEOMETRY_VB_DEFAULT(n)   (D3DPOOL_DEFAULT)
//						 PGE_GEOMERTY_VB_SYSTEMMEM(n) (D3DPOOL_SYSTEMMEM)
//						 (其中n表示几号流)
//						 PGE_GEOMETRY_IB_DEFULAT	  (D3DPOOL_DEFAULT)
//						 PGE_GEOMETRY_IB_SYSTEMMEM	  (D3DPOOL_SYSTEMMEM)	
//-------------------------------------------------------------------
DEVICE_EXPORT IPGEGeometry* WINAPI PGECreateGeometry(	IGraphicDevice* pDevice
													  , DWORD dwNumVertices
													  , DWORD dwNumPrimitives
													  , D3DPRIMITIVETYPE PrimitiveType
													  , const D3DVERTEXELEMENT9* pDecls
													  , DWORD dwOptions);
