#include "../include/pge_mesh.h"
#include <assert.h>
#include <map>


//-------------------------------------------------------------------
// 类名: PGEMesh
// 说明: 
//-------------------------------------------------------------------
class PGEMesh: public IPGEMesh
{
	PGE_NOCOPY_ASSIGN(PGEMesh)
public:
	PGEMesh();
	~PGEMesh();

	BOOL Create(IGraphicDevice* pDevice,  DWORD dwNumVerts, DWORD dwNumFaces, DWORD dwFVF, DWORD dwOptions);
	BOOL Create(IGraphicDevice* pDevice,  DWORD dwNumVerts, DWORD dwNumFaces, const D3DVERTEXELEMENT9* pDecls, DWORD dwOptions);
protected:
	BOOL Create(IGraphicDevice* pDevice,  DWORD dwOptions );
public:
	// 添加引用计数
	virtual ULONG  AddRef();

	// 释放资源
	virtual ULONG  Release();

	// 绘制几何体图元
	virtual HRESULT DrawSubset( IGraphicDevice* pDevice, DWORD nID );

	// 绘制几何体图元
	virtual HRESULT Draw( IGraphicDevice* pDevice );

	// 得到顶点缓冲
	virtual IVertexBuffer* GetVertexBuffer() { return m_pVB; }

	// 得到索引缓冲(可能为空，如果图元类型图元列表那么应该会有索引缓冲)
	virtual IIndexBuffer* GetIndexBuffer() { return m_pIB; }

	// 得到顶点的格式声明
	virtual IVertexDeclaration* GetDeclaration() { return m_pVD; }

	// 得到固定顶点格式声明(如果函数失败返回值为0)
	virtual DWORD GetFVF() { return m_dwFVF; }

	// 得到每个顶点的字节数
	virtual DWORD GetNumBytesPerVertex() { return m_NumBytesPerVertex; } 

	// 得到顶点数
	virtual DWORD GetNumVertices() { return m_NumVertices; }

	// 得到三角形数
	virtual DWORD GetNumFaces() { return m_NumFaces; }

	// 计算包围盒
	virtual void ComputeBoundingBox(const D3DXMATRIX* pWorldMatrix, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax)
	{
		vMin.x = vMin.y = vMin.z = FLT_MAX;
		vMax.x = vMax.y = vMax.z = -FLT_MAX;

		if ( m_pVB )
		{
			D3DXVECTOR3* pVerts = (D3DXVECTOR3*)m_pVB->Lock(0, 0, 0);
			
			if (pWorldMatrix)
			{
				BYTE* pPos = (BYTE*)pVerts;
				D3DXVECTOR3 p;
				for (DWORD i = 0; i < m_NumVertices; ++i )
				{
					D3DXVec3TransformCoord(&p, (D3DXVECTOR3*)pPos, pWorldMatrix );

					if ( p.x < vMin.x ) vMin.x = p.x;
					if ( p.y < vMin.y ) vMin.y = p.y;
					if ( p.z < vMin.z ) vMin.z = p.z;

					if ( p.x > vMax.x ) vMax.x = p.x;
					if ( p.y > vMax.y ) vMax.y = p.y;
					if ( p.z > vMax.z ) vMax.z = p.z;

					// Move to next vertex
					pPos += m_NumBytesPerVertex;
				}
			}else
			{
				PGEComputeBoundingBox( pVerts, m_NumVertices, m_NumBytesPerVertex, vMin, vMax );
			}
			m_pVB->Unlock();
		}
	}

	// 设置属性表
	virtual void SetAttributeTable( CONST PGE_ATTRIBUTE_RANGE * pAttribTable, DWORD cAttribTableSize )
	{
		if ( cAttribTableSize > 0 )
		{
			m_AttrTable.resize( cAttribTableSize );
			memcpy( &m_AttrTable[0], pAttribTable, sizeof(PGE_ATTRIBUTE_RANGE) * cAttribTableSize );
		}
	}

