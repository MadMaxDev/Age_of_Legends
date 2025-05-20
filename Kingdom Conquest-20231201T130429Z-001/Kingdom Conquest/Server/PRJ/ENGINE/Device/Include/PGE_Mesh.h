#pragma once

#include "PGE_Config.h"

struct PGE_ATTRIBUTE_RANGE
{
	DWORD FaceStart;
	DWORD FaceCount;
	DWORD VertexStart;
	DWORD VertexCount;
};

//-------------------------------------------------------------//
// ����: IPGEMesh
// ˵��: ����������ģ�ͽӿڣ����ڲ����õ��ǵ����ĸ�ʽ
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGEMesh
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// �õ����㻺��
	virtual IVertexBuffer* GetVertexBuffer() = 0;

	// �õ���������(����Ϊ�գ����ͼԪ����ͼԪ�б���ôӦ�û�����������)
	virtual IIndexBuffer* GetIndexBuffer() = 0;

	// �õ�����ĸ�ʽ����
	virtual IVertexDeclaration* GetDeclaration() = 0;

	// �õ��̶������ʽ����(�������ʧ�ܷ���ֵΪ0)
	virtual DWORD GetFVF() = 0;

	// �õ�ÿ��������ֽ���
	virtual DWORD GetNumBytesPerVertex() = 0;

	// �õ�������
	virtual DWORD GetNumVertices() = 0;

	// �õ���������
	virtual DWORD GetNumFaces() = 0;

	// ���Ƽ�����ͼԪ���Ӳ���
	virtual HRESULT DrawSubset( IGraphicDevice* pDevice, DWORD nID ) = 0;

	// ���Ƽ�����ͼԪ�������Ӳ���
	virtual HRESULT Draw( IGraphicDevice* pDevice ) = 0;

	// �����Χ��
	virtual void ComputeBoundingBox(const D3DXMATRIX* pWorldMatrix, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax) = 0;

	// �������Ա�
	virtual void SetAttributeTable( CONST PGE_ATTRIBUTE_RANGE * pAttribTable, DWORD cAttribTableSize ) = 0;

	// �õ����Ա�Ĵ�С
	virtual DWORD GetAttributeTableSize() = 0;

	// �õ����Ա�
	virtual const PGE_ATTRIBUTE_RANGE* GetAttributeTable() = 0;
};

//-------------------------------------------------------------//
// ����: IPGESkinInfo
// ˵��:
//-------------------------------------------------------------//
class IPGESkinInfo
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// �õ�����ģ����Ӱ��Ĺ�ͷ��
	virtual DWORD GetNumBones()				       = 0;

	// ���ù���ģ����Ӱ��Ĺ�ͷ��
	// NOTE: �˺���ͬʱ������ԭʼ��ͷ��������
	virtual void  SetNumBones(DWORD dwNumBones)    = 0;	

	// �õ�ԭʼ�Ĺ�ͷ��������
	virtual WORD* GetRawBoneIndices()		       = 0;

	// ����ԭʼ�Ĺ�ͷID�õ��µĹ�ͷID
	// ����ֵ�� ���û���ҵ��Ļ�������ֵΪ-1
	virtual WORD   GetRawBoneIndex( BYTE idx )     = 0;

	// ����ԭʼ�Ĺ�ͷID�õ��µĹ�ͷID
	virtual short  GetNewBoneIndex( WORD boneId )  = 0;

	// ����ӳ���µĹ�ͷ��������
	// pRawIndices��ԭʼ�Ĺ�ͷ���ݣ���ֵ��������ɢ��
	// ��Ϊ�Կ������ƣ����ǲ���ͬʱ����ܶ��ͷ�������
	// �Ǹ���ԭʼ�Ĺ�ͷ������������һ����ͷID��������������pNewIndices

	// ����ֵ��������ģ����Ӱ��Ĺ�ͷ������256����ʧ��
	virtual BOOL  RemapBlendIndices( WORD* pRawIndics, BYTE* pNewIndices, DWORD dwNumVerts ) = 0;
};

//--------------------------------------------------------------------
// Name : PGECreateSkinInfo
// Desc : 
//--------------------------------------------------------------------
DEVICE_EXPORT  IPGESkinInfo* WINAPI PGECreateSkinInfo();

//--------------------------------------------------------------------
// Name : PGECreateMesh
// Desc : ��������ģ�� 
//--------------------------------------------------------------------
DEVICE_EXPORT  IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice, DWORD dwNumVerts, DWORD dwNumFaces, DWORD dwFVF, DWORD dwOptions);
DEVICE_EXPORT  IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice, DWORD dwNumVerts, DWORD dwNumFaces, const D3DVERTEXELEMENT9* pDecls, DWORD dwOptions);


DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateSphere( IGraphicDevice* pDevice, DWORD dwNumRings, DWORD dwNumSegments, DWORD dwOptions );
DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateBox( IGraphicDevice* pDevice, DWORD dwOptions);
DEVICE_EXPORT	IPGEMesh* WINAPI PGECreateCylinder(IGraphicDevice* pDevice, DWORD dwNumStacks, DWORD dwOptions );
DEVICE_EXPORT	void	  WINAPI PGEComputeBoundingBox(const D3DXVECTOR3* pPositions, DWORD dwNumVerts, DWORD dwVertexStride, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax);



