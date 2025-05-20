#pragma once 

#include "BinStream.h"

struct BO_BONE
{
	std::string			BoneName;//��ͷ������
	D3DXVECTOR3			RelativeTranslation;//����ͷ����ڸ���ͷ��ƽ�ƣ�����ʱ�ı�׼��̬��
	D3DXQUATERNION 		RelativeRotation;//����ͷ����ڸ���ͷ����ת������ʱ�ı�׼��̬��
	D3DXVECTOR3			InvertTranslation;//����ͷ��mesh�ָ���������mesh��ȫ�������ϵ�任������ͷ�ľֲ�����ϵ��
	D3DXQUATERNION 		InvertRotation;
	unsigned long		ThisID;//����ͷ��ID
	unsigned long		FatherID;//����ͷ�ĸ���ͷID
	std::vector<unsigned int>	ChildrenIDs;//���к��ӵ�ID
};

class PSFConvert
{
public:

	std::vector< BO_BONE >	m_Bones;
	
	bool Load( BinStream& _Stm );

	void Save( BinStream& _Stm );
};

extern size_t BoneCount;