	// 得到属性表的大小
	virtual DWORD GetAttributeTableSize() 						 { return (DWORD)m_AttrTable.size(); }

	// 得到属性表
	virtual const PGE_ATTRIBUTE_RANGE* GetAttributeTable()		 { return &m_AttrTable[0];			 }

protected:	
	ULONG		   m_RefCount;
	IVertexBuffer* m_pVB;
	IIndexBuffer*  m_pIB;
	IVertexDeclaration* m_pVD;
	DWORD		   m_dwFVF;
	DWORD		   m_NumBytesPerVertex;
	DWORD		   m_NumVertices;
	DWORD		   m_NumFaces;
	std::vector< PGE_ATTRIBUTE_RANGE > m_AttrTable;
};


PGEMesh::PGEMesh():m_pVB(NULL),m_pIB(NULL),m_pVD(NULL),m_RefCount(1)
{

}
PGEMesh::~PGEMesh()
{
	SAFE_RELEASE(m_pVD);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
}

ULONG PGEMesh::AddRef() 
{ 
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount);
}

ULONG PGEMesh::Release() 
{
	if (InterlockedDecrement((LONG*)&m_RefCount) == 0)
	{
		delete this;
		return 0;
	}else
	{
		return m_RefCount;
	}
}

BOOL PGEMesh::Create(IGraphicDevice* pDevice,  DWORD dwNumVerts, DWORD dwNumFaces, DWORD dwFVF, DWORD dwOptions)
{
	D3DVERTEXELEMENT9 decls[MAXD3DDECLLENGTH+1];
	if (FAILED(D3DXDeclaratorFromFVF( dwFVF, decls )))
	{
		return FALSE;
	}
	// Create vertex declaration
	m_pVD = pDevice->CreateVertexDeclaration( decls );
	if (m_pVD == NULL )
		return FALSE;

	m_dwFVF					= dwFVF;
	m_NumFaces				= dwNumFaces;
	m_NumVertices			= dwNumVerts;
	m_RefCount				= 1;
	m_NumBytesPerVertex		= D3DXGetFVFVertexSize(dwFVF);

	return Create( pDevice, dwOptions );
}

BOOL PGEMesh::Create(IGraphicDevice* pDevice,  DWORD dwNumVerts, DWORD dwNumFaces, const D3DVERTEXELEMENT9* pDecls, DWORD dwOptions)
{
	m_pVD = pDevice->CreateVertexDeclaration( pDecls );
	if (m_pVD == NULL )
		return FALSE;

	// Get FVF
	if (FAILED( D3DXFVFFromDeclarator( pDecls, &m_dwFVF ) ))
	{
		m_dwFVF = 0;
	}

	m_NumFaces				= dwNumFaces;
	m_NumVertices			= dwNumVerts;
	m_RefCount				= 1;
	m_NumBytesPerVertex		= D3DXGetDeclVertexSize(pDecls, 0);

	return Create( pDevice, dwOptions );
}

BOOL PGEMesh::Create(IGraphicDevice* pDevice,  DWORD dwOptions )
{
	// Initialize attribute table
	m_AttrTable.resize( 1 );
	m_AttrTable[0].FaceStart = 0;
	m_AttrTable[0].FaceCount = m_NumFaces;
	m_AttrTable[0].VertexStart = 0;
	m_AttrTable[0].VertexCount = m_NumVertices;

	D3DPOOL pool;
	if ( dwOptions & PGEMESH_VB_SYSTEMMEM)
		pool = D3DPOOL_SYSTEMMEM;
	else if ( dwOptions & PGEMESH_VB_DEFAULT)
		pool = D3DPOOL_DEFAULT;	
	else
		pool = D3DPOOL_MANAGED;

	// Create vertex buffer
	m_pVB = pDevice->CreateVertexBuffer( m_NumVertices * m_NumBytesPerVertex, 0, 0, pool );
	if ( m_pVB == NULL )
		return FALSE;

	if ( dwOptions & PGEMESH_IB_SYSTEMMEM)
		pool = D3DPOOL_SYSTEMMEM;
	else if ( dwOptions & PGEMESH_IB_DEFAULT )
		pool = D3DPOOL_DEFAULT;	
	else
		pool = D3DPOOL_MANAGED;

	// Create index buffer
	m_pIB = pDevice->CreateIndexBuffer( m_NumFaces * 3, 0, pool );

	if (m_pIB == NULL)
		return FALSE;

	return TRUE;
}

