#include "stdafx.h"
#include "PMFConvert.h"

struct PGE_ATTRIBUTE_RANGE
{
	DWORD FaceStart;
	DWORD FaceCount;
	DWORD VertexStart;
	DWORD VertexCount;
};

struct PMF_MESH_HEAD
{
	DWORD Magic;
	DWORD Version;
	DWORD bPosition		: 1;
	DWORD bBlendWeight	: 1;
	DWORD bBlendIndex	: 1;	
	DWORD bWeight		: 1;	
	DWORD bNormal		: 1;	
	DWORD bTangent		: 1;
	DWORD bDiffuses		: 1;
	DWORD bSpecular		: 1;
	DWORD bUnused		: 24;
	DWORD NumVertices;
	DWORD NumFaces;
	DWORD NumTCs;
	DWORD AttrTableSize;
};

struct PMFMeshData
{
	DWORD				 AttrTableSize;	
	DWORD				 NumFaces;
	DWORD				 NumVertices;
	DWORD				 NumTCs;

	PGE_ATTRIBUTE_RANGE* pAttrTable;
	D3DXVECTOR3*		 pPositions;
	D3DXVECTOR3*		 pBlendWeights;
	WORD*				 pBlendIndices;
	FLOAT*				 pWeights;
	D3DXVECTOR3*		 pNormals;
	D3DXVECTOR3*		 pTangents;
	D3DCOLOR*			 pDiffuses;
	D3DCOLOR*			 pSpeculars;
	D3DXVECTOR2*		 pTC0s;
	D3DXVECTOR2*		 pTC1s;
	D3DXVECTOR2*		 pTC2s;
	D3DXVECTOR2*		 pTC3s;
	WORD*				 pFaceIndices;

	PMFMeshData()
	{
		pAttrTable		 = NULL;
		pPositions		 = NULL;
		pBlendWeights	 = NULL;
		pBlendIndices	 = NULL;
		pWeights		 = NULL;
		pNormals		 = NULL;
		pTangents		 = NULL;
		pDiffuses		 = NULL;
		pSpeculars		 = NULL;
		pTC0s			 = NULL;
		pTC1s			 = NULL;
		pTC2s			 = NULL;
		pTC3s			 = NULL;
		pFaceIndices	 = NULL;
		NumFaces		 = 0;
		NumVertices		 = 0;
		AttrTableSize	 = 0;
		NumTCs			 = 0;
	}
};

