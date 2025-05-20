/***************************************************************************

Copyright (c) PixelGame( http//www.pixelgame.net ) All rights reserved.

MengXin , GongJing 2005.6.27

程序名称：			PGE_AABB.h	

描述：				定义了描述PGE_AABB(轴对齐绑定盒)的类
	
特定的附加依赖库：	无

特定的附加依赖头：	无

特定的编译设置：	无
 
特定的连接设置：	无

使用者：			Engine 内部逻辑类使用者 ， API使用者

注意：				无
	
例子：				无

***************************************************************************/

#pragma once 

#include "PGE_Config.h"

enum PGE_AABB_PLANE
{	
	PGE_AABB_PLANE_NONE = 0,
	PGE_AABB_PLANE_FRONT,
	PGE_AABB_PLANE_BACK,
	PGE_AABB_PLANE_LEFT,
	PGE_AABB_PLANE_RIGHT,
	PGE_AABB_PLANE_TOP,
	PGE_AABB_PLANE_BOTTOM,

	PGE_AABB_PLANE_FORCEDWORD = 0x7FFFFFFF,
};

enum PGE_AABB_TEST
{
	PGE_OUT_AABB       = 0,
	PGE_IN_AABB		   = 1,
	PGE_INTERSECT_AABB = 2,

	PGE_AABB_TEST_FORCEDWORD = 0x7fffffff,
};

struct PGE_SPHERE
{
	float		Radius;
	D3DXVECTOR3 Center;

	PGE_SPHERE() : Radius(1.0), Center(0.0f,0.0f,0.0f)
	{
	}
	PGE_SPHERE( const D3DXVECTOR3& _Center, float _Radius )
		: Radius(_Radius)
		, Center(_Center)
	{
	}
};

struct DEVICE_EXPORT PGE_AABB
{
	D3DXVECTOR3 vMin;
	D3DXVECTOR3 vMax;

	PGE_AABB() 
		: vMin( FLT_MAX , FLT_MAX , FLT_MAX ) 
		, vMax( -FLT_MAX , -FLT_MAX , -FLT_MAX )
	{
	}

	PGE_AABB(const D3DXVECTOR3& _vMin, const D3DXVECTOR3 & _vMax): vMin(_vMin), vMax(_vMax)
	{
	}

	// 清空这个包围盒
	void Empty()
	{
		vMax.x = vMax.y = vMax.z = -FLT_MAX;
		vMin.x = vMin.y = vMin.z =  FLT_MAX;
	}

	// 判断这个包围是否为空
	bool IsEmpty() const
	{
		return vMax.x < vMin.x || vMax.y < vMin.y || vMax.z < vMin.z;
	}

	// 扩充一个顶点
	void Add( const D3DXVECTOR3 &v )
	{
		if (v.x > vMax.x) vMax.x = v.x;
		if (v.y > vMax.y) vMax.y = v.y;
		if (v.z > vMax.z) vMax.z = v.z;
		if (v.x < vMin.x) vMin.x = v.x;
		if (v.y < vMin.y) vMin.y = v.y;
		if (v.z < vMin.z) vMin.z = v.z;
	}

	// 扩充一个包围盒
	void Add( const PGE_AABB &box )
	{
		if (box.vMin.x < vMin.x) vMin.x = box.vMin.x;
		if (box.vMin.y < vMin.y) vMin.y = box.vMin.y;
		if (box.vMin.z < vMin.z) vMin.z = box.vMin.z;
		if (box.vMax.x > vMax.x) vMax.x = box.vMax.x;
		if (box.vMax.y > vMax.y) vMax.y = box.vMax.y;
		if (box.vMax.z > vMax.z) vMax.z = box.vMax.z;
	}

	// 得到包围盒的中心
	D3DXVECTOR3 GetCenter() const
	{
		return ( vMax + vMin ) * 0.5f;
	}

	// 判断一个点是否在这个包围盒内
	bool PointInBox(const D3DXVECTOR3 &v) const
	{
		if  ( 	(v.x < vMin.x) || (v.x > vMax.x) ||
				(v.y < vMin.y) || (v.y > vMax.y) ||
				(v.z < vMin.z) || (v.z > vMax.z)  )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	// 得到包围盒的八个顶点( 顺序为:   vMin, (vMin.x, vMin.y, vMax.z)
	//								, (vMax.x, vMin.y, vMax.z), (vMax.x, vMin.y, vMin.z)
	//								, (vMin.x, vMax.y, vMin.z), (vMin.x, vMax.y, vMax.z)
	//								,  vMax,  (vMax.x, vMax.y, vMin.z)
	void GetVerts( D3DXVECTOR3* pVerts  ) const;

	// 得到包围盒对角线的长度
	float GetDiagonalLength() const 
	{
		float dx = vMax.x - vMin.x;
		float dy = vMax.y - vMin.y;
		float dz = vMax.z - vMin.z;
		return sqrtf( dx*dx +dy*dy +dz*dz ); 
	}
	
	// 变换这个包围盒
	void Transform(const D3DXMATRIX& mat);

	// 求与包围盒最近的一个点
	D3DXVECTOR3 ClosestPointTo( const D3DXVECTOR3& p ) const;

	// 判断这个盒子在一个平面的那一面
	// 返回值: 1(正面), -1(返面), 0(穿过此平面)
	long ClassifyPlane(const D3DXPLANE &p ) const;

	// 判断这个盒子是否和一个线段相交
	// 参数:   start[in](起点), rayDelta[in](带长度的射线),
	//		   pDist[out]最后的相交的距离相对于rayDelta长度的比率[0.0, 1.0]
	// 返回值: 相交的平面
	PGE_AABB_PLANE RayIntersect(  const D3DXVECTOR3& start
								, const D3DXVECTOR3& rayDelta
								, float* pDist = 0 ) const;

	PGE_AABB_PLANE LineIntersect( const D3DXVECTOR3& start
								, const D3DXVECTOR3& end
								, float* pDist = 0 ) const
	{
		return RayIntersect( start, end - start, pDist );
	}

	// 判断这个盒子是否和另一个包围盒相交
	PGE_AABB_TEST IntersectBox( const PGE_AABB& _aabb, PGE_AABB* pInterBox = 0 ) const;

	// 判断一个包围盒是否被这个包围盒完全包住
	bool ContainBox( const PGE_AABB& _inner_box ) const;

	// 判断一个包围盒是否和这个包围盒完分离
	bool DepartBox( const PGE_AABB& _test_box ) const;

	//判定一个 PGE_AABB 是否和由三个给定的三角形相交
	PGE_AABB_TEST IntersectTri(  const D3DXVECTOR3& _v0 , 
						const D3DXVECTOR3& _v1 , 
						const D3DXVECTOR3& _v2 ) const;

	// 判断一个包围盒是否和一个球相交
	bool IntersectSphere(const D3DXVECTOR3& vOrigin, float fRadius) const;

	//得到距离目标点最近的点
	void GetNearestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const;

	//得到距离目标点最远的点
	void GetFarestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const;
};