HRESULT PGEMesh::DrawSubset( IGraphicDevice* pDevice, DWORD nID  )
{	
	assert( pDevice );
	if ( nID >= (DWORD)m_AttrTable.size())
	{
		return E_FAIL;
	}
	PGE_ATTRIBUTE_RANGE& attr = m_AttrTable[nID];

	// Set vertex buffer
	pDevice->SetStreamSource( 0, m_pVB, 0, m_NumBytesPerVertex );

	// Set index buffer 
	pDevice->SetIndices( m_pIB );

	return pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST
										, attr.VertexStart
										, 0
										, attr.VertexCount
										, attr.FaceStart * 3
										, attr.FaceCount );
}

// 绘制几何体图元的所有子部分
HRESULT PGEMesh::Draw( IGraphicDevice* pDevice )
{
	assert( pDevice );
	// Set vertex buffer
	pDevice->SetStreamSource( 0, m_pVB, 0, m_NumBytesPerVertex );

	// Set index buffer 
	pDevice->SetIndices( m_pIB );

	HRESULT hr;
	for (size_t i = 0; i < m_AttrTable.size(); ++i)
	{
		PGE_ATTRIBUTE_RANGE& attr = m_AttrTable[i];

		if ( FAILED( hr = pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST
											, attr.VertexStart
											, 0
											, attr.VertexCount
											, attr.FaceStart * 3
											, attr.FaceCount ) ))
		{
			return hr;
		}
	}
	return S_OK;
}

//-------------------------------------------------------------//
// class PGESkinInfo
//-------------------------------------------------------------//
class PGESkinInfo: public IPGESkinInfo
{
	LONG			   m_RefCount;
	DWORD			   m_NumBones;
	WORD*			   m_RawBoneIndices;

	typedef std::map<WORD, BYTE> BoneMap_t;
	BoneMap_t m_OldMapNewBones;
public:
	PGESkinInfo();
	~PGESkinInfo();

public:
	// 添加引用计数
	virtual ULONG  AddRef();

	// 释放资源
	virtual ULONG  Release();

	// 得到骨骼模型受影响的骨头数
	virtual DWORD GetNumBones() { return m_NumBones; }

	// 设置骨骼模型受影响的骨头数
	// NOTE: 此函数同时分配了原始骨头索引数据
	virtual void  SetNumBones(DWORD dwNumBones);	

	// 得到原始的骨头索引数据
	virtual WORD* GetRawBoneIndices() { return m_RawBoneIndices; }

	// 根据新的骨头ID得到原始的骨头ID
	virtual WORD  GetRawBoneIndex( BYTE idx ) { return m_RawBoneIndices[idx]; }

	// 根据原始的骨头ID得到新的骨头ID
	// 返回值： 如果没有找到的话，返回值为-1
	virtual short GetNewBoneIndex( WORD boneId );

	// 重新映射新的骨头索引数据
	// pRawIndices是原始的骨头数据，其值可能是离散的
	// 因为显卡的限制，我们不能同时处理很多骨头，因此我
	// 们根据原始的骨头索引重新生成一个骨头ID连续的索引数据pNewIndices

	// 返回值：如果这个模型受影响的骨头数大于256个会失败
	virtual BOOL RemapBlendIndices( WORD* pRawIndics, BYTE* pNewIndices, DWORD dwNumVerts );
};

//-------------------------------------------------------------//
// class PGESkinInfo
//-------------------------------------------------------------//
PGESkinInfo::PGESkinInfo():   m_RawBoneIndices(NULL),
							  m_NumBones(0),
							  m_RefCount(1)
{
}

