#include "PGE_Graphic.h"

//-------------------------------------------------------------//
// ����: IPGEGeometry
// ˵��: ͨ�õļ�����ӿ�
//-------------------------------------------------------------//
class PGE_NOVTABLE IPGEGeometry
{
public:
	// ������ü���
	virtual ULONG  AddRef() = 0;

	// �ͷ���Դ
	virtual ULONG  Release() = 0;

	// �õ���������(�п���Ϊ����)
	virtual DWORD GetNumStreams() = 0;

	// �õ�����ĸ�ʽ����
	virtual IVertexDeclaration* GetDeclaration() = 0;

	// �õ����㻺��
	virtual IVertexBuffer* GetVertexBuffer(DWORD NumStream) = 0;

	// �õ�ÿ��������ֽ���
	virtual DWORD GetNumBytesPerVertex(DWORD NumStream) = 0;

	// �õ���������(����Ϊ�գ����ͼԪ����ͼԪ�б���ôӦ�û�����������)
	virtual IIndexBuffer* GetIndexBuffer() = 0;

	// �õ�������
	virtual DWORD GetNumVertices() = 0;

	// �õ�ͼԪ��
	virtual DWORD GetNumPrimitives() = 0;

	// �õ�ͼԪ������
	virtual D3DPRIMITIVETYPE GetPrimitiveType() = 0;

	// ���Ƽ�����ͼԪ
	virtual HRESULT Draw( IGraphicDevice* pDevice ) = 0;
};

//--------------------------------------------------------------------
// Name: PGECreateGeometry
// Desc: ����������ӿ�
//		dwNumVertices :  ������
//		dwNumPrimitives: ͼԪ��
//		PrimitiveType:   ͼԪ���� 
//		pDecls:			 ��������
//		dwOptions:		 �����建���ѡ��, ������ֵΪ0,��ô���еĶ��㻺��,
//						 �������嶼��D3DPOOL_MANAGED���͵�, ���Ҫ�ı䣬����ͨ�����º�:
//						 PGE_GEOMETRY_VB_DEFAULT(n)   (D3DPOOL_DEFAULT)
//						 PGE_GEOMERTY_VB_SYSTEMMEM(n) (D3DPOOL_SYSTEMMEM)
//						 (����n��ʾ������)
//						 PGE_GEOMETRY_IB_DEFULAT	  (D3DPOOL_DEFAULT)
//						 PGE_GEOMETRY_IB_SYSTEMMEM	  (D3DPOOL_SYSTEMMEM)	
//-------------------------------------------------------------------
DEVICE_EXPORT IPGEGeometry* WINAPI PGECreateGeometry(	IGraphicDevice* pDevice
													  , DWORD dwNumVertices
													  , DWORD dwNumPrimitives
													  , D3DPRIMITIVETYPE PrimitiveType
													  , const D3DVERTEXELEMENT9* pDecls
													  , DWORD dwOptions);
