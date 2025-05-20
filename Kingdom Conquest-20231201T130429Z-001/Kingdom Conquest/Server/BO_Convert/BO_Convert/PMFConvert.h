#pragma once

#include "BinStream.h"

struct BO_VERTEX
{
	D3DXVECTOR3						Position;
	D3DXVECTOR3						Normal;
	D3DXVECTOR2						Tc;
	float							Weight;
	unsigned char					BlendBones[4];
	float							BlendWeights[4];

	BO_VERTEX()
	{
		::ZeroMemory( this , sizeof(*this) );
	}
};

struct BO_TRIANGLE
{
	unsigned short Idx[ 3 ];
};

class PMFConvert
{
public:

	bool							m_IsSoftBody;
	unsigned long					m_MatrixCount;
	std::vector<BO_VERTEX>			m_Vertexes;
	std::vector<BO_TRIANGLE>		m_Triangles;

	void Save( BinStream& _Stm );

	bool Load( BinStream& _Stm );
};