PGESkinInfo::~PGESkinInfo()
{
	SAFE_DELETE_ARRAY(m_RawBoneIndices);
}

ULONG  PGESkinInfo::AddRef() 
{
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount); 
}

ULONG  PGESkinInfo::Release() 
{
	if (InterlockedDecrement((LONG*)&m_RefCount) == 0)
	{
		delete this;
		return 0;
	}else
	{
		return m_RefCount;
	}
}

BOOL PGESkinInfo::RemapBlendIndices( WORD* pIndices, BYTE* pNewIndices, DWORD dwNumVerts )
{
	// Delete old memory
	SAFE_DELETE_ARRAY(m_RawBoneIndices);
	m_OldMapNewBones.clear();

	WORD newBoneId = 0;
	{	
		BYTE* pDestIndices = pNewIndices;
		WORD* pSrcIndices  = pIndices;

		for (DWORD i = 0; i < dwNumVerts; ++i)
		{
			for (DWORD k = 0; k < 4; ++k)
			{
				WORD  boneId = *pSrcIndices;

				BoneMap_t::iterator it = m_OldMapNewBones.find(boneId);
				if (it != m_OldMapNewBones.end())
				{
					*pDestIndices = it->second;
				}else
				{
					*pDestIndices = BYTE(newBoneId);
					m_OldMapNewBones[boneId] = BYTE(newBoneId);

					++newBoneId;
				}
				++pDestIndices;
				++pSrcIndices;
			}
		}
	}

	// Make mapped blend indices
	{
		if ( newBoneId > 256 )
			return FALSE;

		m_NumBones = newBoneId;
		m_RawBoneIndices = new WORD[m_NumBones];
		BoneMap_t::iterator it = m_OldMapNewBones.begin();
		for (; it != m_OldMapNewBones.end(); ++it)
		{
			m_RawBoneIndices[it->second] = it->first;
		}
		return TRUE;
	}
}

short PGESkinInfo::GetNewBoneIndex( WORD idx ) 
{	
	BoneMap_t::const_iterator it = m_OldMapNewBones.find( idx );

	if ( it != m_OldMapNewBones.end() )
		return (short)it->second;
	else
		return -1;
}


void  PGESkinInfo::SetNumBones(DWORD dwNumBones)
{
	SAFE_DELETE_ARRAY(m_RawBoneIndices);

	m_RawBoneIndices = new WORD[dwNumBones];
	m_NumBones = dwNumBones;
}

//--------------------------------------------------------------------
// Name : PGECreateSkinInfo
// Desc : 
//--------------------------------------------------------------------
DEVICE_EXPORT  IPGESkinInfo* WINAPI PGECreateSkinInfo()
{
	return new PGESkinInfo();
}

//--------------------------------------------------------------------
// Name : PGECreateMesh
// Desc : 创建网格模型 
//--------------------------------------------------------------------
DEVICE_EXPORT IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice,  DWORD dwNumVerts, DWORD dwNumFaces, DWORD dwFVF, DWORD dwOptions)
{
	PGEMesh* pMesh = new PGEMesh();
	if (pMesh->Create(pDevice, dwNumVerts, dwNumFaces, dwFVF, dwOptions))
	{	
		return pMesh;
	}else
	{
		pMesh->Release();
		return NULL;
	}
}

DEVICE_EXPORT IPGEMesh* WINAPI PGECreateMesh(IGraphicDevice* pDevice, DWORD dwNumVerts, DWORD dwNumFaces, const D3DVERTEXELEMENT9* pDecls, DWORD dwOptions)
{
	PGEMesh* pMesh = new PGEMesh();
	if (pMesh->Create(pDevice, dwNumVerts, dwNumFaces, pDecls, dwOptions))
	{	
		return pMesh;
	}else
	{
		pMesh->Release();
		return NULL;
	}
}