BOOL PGEParsePMFFileInMemory( PMFMeshData& meshData, void* pFileData, DWORD DataLength )
{
	//-------------------------------------------------------------//
	const DWORD	PMF_MAGIC = MAKEFOURCC('P', 'M', 'F', '\0');
	const DWORD	PMF_VERSION = 200;
	//-------------------------------------------------------------//

	PMF_MESH_HEAD FileHead;

	BYTE* p = (BYTE*)pFileData;

	// Read file head size
	DWORD FileHeadSize = *( (DWORD*)p );	
	FileHeadSize = __min(FileHeadSize, sizeof(FileHead));
	p += sizeof(DWORD);

	// Read file head	
	memset(&FileHead, 0, sizeof(FileHead) );
	memcpy(&FileHead, p, FileHeadSize );
	p += FileHeadSize;

	if ( FileHead.Magic != PMF_MAGIC || FileHead.Version != PMF_VERSION )
	{
		return FALSE;
	}

	DWORD NumVertices = FileHead.NumVertices;
	meshData.NumVertices = FileHead.NumVertices;
	meshData.NumFaces	 = FileHead.NumFaces;
	meshData.NumTCs		 = FileHead.NumTCs;
	meshData.AttrTableSize = FileHead.AttrTableSize;

	// Read attribute table
	if ( meshData.AttrTableSize > 0 )
	{
		meshData.pAttrTable = (PGE_ATTRIBUTE_RANGE*)p;
		p += sizeof(PGE_ATTRIBUTE_RANGE) * meshData.AttrTableSize;
	}

	if ( FileHead.bPosition )
	{
		meshData.pPositions = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bBlendWeight  )
	{
		meshData.pBlendWeights = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bBlendIndex )
	{
		meshData.pBlendIndices = (WORD*)p;
		p += sizeof(WORD) * NumVertices * 4;
	}

	if ( FileHead.bNormal )
	{
		meshData.pNormals = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bTangent  )
	{
		meshData.pTangents = (D3DXVECTOR3*)p;
		p += sizeof(D3DXVECTOR3) * NumVertices;
	}

	if ( FileHead.bWeight )
	{
		meshData.pWeights = (float*)p;
		p += sizeof(FLOAT) * NumVertices;
	}

	if ( FileHead.bDiffuses )
	{
		meshData.pDiffuses = (DWORD*)p;
		p += sizeof(DWORD) * NumVertices;
	}

	if ( FileHead.bSpecular )
	{
		meshData.pSpeculars = (DWORD*)p;
		p += sizeof(DWORD) * NumVertices;
	}

	if ( FileHead.NumTCs > 0 )
	{
		meshData.pTC0s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 1  )
	{
		meshData.pTC1s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 2  )
	{
		meshData.pTC2s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}
	if ( FileHead.NumTCs > 3  ) 
	{
		meshData.pTC3s = (D3DXVECTOR2*)p;
		p += sizeof(D3DXVECTOR2) * NumVertices;
	}

	// Read face indices
	meshData.pFaceIndices = (WORD*)p;
	p += sizeof(WORD) * 3 * meshData.NumFaces;

	return TRUE;
}

extern size_t BoneCount;

void PMFConvert::Save( BinStream& _Stm )
{
	unsigned long magic_token	=	*(unsigned long*)"CMF";
	unsigned long file_version	=	820;
	unsigned long submesh_cnt	=	1;
	std::string	  mat_id		=	"NONE";     
	unsigned long vtx_cnt		=	(unsigned long )m_Vertexes.size(); 
	unsigned long tri_cnt		=	(unsigned long )m_Triangles.size();
	unsigned long lod_cnt		=	0;   
	unsigned long spr_cnt		=	m_IsSoftBody ? 1 : 0;
	unsigned long mat_cnt		=	m_MatrixCount;	
	unsigned long map_cnt		=	1;   
	
	_Stm << magic_token;
	_Stm << file_version;
	_Stm << submesh_cnt;
	_Stm << mat_id;
	_Stm << vtx_cnt;
	_Stm << tri_cnt;
	_Stm << lod_cnt;
	_Stm << spr_cnt;
	_Stm << mat_cnt;
	_Stm << map_cnt;

	for( unsigned long index = 0 ; index < vtx_cnt ; ++index )
	{
		BO_VERTEX&	vertex = m_Vertexes[index];
		D3DXVECTOR3 pos = vertex.Position;

		pos *= 1.0f / 0.0254f;

		std::swap( pos.y , pos.z ); 
		D3DXVECTOR3 nor = vertex.Normal;
		std::swap( nor.y , nor.z ); 
		D3DXVECTOR2 tc = vertex.Tc;
		tc.y = 1.0f - tc.y;
		_Stm << pos;
		_Stm << nor;
		_Stm << 0UL;
		_Stm << 0UL;
		_Stm << tc;
		_Stm << 4UL;
		for( unsigned long j = 0 ; j < 4 ; ++j )
		{
			unsigned long	bone_id		=	vertex.BlendBones[j];
			float			bone_weight	=	vertex.BlendWeights[j];
			
			if( bone_id >= BoneCount )
			{
				bone_id = 0;
			}
			if( bone_weight > 1.0f )
			{
				bone_weight = 1.0f;
			}
			if( bone_weight < 0.0f )
			{
				bone_weight = 0.0f;
			}

			_Stm << bone_id;
			_Stm << bone_weight;
		}
		if( m_IsSoftBody )
		{
			float weight = vertex.Weight;
			_Stm << weight;
		}
	}

	if( m_IsSoftBody )
	{
		unsigned long	id1 = 0UL;
		unsigned long	id2 = 0UL;
		float			l	= 0.0f;
		float			k	= 0.0f;
		_Stm << id1;
		_Stm << id2;
		_Stm << l;
		_Stm << k;
	}

	for( unsigned long index = 0 ; index < tri_cnt ; ++index )
	{
		BO_TRIANGLE& tri = m_Triangles[index];
		unsigned long idx0 = tri.Idx[0];
		unsigned long idx1 = tri.Idx[1];
		unsigned long idx2 = tri.Idx[2];

		_Stm << idx0;
		_Stm << idx1;
		_Stm << idx2;
	}
}

