#include "../include/PGE_Math.h"

//-------------------------------------------------------------------
// 全局变量
//-------------------------------------------------------------------
const D3DXMATRIX  g_MatIdentity(1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f, 
								0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);

const D3DXVECTOR3 g_Vec3Zero(0.0f, 0.0f, 0.0f);
const D3DXVECTOR3 g_Vec3UnitX(1.0f, 0.0f, 0.0f);
const D3DXVECTOR3 g_Vec3UnitY(0.0f, 1.0f, 0.0f);
const D3DXVECTOR3 g_Vec3UnitZ(0.0f, 0.0f, 1.0f);

const D3DXQUATERNION g_QuatIdentity(0.0f, 0.0f, 0.0f, 1.0f);
//--------------------------------------------------------------------
// Name: PGEMatrixToEulerAngle
// Desc: 
//--------------------------------------------------------------------
void WINAPI PGEMatrixToEulerAngle(const D3DXMATRIX& mat, float& Yaw, float& Pitch, float& Roll)
{
	float sp = -mat._32;
	if (sp <= -1.0f)
		Pitch = -D3DX_PI * 0.5f;
	else if ( sp >= 1.0f )
		Pitch = D3DX_PI * 0.5f;
	else
		Pitch = asinf(sp);

	if (sp > 0.9999f)
	{
		// We are lookin straight up or down
		// Set Roll to zero and adjust Yaw
		Roll = 0.0f;
		Yaw = atan2f(-mat._13, mat._11);

	}else
	{
		Roll = atan2f(mat._12, mat._22);
		Yaw = atan2f(mat._31, mat._33);
	}
}

//--------------------------------------------------------------------
// Name: PGEQuaternionAxisToAxis
// Desc: 
//--------------------------------------------------------------------
D3DXQUATERNION* WINAPI PGEQuaternionAxisToAxis(   D3DXQUATERNION* _pQ 
											   , const D3DXVECTOR3* _From 
											   , const D3DXVECTOR3* _To )
{

	D3DXVECTOR3 vA, vB, vHalf, vAxis;
	D3DXVec3Normalize( &vA, _From );
	D3DXVec3Normalize( &vB, _To );
	D3DXVec3Add( &vHalf, &vA , &vB );
	D3DXVec3Normalize( &vHalf , &vHalf );
	D3DXVec3Cross( &vAxis , &vA , &vHalf );
	_pQ->x = vAxis.x;
	_pQ->y = vAxis.y;
	_pQ->z = vAxis.z;
	_pQ->w = D3DXVec3Dot( &vA , &vHalf );
	return _pQ;
}

//--------------------------------------------------------------------
// Name: PGEQuaternionToEulerAngle
// Desc: 
//--------------------------------------------------------------------
void WINAPI PGEQuaternionToEulerAngle(const D3DXQUATERNION& q, float &Yaw, float& Pitch, float& Roll)
{
	float mat32 = 2.0f * ( q.y * q.z - q.w * q.x );  
	float sp = -mat32;

	if (sp <= -1.0f)
		Pitch = -D3DX_PI * 0.5f;
	else if ( sp >= 1.0f )
		Pitch = D3DX_PI * 0.5f;
	else
		Pitch = asinf(sp);

	if (sp > 0.9999f)
	{
		// We are lookin straight up or down
		// Set Roll to zero and adjust Yaw
		Roll = 0.0f;
		float mat13 = 2.0f * ( q.x * q.z - q.w * q.y );
		float mat11 = 1.0f - 2.0f * ( q.y * q.y + q.z * q.z );

		Yaw = atan2f(-mat13, mat11);
	}else
	{
		float mat12 = 2.0f * ( q.x * q.y + q.w * q.z );
		float mat22 = 1.0f - 2.0f * ( q.x * q.x + q.z * q.z );
		float mat31 = 2.0f * ( q.x * q.z + q.w * q.y );  
		float mat33 = 1.0f - 2.0f * ( q.x * q.x + q.y * q.y );  
		Roll = atan2f(mat12, mat22);
		Yaw = atan2f(mat31, mat33);
	}
}

