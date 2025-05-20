#pragma once
#include "PGE_Config.h"
#include "PGE_Aabb.h"

struct PGE_OBB
{
	D3DXVECTOR3 vXAxis;			// ��Χ�е�X��
	D3DXVECTOR3 vYAxis;			// ��Χ�е�Y��
	D3DXVECTOR3 vZAxis;			// ��Χ�е�Z��

	D3DXVECTOR3 vCenter;		// ��Χ�е�����
	D3DXVECTOR3 vHalfDim;		// ��Χ�еĴ�С(X, Y, Z) (ע:����Ĵ�СΪ�뾶)
								
	PGE_OBB() : vCenter(0, 0, 0)
		 	  , vXAxis(1, 0, 0)
			  , vYAxis(0, 1, 0)
			  , vZAxis(0, 0, 1)
			  , vHalfDim(0, 0, 0)
	{
	}

	// �õ���Χ�еİ˸�����( ˳��Ϊ:   vMin, (vMin.x, vMin.y, vMax.z)
	//								, (vMax.x, vMin.y, vMax.z), (vMax.x, vMin.y, vMin.z)
	//								, (vMin.x, vMax.y, vMin.z), (vMin.x, vMax.y, vMax.z)
	//								,  vMax,  (vMax.x, vMax.y, vMin.z)
	DEVICE_EXPORT void GetVerts( D3DXVECTOR3* pVerts  ) const;

	// ��һ�鶥�㼯���й������OBB
	DEVICE_EXPORT void Construct( const D3DXVECTOR3* points, DWORD nPoints );

	// �õ�OBB��ת������
	inline void GetTransformMatrix( D3DXMATRIX& _M) const
	{
		_M._11 = vXAxis.x; _M._12 = vYAxis.x; _M._13 = vZAxis.x; _M._14 = 0.0f;
		_M._21 = vXAxis.y; _M._22 = vYAxis.y; _M._23 = vZAxis.y; _M._24 = 0.0f;
		_M._31 = vXAxis.z; _M._32 = vYAxis.z; _M._33 = vZAxis.z; _M._34 = 0.0f;
		_M._41 = vCenter.x;_M._42 = vCenter.y;_M._43 = vCenter.z;_M._44 = 1.0f;
	}

	// ��һ��ת������任�˰�Χ��(������������, ֻ����ƽ�ƺ���ת )
	DEVICE_EXPORT void TransformByMatrix(const D3DXMATRIX& _M);

	// ��һ�������ת������Χ�еľֲ��ռ�
	DEVICE_EXPORT D3DXVECTOR3& TransformCoord( D3DXVECTOR3& _V, const D3DXVECTOR3& _Point ) const;

	// ��һ��������ת������Χ�еľֲ��ռ�
	DEVICE_EXPORT D3DXVECTOR3& TransformNormal( D3DXVECTOR3& _V, const D3DXVECTOR3& _Normal ) const;

	// ��һ��ƽ��ת������Χ�еľֲ��ռ�
	DEVICE_EXPORT D3DXPLANE& TransformPlane( D3DXPLANE& _P, const D3DXPLANE& _Plane ) const ;

	// ������������ϵ��һ����
	DEVICE_EXPORT void Add( const D3DXVECTOR3& _V );

	// ������������ϵ��һ��AABB
	DEVICE_EXPORT void Add( const PGE_AABB& _AABB );

	// ����һ��OBB
	DEVICE_EXPORT void Add( const PGE_OBB& _OBB );

	// �ж�һ�����Ƿ��������Χ����
	DEVICE_EXPORT bool PointInBox(const D3DXVECTOR3& _Point) const;

	// �ж����������һ��ƽ�����һ��
	// ����ֵ: 1(����), -1(����), 0(������ƽ��)
	DEVICE_EXPORT long ClassifyPlane(const D3DXPLANE &_Plane ) const;

	// �ж���������Ƿ��һ���߶��ཻ
	// ����:   start(���), end(�յ�), pDist�õ������ཻ�ľ���
	// ����ֵ: �ཻ��ƽ��
	DEVICE_EXPORT PGE_AABB_PLANE LineIntersect(  const D3DXVECTOR3& start
							  	 , const D3DXVECTOR3& end
								 , float* pDist = 0 ) const;
};