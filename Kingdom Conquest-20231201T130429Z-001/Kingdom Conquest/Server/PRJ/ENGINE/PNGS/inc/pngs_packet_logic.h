// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_logic.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��ģ����ָ�����������(����0~19�����Logic����ռ����Щ)
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-09-20
// Change LOG   :

#ifndef	__PNGS_PACKET_LOGIC_H__
#define	__PNGS_PACKET_LOGIC_H__

namespace n_pngs
{

// ��Щcmd��������CMN::ILogic::cmd_t�����ڶ���Ϊint
// �����Ϊͨ�õĶ��壬��ΧΪ��0~100
// ����logic���岻��ʹ����Щֵ
enum
{
	// (����ִ��)֪ͨ���ڱ�׼�������汾�б�
	PNGSPACKET_2LOGIC_DISPHISTORY	= 0,
	// �汾У�飨������ֱ���ڻ�ȡģ��ʱУ�飩
	// PNGSPACKET_2LOGIC_VERIFY_VER	= 1,
	// ͨ�÷����������ILogic����MOOD
	PNGSPACKET_2LOGIC_SETMOOD		= 2,
	// �ַ�ָ�һ������mainstructure������logic���ɣ�
	// ���ݲ��־���ָ����
	PNGSPACKET_2LOGIC_DISPATCHCMD	= 3,
	// ��mainstructureע��ָ�pRstAccepter���ǿ��Դ���ָ����߼�ģ�顣
	PNGSPACKET_2LOGIC_REGCMDDEAL	= 4,
	// GMָ�ע�⣺PNGSPACKET_2LOGIC_GMCMD��PNGSPACKET_2LOGIC_GMCMD_RST��ʹ������ṹ��
	PNGSPACKET_2LOGIC_GMCMD			= 10,
	// GMָ��أ����ڴ��ܷ��͵�ģ���յ�����ͷ��͸���Ӧ�����ӣ�
	PNGSPACKET_2LOGIC_GMCMD_RST		= 11,
	// ֪ͨ
	PNGSPACKET_2LOGIC_NOTIFY		= 12,
};
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	PNGSPACKET_2LOGIC_DISPHISTORY_T
{
	char			szVerToQuery[16];								// ���ڲ�ѯ�İ汾��
};
//struct	PNGSPACKET_2LOGIC_VERIFY_VER_T
//{
//	unsigned int	nVerExpected;									// �����İ汾
//	unsigned int	nVerReal;										// ʵ�ʵİ汾
//	bool			bOK;											// �Ƿ�У��ɹ�
//	PNGSPACKET_2LOGIC_VERIFY_VER_T()
//		: nVerExpected(0)
//		, nVerReal(0)
//		, bOK(true)													// �������ҪУ��汾����Ͳ���仯����ֱ�ӷ��سɹ���
//	{
//	}
//	inline void	check(int _nVerReal)
//	{
//		nVerReal	= _nVerReal;
//		bOK			= (nVerExpected==nVerReal);
//	}
//};
struct	PNGSPACKET_2LOGIC_REGCMDDEAL_T
{
	pngs_cmd_t		nCmdReg;
};
struct	PNGSPACKET_2LOGIC_GMCMD_T									// ע��PNGSPACKET_2LOGIC_GMCMD��PNGSPACKET_2LOGIC_GMCMD_RST��ʹ������ṹ
{
	int				nCntrID;										// ��Ӧ������ID��ԭ�����أ�
	int				nExt;											// ԭ������
	size_t			nDSize;
	void			*pData;											// �߼�ģ�����н���pData������
																	// һ��pData����ǰ��pngs_cmd_t��GMָ�����ͣ�Ĭ��0��ʾ�ִ���������ֵ�ɸ���ģ���Լ�����
};
typedef	PNGSPACKET_2LOGIC_GMCMD_T	PNGSPACKET_2LOGIC_GMCMD_RST_T;
struct	PNGSPACKET_2LOGIC_SETMOOD_T
{
	int				nMood;											// CMN::ILogic::MOOD_XXX
};
struct	PNGSPACKET_2LOGIC_NOTIFY_T
{
	int				nWhat;											// ֪ͨ��������ʲô�������Ҫ֪ͨ�ߺͱ�֪ͨ����һ�������ͷ�ļ��ж���ã�
	size_t			nDSize;
	void			*pData;
};
////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_LOGIC_H__
