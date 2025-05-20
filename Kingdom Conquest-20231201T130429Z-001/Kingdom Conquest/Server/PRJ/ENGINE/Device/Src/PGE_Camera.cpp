#include "../Include/PGE_Camera.h"
#include "../Include/PGE_Aabb.h"
#include "../Include/PGE_OBB.h"

PGECamera::PGECamera()
{	
	D3DXQuaternionIdentity(&m_Orientation);

	m_Eye	= D3DXVECTOR3( 0.0f , 0.0f , 0.0f );
	m_YawFixedAxis = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	m_FOV	= D3DX_PI * 0.25f;
	m_Far	= 10000.0f;
	m_Near	= 1.0f;
	m_Aspect= 1.0f;

	m_bDirty		= TRUE;
	m_bYawFixed		= TRUE;
	Update();
}

void PGECamera::SetDirection( const D3DXVECTOR3& vDir )
{
	float length = D3DXVec3Length(&vDir);
	if( length < 0.00001f )
		return;

	D3DXVECTOR3 zAxis = vDir/length;

	if ( m_bYawFixed )
	{
		D3DXVECTOR3 xAxis;
		D3DXVec3Cross( &xAxis, &m_YawFixedAxis, &zAxis );
		D3DXVec3Normalize( &xAxis, &xAxis );

		D3DXVECTOR3 yAxis;
		D3DXVec3Cross( &yAxis, &zAxis, &xAxis );
		D3DXVec3Normalize( &yAxis, &yAxis );

		D3DXMATRIX m( xAxis.x, yAxis.x, zAxis.x, 0.0f, 
					  xAxis.y, yAxis.y, zAxis.y, 0.0f,
					  xAxis.z, yAxis.z, zAxis.z, 0.0f,
					  0.0f,	   0.0f,    0.0f,    1.0f );
		D3DXQuaternionRotationMatrix( &m_Orientation, &m );
		D3DXQuaternionConjugate( &m_Orientation, &m_Orientation );
	}else
	{
		Update();

		D3DXVECTOR3 Y( m_ViewMatrix._12, m_ViewMatrix._22, m_ViewMatrix._32 );
		D3DXVECTOR3 Z( m_ViewMatrix._13, m_ViewMatrix._23, m_ViewMatrix._33 );

		D3DXQUATERNION rot;
		if ( zAxis == -Z )
		{
			D3DXQuaternionRotationAxis( &rot, &Y, D3DX_PI );
		}else
		{
			PGEQuaternionAxisToAxis( &rot, &Z, &zAxis );
		}
		m_Orientation *= rot;
	}
	m_bDirty = TRUE;
}

void PGECamera::SetVPMatrix( const D3DXMATRIX& _ViewMat , const D3DXMATRIX& _ProjMat )
{
	m_ViewMatrix	=	_ViewMat;
	m_ProjMatrix	=	_ProjMat;
	D3DXMatrixMultiply( &m_FinalMatrix , &m_ViewMatrix , &m_ProjMatrix );
	D3DXMATRIX& Matrix = m_FinalMatrix;
	
	m_FrustumPlanes[0].a = Matrix._14 + Matrix._13; // Near
	m_FrustumPlanes[0].b = Matrix._24 + Matrix._23;
	m_FrustumPlanes[0].c = Matrix._34 + Matrix._33;
	m_FrustumPlanes[0].d = Matrix._44 + Matrix._43;
	D3DXPlaneNormalize(&m_FrustumPlanes[0], &m_FrustumPlanes[0]);
	
	m_FrustumPlanes[1].a = Matrix._14 - Matrix._13; // Far
	m_FrustumPlanes[1].b = Matrix._24 - Matrix._23;
	m_FrustumPlanes[1].c = Matrix._34 - Matrix._33;
	m_FrustumPlanes[1].d = Matrix._44 - Matrix._43;
	D3DXPlaneNormalize(&m_FrustumPlanes[1], &m_FrustumPlanes[1]);
	
	m_FrustumPlanes[2].a = Matrix._14 + Matrix._11; // Left
	m_FrustumPlanes[2].b = Matrix._24 + Matrix._21;
	m_FrustumPlanes[2].c = Matrix._34 + Matrix._31;
	m_FrustumPlanes[2].d = Matrix._44 + Matrix._41;
	D3DXPlaneNormalize(&m_FrustumPlanes[2], &m_FrustumPlanes[2]);
	
	m_FrustumPlanes[3].a = Matrix._14 - Matrix._11; // Right
	m_FrustumPlanes[3].b = Matrix._24 - Matrix._21;
	m_FrustumPlanes[3].c = Matrix._34 - Matrix._31;
	m_FrustumPlanes[3].d = Matrix._44 - Matrix._41;
	D3DXPlaneNormalize(&m_FrustumPlanes[3], &m_FrustumPlanes[3]);
	
	m_FrustumPlanes[4].a = Matrix._14 - Matrix._12; // Top
	m_FrustumPlanes[4].b = Matrix._24 - Matrix._22;
	m_FrustumPlanes[4].c = Matrix._34 - Matrix._32;
	m_FrustumPlanes[4].d = Matrix._44 - Matrix._42;
	D3DXPlaneNormalize(&m_FrustumPlanes[4], &m_FrustumPlanes[4]);
	
	m_FrustumPlanes[5].a = Matrix._14 + Matrix._12; // Bottom
	m_FrustumPlanes[5].b = Matrix._24 + Matrix._22;
	m_FrustumPlanes[5].c = Matrix._34 + Matrix._32;
	m_FrustumPlanes[5].d = Matrix._44 + Matrix._42;
	D3DXPlaneNormalize(&m_FrustumPlanes[5], &m_FrustumPlanes[5]);
	
	m_bDirty = FALSE;
}