//--------------------------------------------------------------------
// Name: PGEMakeCubeMapViewMatrix
// Desc: 
//--------------------------------------------------------------------
D3DXMATRIX& WINAPI PGEMakeCubeMapViewMatrix(DWORD iFace, D3DXMATRIX& matView)
{
	// Standard view that will be overridden below.
	D3DXVECTOR3 vEnvEyePt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLookatPt, vUpVec;

	switch( iFace )
	{
	case 0://CUBEMAP_FACE_POSITIVE_X:
		vLookatPt = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case 1://CUBEMAP_FACE_NEGATIVE_X:
		vLookatPt = D3DXVECTOR3(-1.0f, 0.0f, 0.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case 2://CUBEMAP_FACE_POSITIVE_Y:
		vLookatPt = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
		break;
	case 3://CUBEMAP_FACE_NEGATIVE_Y:
		vLookatPt = D3DXVECTOR3( 0.0f,-1.0f, 0.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		break;
	case 4://CUBEMAP_FACE_POSITIVE_Z:
		vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case 5://CUBEMAP_FACE_NEGATIVE_Z:
		vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
		vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	}	
	D3DXMatrixLookAtLH(&matView, &vEnvEyePt, &vLookatPt, &vUpVec);
	return matView;
}

//--------------------------------------------------------------------
// Name: PGEComputeTangents
// Desc: 
//--------------------------------------------------------------------
void WINAPI PGEComputeTangents(const D3DXVECTOR3* positions, const D3DXVECTOR3* normals, const D3DXVECTOR2* uvs, DWORD dwNumVerts, const WORD* pIndices, DWORD dwNumFaces, D3DXVECTOR3* tangents)
{
	// Initialize tangents
	{
		for (DWORD i = 0; i < dwNumVerts; ++i)
		{
			tangents[i].x = tangents[i].y = tangents[i].z = 0.0f;
		}
	}

	// Compute tangents
	{
		D3DXVECTOR3 vAB, vAC, s;

		D3DXVECTOR3 planes[3];
		for (DWORD i = 0; i < dwNumFaces; ++i)
		{
			WORD i0 = pIndices[i * 3 + 0];
			WORD i1 = pIndices[i * 3 + 1];
			WORD i2 = pIndices[i * 3 + 2];

			// Vectors between points in triangle
			vAB = positions[i1] - positions[i0];
			vAC = positions[i2] - positions[i0];

			// Vectors between points in texture coordinate
			float s1 = uvs[i1].x - uvs[i0].x;
			float s2 = uvs[i2].x - uvs[i0].x;
			float t1 = uvs[i1].y - uvs[i0].y;
			float t2 = uvs[i2].y - uvs[i0].y;

			float r = 1.0f/(s1 * t2 - s2 * t1);
			s = (t2 * vAB - t1 * vAC) * r;

			tangents[i0] += s;
			tangents[i1] += s;
			tangents[i2] += s;
		}
	}

	// Normalize tangents
	{
		for (DWORD i = 0; i < dwNumVerts; ++i)
		{
			tangents[i] -= D3DXVec3Dot(&tangents[i], &normals[i]) * normals[i];
			D3DXVec3Normalize(&tangents[i], &tangents[i]);
		}
	}
}

//--------------------------------------------------------------------
// Name: PGEScreenToVector3
// Desc: 
//--------------------------------------------------------------------
void WINAPI PGEScreenToVector3(D3DXVECTOR3& v, int sx, int sy, float width, float height)
{
	// Scale to screen
	v.x   = -(FLOAT)(sx - width/2)  /(FLOAT) (height/2);
	v.y   =  (FLOAT)(sy - height/2) /(FLOAT) (height/2);

	v.z   = 0.0f;
	FLOAT mag = v.x*v.x + v.y*v.y;

	if( mag > 1.0f )
	{
		FLOAT scale = 1.0f/sqrtf(mag);
		v.x *= scale;
		v.y *= scale;
	}
	else
		v.z = sqrtf( 1.0f - mag );
}

//--------------------------------------------------------------------
// Name: PGEGetVec3Vertical
// Desc: 
//--------------------------------------------------------------------
void WINAPI PGEGetVec3Vertical(D3DXVECTOR3& vOut, const D3DXVECTOR3& vDir)
{
	const float EPSILON = 1e-6f;

	if (vDir.x > EPSILON || vDir.x < -EPSILON)
	{	
		vOut.x = -(vDir.y + vDir.z)/vDir.x;
		vOut.y = vOut.z = 1.0f;
	}else if (vDir.y > EPSILON || vDir.y < -EPSILON)
	{
		vOut.y = -(vDir.x + vDir.z)/vDir.y;
		vOut.x = vOut.z = 1.0f;
	}else if (vDir.z > EPSILON || vDir.z < -EPSILON)
	{
		vOut.z = -(vDir.x + vDir.y)/vDir.z;
		vOut.x = vOut.y = 1.0f;
	}else
	{
		vOut.x = vOut.y = vOut.z = 0.0f;
	}
	D3DXVec3Normalize(&vOut, &vOut);
}

//--------------------------------------------------------------------
// Name: PGEIntersectTriangle
// Desc: 
//--------------------------------------------------------------------
BOOL WINAPI PGEIntersectTriangle(
								 const D3DXVECTOR3* orig,
								 const D3DXVECTOR3* dir,
								 const D3DXVECTOR3* v0,
								 const D3DXVECTOR3* v1,
								 const D3DXVECTOR3* v2,
								 float* t,
								 float* u,
								 float* v )
{
	// Find vectors for two edges sharing vert0
	D3DXVECTOR3 edge1;
	D3DXVECTOR3 edge2;

	D3DXVec3Subtract(&edge1, v1, v0);
	D3DXVec3Subtract(&edge2, v2, v0);

	// Begin calculating determinant - also used to calculate U parameter
	D3DXVECTOR3 pvec;
	D3DXVec3Cross( &pvec, dir, &edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	float det = D3DXVec3Dot( &edge1, &pvec );

	D3DXVECTOR3 tvec;
	if( det > 0 )
	{
		D3DXVec3Subtract(&tvec, orig, v0);
	}
	else
	{
		D3DXVec3Subtract(&tvec, v0, orig);
		det = -det;
	}

	if( det < 0.0001f )
	{
		return FALSE;
	}

	// Calculate U parameter and test bounds
	*u = D3DXVec3Dot( &tvec, &pvec );
	if( *u < 0.0f || *u > det )
	{
		return FALSE;
	}

	// Prepare to test V parameter
	D3DXVECTOR3 qvec;
	D3DXVec3Cross( &qvec, &tvec, &edge1 );

	// Calculate V parameter and test bounds
	*v = D3DXVec3Dot( dir, &qvec );
	if( *v < 0.0f || *u + *v > det )
	{
		return FALSE;
	}

	// Calculate t, scale parameters, ray intersects triangle
	*t = D3DXVec3Dot( &edge2, &qvec );
	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return (*t >= 0.0f );
}

//--------------------------------------------------------------------
// Name: PGEIsPointInTriangle
// Desc: 
//--------------------------------------------------------------------
bool WINAPI PGEIsPointInTriangle(const D3DXVECTOR2 &va, const D3DXVECTOR2 &vb, const D3DXVECTOR2 &vc, const D3DXVECTOR2 &vp)
{
	// 用直线去判断
	float k0, c0;
	float dp, dc, da, db;
	// ab
	if (vb.x != va.x)
	{
		k0 = (vb.y - va.y)/(vb.x - va.x);
		c0 = va.y - k0 * va.x;
		dp = vp.y - vp.x * k0 - c0;
		dc = vc.y - vc.x * k0 - c0;
	}else
	{
		dp = vp.x - va.x;
		dc = vc.x - va.x;
	}
	if ((dp > 0 && dc < 0) || (dp < 0 && dc > 0))
		return false;
	// ac
	if (va.x != vc.x)
	{
		k0 = (vc.y - va.y)/(vc.x - va.x);
		c0 = va.y - k0 * va.x;
		dp = vp.y - vp.x * k0 - c0;
		db = vb.y - vb.x * k0 - c0;
	}else
	{
		dp = vp.x - va.x;
		db = vb.x - va.x;
	}
	if ((dp > 0 && db < 0) || (dp < 0 && db > 0))
		return false;
	// bc
	if (vb.x != vc.x)
	{
		k0 = (vc.y - vb.y)/(vc.x - vb.x);
		c0 = vb.y - k0 * vb.x;
		dp = vp.y - vp.x * k0 - c0;
		da = va.y - va.x * k0 - c0;
	}else
	{
		dp = vp.x - vb.x;
		da = va.x - vb.x;
	}
	if ((dp > 0 && da < 0) || (dp < 0 && da > 0))
		return false;
	return true;

/*	// 用向量法判断
	D3DXVECTOR3 edge1(va.x - vp.x, va.y - vp.y, 0.0f);
	D3DXVECTOR3 edge2(vb.x - vp.x, vb.y - vp.y, 0.0f);
	
	D3DXVec3Cross( &v, &edge1, &edge2 );
	if (v.z > 0.0f )
		return false;

	D3DXVECTOR3 edge3(vc.x - vp.x, vc.y - vp.y, 0.0f);
	D3DXVec3Cross( &v, &edge2, &edge3 );
	if ( v.z > 0.0f )
		return false;

	D3DXVec3Cross( &v, &edge3, &edge1 );
	if ( v.z > 0.0f )
		return false;

	return true;*/
}

//--------------------------------------------------------------------
// Name: PGEIsPointIn3DTraingle
// Desc: 
//--------------------------------------------------------------------
bool WINAPI PGEIsPointIn3DTraingle(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p)
{
	D3DXVECTOR2 v0, v1, v2, v;

	D3DXVECTOR3 n;
	D3DXVec3Cross(&n, &(p2 - p0), &(p1 - p0));
	D3DXVec3Normalize(&n, &n);

	float absX = fabsf(n.x);
	float absY = fabsf(n.y);
	float absZ = fabsf(n.z);
	if (absX > absY && absX > absZ)
	{
		// Project to YZ plane
		v.x = p.y;		v.y = p.z;	
		v0.x = p0.y;	v0.y = p0.z;
		v1.x = p1.y;	v1.y = p1.z;
		v2.x = p2.y;	v2.y = p2.z;
	}else if (absY > absX && absY > absZ)
	{
		// Project to XZ plane	
		v.x = p.x;		v.y = p.z;
		v0.x = p0.x;	v0.y = p0.z;
		v1.x = p1.x;	v1.y = p1.z;
		v2.x = p2.x;	v2.y = p2.z;
	}else
	{	
		// Project to XY plane	
		v.x = p.x;		v.y = p.y;
		v0.x = p0.x;	v0.y = p0.y;
		v1.x = p1.x;	v1.y = p1.y;
		v2.x = p2.x;	v2.y = p2.y;
	}

	return PGEIsPointInTriangle(v0, v1, v2, v);
}

FLOAT WINAPI PGEPointToLineDist2D(FLOAT X0, FLOAT Y0, FLOAT X1, FLOAT Y1, FLOAT X, FLOAT Y)
{
	FLOAT d = 0.0f;
	// Point to line distance = (y-Y)^2/(k^2 + 1)
	if (fabs(X1 - X0) > 0.0001f)
	{
		FLOAT k = (FLOAT)(Y1 - Y0)/(X1 - X0);
		FLOAT c = (FLOAT)Y0 - k * X0;

		FLOAT y = k * X + c - Y;

		d = sqrtf(y*y/(k*k + 1.0f));
	}else
	{
		d = (FLOAT)fabs((FLOAT)(X - X0));
	}
	return d;
}

FLOAT WINAPI PGEPointToLineDist(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v, FLOAT& t)
{
	D3DXVECTOR3 edge = v - v0;
	D3DXVECTOR3 dir = v1 - v0;
	FLOAT fLen = D3DXVec3Length(&dir);
	dir /= fLen;
	FLOAT dist;
	t    = D3DXVec3Dot(&edge, &dir);
	dist = sqrtf( D3DXVec3LengthSq(&edge) - t*t );
	t	/= fLen;
	return dist;
}


D3DXVECTOR3& WINAPI PGECreateVertically(D3DXVECTOR3& n, const D3DXVECTOR3& p)
{
	if (p.x > 0.00001f || p.x < -0.00001f)
	{	
		n.x = -(p.y + p.z)/p.x;
		n.y = n.z = 1.0f;
	}else if (p.y > 0.00001f || p.y < -0.00001f)
	{
		n.y = -(p.x + p.z)/p.y;
		n.x = n.z = 1.0f;
	}else if (p.z > 0.00001f || p.z < -0.00001f)
	{
		n.z = -(p.x + p.y)/p.z;
		n.x = n.y = 1.0f;
	}else
	{
		n.x = n.y = n.z = 0.0f;
	}
	return n;
}

void WINAPI PGE_HSV2RGB( float _H 
			 , float _S 
			 , float _V 
			 , float& _R 
			 , float& _G 
			 , float& _B )
{
	_H = __max( _H , 0.0f );
	_H = __min( _H , 1.0f );

	_S = __max( _S , 0.0f );
	_S = __min( _S , 1.0f );

	_V = __max( _V , 0.0f );
	_V = __min( _V , 1.0f );

	if ( _S == 0.0f )
	{
		_R = _G = _B = _V;     
	}
	else
	{
		_H *= 6.0f;
		long var_i = (long)_H;
		float var_1 = _V * ( 1.0f - _S );
		float var_2 = _V * ( 1.0f - _S * ( _H - var_i ) );
		float var_3 = _V * ( 1.0f - _S * ( 1.0f - ( _H - var_i ) ) );
		switch( var_i )
		{
		case 0:		_R = _V;     _G = var_3;     _B = var_1;	break;
		case 1:		_R = var_2;  _G = _V;        _B = var_1;    break;
		case 2:		_R = var_1;  _G = _V;        _B = var_3;	break;
		case 3:		_R = var_1;  _G = var_2;     _B = _V;		break;
		case 4:		_R = var_3;  _G = var_1;     _B = _V;		break;	
		case 5:	    _R = _V;     _G = var_1;     _B = var_2;	break;
		}
	}
}

void WINAPI PGE_RGB2HSV(  float _R 
						, float _G 
						, float _B 
						, float& _H 
						, float& _S 
						, float& _V )
{
	enum { R_MAX , G_MAX , B_MAX, } who_max = R_MAX;
	float Max = _R;
	float Min = _R;
	if(_G > Max) {    Max = _G;	who_max = G_MAX; }
	if(_B > Max) {    Max = _B;	who_max = B_MAX; }
	if(_G < Min) {    Min = _G;	}
	if(_B < Min) {	  Min = _B;	}
	float Del = Max - Min;
	_V = Max;
	if( Del == 0.0f )
	{
		_H = 0.0f;
		_S = 0.0f;
	}
	else
	{
		_S = Del / Max;
		float K = 1.0f / ( 6.0f * Del );        
		float del_R = ( ( Max - _R ) * K ) + 0.5f;
		float del_G = ( ( Max - _G ) * K ) + 0.5f;
		float del_B = ( ( Max - _B ) * K ) + 0.5f;
		switch( who_max )
		{
		case	R_MAX:	_H = 0.0f			+ del_B - del_G;		break;
		case	G_MAX:	_H = 0.33333333f	+ del_R - del_B;		break;
		case	B_MAX:	_H = 0.66666666f	+ del_G - del_R;		break;
		}
		if( _H < 0.0f ){ _H += 1.0f; }
		if( _H > 1.0f ){ _H -= 1.0f; }
	}
}

void WINAPI PGEMatrixDecompose( D3DXVECTOR3& scaling, D3DXQUATERNION& rotation, D3DXVECTOR3& translation, const D3DXMATRIX& m )
{
	translation.x = m._41;
	translation.y = m._42;
	translation.z = m._43;
	/*----------------------------------------------------------
	sx 0   0      m11  m12  m13		sx*m11  sx*m12 sx*m13 
	0  sy  0   *  m21  m22  m23  =  sy*m21  sy*m22 sy*m23
	0  0   sz     m31  m21  m33		sz*m31  sz*m32 sz*m33

	sx*sx = m._11 * m._11 + m._12 * m._12 + m._13*m._13;
	sy*sy = m._21 * m._21 + m._22 * m._22 + m._32*m._32;
	sz*sz = m._31 * m._31 + m._32 * m._32 + m._33*m._33;
	/----------------------------------------------------------*/
	scaling.x = (float)sqrt( (double) m._11 * m._11 + m._12 * m._12 + m._13*m._13 );
	scaling.y = (float)sqrt( (double) m._21 * m._21 + m._22 * m._22 + m._23*m._23 );
	scaling.z = (float)sqrt( (double) m._31 * m._31 + m._32 * m._32 + m._33*m._33 );

	D3DXMATRIX matRot( m._11/scaling.x, m._12/scaling.x, m._13/scaling.x, 0.0f,
					   m._21/scaling.y,	m._22/scaling.y, m._23/scaling.y, 0.0f,
					   m._31/scaling.z,	m._32/scaling.z, m._33/scaling.z, 0.0f,
					   0.0f,			0.0f,			 0.0f,			  1.0f );
	D3DXQuaternionRotationMatrix( &rotation, &matRot );
}
//////////////////////////////////////////////////////////////////////////
// OBB
//////////////////////////////////////////////////////////////////////////
namespace  
{
	//-------------------------------------------------------------------------------
	// Compute covariance matrix for set of points
	// Returns centroid and unique values of matrix
	// Does not normalize matrix by 1/(n-1) (not needed)
	//-------------------------------------------------------------------------------
	void CovarianceMatrix( const D3DXVECTOR3* points, DWORD numPoints, D3DXVECTOR3& mean, float C[6] )
	{
		DWORD i;

		// compute mean
		mean = points[0];
		for (i = 1; i < numPoints; ++i)
		{
			mean += points[i];	
		}
		float recip = 1.0f/numPoints;
		mean *= recip;

		// compute each element of matrix
		memset( C, 0, sizeof(float)*6 );
		for (i = 0; i < numPoints; ++i)
		{
			D3DXVECTOR3 diff = points[i]-mean;
			C[0] += diff.x*diff.x;
			C[1] += diff.x*diff.y;
			C[2] += diff.x*diff.z;
			C[3] += diff.y*diff.y;
			C[4] += diff.y*diff.z;
			C[5] += diff.z*diff.z;
		}	
	}

	//-------------------------------------------------------------------------------
	// Given symmetric matrix A and eigenvalue l, returns corresponding eigenvector
	// Assumes that order of eigenvalue is 1
	//-------------------------------------------------------------------------------
	void CalcSymmetricEigenvector( float A[6], float l, D3DXVECTOR3& v1 )
	{
		float m11 = A[0] - l;
		float m12 = A[1];
		float m13 = A[2];
		float m22 = A[3] - l;
		float m23 = A[4];
		float m33 = A[5] - l;

		// compute cross product matrix, and find column with maximal entry
		float u11 = m22*m33 - m23*m23;
		float max = fabsf(u11);
		int c = 1;
		float u12 = m13*m23 - m12*m33;
		if (fabsf(u12) > max)
		{
			max = fabsf(u12);
			c = 2;
		}
		float u13 = m12*m23 - m13*m22;
		if (fabsf(u13) > max)
		{
			max = fabsf(u13);
			c = 3;
		}
		float u22 = m11*m33 - m13*m13;
		if (fabsf(u22) > max)
		{
			max = fabsf(u22);
			c = 2;
		}
		float u23 = m12*m13 - m23*m11;
		if (fabsf(u23) > max)
		{
			max = fabsf(u23);
			c = 3;
		}
		float u33 = m11*m22 - m12*m12;
		if (fabsf(u33) > max)
		{
			max = fabsf(u33);
			c = 3;
		}

		// return column with maximal entry
		if (c == 1)
		{
			v1 = D3DXVECTOR3(u11, u12, u13);
		}
		else if (c == 2)
		{
			v1 = D3DXVECTOR3(u12,u22,u23);
		}
		else
		{
			v1 = D3DXVECTOR3(u13,u23,u33);
		}
	}


	//-------------------------------------------------------------------------------
	// Given symmetric matrix A and eigenvalue l, returns eigenvector pair
	// Assumes that order of eigenvalue is 2
	//-------------------------------------------------------------------------------
	void CalcSymmetricEigenPair( float A[6], float l, D3DXVECTOR3& v1, D3DXVECTOR3& v2 )
	{
		// find maximal entry in M
		float m11 = A[0] - l;
		float max = fabsf(m11);
		int r=1,c=1;
		if (fabsf(A[1]) > max)
		{
			max = fabsf(A[1]);
			r = 1; c = 2;
		}
		if (fabsf(A[2]) > max)
		{
			max = fabsf(A[2]);
			r = 1; c = 3;
		}
		float m22 = A[3]-l;
		if (fabsf(m22) > max)
		{
			max = fabsf(m22);
			r = 2; c = 2;
		}
		if (fabsf(A[4]) > max)
		{
			max = fabsf(A[4]);
			r = 2; c = 3;
		}
		float m33 = A[5]-l;
		if (fabsf(m33) > max)
		{
			r = 3; c = 3;
		}

		// compute eigenvectors for each case
		if ( r == 1 )
		{
			if ( c == 3 )
			{
				v1 = D3DXVECTOR3(A[2],0.0f,-m11);
				v2 = D3DXVECTOR3(-A[1]*m11, m11*m11 + A[2]*A[2],-A[1]*A[2]);
			}
			else
			{
				v1 = D3DXVECTOR3(-A[1],m11,0.0f);
				v2 = D3DXVECTOR3(-A[2]*m11,-A[2]*A[1],m11*m11 + A[1]*A[1]);
			}
		}
		else if ( r == 2 )
		{
			v1 = D3DXVECTOR3(0.0f,-A[4],m22);
			v2 = D3DXVECTOR3(m22*m22 + A[4]*A[4],-A[1]*m22,-A[1]*A[4]);		
		}
		else if ( r == 3 )
		{
			v1 = D3DXVECTOR3(0.0f,-m33,A[4]);
			v2 = D3DXVECTOR3(A[4]*A[4] + m33*m33,-A[2]*A[4],-A[2]*m33);		
		}
	}

	//-------------------------------------------------------------------------------
	// Given symmetric matrix in A, returns three eigenvectors in order of 
	// decreasing eigenvalue
	//-------------------------------------------------------------------------------
	void GetRealSymmetricEigenvectors( float A[6], D3DXVECTOR3& v1, D3DXVECTOR3& v2, D3DXVECTOR3& v3 )
	{
		// compute coefficients for cubic equation
		float c2 = A[0] + A[3] + A[5];
		float a12sq = A[1]*A[1];
		float a13sq = A[2]*A[2];
		float a23sq = A[4]*A[4];
		float a11a22 = A[0]*A[3];
		float c1 = a11a22 - a12sq + A[0]*A[5] - a13sq 
			+ A[3]*A[5] - a23sq;
		float c0 = a11a22*A[5] + 2.0f*A[1]*A[2]*A[4] - A[0]*a23sq - A[3]*a13sq - A[5]*a12sq;

		// compute intermediate values for root solving
		float c2sq = c2*c2;
		float a = (3.0f*c1 - c2sq)/3.0f;
		float b = (9.0f*c1*c2 - 2.0f*c2sq*c2 - 27.f*c0)/27.0f; 
		float halfb = b*0.5f;
		float halfb2 = halfb*halfb;
		float Q = halfb2 + a*a*a/27.0f;

		// determine type of eigenspaces
		if ( Q > 1.0e-6f )
		{
			// one eigenvalue, use standard basis
			v1 = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
			v2 = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v3 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

			return;
		}

		else if ( Q < -1.0e-6f )
		{
			// three distinct eigenvalues

			// intermediate terms
			float theta_3 = atan2f(sqrtf(-Q),-halfb)/3.0f;
			float rho = sqrtf(halfb2 - Q);
			float c2_3 = c2/3.0f;
			float rho_13 = powf(fabsf(rho),1.0f/3.0f);
			if (rho < 0.0f)
				rho_13 = -rho_13;
			float ct_3, st_3;
			const float sqrt3 = sqrtf(3.0f);
			ct_3 = cosf(theta_3);
			st_3 = sinf(theta_3);

			// compute each eigenvalue and eigenvector
			// sort from largest to smallest
			float lambda1 = c2_3 + 2.0f*rho_13*ct_3;
			CalcSymmetricEigenvector( A, lambda1, v1 );

			float lambda2 = c2_3 - rho_13*(ct_3 + sqrt3*st_3);
			if (lambda2 > lambda1)
			{
				v2 = v1;
				float temp = lambda2;
				lambda2 = lambda1;
				lambda1 = temp;
				CalcSymmetricEigenvector( A, lambda2, v1 );
			}
			else
			{
				CalcSymmetricEigenvector( A, lambda2, v2 );               
			}

			float lambda3 = c2_3 - rho_13*(ct_3 - sqrt3*st_3);
			if ( lambda3 > lambda1 )
			{
				v3 = v2;
				v2 = v1;
				CalcSymmetricEigenvector( A, lambda3, v1 );
			}
			else if (lambda3 > lambda2 )
			{
				v3 = v2;
				CalcSymmetricEigenvector( A, lambda3, v2 );
			}
			else
			{
				CalcSymmetricEigenvector( A, lambda3, v3 );
			}
		}
		else
		{
			// two distinct eigenvalues

			// intermediate terms
			float c2_3 = c2/3.0f;
			float halfb_13 = powf(fabsf(halfb),1.0f/3.0f);
			if (halfb < 0.0f)
				halfb_13 = -halfb_13;

			// compute each eigenvalue and eigenvector
			// sort from largest to smallest
			float lambda1 = c2_3 + halfb_13;					
			CalcSymmetricEigenPair(A, lambda1, v1, v2 ); 

			float lambda2 = c2_3 - 2.0f*halfb_13;
			if (lambda2 > lambda1)
			{
				v3 = v2;
				v2 = v1;
				CalcSymmetricEigenvector( A, lambda2, v1 );
			}
			else
			{
				CalcSymmetricEigenvector( A, lambda2, v3 );
			}
		}

		D3DXVec3Normalize(&v1, &v1);
		D3DXVec3Normalize(&v2, &v2);
		D3DXVec3Normalize(&v3, &v3);

		D3DXVECTOR3 v;
		D3DXVec3Cross(&v, &v1, &v2);
		if ( D3DXVec3Dot(&v, &v3) < 0.0f )
			v3 = -v3;
	}
}

//-------------------------------------------------------------------------------
// Compute bounding box for set of points
// Returns centroid, basis for bounding box, and min/max ranges 
//         (in terms of centroid/basis coordinate frame)
//-------------------------------------------------------------------------------
void WINAPI PGEComputeBoundingBox( const D3DXVECTOR3* points, DWORD nPoints, 
						   D3DXVECTOR3& centroid, D3DXVECTOR3 basis[3],
						   D3DXVECTOR3& vMin, D3DXVECTOR3& vMax )
{
	// compute covariance matrix
	float C[6];
	CovarianceMatrix( points, nPoints, centroid, C );

	// get principle axes
	GetRealSymmetricEigenvectors( C, basis[0], basis[1], basis[2] );

	vMin = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
	vMax = D3DXVECTOR3(FLT_MIN, FLT_MIN, FLT_MIN);

	D3DXVECTOR3 diff;
	float		length;
	// for each point do
	for ( DWORD i = 0; i < nPoints; ++i )
	{
		diff = points[i]-centroid;

		length = D3DXVec3Dot(&diff, &basis[0]);
		if (length > vMax.x)
		{
			vMax.x = length;
		}
		else if (length < vMin.x)
		{
			vMin.x = length;
		}

		length = D3DXVec3Dot(&diff, &basis[1]);
		if (length > vMax.y)
		{
			vMax.y = length;
		}
		else if (length < vMin.y)
		{
			vMin.y = length;
		}

		length = D3DXVec3Dot(&diff, &basis[2]);
		if (length > vMax.z)
		{
			vMax.z = length;
		}
		else if (length < vMin.z)
		{
			vMin.z = length;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
