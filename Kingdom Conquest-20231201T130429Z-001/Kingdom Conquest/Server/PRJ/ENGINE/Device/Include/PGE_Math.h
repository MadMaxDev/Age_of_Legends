#pragma once

#include "PGE_Config.h"

//----------------------------------------------------------------------//
// 结构定义
//----------------------------------------------------------------------//
#pragma pack(push, 4)

struct PGE_TRIMESH_DESC
{
	VOID* pIndices;
	VOID* pVerts;
	DWORD dwNumVerts;
	DWORD dwNumFaces;
	BOOL  bBits16Indices;
	DWORD dwVertexStride;
};
#pragma pack(pop)

typedef unsigned short D3DINDEX; 

//-------------------------------------------------------------------
// 全局变量
//-------------------------------------------------------------------
DEVICE_EXPORT extern const D3DXMATRIX	   g_MatIdentity;
DEVICE_EXPORT extern const D3DXVECTOR3	   g_Vec3Zero;
DEVICE_EXPORT extern const D3DXVECTOR3	   g_Vec3UnitX;
DEVICE_EXPORT extern const D3DXVECTOR3	   g_Vec3UnitY;
DEVICE_EXPORT extern const D3DXVECTOR3	   g_Vec3UnitZ;
DEVICE_EXPORT extern const D3DXQUATERNION g_QuatIdentity;

//--------------------------------------------------------------------
// Desc: 重载操作符'*'实现两三维向量相乘
//--------------------------------------------------------------------
inline D3DXVECTOR3 operator * (const D3DXVECTOR3& _a , const D3DXVECTOR3& _b )
{
	return D3DXVECTOR3( _a.x * _b.x , _a.y * _b.y , _a.z * _b.z );
}

//--------------------------------------------------------------------
// Desc: 重载操作符'*'实现两三维向量相除
//--------------------------------------------------------------------
inline D3DXVECTOR3 operator / (const D3DXVECTOR3& _a , const D3DXVECTOR3& _b )
{
	return D3DXVECTOR3( _a.x / _b.x , _a.y / _b.y , _a.z / _b.z );
}

//--------------------------------------------------------------------
// Desc: 重载操作符'*'实现三维向量被一个矩阵变换
//--------------------------------------------------------------------
inline D3DXVECTOR3 operator * ( const D3DXVECTOR3& _V , const D3DXMATRIX& _M )
{
	D3DXVECTOR3 v;
	D3DXVec3TransformCoord( &v , &_V , &_M );
	return v;
}

//----------------------------------------------------------------------//
// Desc: 
//----------------------------------------------------------------------// 
inline D3DXVECTOR3& PGEVec3MultiplyQuat( D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXQUATERNION& q)
{
	D3DXMATRIX	M;
	D3DXMatrixRotationQuaternion( &M , &q );
	D3DXVec3TransformCoord(&v0, &v1, &M);
	return v0;
}


//----------------------------------------------------------------------//
// Desc: 
//----------------------------------------------------------------------// 
inline D3DXMATRIX& PGEMatrixRotationTranslation(D3DXMATRIX& m, const D3DXQUATERNION& q, const D3DXVECTOR3& v)
{
	D3DXMatrixRotationQuaternion(&m, &q);
	m._41 = v.x;
	m._42 = v.y;
	m._43 = v.z;
	return m;
}

//--------------------------------------------------------------------
// Desc: 重载操作符'*'实现三维向量被一个四元数变换
//--------------------------------------------------------------------
inline D3DXVECTOR3 operator * ( const D3DXVECTOR3& _V , const D3DXQUATERNION& _Q )
{
	D3DXMATRIX	M;
	D3DXMatrixRotationQuaternion( &M , &_Q );
	return _V * M;
}


//--------------------------------------------------------------------
// Name: PGERoundUpTo2N
// Desc: 
//--------------------------------------------------------------------
inline unsigned int PGERoundUpTo2N(unsigned int s)
{
	unsigned int news = 1;
	while (news < s)
	{
		news *= 2;
	}
	return news;
}



