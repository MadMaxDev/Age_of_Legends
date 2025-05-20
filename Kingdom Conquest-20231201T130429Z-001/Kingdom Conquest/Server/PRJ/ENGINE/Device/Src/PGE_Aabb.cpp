#include "../Include/PGE_Aabb.h"
#include "../Include/PGE_Math.h"
#include <assert.h>

inline float fast_fabs(float v)
{
	register DWORD dw;
	dw  = *(DWORD*)(&v);
	dw &= 0x7FFFFFFF;
	return *(float*)(&dw);
}

long PGE_AABB::ClassifyPlane(const D3DXPLANE &p ) const
{
	D3DXVECTOR3 center	 = (vMax + vMin)*0.5f;
	D3DXVECTOR3 halfsize = vMax - center;

	// Calculate the distance between box center and the plane
	float dist = D3DXPlaneDotCoord(&p, &center);

	// Calculate the maximise allows absolute distance for
	// the distance between box center and plane
	float maxAbsDist = fast_fabs(halfsize.x*p.a) + fast_fabs(halfsize.y*p.b) + fast_fabs(halfsize.z*p.c);

	if (dist < -maxAbsDist)
		return -1;

	if (dist > +maxAbsDist)
		return +1;

	// We straddle the plane
	return 0;
/*
	float minD, maxD;
	if (p.a > 0.0f)
	{
		minD = p.a * vMin.x;
		maxD = p.a * vMax.x;
	}else
	{
		minD = p.a * vMax.x;
		maxD = p.a * vMin.x;
	}

	if (p.b > 0.0f)
	{
		minD += p.b * vMin.y;
		maxD += p.b * vMax.y;
	}else
	{
		minD += p.b * vMax.y;
		maxD += p.b * vMin.y;
	}

	if (p.c > 0.0f)
	{ 
		minD += p.c * vMin.z;
		maxD += p.c * vMax.z;
	}else
	{
		minD += p.c * vMax.z;
		maxD += p.c * vMin.z;
	}

	float d = -p.d;
	// Check if complete on the front side of the plane
	if (minD >= d)
		return 1;

	// Check if complete on the back side of the plane
	if (maxD <= d)
		return -1;

	// We straddle the plane
	return 0;*/
}

