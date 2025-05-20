/***************************************************************************

Copyright (c) PixelGame( http//www.pixelgame.net ) All rights reserved.

MengXin , GongJing 2005.6.27

�������ƣ�			PGE_AABB.h	

������				����������PGE_AABB(�����󶨺�)����
	
�ض��ĸ��������⣺	��

�ض��ĸ�������ͷ��	��

�ض��ı������ã�	��
 
�ض����������ã�	��

ʹ���ߣ�			Engine �ڲ��߼���ʹ���� �� APIʹ����

ע�⣺				��
	
���ӣ�				��

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

	// ��������Χ��
	void Empty()
	{
		vMax.x = vMax.y = vMax.z = -FLT_MAX;
		vMin.x = vMin.y = vMin.z =  FLT_MAX;
	}

	// �ж������Χ�Ƿ�Ϊ��
	bool IsEmpty() const
	{
		return vMax.x < vMin.x || vMax.y < vMin.y || vMax.z < vMin.z;
	}

	// ����һ������
	void Add( const D3DXVECTOR3 &v )
	{
		if (v.x > vMax.x) vMax.x = v.x;
		if (v.y > vMax.y) vMax.y = v.y;
		if (v.z > vMax.z) vMax.z = v.z;
		if (v.x < vMin.x) vMin.x = v.x;
		if (v.y < vMin.y) vMin.y = v.y;
		if (v.z < vMin.z) vMin.z = v.z;
	}

	// ����һ����Χ��
	void Add( const PGE_AABB &box )
	{
		if (box.vMin.x < vMin.x) vMin.x = box.vMin.x;
		if (box.vMin.y < vMin.y) vMin.y = box.vMin.y;
		if (box.vMin.z < vMin.z) vMin.z = box.vMin.z;
		if (box.vMax.x > vMax.x) vMax.x = box.vMax.x;
		if (box.vMax.y > vMax.y) vMax.y = box.vMax.y;
		if (box.vMax.z > vMax.z) vMax.z = box.vMax.z;
	}

	// �õ���Χ�е�����
	D3DXVECTOR3 GetCenter() const
	{
		return ( vMax + vMin ) * 0.5f;
	}

	// �ж�һ�����Ƿ��������Χ����
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

	// �õ���Χ�еİ˸�����( ˳��Ϊ:   vMin, (vMin.x, vMin.y, vMax.z)
	//								, (vMax.x, vMin.y, vMax.z), (vMax.x, vMin.y, vMin.z)
	//								, (vMin.x, vMax.y, vMin.z), (vMin.x, vMax.y, vMax.z)
	//								,  vMax,  (vMax.x, vMax.y, vMin.z)
	void GetVerts( D3DXVECTOR3* pVerts  ) const;

	// �õ���Χ�жԽ��ߵĳ���
	float GetDiagonalLength() const 
	{
		float dx = vMax.x - vMin.x;
		float dy = vMax.y - vMin.y;
		float dz = vMax.z - vMin.z;
		return sqrtf( dx*dx +dy*dy +dz*dz ); 
	}
	
	// �任�����Χ��
	void Transform(const D3DXMATRIX& mat);

	// �����Χ�������һ����
	D3DXVECTOR3 ClosestPointTo( const D3DXVECTOR3& p ) const;

	// �ж����������һ��ƽ�����һ��
	// ����ֵ: 1(����), -1(����), 0(������ƽ��)
	long ClassifyPlane(const D3DXPLANE &p ) const;

	// �ж���������Ƿ��һ���߶��ཻ
	// ����:   start[in](���), rayDelta[in](�����ȵ�����),
	//		   pDist[out]�����ཻ�ľ��������rayDelta���ȵı���[0.0, 1.0]
	// ����ֵ: �ཻ��ƽ��
	PGE_AABB_PLANE RayIntersect(  const D3DXVECTOR3& start
								, const D3DXVECTOR3& rayDelta
								, float* pDist = 0 ) const;

	PGE_AABB_PLANE LineIntersect( const D3DXVECTOR3& start
								, const D3DXVECTOR3& end
								, float* pDist = 0 ) const
	{
		return RayIntersect( start, end - start, pDist );
	}

	// �ж���������Ƿ����һ����Χ���ཻ
	PGE_AABB_TEST IntersectBox( const PGE_AABB& _aabb, PGE_AABB* pInterBox = 0 ) const;

	// �ж�һ����Χ���Ƿ������Χ����ȫ��ס
	bool ContainBox( const PGE_AABB& _inner_box ) const;

	// �ж�һ����Χ���Ƿ�������Χ�������
	bool DepartBox( const PGE_AABB& _test_box ) const;

	//�ж�һ�� PGE_AABB �Ƿ���������������������ཻ
	PGE_AABB_TEST IntersectTri(  const D3DXVECTOR3& _v0 , 
						const D3DXVECTOR3& _v1 , 
						const D3DXVECTOR3& _v2 ) const;

	// �ж�һ����Χ���Ƿ��һ�����ཻ
	bool IntersectSphere(const D3DXVECTOR3& vOrigin, float fRadius) const;

	//�õ�����Ŀ�������ĵ�
	void GetNearestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const;

	//�õ�����Ŀ�����Զ�ĵ�
	void GetFarestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const;
};

