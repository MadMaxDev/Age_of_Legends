/***************************************************************************

Copyright (c) PixelGame( http//www.pixelgame.net ) All rights reserved.

MengXin , GongJing 2005.6.27

程序名称：			PGECamera.h	

描述：				定义了描述 PGECamera 照相机类 ， 封装了一些关于矩阵和视锥的
					操作
	
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
#include "PGE_Math.h"

struct PGE_AABB;
struct PGE_OBB;
struct PGE_SPHERE;

enum PGE_FRUSTUM_PLANE
{
	PGE_FRUSTUM_PLANE_NEAR   = 0,
	PGE_FRUSTUM_PLANE_FAR    = 1,
	PGE_FRUSTUM_PLANE_LEFT   = 2,
	PGE_FRUSTUM_PLANE_RIGHT  = 3,
	PGE_FRUSTUM_PLANE_TOP    = 4,
	PGE_FRUSTUM_PLANE_BOTTOM = 5,

	PGE_FRUSTUM_FORCEDWORD = 0x7FFFFFFF,
};

enum PGE_FRUSTUM_TEST
{
	PGE_OUT_FRUSTUM			= 0,
	PGE_IN_FRUSTUM			= 1,
	PGE_INTERSECT_FRUSTUM	= 2,

	PGE_FRUSTUM_TEST_FORCEDWORD = 0x7FFFFFFF,
};

class DEVICE_EXPORT PGECamera
{

protected:
	D3DXQUATERNION  m_Orientation;
	D3DXVECTOR3		m_Eye;
	D3DXVECTOR3		m_YawFixedAxis;
	float			m_FOV;
	float			m_Far;
	float			m_Near;
	float			m_Aspect;

protected:

	D3DXPLANE		m_FrustumPlanes[6];
	D3DXMATRIX		m_ViewMatrix;
	D3DXMATRIX		m_ProjMatrix;
	D3DXMATRIX		m_FinalMatrix;

	BOOL			m_bDirty;
	BOOL		    m_bYawFixed;

	void			Update();
public:

	PGECamera();
public:
	inline void SetEye ( const D3DXVECTOR3& _Eye )			{ m_Eye = _Eye;		     m_bDirty = TRUE;	 }
	inline void SetYawFixedAxis(const D3DXVECTOR3& axis)	{ m_YawFixedAxis = axis; m_bYawFixed = TRUE; }
	inline void SetOrientation(const D3DXQUATERNION& rot)   { m_Orientation = rot;   m_bDirty = TRUE;    }

	inline void SetFOV( float _v ) 							{  m_FOV = _v; m_bDirty = TRUE;    }
	inline void SetFar( float _f )							{  m_Far = _f; m_bDirty = TRUE;    }
	inline void SetNear( float _n )							{  m_Near = _n; m_bDirty = TRUE;   }
	inline void SetAspect( float _a )						{  m_Aspect = _a; m_bDirty = TRUE; }
	inline void SetDirty(BOOL bDirty)						{  m_bDirty = bDirty;			   }
	inline void SetYawFixed(BOOL bFixed)					{  m_bYawFixed = bFixed;		   }
	
	inline const D3DXVECTOR3&	 GetEye() const				{ return m_Eye;			 }
	inline const D3DXVECTOR3&	 GetYawFixedAxis() const	{ return m_YawFixedAxis; }
	inline const D3DXQUATERNION& GetOrientation() const		{ return m_Orientation;  }

	inline float				GetFOV() const				{ return m_FOV;    }
	inline float				GetFar() const				{ return m_Far;    }
	inline float				GetNear() const				{ return m_Near;   }
	inline float				GetAspect() const			{ return m_Aspect; }
	inline BOOL					IsDirty() const				{ return m_bDirty; }
	inline BOOL					IsYawFixed() const			{ return m_bYawFixed; }

	inline const D3DXPLANE&		GetFrustumPlane( PGE_FRUSTUM_PLANE _which )
	{
		Update();
		return m_FrustumPlanes[ _which ];
	}
	inline const D3DXMATRIX&		GetViewMatrix()
	{
		Update();
		return m_ViewMatrix;
	}
	inline const D3DXMATRIX&		GetProjMatrix()
	{
		Update();
		return m_ProjMatrix;
	}
	inline const D3DXMATRIX&		GetFinalMatrix()
	{
		Update();
		return m_FinalMatrix;
	}
	inline D3DXVECTOR3 GetForward()
	{
		Update();
		return D3DXVECTOR3(m_ViewMatrix._13, 
						   m_ViewMatrix._23, 
						   m_ViewMatrix._33);
	}
	inline void GetForward(D3DXVECTOR3& v)
	{
		Update();
		v.x = m_ViewMatrix._13; 
		v.y = m_ViewMatrix._23; 
		v.z = m_ViewMatrix._33;
	}

	inline void GetUp(D3DXVECTOR3& v)
	{
		Update();
		v.x = m_ViewMatrix._12;
		v.y = m_ViewMatrix._22;
		v.z = m_ViewMatrix._32;
	}
	inline D3DXVECTOR3 GetUp()
	{
		Update();
		return D3DXVECTOR3( m_ViewMatrix._12, 
							m_ViewMatrix._22, 
							m_ViewMatrix._32);
	}
	inline void GetLeft(D3DXVECTOR3& v)
	{
		Update();
		v.x = m_ViewMatrix._11; 
		v.y = m_ViewMatrix._21; 
		v.z = m_ViewMatrix._31;
	}
	inline D3DXVECTOR3 GetLeft()
	{
		Update();
		return D3DXVECTOR3(m_ViewMatrix._11, 
						   m_ViewMatrix._21,
						   m_ViewMatrix._31);
	}

	inline void Move(float x, float y, float z)
	{
		m_Eye.x += x;
		m_Eye.y += y;
		m_Eye.z += z;
		m_bDirty = TRUE;
	}
	inline void Move( const D3DXVECTOR3& v)
	{
		m_Eye += v;
		m_bDirty = TRUE;
	}
	inline void Rotate( const D3DXVECTOR3& vAxis, float angle)
	{
		D3DXQUATERNION q;
		D3DXQuaternionRotationAxis(&q, &vAxis, angle);
		m_Orientation *= q;
		m_bDirty = TRUE;
	}
	inline void Rotate( const D3DXQUATERNION& rot )
	{
		m_Orientation *= rot;
		m_bDirty = TRUE;
	}	
	inline void Yaw( float angle )
	{
		if ( m_bYawFixed )
		{	
			// Rotate around local Y axis
			Rotate( m_YawFixedAxis, angle );
		}
		else
		{
			// Rotate around world Y axis
			D3DXVECTOR3 v;
			PGEVec3MultiplyQuat( v, m_YawFixedAxis, m_Orientation );
			Rotate( v, angle );
		}
	}
	inline void Pitch( float angle )
	{
		// Rotate around world X axis
		D3DXVECTOR3 v(1, 0, 0);
		PGEVec3MultiplyQuat(v, v, m_Orientation);
		Rotate( v, angle );
	}
	inline void Roll(float angle )
	{
		// Rotate around world Z axis
		D3DXVECTOR3 v(0, 0, 1);
		PGEVec3MultiplyQuat(v, v, m_Orientation);
		Rotate( v, angle );
	}
	inline void SetYawPitchRoll(float yaw, float pitch, float roll)
	{
		D3DXQuaternionRotationYawPitchRoll(&m_Orientation, yaw, pitch, roll);
		m_bDirty = TRUE;
	}

	// 设置摄像机的方向
	// 'vDir'不必单位化
	void SetDirection( const D3DXVECTOR3& vDir );

	// 让摄像机指向目的点
	void LookAt( const D3DXVECTOR3& vTarget )
	{
		SetDirection( vTarget - m_Eye );
	}

	void SetVPMatrix( const D3DXMATRIX& _ViewMat , const D3DXMATRIX& _ProjMat );
public:

	bool operator == ( const PGECamera& _Cam2 ) const;

public:
		
	PGE_FRUSTUM_TEST IsVisible(const PGE_AABB& _Box	,	  PGE_FRUSTUM_PLANE* _CulledBy = NULL );
	PGE_FRUSTUM_TEST IsVisible(const PGE_OBB& _Box,		  PGE_FRUSTUM_PLANE* _CulledBy = NULL );
	PGE_FRUSTUM_TEST IsVisible(const PGE_SPHERE& _Sphere, PGE_FRUSTUM_PLANE* _CulledBy = NULL );	
	PGE_FRUSTUM_TEST IsVisible(const D3DXVECTOR3& _Point, PGE_FRUSTUM_PLANE* _CulledBy = NULL );
};