PGE_AABB_PLANE PGE_AABB::RayIntersect( const D3DXVECTOR3& vStart
									 , const D3DXVECTOR3& rayDelta
									 , float* pDist ) const
{

	bool bInside = true;
	if ( pDist )
		*pDist = -1.0f;

	float xt = -1.0f, yt = -1.0f, zt = -1.0f;
	if (vStart.x < vMin.x)
	{
		xt = vMin.x - vStart.x;
		if (xt > rayDelta.x) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.x != 0.0f )
			xt /= rayDelta.x;
		bInside = false;
	}else if (vStart.x > vMax.x)
	{
		xt = vMax.x - vStart.x;
		if (xt < rayDelta.x) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.x != 0.0f )
			xt /= rayDelta.x;
		bInside = false;
	}

	if (vStart.y < vMin.y)
	{
		yt = vMin.y - vStart.y;
		if (yt > rayDelta.y) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.y != 0.0f )
			yt /= rayDelta.y;
		bInside = false;
	}else if (vStart.y > vMax.y)
	{
		yt = vMax.y - vStart.y;
		if (yt < rayDelta.y ) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.y != 0.0f )
			yt /= rayDelta.y;
		bInside = false;
	}

	if (vStart.z < vMin.z)
	{
		zt = vMin.z - vStart.z;
		if (zt > rayDelta.z) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.z != 0.0f )
			zt /= rayDelta.z;
		bInside =false;
	}else if (vStart.z > vMax.z)
	{
		zt = vMax.z - vStart.z;
		if (zt < rayDelta.z) return PGE_AABB_PLANE_NONE;
		if ( rayDelta.z != 0.0f )
			zt /= rayDelta.z;
		bInside =false;
	}

	// Inside box?
	if (bInside)
	{
		// yz plane
		if ( rayDelta.x > 0.0f )
			xt = (vMax.x - vStart.x)/rayDelta.x;
		else
			xt = (vMin.x - vStart.x)/rayDelta.x;

		// xz plane
		if ( rayDelta.y > 0.0f )
			yt = (vMax.y - vStart.y)/rayDelta.y;
		else
			yt = (vMin.y - vStart.y)/rayDelta.y;
	
		// xy plane
		if ( rayDelta.z > 0.0f )
			zt = (vMax.z - vStart.z)/rayDelta.z;
		else
			zt = (vMin.z - vStart.z)/rayDelta.z;

		if ( xt < yt && xt < zt )
		{
			if( pDist )
				*pDist = xt;
			return rayDelta.x > 0 ? PGE_AABB_PLANE_RIGHT : PGE_AABB_PLANE_LEFT;
		}else if ( yt < xt && yt < zt )
		{
			if( pDist )
				*pDist = yt;
			return rayDelta.y > 0 ? PGE_AABB_PLANE_TOP : PGE_AABB_PLANE_BOTTOM;
		}else
		{
			if ( pDist )
				*pDist = zt;	
			return rayDelta.z > 0 ? PGE_AABB_PLANE_FRONT : PGE_AABB_PLANE_BACK;
		}
	}

	// Select farthest plane
	long which = 0;
	float t = xt;
	if (yt > t)
	{ 
		t = yt;
		which = 1;
	}
	if (zt > t)
	{
		t = zt;
		which = 2;
	}

	if (pDist)
		*pDist = t;

	switch (which)
	{
	case 0:
		{
			// Intersect with yz plane
			float y = vStart.y + rayDelta.y * t;
			if (y < vMin.y || y > vMax.y) return PGE_AABB_PLANE_NONE;
			float z = vStart.z + rayDelta.z * t;
			if (z < vMin.z || z > vMax.z) return PGE_AABB_PLANE_NONE;

			return (vStart.x > vMax.x) ? PGE_AABB_PLANE_RIGHT : PGE_AABB_PLANE_LEFT;
		}break;
	case 1:
		{
			// Intersect with xz plane
			float x = vStart.x + rayDelta.x * t;
			if (x < vMin.x || x > vMax.x) return PGE_AABB_PLANE_NONE;
			float z = vStart.z + rayDelta.z * t;
			if (z < vMin.z || z > vMax.z) return PGE_AABB_PLANE_NONE;

			return (vStart.y > vMax.y) ? PGE_AABB_PLANE_TOP : PGE_AABB_PLANE_BOTTOM;

		}break;
	case 2:
		{	
			// Intersect with xy plane
			float x = vStart.x + rayDelta.x * t;
			if (x < vMin.x || x > vMax.x) return PGE_AABB_PLANE_NONE;
			float y = vStart.y + rayDelta.y * t;
			if (y < vMin.y || y > vMax.y) return PGE_AABB_PLANE_NONE;

			return (vStart.z > vMax.z) ? PGE_AABB_PLANE_FRONT : PGE_AABB_PLANE_BACK;
		}break;
	default:
		return PGE_AABB_PLANE_NONE;
	}

}

PGE_AABB_TEST PGE_AABB::IntersectBox( const PGE_AABB& box, PGE_AABB* pInterBox ) const
{
	if ( vMin.x > box.vMax.x ) return PGE_OUT_AABB;
	if ( vMin.y > box.vMax.y ) return PGE_OUT_AABB;
	if ( vMin.z > box.vMax.z ) return PGE_OUT_AABB;
	if ( vMax.x < box.vMin.x ) return PGE_OUT_AABB;
	if ( vMax.y < box.vMin.y ) return PGE_OUT_AABB;
	if ( vMax.z < box.vMin.z ) return PGE_OUT_AABB;

	if ( pInterBox )
	{
		pInterBox->vMin.x = __max(vMin.x, box.vMin.x);
		pInterBox->vMin.y = __max(vMin.y, box.vMin.y);
		pInterBox->vMin.z = __max(vMin.z, box.vMin.z);
		pInterBox->vMax.x = __min(vMax.x, box.vMax.x);
		pInterBox->vMax.y = __min(vMax.y, box.vMax.y);
		pInterBox->vMax.z = __min(vMax.z, box.vMax.z);
	}

	if (box.vMax.x > vMax.x ||
		box.vMax.y > vMax.y ||
		box.vMax.z > vMax.z ||
		box.vMin.x < vMin.x ||
		box.vMin.y < vMin.y ||
		box.vMin.z < vMin.z  )
	{
		return PGE_INTERSECT_AABB;
	}
	else
	{
		return PGE_IN_AABB;
	}
}

