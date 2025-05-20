#include "../Include/PGE_Geometry.h"
#include <assert.h>

//-------------------------------------------------------------//
// class PGEGeometry
//-------------------------------------------------------------//
class PGEGeometry: public IPGEGeometry
{
	PGE_NOCOPY_ASSIGN(PGEGeometry)
public:
	// ������ü���
	virtual ULONG  AddRef();

	// �ͷ���Դ
	virtual ULONG Release();

	// �õ���������(�п���Ϊ����)
	virtual DWORD GetNumStreams() { return m_NumStreams; }

	// �õ�����ĸ�ʽ����
	virtual IVertexDeclaration* GetDeclaration() { return m_pVD; }

	// �õ����㻺��
	virtual IVertexBuffer* GetVertexBuffer(DWORD NumStream) { return m_pVBs[NumStream]; }

	// �õ�ÿ��������ֽ���
	virtual DWORD GetNumBytesPerVertex(DWORD NumStream)		{ return m_NumBytesPerVertex[NumStream]; }

	// �õ���������(����Ϊ�գ����ͼԪ����ͼԪ�б���ôӦ�û�����������)
	virtual IIndexBuffer* GetIndexBuffer() { return m_pIB; }

	// �õ�������
	virtual DWORD GetNumVertices() { return m_NumVertices; }

	// �õ�ͼԪ��
	virtual DWORD GetNumPrimitives() { return m_NumPrimitives; }

	// �õ�ͼԪ������
	virtual D3DPRIMITIVETYPE GetPrimitiveType() { return m_PrimitiveType; }

	// ����ͼԪ
	virtual HRESULT Draw( IGraphicDevice* pDevice );

public:
	PGEGeometry();
	~PGEGeometry();

	BOOL  Create(   IGraphicDevice* pDevice
		, DWORD dwNumVertices
		, DWORD dwNumPrimitives
		, D3DPRIMITIVETYPE PrimitiveType
		, const D3DVERTEXELEMENT9* pDecls 
		, DWORD dwOptions);
protected:
	ULONG				m_RefCount;
	WORD				m_NumStreams;
	WORD				m_MaxNumStream;
	IVertexDeclaration* m_pVD;
	IVertexBuffer*		m_pVBs[PGE_MAX_NUM_STREAMS];
	DWORD				m_NumBytesPerVertex[PGE_MAX_NUM_STREAMS];
	IIndexBuffer*		m_pIB;
	DWORD				m_NumVertices;
	DWORD				m_NumPrimitives;
	D3DPRIMITIVETYPE	m_PrimitiveType;
};

PGEGeometry::PGEGeometry()
{
	for (int i = 0; i < PGE_MAX_NUM_STREAMS; ++i)
	{
		m_pVBs[i] = NULL;
		m_NumBytesPerVertex[i] = 0;
	}
	m_pIB			= NULL;
	m_pVD			= NULL;
	m_RefCount		= 1;
	m_NumStreams    = 0;
	m_MaxNumStream  = 0;
	m_NumVertices   = 0;
	m_NumPrimitives = 0;
	m_PrimitiveType = D3DPT_FORCE_DWORD;
}

PGEGeometry::~PGEGeometry()
{
	for (int i = 0; i < PGE_MAX_NUM_STREAMS; ++i)
	{
		SAFE_RELEASE(m_pVBs[i]);
	}
	SAFE_RELEASE(m_pIB);
	SAFE_RELEASE(m_pVD);
}

ULONG PGEGeometry::AddRef()
{
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount);
}

ULONG PGEGeometry::Release()
{
	if ( InterlockedDecrement((LONG*)&m_RefCount) == 0 )
	{
		delete this;
		return 0;
	}else
	{
		return m_RefCount;
	}
}