bool PMFConvert::Load( BinStream& _Stm )
{
	int s = _Stm.Size();
	BYTE* pBuffer = new BYTE[ s ];
	_Stm.Read( pBuffer , s );
	PMFMeshData MData;
	if( ! PGEParsePMFFileInMemory( MData , pBuffer , s ) )
	{
		delete[] pBuffer;
		return false;
	}

	m_Vertexes.resize( MData.NumVertices );
	m_Triangles.resize( MData.NumFaces );
	
	bool b_bit[ 256 ]; 

	for( size_t i = 0 ; i < MData.NumVertices ; ++i )
	{
		BO_VERTEX& vtx = m_Vertexes[ i ];
		if( MData.pPositions != NULL )
		{
			vtx.Position = MData.pPositions[ i ];
		}
		if( MData.pNormals != NULL )
		{
			vtx.Normal = MData.pNormals[ i ];
		}
		if( MData.pTC0s != NULL )
		{
			vtx.Tc = MData.pTC0s[ i ];
		}
		if( MData.pWeights != NULL )
		{	
			vtx.Weight = MData.pWeights[ i ];
		}
		if( MData.pBlendWeights != NULL )
		{
			vtx.BlendWeights[0] = MData.pBlendWeights[ i ].x;
			vtx.BlendWeights[1] = MData.pBlendWeights[ i ].y;
			vtx.BlendWeights[2] = MData.pBlendWeights[ i ].z;
			vtx.BlendWeights[3] = 1.0f - vtx.BlendWeights[0] - vtx.BlendWeights[1] - vtx.BlendWeights[2];
		}
		if( MData.pBlendIndices )
		{
			BYTE b0 = (BYTE)MData.pBlendIndices[ i * 4 + 0 ];
			BYTE b1 = (BYTE)MData.pBlendIndices[ i * 4 + 1 ];
			BYTE b2 = (BYTE)MData.pBlendIndices[ i * 4 + 2 ];
			BYTE b3 = (BYTE)MData.pBlendIndices[ i * 4 + 3 ];

			vtx.BlendBones[0] = b0;
			vtx.BlendBones[1] = b1;
			vtx.BlendBones[2] = b2;
			vtx.BlendBones[3] = b3;

			b_bit[ b0 ] = true;
			b_bit[ b1 ] = true;
			b_bit[ b2 ] = true;
			b_bit[ b3 ] = true;
		
		}
	}

	m_IsSoftBody = MData.pWeights != NULL;
	m_MatrixCount = 0;
	for( int i = 0 ; i < 256 ; ++i )
	{
		if( b_bit[ i ] )
		{
			m_MatrixCount++;
		}
	}
	
	for( size_t i = 0 ; i < MData.NumFaces ; ++i )
	{
		BO_TRIANGLE& tri = m_Triangles[ i ];
		tri.Idx[ 0 ] = MData.pFaceIndices[ i * 3 + 0 ];
		tri.Idx[ 1 ] = MData.pFaceIndices[ i * 3 + 1 ];
		tri.Idx[ 2 ] = MData.pFaceIndices[ i * 3 + 2 ];
	}

	delete[] pBuffer;
	return true;
}