bool PGE_AABB::ContainBox( const PGE_AABB& _inner_box ) const
{
	return 
	( vMax.x >= _inner_box.vMax.x ) &&
	( vMin.x <= _inner_box.vMin.x ) &&
	( vMax.y >= _inner_box.vMax.y ) &&
	( vMin.y <= _inner_box.vMin.y ) &&
	( vMax.z >= _inner_box.vMax.z ) &&
	( vMin.z <= _inner_box.vMin.z );
}

bool PGE_AABB::DepartBox( const PGE_AABB& _test_box ) const
{
	return 
	( vMax.x < _test_box.vMin.x ) ||
	( vMin.x > _test_box.vMax.x ) ||
	( vMax.y < _test_box.vMin.y ) ||
	( vMin.y > _test_box.vMax.y ) ||
	( vMax.z < _test_box.vMin.z ) ||
	( vMin.z > _test_box.vMax.z );
}

//判定一个 PGE_AABB 是否和由三个给定的三角形相交
PGE_AABB_TEST PGE_AABB::IntersectTri( 
									const D3DXVECTOR3& _v0 , 
									const D3DXVECTOR3& _v1 , 
									const D3DXVECTOR3& _v2 ) const
{
    // By Jiangli 先进行这一步简单判断，多数情况下可直接返回。
	// 判断三角形三个顶点是否被AABB的六个平面中的一个剪载
	if ( _v0.x > vMax.x && _v1.x > vMax.x && _v2.x > vMax.x )
		return PGE_OUT_AABB;
	if ( _v0.x < vMin.x && _v1.x < vMin.x && _v2.x < vMin.x )
		return PGE_OUT_AABB;
	if ( _v0.z > vMax.z && _v1.z > vMax.z && _v2.z > vMax.z )
		return PGE_OUT_AABB;
	if ( _v0.z < vMin.z && _v1.z < vMin.z && _v2.z < vMin.z )
		return PGE_OUT_AABB;
	if ( _v0.y > vMax.y && _v1.y > vMax.y && _v2.y > vMax.y )
		return PGE_OUT_AABB;
	if ( _v0.y < vMin.y && _v1.y < vMin.y && _v2.y < vMin.y )
		return PGE_OUT_AABB;

	//先判断aabb是否和三个点构成的平面相交
	D3DXPLANE plane;
	D3DXPlaneFromPoints( &plane , &_v0 , &_v1 , &_v2 );
	if( 0 != ClassifyPlane( plane ) )
	{	
		return PGE_OUT_AABB;
	}

	// 判断三角形是否在AABB里面
	if ( !PointInBox(_v0) ||
		 !PointInBox(_v1) || 
		 !PointInBox(_v2) )
	{
		return PGE_INTERSECT_AABB;
	}
	else
	{
		return PGE_IN_AABB;
	}
}

D3DXVECTOR3 PGE_AABB::ClosestPointTo(const D3DXVECTOR3& p) const
{
	D3DXVECTOR3 r;
	if (p.x < vMin.x) 
		r.x = vMin.x;
	else if (p.x > vMax.x)
		r.x = vMax.x;
	else
		r.x = p.x;

	if (p.y < vMin.y) 
		r.y = vMin.y;
	else if (p.y > vMax.y)
		r.y = vMax.y;
	else
		r.y = p.y;

	if (p.z < vMin.z) 
		r.z = vMin.z;
	else if (p.z > vMax.z)
		r.z = vMax.z;
	else
		r.z = p.z;
	return r;
}

bool PGE_AABB::IntersectSphere(const D3DXVECTOR3& vOrigin, float fRadius) const
{
	D3DXVECTOR3 closestPoint = ClosestPointTo(vOrigin);
	return  D3DXVec3LengthSq( &(closestPoint - vOrigin) ) < fRadius * fRadius;
}