DEVICE_EXPORT IPGEMesh* WINAPI PGECreateSphere( IGraphicDevice* pDevice, DWORD dwNumRings, DWORD dwNumSegments , DWORD dwOptions)
{
	DWORD dwNumVerts = dwNumRings * dwNumSegments;
	DWORD dwNumFaces = (dwNumRings - 1) * dwNumSegments * 2;

	IPGEMesh* pMesh = PGECreateMesh( pDevice, dwNumVerts, dwNumFaces, D3DFVF_XYZ, dwOptions );
	if ( pMesh )
	{
		// Fill vertex buffer
		{		
			D3DXVECTOR3* pPos = (D3DXVECTOR3*)pMesh->GetVertexBuffer()->Lock(0, 0, 0);

			float ThetaDelta = D3DX_PI * 2.0f/(float)dwNumSegments;
			float PhiDelta   = D3DX_PI/(float)(dwNumRings - 1);
			float Phi = D3DX_PI*0.5f;

			for (DWORD iRing = 0; iRing < dwNumRings; ++iRing)
			{		
				float Theta = 0;

				for (DWORD iSegment = 0; iSegment < dwNumSegments; ++iSegment)
				{	
					D3DXVECTOR3 n(cosf(Phi) * cosf(Theta),
								  sinf(Phi),
								  cosf(Phi) * sinf(Theta));
					// Set position
					*pPos = n;
					++pPos;

					Theta += ThetaDelta;
				}
				Phi -= PhiDelta;
			}
			pMesh->GetVertexBuffer()->Unlock();
		}

		// Fill index buffer
		{
			WORD* pIndices = (WORD*)pMesh->GetIndexBuffer()->Lock(0, 0, 0);

			for (DWORD iRing = 0; iRing < dwNumRings - 1; ++iRing)
				for (DWORD iSegment = 0; iSegment < dwNumSegments; ++iSegment)
				{
					DWORD i0 = (iRing + 1) *(dwNumSegments) + iSegment;
					DWORD i1 = i0 - dwNumSegments;

					DWORD i2 = i0 + 1;
					DWORD i3 = i1 + 1;
					if ( iSegment == dwNumSegments - 1 )
					{
						i2 = i0 - iSegment;
						i3 = i1 - iSegment;
					}

					// Add a quad
					*pIndices++ = (WORD)i0;
					*pIndices++ = (WORD)i1;
					*pIndices++ = (WORD)i2;

					*pIndices++ = (WORD)i2;
					*pIndices++ = (WORD)i1;
					*pIndices++ = (WORD)i3;
				}
			pMesh->GetIndexBuffer()->Unlock();
		}
	}
	return pMesh;
}

DEVICE_EXPORT IPGEMesh* WINAPI PGECreateBox( IGraphicDevice* pDevice , DWORD dwOptions)
{
	IPGEMesh* pMesh = PGECreateMesh(pDevice,  24, 12, D3DFVF_XYZ, dwOptions);
	if ( pMesh )
	{
		// Top Plane
		D3DXVECTOR3* pPos = (D3DXVECTOR3*)pMesh->GetVertexBuffer()->Lock(0, 0, 0);
		*pPos++ = D3DXVECTOR3( -0.5f, 0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f, 0.5f, 0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f, 0.5f );

		// Bottom Plane
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f, 0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f, -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f, -0.5f, 0.5f );

		// Left Plane
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f,  0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f,  0.5f,  0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f,  0.5f, -0.5f );

		// Right Plane
		*pPos++ = D3DXVECTOR3( 0.5f, -0.5f,  0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f,  0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f, -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f, -0.5f );

		// Back plane
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f,  0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  -0.5f, -0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f,  -0.5f );

		// Front plane
		*pPos++ = D3DXVECTOR3( -0.5f, -0.5f, 0.5f );
		*pPos++ = D3DXVECTOR3( -0.5f,  0.5f, 0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  -0.5f, 0.5f );
		*pPos++ = D3DXVECTOR3( 0.5f,  0.5f,  0.5f );
		pMesh->GetVertexBuffer()->Unlock();

		// Fill index buffer
		WORD* pIndices = (WORD*)pMesh->GetIndexBuffer()->Lock(0, 0, 0);
		BOOL bSwap = FALSE;
		for (WORD i = 0; i < 6; ++i)
		{
			WORD s = (WORD)i*4;
			if (bSwap)
			{
				pIndices[0] = s + 0;
				pIndices[1] = s + 2;
				pIndices[2] = s + 1;
				pIndices[3] = s + 2;
				pIndices[4] = s + 3;
				pIndices[5] = s + 1;
			}else
			{
				pIndices[0] = s + 0;
				pIndices[1] = s + 1;
				pIndices[2] = s + 2;
				pIndices[3] = s + 2;
				pIndices[4] = s + 1;
				pIndices[5] = s + 3;
			}
			bSwap = !bSwap;
			pIndices += 6;
		}
		pMesh->GetIndexBuffer()->Unlock();
	}
	return pMesh;
}