void PGECamera::Update()
{
	if ( m_bDirty )
	{
		D3DXMATRIX matRot;
		D3DXMATRIX matT;	
		D3DXMatrixTranslation( &matT , -m_Eye.x , -m_Eye.y , -m_Eye.z );

		D3DXQUATERNION rot(-m_Orientation.x, -m_Orientation.y, -m_Orientation.z, m_Orientation.w);
		D3DXMatrixRotationQuaternion(&matRot, &rot);

		m_ViewMatrix = matT * matRot;
	
		D3DXMatrixPerspectiveFovLH( &m_ProjMatrix, m_FOV , m_Aspect, m_Near, m_Far );
		D3DXMatrixMultiply( &m_FinalMatrix , &m_ViewMatrix , &m_ProjMatrix );
		D3DXMATRIX& Matrix = m_FinalMatrix;
		
		m_FrustumPlanes[0].a = Matrix._14 + Matrix._13; // Near
		m_FrustumPlanes[0].b = Matrix._24 + Matrix._23;
		m_FrustumPlanes[0].c = Matrix._34 + Matrix._33;
		m_FrustumPlanes[0].d = Matrix._44 + Matrix._43;
		D3DXPlaneNormalize(&m_FrustumPlanes[0], &m_FrustumPlanes[0]);

		m_FrustumPlanes[1].a = Matrix._14 - Matrix._13; // Far
		m_FrustumPlanes[1].b = Matrix._24 - Matrix._23;
		m_FrustumPlanes[1].c = Matrix._34 - Matrix._33;
		m_FrustumPlanes[1].d = Matrix._44 - Matrix._43;
		D3DXPlaneNormalize(&m_FrustumPlanes[1], &m_FrustumPlanes[1]);

		m_FrustumPlanes[2].a = Matrix._14 + Matrix._11; // Left
		m_FrustumPlanes[2].b = Matrix._24 + Matrix._21;
		m_FrustumPlanes[2].c = Matrix._34 + Matrix._31;
		m_FrustumPlanes[2].d = Matrix._44 + Matrix._41;
		D3DXPlaneNormalize(&m_FrustumPlanes[2], &m_FrustumPlanes[2]);

		m_FrustumPlanes[3].a = Matrix._14 - Matrix._11; // Right
		m_FrustumPlanes[3].b = Matrix._24 - Matrix._21;
		m_FrustumPlanes[3].c = Matrix._34 - Matrix._31;
		m_FrustumPlanes[3].d = Matrix._44 - Matrix._41;
		D3DXPlaneNormalize(&m_FrustumPlanes[3], &m_FrustumPlanes[3]);

		m_FrustumPlanes[4].a = Matrix._14 - Matrix._12; // Top
		m_FrustumPlanes[4].b = Matrix._24 - Matrix._22;
		m_FrustumPlanes[4].c = Matrix._34 - Matrix._32;
		m_FrustumPlanes[4].d = Matrix._44 - Matrix._42;
		D3DXPlaneNormalize(&m_FrustumPlanes[4], &m_FrustumPlanes[4]);

		m_FrustumPlanes[5].a = Matrix._14 + Matrix._12; // Bottom
		m_FrustumPlanes[5].b = Matrix._24 + Matrix._22;
		m_FrustumPlanes[5].c = Matrix._34 + Matrix._32;
		m_FrustumPlanes[5].d = Matrix._44 + Matrix._42;
		D3DXPlaneNormalize(&m_FrustumPlanes[5], &m_FrustumPlanes[5]);
		
		m_bDirty = FALSE;
	}
}

