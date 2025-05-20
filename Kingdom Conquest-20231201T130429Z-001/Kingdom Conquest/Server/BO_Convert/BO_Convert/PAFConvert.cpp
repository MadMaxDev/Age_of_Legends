#include "stdafx.h"
#include "PAFConvert.h"

void PAFConvert::Save( BinStream& _Stm )
{
	unsigned long valid_tag		= *(unsigned long*)"CAF";
	unsigned long valid_version	= 820; 
	_Stm << valid_tag;
	_Stm << valid_version;
	unsigned long tracks_count = unsigned long( m_Tracks.size() );
	_Stm << m_Duration;
	_Stm << tracks_count;
	for( unsigned long i = 0 ; i < tracks_count ; ++i )
	{	
		const BO_TRACK& track = m_Tracks[ i ];
		unsigned long bone_id = track.BoneID;
		_Stm << bone_id;
		unsigned long keyframes_count = unsigned long( track.RKF.size() );
		_Stm << keyframes_count;
		for( unsigned long j = 0 ; j < keyframes_count ; ++j )
		{
			_Stm << 0.0f;
			D3DXVECTOR3 tran = track.TKF[ j ];
			D3DXQUATERNION rot = track.RKF[ j ];
			std::swap( tran.y , tran.z );
			std::swap( rot.y , rot.z );
		//	rot.w = -rot.w; 
			_Stm << tran;
			_Stm << rot;
		}
	}
}

bool PAFConvert::Load( BinStream& _Stm )
{
	//----------------------------------------------------------------------------
	const int PAF_MAGIC = MAKEFOURCC('P', 'A', 'F', '\0');
	const int PAF_VERSION = 100;
	//---------------------------------------------------------------------------
	
	// Test magic number and version
	int magic = 0;
	int version = 0;
	_Stm >> magic;
	_Stm >> version;

	if (magic != PAF_MAGIC || version != PAF_VERSION)
	{
		return false;
	}
	float m_SampleFps;
	// Read animation sample rate
	_Stm >> m_SampleFps;

	// Read animation duration
	_Stm >> m_Duration;

	// Read number of animation track
	unsigned int numAnimTrack = 0;
	_Stm >> numAnimTrack;

	m_Tracks.resize(numAnimTrack);

	for (size_t i = 0; i < numAnimTrack; ++i)
	{
		BO_TRACK* pAnimTrack = &( m_Tracks[i] );
		_Stm >> pAnimTrack->BoneID;
		_Stm >> pAnimTrack->RKF;
		_Stm >> pAnimTrack->TKF;
	}

	return true;
}