//--------------------------------------------------------------------
// Name: PGEIntersecterLineSegment
// Desc: 
//--------------------------------------------------------------------
inline bool PGEIntersecterLineSegment(	  const D3DXPLANE&		_p
									  , const D3DXVECTOR3&	_a 
									  , const D3DXVECTOR3&	_b
									  , float& _k )
{
	/*
	1 )
	_p.a * ( _a.x + ( _b.x - _a.x ) * K ) +
	_p.b * ( _a.y + ( _b.x - _a.x ) * K ) +
	_p.c * ( _a.z + ( _b.x - _a.x ) * K ) +
	_p.d = 0;

	2 )
	_p.a * _a.x + x * ( _b.x - _a.x ) * K + 
	_p.b * _a.y + y * ( _b.y - _a.y ) * K + 
	_p.c * _a.z + z * ( _b.z - _a.z ) * K + 
	_p.d = 0;

	3 )
	( _p.a * ( _b.x - _a.x ) + _p.b * ( _b.y - _a.y ) + _p.c * ( _b.z - _a.z ) ) * K + 
	_p.a * _a.x + _p.b * _a.y + _p.c * _a.z + p.d = 0

	4 )
	u => ( _p.a * ( _b.x - _a.x ) + _p.b * ( _b.y - _a.y ) + _p.c * ( _b.z - _a.z ) )
	v => _p.a * _a.x + _p.b * _a.y + _p.c * _a.z + p.d 

	K = -v / u
	*/

	D3DXVECTOR3 p( _p.a , _p.b , _p.c );
	D3DXVECTOR3 l( _b - _a );
	float u = D3DXVec3Dot(&p, &l);
	float v = D3DXVec3Dot(&p, &_a) + _p.d;
	if( fabsf( u ) < 1e-6f )
	{
		return false;
	}
	_k = -v / u;
	return true;
}

//--------------------------------------------------------------------
// Name: PGEIntersecterLineSegment
// Desc: 
//--------------------------------------------------------------------
inline float PGEIntersecterLineSegment(   const D3DXPLANE&		_p
									   , const D3DXVECTOR3&	_a 
									   , const D3DXVECTOR3&	_b )
{
	D3DXVECTOR3 p( _p.a , _p.b , _p.c );
	D3DXVECTOR3 l( _b - _a );
	float u	= D3DXVec3Dot(&p, &l);
	float v = D3DXVec3Dot(&p, &_a) + _p.d;
	return -v / u;
}

//--------------------------------------------------------------------
// Name: PGEFindNearestPointOnLineSegment
// Desc: 
//-------------------------------------------------------------------
inline void PGEFindNearestPointOnLineSegment( const D3DXVECTOR3& _A ,
											 const D3DXVECTOR3& _B ,
											 const D3DXVECTOR3& _P ,
											 D3DXVECTOR3& _NP )
{
	float	Lx = _B.x - _A.x;
	float	Ly = _B.y - _A.y;
	float	Lz = _B.z - _A.z;

	float D = Lx * Lx + Ly * Ly + Lz * Lz;

	if ( D < 1e-9f )
	{
		_NP = _A;
		return;
	}

	float a = ( Lx * ( _P.x - _A.x ) +
		Ly * ( _P.y - _A.y ) +
		Lz * ( _P.z - _A.z ) ) / D;

	if( a < 0.0f )
	{
		a = 0.0f;
	}
	if( a > 1.0f )
	{
		a = 1.0f;
	}

	_NP.x = _A.x + a * Lx;
	_NP.y = _A.y + a * Ly;
	_NP.z = _A.z + a * Lz;
}

