#pragma once 

#include "BinStream.h"

struct BO_TRACK
{	
	int								BoneID;
	std::vector<D3DXQUATERNION>		RKF;
	std::vector<D3DXVECTOR3>		TKF;
};

class PAFConvert
{
public:

	float						m_Duration;
	std::vector<BO_TRACK>		m_Tracks;

	void Save( BinStream& _Stm );

	bool Load( BinStream& _Stm );
};