BOOL PGEGeometry::Create(  IGraphicDevice* pDevice 
							, DWORD dwNumVertices 
							, DWORD dwNumPrimitives 
							, D3DPRIMITIVETYPE PrimitiveType
							, const D3DVERTEXELEMENT9* pDecls 
							, DWORD dwOptions)
{
	m_NumVertices   = dwNumVertices;
	m_NumPrimitives = dwNumPrimitives;
	m_PrimitiveType	= PrimitiveType;

	// Create vertex declaration
	m_pVD			= pDevice->CreateVertexDeclaration( pDecls );
	if ( m_pVD == NULL )
		return FALSE;

	UINT NumElements = 0;
	m_pVD->GetDeclaration(NULL, &NumElements);
	if (NumElements < 1)
		return FALSE;
	NumElements--;

	// Create vertex buffers	
	m_NumStreams	= 0;
	for (DWORD i = 0; i < NumElements; ++i)
	{
		WORD iStream = pDecls[i].Stream;
		assert( iStream < PGE_MAX_NUM_STREAMS );
		if ( m_pVBs[iStream] == NULL )
		{
			DWORD NumBytesPerVertex = D3DXGetDeclVertexSize( pDecls, iStream );

			D3DPOOL Pool = D3DPOOL_MANAGED;
			if ( dwOptions & PGEGEOM_VB_DEFAULT(iStream) )
				Pool = D3DPOOL_DEFAULT;
			else if ( dwOptions & PGEGEOM_VB_SYSTEMMEM(iStream) )
				Pool = D3DPOOL_SYSTEMMEM;

			m_NumBytesPerVertex[iStream] = NumBytesPerVertex;
			m_pVBs[iStream] = pDevice->CreateVertexBuffer( NumBytesPerVertex * dwNumVertices, 0, 0, Pool );
			m_NumStreams++;

			if ( iStream > m_MaxNumStream )
				m_MaxNumStream = iStream;

			if ( m_pVBs[iStream] == NULL )
				return FALSE;
		}
	}

	// Create index buffer
	DWORD IBCount = 0;
	if ( m_PrimitiveType == D3DPT_TRIANGLELIST )
	{
		IBCount = dwNumPrimitives * 3;
	}else if (m_PrimitiveType == D3DPT_LINELIST )
	{
		IBCount = dwNumPrimitives * 2;
	}else if ( m_PrimitiveType == D3DPT_POINTLIST )
	{
		IBCount = dwNumPrimitives;
	}

	if ( IBCount > 0 )
	{
		D3DPOOL Pool = D3DPOOL_MANAGED;
		if ( dwOptions & PGEGEOM_IB_DEFAULT )
			Pool = D3DPOOL_DEFAULT;
		else if ( dwOptions & PGEGEOM_IB_SYSTEMMEM )
			Pool = D3DPOOL_SYSTEMMEM;

		m_pIB = pDevice->CreateIndexBuffer( IBCount, 0, Pool);
		if ( m_pIB == NULL )
			return FALSE;
	}
	return TRUE;
}

HRESULT PGEGeometry::Draw( IGraphicDevice* pDevice )
{
	assert( pDevice && m_pVD );

	// Set vertex declaration
	pDevice->SetVertexDeclaration( m_pVD );

	// Set stream sources
	for (DWORD i = 0; i <= m_MaxNumStream; ++i )
	{
		pDevice->SetStreamSource( i, m_pVBs[i], 0, m_NumBytesPerVertex[i] );
	}

	if ( m_pIB )
	{	
		// Set index buffer
		pDevice->SetIndices(m_pIB);

		// Draw indexed primitive
		return pDevice->DrawIndexedPrimitive( m_PrimitiveType, 0, 0, m_NumVertices, 0, m_NumPrimitives );
	}else
	{
		// Draw primitive
		return pDevice->DrawPrimitive( m_PrimitiveType, 0, m_NumPrimitives );
	}
}

//--------------------------------------------------------------------
// Name: PGECreateGeometry
// Desc: ����������ӿ�
//-------------------------------------------------------------------
DEVICE_EXPORT IPGEGeometry* WINAPI PGECreateGeometry(	IGraphicDevice* pDevice
													  , DWORD dwNumVertices
													  , DWORD dwNumPrimitives
													  , DWORD dwNumStreams
													  , D3DPRIMITIVETYPE PrimitiveType
													  , const D3DVERTEXELEMENT9* pDecls
													  , DWORD dwOptions)
{
	assert( pDecls && pDevice );

	PGEGeometry* pGeom = new PGEGeometry();
	if ( !pGeom->Create( pDevice, dwNumVertices, dwNumPrimitives, PrimitiveType, pDecls, dwOptions ) )
	{
		pGeom->Release();
		return NULL;
	}
	return pGeom;
}