//--------------------------------------------------------------------
// Name: PGEPlaneIntersectRay
// Desc: 
//--------------------------------------------------------------------
inline float WINAPI PGEPlaneIntersectRay(const D3DXPLANE& plane, const D3DXVECTOR3& vOrigin, const D3DXVECTOR3& vDir)
{
	float denominator = D3DXPlaneDotNormal(&plane, &vDir);
	if (fabs(denominator) < 0.0001f )
	{
		return 0.0f;
	}else
	{	
		float numerator = D3DXPlaneDotCoord(&plane, &vOrigin);
		return -numerator/denominator;
	}

}
//--------------------------------------------------------------------
// Name: PGEGetPlanePoint
// Desc: 
//--------------------------------------------------------------------
inline BOOL WINAPI PGEGetPlanePoint( D3DXVECTOR3& v, const D3DXPLANE& plane)
{
	const float EPSILON = 0.00001f;

	if ( fabs(plane.a) > EPSILON )
	{
		v.y = v.z = 0;
		v.x = -plane.d/plane.a;
		return TRUE;
	}else if ( fabs(plane.b) > EPSILON )
	{
		v.x = v.z = 0;
		v.y = -plane.d/plane.b;	
		return TRUE;
	}else if ( fabs( plane.c ) > EPSILON )
	{
		v.x = v.y = 0;
		v.z = -plane.d/plane.c;	
		return TRUE;
	}else
	{
		return FALSE;
	}
}

inline void WINAPI PGEMoveProjNearPlane( D3DXMATRIX& MatProj, float Dist )
{
	float zNear = -MatProj._43/MatProj._33;
	float zFar = MatProj._33 * zNear/(MatProj._33 - 1.0f);

	zNear += Dist;
	zFar  += Dist;

	MatProj._33 = zFar/(zFar - zNear);
	MatProj._43 = -MatProj._33 * zNear;
}

//--------------------------------------------------------------------
// Name: PGE_Blend_T
// Desc: 
//--------------------------------------------------------------------
template< typename T > 
inline T PGE_Blend_T( float K , const T& _V1 , const T& _V2 )
{
	return _V1 * ( 1.0f - K ) + _V2 * K;
}

template<> 
inline D3DXCOLOR PGE_Blend_T< D3DXCOLOR >( float K , const D3DXCOLOR& _V1 , const D3DXCOLOR& _V2 )
{
	D3DXCOLOR Col;
	D3DXColorLerp( &Col , &_V1 , &_V2 , K );
	return Col;
}

template<> 
inline D3DXQUATERNION PGE_Blend_T< D3DXQUATERNION >( float K , const D3DXQUATERNION& _V1 , const D3DXQUATERNION& _V2 )
{
	D3DXQUATERNION q;
	D3DXQuaternionSlerp( &q , &_V1 , &_V2 , K );
	return q;
}

//--------------------------------------------------------------------
// Name: PGEClamp
// Desc: 
//--------------------------------------------------------------------
template < typename T >
inline void PGEClamp(T& Val, const T& Min, const T& Max)
{
	if ( Val > Max)		  Val = Max;
	else if ( Val < Min ) Val = Min;
}

//--------------------------------------------------------------------
// Name: PGEColorRefToD3DXColor
// Desc: 
//--------------------------------------------------------------------
inline D3DXCOLOR PGEColorRefToD3DXColor( COLORREF _C , float _a = 1.0f )
{
	D3DXCOLOR Col;
	Col.a = _a;
	Col.r = GetRValue( _C ) / 255.0f;
	Col.g = GetGValue( _C ) / 255.0f;
	Col.b = GetBValue( _C ) / 255.0f;
	return Col;
}

//--------------------------------------------------------------------
// Name: PGED3DColorToColorRef
// Desc: 
//--------------------------------------------------------------------
inline COLORREF PGED3DXColorToColorRef( D3DXCOLOR _C )
{
	return RGB( _C.r * 255 ,  _C.g * 255 ,  _C.b * 255 );
}

