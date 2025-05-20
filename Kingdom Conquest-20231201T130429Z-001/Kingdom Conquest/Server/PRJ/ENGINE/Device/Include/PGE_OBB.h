#pragma once
#include "PGE_Config.h"
#include "PGE_Aabb.h"

struct PGE_OBB
{
	D3DXVECTOR3 vXAxis;			// 包围盒的X轴
	D3DXVECTOR3 vYAxis;			// 包围盒的Y轴
	D3DXVECTOR3 vZAxis;			// 包围盒的Z轴

	D3DXVECTOR3 vCenter;		// 包围盒的中心
	D3DXVECTOR3 vHalfDim;		// 包围盒的大小(X, Y, Z) (注:这里的大小为半径)
								
	PGE_OBB() : vCenter(0, 0, 0)
		 	  , vXAxis(1, 0, 0)
			  , vYAxis(0, 1, 0)
			  , vZAxis(0, 0, 1)
			  , vHalfDim(0, 0, 0)
	{
	}

	// 得到包围盒的八个顶点( 顺序为:   vMin, (vMin.x, vMin.y, vMax.z)
	//								, (vMax.x, vMin.y, vMax.z), (vMax.x, vMin.y, vMin.z)
	//								, (vMin.x, vMax.y, vMin.z), (vMin.x, vMax.y, vMax.z)
	//								,  vMax,  (vMax.x, vMax.y, vMin.z)
	DEVICE_EXPORT void GetVerts( D3DXVECTOR3* pVerts  ) const;

	// 从一组顶点集合中构造这个OBB
	DEVICE_EXPORT void Construct( const D3DXVECTOR3* points, DWORD nPoints );

	// 得到OBB的转换矩阵
	inline void GetTransformMatrix( D3DXMATRIX& _M) const
	{
		_M._11 = vXAxis.x; _M._12 = vYAxis.x; _M._13 = vZAxis.x; _M._14 = 0.0f;
		_M._21 = vXAxis.y; _M._22 = vYAxis.y; _M._23 = vZAxis.y; _M._24 = 0.0f;
		_M._31 = vXAxis.z; _M._32 = vYAxis.z; _M._33 = vZAxis.z; _M._34 = 0.0f;
		_M._41 = vCenter.x;_M._42 = vCenter.y;_M._43 = vCenter.z;_M._44 = 1.0f;
	}

	// 用一个转换矩阵变换此包围盒(不能有缩放量, 只能是平移和旋转 )
	DEVICE_EXPORT void TransformByMatrix(const D3DXMATRIX& _M);

	// 把一个坐标点转化到包围盒的局部空间
	DEVICE_EXPORT D3DXVECTOR3& TransformCoord( D3DXVECTOR3& _V, const D3DXVECTOR3& _Point ) const;

	// 把一个法向量转化到包围盒的局部空间
	DEVICE_EXPORT D3DXVECTOR3& TransformNormal( D3DXVECTOR3& _V, const D3DXVECTOR3& _Normal ) const;

	// 把一个平面转化到包围盒的局部空间
	DEVICE_EXPORT D3DXPLANE& TransformPlane( D3DXPLANE& _P, const D3DXPLANE& _Plane ) const ;

	// 扩允世界坐标系的一个点
	DEVICE_EXPORT void Add( const D3DXVECTOR3& _V );

	// 扩允世界坐标系的一个AABB
	DEVICE_EXPORT void Add( const PGE_AABB& _AABB );

	// 扩允一个OBB
	DEVICE_EXPORT void Add( const PGE_OBB& _OBB );

	// 判断一个点是否在这个包围盒内
	DEVICE_EXPORT bool PointInBox(const D3DXVECTOR3& _Point) const;

	// 判断这个盒子在一个平面的那一面
	// 返回值: 1(正面), -1(返面), 0(穿过此平面)
	DEVICE_EXPORT long ClassifyPlane(const D3DXPLANE &_Plane ) const;

	// 判断这个盒子是否和一个线段相交
	// 参数:   start(起点), end(终点), pDist得到最后的相交的距离
	// 返回值: 相交的平面
	DEVICE_EXPORT PGE_AABB_PLANE LineIntersect(  const D3DXVECTOR3& start
							  	 , const D3DXVECTOR3& end
								 , float* pDist = 0 ) const;
};