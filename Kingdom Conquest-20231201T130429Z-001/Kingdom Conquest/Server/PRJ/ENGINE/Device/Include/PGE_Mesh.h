#pragma once

#include "PGE_Config.h"

struct PGE_ATTRIBUTE_RANGE
{
	DWORD FaceStart;
	DWORD FaceCount;
	DWORD VertexStart;
	DWORD VertexCount;
};

//-------------------------------------------------------------//
// 类名: IPGEMesh
// 说明: 三角形网格模型接口，其内部采用的是单流的格式
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGEMesh
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 得到顶点缓冲
	virtual IVertexBuffer* GetVertexBuffer() = 0;

	// 得到索引缓冲(可能为空，如果图元类型图元列表那么应该会有索引缓冲)
	virtual IIndexBuffer* GetIndexBuffer() = 0;

	// 得到顶点的格式声明
	virtual IVertexDeclaration* GetDeclaration() = 0;

	// 得到固定顶点格式声明(如果函数失败返回值为0)
	virtual DWORD GetFVF() = 0;

	// 得到每个顶点的字节数
	virtual DWORD GetNumBytesPerVertex() = 0;

	// 得到顶点数
	virtual DWORD GetNumVertices() = 0;

	// 得到三角形数
	virtual DWORD GetNumFaces() = 0;

	// 绘制几何体图元的子部分
	virtual HRESULT DrawSubset( IGraphicDevice* pDevice, DWORD nID ) = 0;

	// 绘制几何体图元的所有子部分
	virtual HRESULT Draw( IGraphicDevice* pDevice ) = 0;

	// 计算包围盒
	virtual void ComputeBoundingBox(const D3DXMATRIX* pWorldMatrix, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax) = 0;

	// 设置属性表
	virtual void SetAttributeTable( CONST PGE_ATTRIBUTE_RANGE * pAttribTable, DWORD cAttribTableSize ) = 0;

	// 得到属性表的大小
	virtual DWORD GetAttributeTableSize() = 0;

	// 得到属性表
	virtual const PGE_ATTRIBUTE_RANGE* GetAttributeTable() = 0;
};

//-------------------------------------------------------------//
// 类名: IPGESkinInfo
// 说明:
//-------------------------------------------------------------//
class IPGESkinInfo
{
public:
	// 添加引用计数
	virtual ULONG  AddRef() = 0;

	// 释放资源
	virtual ULONG  Release() = 0;

	// 得到骨骼模型受影响的骨头数
	virtual DWORD GetNumBones()				       = 0;

	// 设置骨骼模型受影响的骨头数
	// NOTE: 此函数同时分配了原始骨头索引数据
	virtual void  SetNumBones(DWORD dwNumBones)    = 0;	

	// 得到原始的骨头索引数据
	virtual WORD* GetRawBoneIndices()		       = 0;

	// 根据原始的骨头ID得到新的骨头ID
	// 返回值： 如果没有找到的话，返回值为-1
	virtual WORD   GetRawBoneIndex( BYTE idx )     = 0;

	// 根据原始的骨头ID得到新的骨头ID
	virtual short  GetNewBoneIndex( WORD boneId )  = 0;

	// 重新映射新的骨头索引数据
	// pRawIndices是原始的骨头数据，其值可能是离散的
	// 因为显卡的限制，我们不能同时处理很多骨头，因此我
	// 们根据原始的骨头索引重新生成一个骨头ID连续的索引数据pNewIndices

	// 返回值：如果这个模型受影响的骨头数大于256个会失败
	virtual BOOL  RemapBlendIndices( WORD* pRawIndics, BYTE* pNewIndices, DWORD dwNumVerts ) = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateSkinInfo
// Desc : 
//--------------------------------------------------------------------
DEVICE_EXPORT  IPGESkinInfo* WINAPI PGECreateSkinInfo();

//--------------------------------------------------------------------
// Name : PGECreateMesh
// Desc : 创建网格模型 
//--------------------------------------------------------------------
DEVICE_EXPORT  IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice, DWORD dwNumVerts, DWORD dwNumFaces, DWORD dwFVF, DWORD dwOptions);
DEVICE_EXPORT  IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice, DWORD dwNumVerts, DWORD dwNumFaces, const D3DVERTEXELEMENT9* pDecls, DWORD dwOptions);


DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateSphere( IGraphicDevice* pDevice, DWORD dwNumRings, DWORD dwNumSegments, DWORD dwOptions );
DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateBox( IGraphicDevice* pDevice, DWORD dwOptions);
DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateCylinder(IGraphicDevice* pDevice, DWORD dwNumStacks, DWORD dwOptions );
DEVICE_EXPORT	void	  WINAPI PGEComputeBoundingBox(const D3DXVECTOR3* pPositions, DWORD dwNumVerts, DWORD dwVertexStride, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax);



