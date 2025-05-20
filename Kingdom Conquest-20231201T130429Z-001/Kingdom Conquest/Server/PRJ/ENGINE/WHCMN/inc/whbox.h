// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbox.h
// Creator      : Wei Hua (κ��)
// Comment      : �Ͷ�ά������ص�Ӧ��
//              : ����ϵΪ����Ϊ�����򣬽Ƕ�������ʹ�����ֶ��򣨼�X�ᵽY��ķ���
// �жϵ���߶εĹ�ϵ�������ƣ���01��10�պ��໥ȡ����
// 01 00 10
// ��ʮ���Ƶ�
// 1 0 2
// ����0��ʾ���߶��ϣ������˵㣩
// �жϵ�;��εĹ�ϵ�������ƣ���Y�����ϵ���ȣ�
// 0101 0100 0110
// 0001 0000 0010
// 1001 1000 1010
// ��ʮ���Ƶ�
// 5 4 6
// 1 0 2
// 9 8 10
// �ĸ�����Ĺ�ϵת����ͨ����ת����õ���
// LT��Ϊ���Ͻ�
//  5 4 6
//  1 0 2
//  9 8 10
// RB��Ϊ���Ͻ�
// 10 8 9
//  2 0 1
//  6 4 5
// TR��Ϊ���Ͻ�
//  6 2 10
//  4 0 8
//  5 1 9
// BL��Ϊ���Ͻ�
//  9 1 5
//  8 0 4
// 10 2 6
// CreationDate : 2005-12-08
// ChangeLOG    : 

#ifndef	__WHBOX_H__
#define	__WHBOX_H__

namespace n_whcmn
{

// �жϵ���߶εĹ�ϵ�������ƣ���01��10�պ��໥ȡ����
// 01 00 10
// ��ʮ���Ƶ�
// 1 0 2
// ����0��ʾ���߶��ϣ������˵㣩
inline unsigned int	whline_check_point_relation(int nLineL, int nLineR, int nPoint)
{
	if( (nPoint - nLineL) < 0 )			return	1;
	else if( (nPoint - nLineR) > 0 )	return	2;
	else								return	0;
}

// ����������Ϊ��ά������˵������������Z�Ĳ���
inline int	whbox_smp_cross_product(int nX1, int nY1, int nX2, int nY2)
{
	return	nX1*nY2 - nY1*nX2;
}
// ��������������ж����򣨲��Ϊ���������������ۣ�
// ��ѭ��Ϊ1,2,3
// ���Ϊ(2-1)x(3-2)
inline int	whbox_3pointline_cross_product(int nX1, int nY1, int nX2, int nY2, int nX3, int nY3)
{
	return	whbox_smp_cross_product(nX2-nX1, nY2-nY1, nX3-nX2, nY3-nY2);
}

// �жϵ�;��εĹ�ϵ�������ƣ���Y�����ϵ���ȣ�
// 0101 0100 0110
// 0001 0000 0010
// 1001 1000 1010
// ��ʮ���Ƶ�
// 5 4 6
// 1 0 2
// 9 8 10
inline unsigned int	whbox_check_point_relation(
		int nBoxL, int nBoxT, int nBoxR, int nBoxB
		, int nPointX, int nPointY
		)
{
	return	(whline_check_point_relation(nBoxT, nBoxB, nPointY) << 2)
		|	whline_check_point_relation(nBoxL, nBoxR, nPointX)
		;
}
// �ж��߶��Ƿ��ָ�������н����������߶��������ھ����ڲ��������
bool	whbox_check_intersection_with_line(
		int nBoxL, int nBoxT, int nBoxR, int nBoxB
		, int nLineX1, int nLineY1, int nLineX2, int nLineY2
		);

}		// EOF namespace n_whcmn

#endif	// EOF __WHBOX_H__