PGE_FRUSTUM_TEST PGECamera::IsVisible( const D3DXVECTOR3& _Point , PGE_FRUSTUM_PLANE* _CulledBy )
{
	Update();
	for ( long i = 0 ; i < 6 ; ++i )
	{
		float dist = D3DXPlaneDotCoord( &( m_FrustumPlanes[i] ), &_Point );
		if( dist < 0.0f )
		{
			if( _CulledBy != NULL )
			{
				*_CulledBy = (PGE_FRUSTUM_PLANE)i;
			}
			return PGE_OUT_FRUSTUM;
		}
	}
	return PGE_IN_FRUSTUM;
}

PGE_FRUSTUM_TEST PGECamera::IsVisible(const PGE_SPHERE& _Sphere , PGE_FRUSTUM_PLANE* _CulledBy )
{
	if ( D3DXVec3Length(&(_Sphere.Center - m_Eye)) < _Sphere.Radius )
		return PGE_IN_FRUSTUM;

	bool bIntersect = false;
	Update();
	// For each plane, see if sphere is on negative side
	// If so, object is not visible
	for( long i = 0 ; i < 6 ; ++i )
	{
		float dist = D3DXPlaneDotCoord( &( m_FrustumPlanes[i] ) , &( _Sphere.Center ) );
		// If the distance from sphere center to plane is negative, and 'more negative' 
		// than the radius of the sphere, sphere is outside frustum
		if(dist < -_Sphere.Radius )
		{
			// ALL corners on negative side therefore out of view
			if( _CulledBy != NULL )
			{
				*_CulledBy = (PGE_FRUSTUM_PLANE)i;
			}
			return PGE_OUT_FRUSTUM;
		}
		else if ( !( dist > _Sphere.Radius ) )
		{
			bIntersect = true;
		}
	}
	
	return bIntersect ? PGE_INTERSECT_FRUSTUM : PGE_IN_FRUSTUM;
}

PGE_FRUSTUM_TEST PGECamera::IsVisible( const PGE_AABB& _Box , PGE_FRUSTUM_PLANE* _CulledBy )
{
	if ( _Box.PointInBox( m_Eye ) )
		return PGE_IN_FRUSTUM;

	bool bIntersect = false;
	Update();
	for (long i = 0; i < 6; ++i)
	{
		long r = _Box.ClassifyPlane( m_FrustumPlanes[i] );
		if (r < 0)
		{
			// ALL corners on negative side therefore out of view
			if ( _CulledBy != NULL )
			{
				*_CulledBy = (PGE_FRUSTUM_PLANE)i;
			}
			return PGE_OUT_FRUSTUM;
		}
		else if ( r == 0 )
		{
			bIntersect = true;
		}
	}
	return bIntersect ? PGE_INTERSECT_FRUSTUM : PGE_IN_FRUSTUM;
}


PGE_FRUSTUM_TEST PGECamera::IsVisible( const PGE_OBB& _Box , PGE_FRUSTUM_PLANE* _CulledBy )
{
	if ( _Box.PointInBox( m_Eye ) )
		 return PGE_IN_FRUSTUM;

	bool bIntersect = false;
	Update();
	for (long i = 0; i < 6; ++i)
	{
		long r = _Box.ClassifyPlane( m_FrustumPlanes[i] );
		if (r < 0)
		{
			// ALL corners on negative side therefore out of view
			if ( _CulledBy != NULL )
			{
				*_CulledBy = (PGE_FRUSTUM_PLANE)i;
			}
			return PGE_OUT_FRUSTUM;
		}
		else if ( r == 0 )
		{
			bIntersect = true;
		}
	}
	return bIntersect ? PGE_INTERSECT_FRUSTUM : PGE_IN_FRUSTUM;
}

bool PGECamera::operator == ( const PGECamera& _Cam2 ) const
{
	return 
	m_Orientation == _Cam2.m_Orientation &&
	m_Eye	==_Cam2.m_Eye&&
	m_FOV	==_Cam2.m_FOV&&
	m_Far	==_Cam2.m_Far&&
	m_Near	==_Cam2.m_Near&&
	m_Aspect==_Cam2.m_Aspect;
}
