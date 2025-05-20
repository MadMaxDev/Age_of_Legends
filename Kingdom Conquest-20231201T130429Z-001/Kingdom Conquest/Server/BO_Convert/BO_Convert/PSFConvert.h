#pragma once 

#include "BinStream.h"

struct BO_BONE
{
	std::string			BoneName;//骨头的名称
	D3DXVECTOR3			RelativeTranslation;//这块骨头相对于父骨头的平移（导出时的标准姿态）
	D3DXQUATERNION 		RelativeRotation;//这块骨头相对于父骨头的旋转（导出时的标准姿态）
	D3DXVECTOR3			InvertTranslation;//这块骨头的mesh恢复参数（将mesh从全身的坐标系变换到这块骨头的局部坐标系）
	D3DXQUATERNION 		InvertRotation;
	unsigned long		ThisID;//这块骨头的ID
	unsigned long		FatherID;//这块骨头的父骨头ID
	std::vector<unsigned int>	ChildrenIDs;//所有孩子的ID
};

class PSFConvert
{
public:

	std::vector< BO_BONE >	m_Bones;
	
	bool Load( BinStream& _Stm );

	void Save( BinStream& _Stm );
};

extern size_t BoneCount;