DEVICE_EXPORT IPGEMesh* WINAPI PGECreateCylinder(IGraphicDevice* pDevice, DWORD dwNumStacks , DWORD dwOptions)
{
	DWORD dwNumVerts =  dwNumStacks * 2 ;
	DWORD dwNumFaces = dwNumStacks * 2 ;

	IPGEMesh* pMesh = PGECreateMesh( pDevice, dwNumVerts, dwNumFaces, D3DFVF_XYZ, dwOptions);
	if (pMesh == NULL)
		return NULL;

	D3DXVECTOR3* pPos = (D3DXVECTOR3*)pMesh->GetVertexBuffer()->Lock(0, 0, 0);
	WORD* pIndices = (WORD*)pMesh->GetIndexBuffer()->Lock(0, 0, 0);;

	// Create a cylinder
	float ThetaDelta = D3DX_PI * 2.0f/(float)dwNumStacks;
	float Theta = 0;
	for (DWORD i = 0; i < dwNumStacks; ++i)
	{
		float x = cosf(Theta);
		float z = sinf(Theta);
		
		D3DXVECTOR3 n0(x, -0.5f, z);
		D3DXVECTOR3 n1(x, 0.5f, z);

		*pPos++ = n0;
		*pPos++ = n1;
		Theta += ThetaDelta;
	}

	for (DWORD i = 0; i < dwNumStacks; ++i)
	{
		WORD s = (WORD)i*2;
		WORD i0 = s + 0;
		WORD i1 = s + 1;

		WORD i2 = s + 2;
		WORD i3 = s + 3;
		if ( i == dwNumStacks - 1)
		{
			i2 = 0;
			i3 = 1;
		}
		*pIndices++ = i0;
		*pIndices++ = i1;
		*pIndices++ = i2;

		*pIndices++ = i2;
		*pIndices++ = i1;
		*pIndices++ = i3;
	}

	// Unlock vertex buffer and index buffer
	pMesh->GetVertexBuffer()->Unlock();
	pMesh->GetIndexBuffer()->Unlock();

	return pMesh;
}

DEVICE_EXPORT void WINAPI PGEComputeBoundingBox(const D3DXVECTOR3* pPositions, DWORD dwNumVerts, DWORD dwVertexStride, D3DXVECTOR3& vMin, D3DXVECTOR3& vMax)
{
	vMin.x = vMin.y = vMin.z = FLT_MAX;
	vMax.x = vMax.y = vMax.z = -FLT_MAX;

	for (DWORD i = 0; i < dwNumVerts; ++i)
	{
		D3DXVECTOR3* p = (D3DXVECTOR3*)((BYTE*)pPositions + i * dwVertexStride);
		if ( p->x < vMin.x ) vMin.x = p->x;
		if ( p->y < vMin.y ) vMin.y = p->y;
		if ( p->z < vMin.z ) vMin.z = p->z;

		if ( p->x > vMax.x ) vMax.x = p->x;
		if ( p->y > vMax.y ) vMax.y = p->y;
		if ( p->z > vMax.z ) vMax.z = p->z;
	}
}

