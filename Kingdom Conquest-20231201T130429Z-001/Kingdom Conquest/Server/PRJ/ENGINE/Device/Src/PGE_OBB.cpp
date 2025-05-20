#include "../Include/PGE_OBB.h"
#include "../Include/PGE_Math.h"
#include <assert.h>

void PGE_OBB::GetVerts( D3DXVECTOR3* pVerts ) const
{
	assert( pVerts );
	D3DXVECTOR3 topRight	=  vXAxis * vHalfDim.x + vZAxis * vHalfDim.z;
	D3DXVECTOR3 topLeft		= -vXAxis * vHalfDim.x + vZAxis * vHalfDim.z;
	D3DXVECTOR3 bottomRight =  vXAxis * vHalfDim.x - vZAxis * vHalfDim.z;
	D3DXVECTOR3 bottomLeft  = -vXAxis * vHalfDim.x - vZAxis * vHalfDim.z;
	D3DXVECTOR3 depth		=  vHalfDim.y * vYAxis;

	pVerts[0] = vCenter + bottomLeft  - depth;
	pVerts[1] = vCenter + topLeft	  - depth;
	pVerts[2] = vCenter + topRight	  - depth;
	pVerts[3] = vCenter + bottomRight - depth;

	pVerts[4] = vCenter + bottomLeft  + depth;
	pVerts[5] = vCenter + topLeft	  + depth;
	pVerts[6] = vCenter + topRight	  + depth;
	pVerts[7] = vCenter + bottomRight + depth;
}

void PGE_OBB::Construct( const D3DXVECTOR3* points, DWORD nPoints )
{
	D3DXVECTOR3 centeroid, vMin, vMax, vBasis[3];
	PGEComputeBoundingBox( points, nPoints, centeroid, vBasis, vMin, vMax ); 

	vXAxis   = vBasis[0];
	vYAxis   = vBasis[1];
	vZAxis	 = vBasis[2];
	vHalfDim = (vMax - vMin)*0.5f;
	vCenter  = centeroid +
			   (vMax.x - vHalfDim.x) * vXAxis + 
			   (vMax.y - vHalfDim.y) * vYAxis +
			   (vMax.z - vHalfDim.z) * vZAxis;
}

void PGE_OBB::TransformByMatrix(const D3DXMATRIX& _M)
{
	D3DXVec3TransformCoord(&vCenter, &vCenter, &_M);
	D3DXVec3TransformNormal(&vXAxis, &vXAxis,  &_M);
	D3DXVec3TransformNormal(&vYAxis, &vYAxis,  &_M);
	D3DXVec3TransformNormal(&vZAxis, &vZAxis,  &_M);
}

D3DXVECTOR3& PGE_OBB::TransformCoord(  D3DXVECTOR3& _V, const D3DXVECTOR3& _Point ) const
{
	D3DXVECTOR3 temp = _Point - vCenter;
	_V.x   = D3DXVec3Dot(&temp, &vXAxis);
	_V.y   = D3DXVec3Dot(&temp, &vYAxis);
	_V.z   = D3DXVec3Dot(&temp, &vZAxis);
	return _V;
}

D3DXVECTOR3& PGE_OBB::TransformNormal( D3DXVECTOR3& _V, const D3DXVECTOR3& _Normal ) const
{
	D3DXVECTOR3 temp = _Normal;
	_V.x   = D3DXVec3Dot(&temp, &vXAxis);
	_V.y   = D3DXVec3Dot(&temp, &vYAxis);
	_V.z   = D3DXVec3Dot(&temp, &vZAxis);
	return _V;
}

D3DXPLANE& PGE_OBB::TransformPlane( D3DXPLANE& _P, const D3DXPLANE& _Plane ) const
{
	D3DXVECTOR3 n(_Plane.a, _Plane.b, _Plane.c);
	D3DXVECTOR3 p;
	PGEGetPlanePoint( p, _Plane );

	D3DXPLANE LocalPlane;
	TransformCoord(p, p);
	TransformNormal(n, n);
	D3DXPlaneFromPointNormal(&_P, &p, &n);

	return _P;
}