extern "C"
{
	//--------------------------------------------------------------------
	// Name: PGEQuaternionToEulerAngle
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEQuaternionToEulerAngle(const D3DXQUATERNION& q, float &Yaw, float& Pitch, float& Roll);

	//--------------------------------------------------------------------
	// Name: PGEQuaternionAxisToAxis
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT D3DXQUATERNION* WINAPI PGEQuaternionAxisToAxis(   D3DXQUATERNION* _pQ 
		, const D3DXVECTOR3* _From 
		, const D3DXVECTOR3* _To );

	//--------------------------------------------------------------------
	// Name: PGEMatrixToEulerAngle
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEMatrixToEulerAngle(const D3DXMATRIX& mat, float& Yaw, float& Pitch, float& Roll);

	//--------------------------------------------------------------------
	// Name: PGEMakeCubeMapViewMatrix
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT D3DXMATRIX& WINAPI PGEMakeCubeMapViewMatrix(DWORD iFace, D3DXMATRIX& matView);

	//--------------------------------------------------------------------
	// Name: PGEComputeTangents
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEComputeTangents(const D3DXVECTOR3* positions, const D3DXVECTOR3* normals, const D3DXVECTOR2* uvs, DWORD dwNumVerts, const WORD* pIndices, DWORD dwNumFaces, D3DXVECTOR3* tangents);

	//--------------------------------------------------------------------
	// Name: PGEScreenToVector3
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEScreenToVector3(D3DXVECTOR3& v, int sx, int sy, float width, float height);

	//--------------------------------------------------------------------
	// Name: PGEGetVec3Vertical
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEGetVec3Vertical(D3DXVECTOR3& vOut, const D3DXVECTOR3& vDir);


	//--------------------------------------------------------------------
	// Name: PGEIntersectTriangle
	// Desc: 检测一条射线是否与一个三角形相交
	// 参数: 'orig'[in]: 射线原点
	//		 'dir'[in]: 射线的方向(规一化)
	//		 'v0', 'v1', 'v2'[in]: 三角形的三个顶点
	//		 't'[in,out]:  交点到原点的距离, 这个值不能为空.
	//		 'u', 'v'[in,out]: v(交点) = (v1 - v0) * u + (v2 - v0) * v 这个值不能为空.
	// 返回值: TRUE: 相交 FALSE: 不相交
	//--------------------------------------------------------------------
	DEVICE_EXPORT BOOL WINAPI PGEIntersectTriangle( const D3DXVECTOR3* orig,
													const D3DXVECTOR3* dir,
													const D3DXVECTOR3* v0,
													const D3DXVECTOR3* v1,
													const D3DXVECTOR3* v2,
													float* t,
													float* u,
													float* v );

	//--------------------------------------------------------------------
	// Name: PGEIsPointInTriangle
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT bool WINAPI PGEIsPointInTriangle(const D3DXVECTOR2 &va , const D3DXVECTOR2 &vb, const D3DXVECTOR2 &vc, const D3DXVECTOR2 &vp);

	//--------------------------------------------------------------------
	// Name: PGEIsPointIn3DTraingle
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT bool WINAPI PGEIsPointIn3DTraingle(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p);

	//--------------------------------------------------------------------
	// Name: PGEPointToLine
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT FLOAT WINAPI PGEPointToLineDist(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v, FLOAT& t );

	//--------------------------------------------------------------------
	// Name: PGEPointToLineDist
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT FLOAT WINAPI PGEPointToLineDist2D(FLOAT X0, FLOAT Y0, FLOAT X1, FLOAT Y1, FLOAT X, FLOAT Y);

	//--------------------------------------------------------------------
	// Name: PGECreateVertically
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT D3DXVECTOR3& WINAPI PGECreateVertically(D3DXVECTOR3& n, const D3DXVECTOR3& p);

	DEVICE_EXPORT void WINAPI PGEMatrixDecompose( D3DXVECTOR3& scaling, D3DXQUATERNION& rotation, D3DXVECTOR3& translation, const D3DXMATRIX& m );

	//--------------------------------------------------------------------
	// Name: PGECreateVertically
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGE_HSV2RGB(  float _H 
								, float _S 
								, float _V 
								, float& _R 
								, float& _G 
								, float& _B );

	//--------------------------------------------------------------------
	// Name: PGECreateVertically
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGE_RGB2HSV(  float _R 
											, float _G 
											, float _B 
											, float& _H 
											, float& _S 
											, float& _V );

	//--------------------------------------------------------------------
	// Name: PGEComputeBoundingBox
	// Desc: 
	//--------------------------------------------------------------------
	DEVICE_EXPORT void WINAPI PGEComputeBoundingBox( const D3DXVECTOR3* points, DWORD nPoints, 
													D3DXVECTOR3& centroid, D3DXVECTOR3 basis[3],
													D3DXVECTOR3& vMin, D3DXVECTOR3& vMax );
}

