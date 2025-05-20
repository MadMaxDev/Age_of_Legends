#include "stdafx.h"
#include "PSFConvert.h"

std::string	UnToMB( const std::wstring& _str )
{
	DWORD	WSize	= DWORD( _str.size() );
	char*	pAC		= NULL;
	BOOL	bAlloc	= FALSE;
	if( WSize < 4096 )
	{
		pAC = (char*)_alloca( ( WSize + 1 ) * 2 * sizeof( char ) );
	}
	else
	{
		bAlloc = true;
		pAC = (char*)malloc( ( WSize + 1 ) * 2 * sizeof( char ) );
	}
	memset( pAC , 0 , ( WSize + 1 ) * 2 * sizeof( char ) );
	::WideCharToMultiByte( CP_ACP , 0 , _str.c_str() , -1 , pAC , ( WSize + 1 ) * 2	, NULL , NULL );
	if( bAlloc )
	{
		free( pAC );
	}
	return std::string( pAC );
}

struct Bone
{
	std::wstring				mName;			// Name	
	unsigned int				mBoneId;		// ID	
	int							mParentId;		// Parent bone ID
	std::vector< unsigned int > mChildIDs;		// Children bone ID

	D3DXVECTOR3					mRelTrans;		// Relative translation to parent
	D3DXQUATERNION				mRelRot;		// Relative rotation to parent
	D3DXQUATERNION				mInvRot;		// 
	D3DXVECTOR3					mInvTrans;		//

	D3DXMATRIX					mInvMat;		// 转换到骨头空间的矩阵

	D3DXVECTOR3					mRotLimitMin;	// 旋转限制的最小值(IK)
	D3DXVECTOR3					mRotLimitMax;	// 旋转限制的最大值(IK)
	bool						mAxisLocked[4];	// 旋转轴是否被锁定(X,Y,Z,unused)

	BO_BONE ToBOBone()
	{
		BO_BONE bob;
		
		bob.BoneName			=	UnToMB( mName );
		bob.RelativeTranslation	=	mRelTrans;
		bob.RelativeRotation	=	mRelRot;
		bob.InvertTranslation	=	mInvTrans;
		bob.InvertRotation		=	mInvRot;
		bob.ThisID				=	mBoneId;
		bob.FatherID			=	mParentId;
		bob.ChildrenIDs			=	mChildIDs;
		
		return bob;
	}
};

struct Skeleton
{

	~Skeleton();

	bool Load(BinStream& _Stm);

	std::vector<Bone*> mBones;
	std::vector<int>   mRootBoneIds;
	float			   mScaling;
};

Skeleton::~Skeleton()
{
	for (unsigned int i = 0; i < mBones.size(); ++i)
		delete mBones[i];
	mBones.clear();
	mRootBoneIds.clear();
}

bool Skeleton::Load(BinStream& _Stm)
{
	const int PSF_MAGIC = MAKEFOURCC('P', 'S', 'F', '\0');
	const int PSF_VERSION = 200;

	// Test magic number and version
	int magic;
	int version;
	_Stm >> magic;
	_Stm >> version;

	if (magic != PSF_MAGIC || version != PSF_VERSION)
	{	
		return false;
	}
	// Destroy old skeleton

	// Read all root bones id
	_Stm >> mRootBoneIds;

	unsigned int numBones = 0;

	// Read number of bones
	_Stm >> numBones;
	mBones.resize(numBones);

	D3DXVECTOR3 InvTrans;

	const DWORD MAX_BONE_NAME_LEN = 512;
	wchar_t nameBuf[MAX_BONE_NAME_LEN];

	// Read all bones
	for (unsigned int i = 0; i < numBones; ++i)
	{
		Bone* bone = new Bone();

		// Read bone name
		DWORD nameLen;
		_Stm >> nameLen;
		if (nameLen > MAX_BONE_NAME_LEN)
		{
			_Stm.Read( nameBuf, sizeof(nameBuf));
			_Stm.Skip( nameLen - sizeof(nameBuf) );
			nameBuf[MAX_BONE_NAME_LEN - 1] = 0;
		}else
		{
			_Stm.Read( nameBuf, nameLen * sizeof(wchar_t) );
			nameBuf[nameLen] = 0;
		}
		bone->mName = nameBuf;
		_Stm >> bone->mParentId;

		_Stm >> bone->mRelTrans;
		_Stm >> bone->mRelRot;
		_Stm >> bone->mInvTrans;
		_Stm >> bone->mInvRot;

		_Stm >> bone->mRotLimitMin;
		_Stm >> bone->mRotLimitMax;
		_Stm.Read(bone->mAxisLocked, sizeof(bone->mAxisLocked) );

		// Read children ids
		_Stm >> bone->mChildIDs;

		// Calculate the inverse matrix of this bone
		D3DXMatrixRotationQuaternion(&bone->mInvMat, &bone->mInvRot);
		bone->mInvMat._41 = InvTrans.x;
		bone->mInvMat._42 = InvTrans.y;
		bone->mInvMat._43 = InvTrans.z;

		// Set bone id
		bone->mBoneId = i;

		// Add to skeleton
		mBones[i] = bone;
	}

	return true;
}

size_t BoneCount;

bool PSFConvert::Load( BinStream& _Stm )
{
	Skeleton sk;
	if( ! sk.Load( _Stm ) )
	{
		return false;
	}
	for( size_t i = 0 ; i < sk.mBones.size() ; ++i )
	{
		m_Bones.push_back( sk.mBones[ i ]->ToBOBone() );
	}
	BoneCount = sk.mBones.size();
	return true;
}

void PSFConvert::Save( BinStream& _Stm )
{
	unsigned long valid_tag		= *(unsigned long*)"CSF";
	unsigned long valid_version	= 820; 
	unsigned long bone_count	= (unsigned long )m_Bones.size();
	
	_Stm << valid_tag;
	_Stm << valid_version;
	_Stm << bone_count;

	for( unsigned long i = 0 ; i < bone_count ; ++i )
	{
		BO_BONE& bone = m_Bones[i];
		
		std::string	name = bone.BoneName;

		D3DXVECTOR3 tran = bone.RelativeTranslation;
		std::swap( tran.y , tran.z );
		D3DXQUATERNION rot = bone.RelativeRotation;
		std::swap( rot.y , rot.z );
		//rot.w = -rot.w; 
		D3DXVECTOR3 r_tran = bone.InvertTranslation;
		std::swap( r_tran.y , r_tran.z );
		D3DXQUATERNION r_rot = bone.InvertRotation;
		std::swap( r_rot.y , r_rot.z );
	//	r_rot.w = -r_rot.w; 

		unsigned long f_id = bone.FatherID;
		unsigned long c_cnt = (unsigned long )bone.ChildrenIDs.size();

		_Stm << name;
		_Stm << tran;
		_Stm << rot;
		_Stm << r_tran;
		_Stm << r_rot;
		_Stm << f_id;
		_Stm << c_cnt;

		for( unsigned long j = 0 ; j < c_cnt ; ++j )
		{
			unsigned long child_id = bone.ChildrenIDs[ j ];
			_Stm << child_id;
		}
	}
}