void PGE_OBB::Add( const D3DXVECTOR3& _V )
{
	D3DXVECTOR3 v;
	TransformCoord(v, _V);

	D3DXVECTOR3 d(0, 0, 0);
	if ( v.x > vHalfDim.x ) 
	{
		d.x  = (v.x - vHalfDim.x) * 0.5f;
		vHalfDim.x += d.x;
	}else if ( v.x < -vHalfDim.x )
	{
		d.x  = (v.x + vHalfDim.x) * 0.5f;
		vHalfDim.x -= d.x;
	}
	if ( v.y > vHalfDim.y ) 
	{
		d.y = (v.y - vHalfDim.y) * 0.5f;
		vHalfDim.y += d.y;
	}else if ( v.y < -vHalfDim.y )
	{
		d.y = ( v.y + vHalfDim.y) * 0.5f;
		vHalfDim.y -= d.y;
	}
	if ( v.z > vHalfDim.z ) 
	{
		d.z = (v.z - vHalfDim.z) * 0.5f;
		vHalfDim.z += d.z;
	}else if ( v.z < -vHalfDim.z )
	{
		d.z = (v.z + vHalfDim.z) * 0.5f;
		vHalfDim.z -= d.z;
	}

	// Move center
	vCenter += d.x * vXAxis + d.y * vYAxis + d.z * vZAxis;
}

void PGE_OBB::Add( const PGE_AABB& _AABB  )
{
	D3DXVECTOR3 v = _AABB.vMin;
	// vMin
	Add( v );

	// D3DXVECTOR3(vMin.x, vMin.y, vMax.z)
	v.z = _AABB.vMax.z;
	Add( v );

	// D3DXVECTOR3(vMin.x, vMax.y, vMin.z)
	v.y = _AABB.vMax.y;
	Add( v );

	// D3DXVECTOR3(vMax.x, vMin.y, vMin.z)
	Add( v );

	// vMax
	v = _AABB.vMax;
	Add( v );

	// D3DXVECTOR3(vMax.x, vMax.y, vMin.z)
	v.z = _AABB.vMin.z;
	Add( v );

	// D3DXVECTOR3(vMax.x, vMin.y, vMax.z)
	v.y = _AABB.vMin.y;
	Add( v );

	// D3DXVECTOR3(vMin.x, vMax.y, vMax.z)
	v.x = _AABB.vMax.x;
	Add( v );
}

void PGE_OBB::Add( const PGE_OBB& _OBB )
{
	PGE_AABB aabb1(_OBB.vCenter - _OBB.vHalfDim, _OBB.vCenter + _OBB.vHalfDim );
	PGE_AABB aabb(vCenter - vHalfDim, vCenter + vHalfDim );

	aabb.Add( aabb1 );

	vCenter    = aabb.GetCenter();
	vHalfDim   = (aabb.vMax - aabb.vMin) * 0.5f;
}

bool PGE_OBB::PointInBox(const D3DXVECTOR3& _Point) const
{
	// Transform point to local space
	D3DXVECTOR3 v;
	TransformCoord( v, _Point );

	const float EPSILON = 0.00001f;

	if ( fabs(v.x - vHalfDim.x ) > EPSILON ||
		 fabs(v.y - vHalfDim.y ) > EPSILON ||
		 fabs(v.z - vHalfDim.z ) > EPSILON )
	{
		 return false;
	}
	else
	{
		return true;
	}
}


long PGE_OBB::ClassifyPlane(const D3DXPLANE &_Plane ) const
{	
	// Transform plane to local space
	D3DXPLANE LocalPlane;
	TransformPlane( LocalPlane, _Plane );

	PGE_AABB aabb(-vHalfDim, vHalfDim);
	return aabb.ClassifyPlane( LocalPlane );
}


PGE_AABB_PLANE PGE_OBB::LineIntersect(  const D3DXVECTOR3& start
									  , const D3DXVECTOR3& end
									  , float* pDist ) const
{
	// Transform line to local space
	D3DXVECTOR3 LocalStart, LocalEnd;
	TransformCoord(LocalStart, start);
	TransformCoord(LocalEnd, end );

	PGE_AABB aabb(-vHalfDim, vHalfDim);
	return aabb.LineIntersect( LocalStart, LocalEnd, pDist );
}