void PGE_AABB::Transform(const D3DXMATRIX& mat)
{
	if( IsEmpty() )
	{
		return;
	}

	D3DXVECTOR3 OldMin(vMin), OldMax(vMax);
	Empty();

	// OldMin
	D3DXVECTOR3 v;
	D3DXVec3TransformCoord(&v, &OldMin, &mat);
	Add( v );

	// D3DXVECTOR3(OldMin.x, OldMin.y, OldMax.z)
	v = D3DXVECTOR3(OldMin.x, OldMin.y, OldMax.z );
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );

	// D3DXVECTOR3(OldMin.x, OldMax.y, OldMin.z)
	v = D3DXVECTOR3(OldMin.x, OldMax.y, OldMin.z);
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );

	// D3DXVECTOR3(OldMax.x, OldMin.y, OldMin.z)
	v = D3DXVECTOR3( OldMax.x, OldMin.y, OldMin.z);
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );

	// OldMax
	D3DXVec3TransformCoord(&v, &OldMax, &mat);
	Add( v );

	// D3DXVECTOR3(OldMax.x, OldMax.y, OldMin.z)
	v = D3DXVECTOR3(OldMax.x, OldMax.y, OldMin.z);
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );

	// D3DXVECTOR3(OldMax.x, OldMin.y, OldMax.z)
	v = D3DXVECTOR3(OldMax.x, OldMin.y, OldMax.z);
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );

	// D3DXVECTOR3(OldMin.x, OldMax.y, OldMax.z)
	v = D3DXVECTOR3(OldMin.x, OldMax.y, OldMax.z);
	D3DXVec3TransformCoord(&v, &v, &mat);
	Add( v );
}

void PGE_AABB::GetVerts( D3DXVECTOR3* pVerts ) const
{
	assert( pVerts );

	pVerts[0] = vMin;
	pVerts[1] = D3DXVECTOR3(vMin.x, vMin.y, vMax.z);
	pVerts[2] = D3DXVECTOR3(vMax.x, vMin.y, vMax.z);
	pVerts[3] = D3DXVECTOR3(vMax.x, vMin.y, vMin.z);

	pVerts[4] = D3DXVECTOR3(vMin.x, vMax.y, vMin.z);
	pVerts[5] = D3DXVECTOR3(vMin.x, vMax.y, vMax.z);
	pVerts[6] = vMax;
	pVerts[7] = D3DXVECTOR3(vMax.x, vMax.y, vMin.z);
}

void PGE_AABB::GetNearestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const
{
	if (_vDist.x < vMin.x) 
	{
		_vNearest.x = vMin.x;
	}
	else if (_vDist.x > vMax.x)
	{
		_vNearest.x = vMax.x;
	}
	else
	{
		_vNearest.x = _vDist.x;
	}

	if (_vDist.y < vMin.y)
	{
		_vNearest.y = vMin.y;
	}
	else if (_vDist.y > vMax.y)
	{
		_vNearest.y = vMax.y;
	}
	else
	{
		_vNearest.y = _vDist.y;
	}

	if (_vDist.z < vMin.z) 
	{
		_vNearest.z = vMin.z;
	}
	else if (_vDist.z > vMax.z)
	{
		_vNearest.z = vMax.z;
	}
	else
	{
		_vNearest.z = _vDist.z;
	}
}

void PGE_AABB::GetFarestPoint( const D3DXVECTOR3& _vDist , D3DXVECTOR3& _vNearest ) const
{
	D3DXVECTOR3 c = GetCenter();
	
	if (_vDist.x < c.x) 
	{
		_vNearest.x = vMax.x;
	}
	else
	{
		_vNearest.x = vMin.x;
	}

	if (_vDist.y < c.y) 
	{
		_vNearest.y = vMax.y;
	}
	else
	{
		_vNearest.y = vMin.y;
	}

	if (_vDist.z < c.z) 
	{
		_vNearest.z = vMax.z;
	}
	else
	{
		_vNearest.z = vMin.z;
	}
}
