// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbox.h
// Creator      : Wei Hua (κ��)
// Comment      : �Ͷ�ά������ص�Ӧ��
//              : ����ϵΪ����Ϊ�����򣬽Ƕ�������ʹ�����ֶ��򣨼�X�ᵽY��ķ���
// CreationDate : 2005-12-08
// ChangeLOG    : 2005-12-09 �Ѻ����Ϊģ�庯��

#include "../inc/whbox.h"
#include <assert.h>

namespace n_whcmn
{

// ���Ľ�ʱ�Ĵ���ʽ
template<int SW_2, int SW_8, int SW_10, class _JudgeFunc>
inline bool	WHBOX_CHECK_INTERSECTION_WITH_LINE_CONNER(
	unsigned int	nR2
	, int nBoxL, int nBoxT, int nBoxR, int nBoxB
	, int nLineX1, int nLineY1, int nLineX2, int nLineY2
	)
{
	_JudgeFunc	jf;
	switch( nR2 )
	{
		default/*1,9,4,6*/:
			/* �ض����� */
			return	false;
		break;
		case	SW_8:
		{
			if( jf(whbox_3pointline_cross_product(nLineX2, nLineY2, nBoxL, nBoxB, nLineX1, nLineY1)) )
			{
				return	false;
			}
			else
			{
				return	true;
			}
		}
		break;
		case	SW_2:
		{
			if( jf(whbox_3pointline_cross_product(nLineX1, nLineY1, nBoxR, nBoxT, nLineX2, nLineY2)) )
			{
				return	false;
			}
			else
			{
				return	true;
			}
		}
		break;
		case	SW_10:
		{
			/* ���߶�Ҫ�ж� */
			if( jf(whbox_3pointline_cross_product(nLineX2, nLineY2, nBoxL, nBoxB, nLineX1, nLineY1)) )
			{
				return	false;
			}
			else if( jf(whbox_3pointline_cross_product(nLineX1, nLineY1, nBoxR, nBoxT, nLineX2, nLineY2)) )
			{
				return	false;
			}
			else
			{
				return	true;
			}
		}
		break;
	}
}
// ���Ľ�ʱ�Ĵ���ʽ
template<int SW_1, int SW_2, int SW_9, int SW_8, int SW_10, class _JudgeFunc>
inline bool	WHBOX_CHECK_INTERSECTION_WITH_LINE_MIDDLE(
	unsigned int	nR2
	, int nBoxL, int nBoxT, int nBoxR, int nBoxB
	, int nLineX1, int nLineY1, int nLineX2, int nLineY2
	)
{
	_JudgeFunc	jf;
	switch( nR2 )
	{
		default/*5,6*/:
			/* �ض����� */
			return	false;
		break;
		case	SW_8:
			/* �ض��ཻ */
			return	true;
		break;
		case	SW_1:
		case	SW_9:
		{
			/* ��� */
			if( jf(whbox_3pointline_cross_product(nLineX2, nLineY2, nBoxL, nBoxT, nLineX1, nLineY1)) )
			{
				return	false;
			}
			else
			{
				return	true;
			}
		}
		break;
		case	SW_2:
		case	SW_10:
		{
			/* �ұ� */
			if( jf(whbox_3pointline_cross_product(nLineX1, nLineY1, nBoxR, nBoxT, nLineX2, nLineY2)) )
			{
				return	false;
			}
			else
			{
				return	true;
			}
		}
		break;
	}
}
struct	JF_LT
{
	inline bool	operator()(int nCrossProduct)	const	{return nCrossProduct<0;}
};
struct	JF_GT
{
	inline bool	operator()(int nCrossProduct)	const	{return nCrossProduct>0;}
};
bool	whbox_check_intersection_with_line(
		int nBoxL, int nBoxT, int nBoxR, int nBoxB
		, int nLineX1, int nLineY1, int nLineX2, int nLineY2
		)
{
	// ȷ��һ���߶������˵�ͺ��ӵĹ�ϵ
	unsigned int	nR1	= whbox_check_point_relation(nBoxL, nBoxT, nBoxR, nBoxB, nLineX1, nLineY1);
	unsigned int	nR2	= whbox_check_point_relation(nBoxL, nBoxT, nBoxR, nBoxB, nLineX2, nLineY2);
	// ��������κ�һ���ں����ڣ���˵��һ���н���
	if( nR1 == 0 || nR2 == 0 )
	{
		return	true;
	}
	// �ų���������жϣ����������ϵһ�£�˵������ͬһ���򣩣���һ��������
	if( nR1 == nR2 )
	{
		return	false;
	}
	// ʣ�µİ������һһ�ж�
	switch( nR1 )
	{
		// �ǵ����
		// LT
		case	5:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_CONNER<2, 8, 10, JF_LT>(
				nR2
				, nBoxL ,nBoxT ,nBoxR ,nBoxB
				, nLineX1 ,nLineY1 ,nLineX2 ,nLineY2
				);
		}
		break;
		// RB
		case	10:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_CONNER<1, 4, 5, JF_LT>(
				nR2
				, nBoxR ,nBoxB ,nBoxL ,nBoxT
				, nLineX1 ,nLineY1 ,nLineX2 ,nLineY2
				);
		}
		break;
		// TR
		case	6:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_CONNER<8, 1, 9, JF_GT>(
				nR2
				, nBoxT ,nBoxR ,nBoxB ,nBoxL
				, nLineY1 , nLineX1 ,nLineY2 ,nLineX2
				);
		}
		break;
		// BL
		case	9:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_CONNER<4, 2, 6, JF_GT>(
				nR2
				, nBoxB ,nBoxL ,nBoxT ,nBoxR
				, nLineY1 , nLineX1 ,nLineY2 ,nLineX2
				);
		}
		break;
		// T
		case	4:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_MIDDLE<1, 2, 9, 8, 10, JF_LT>(
				nR2
				, nBoxL, nBoxT, nBoxR, nBoxB
				, nLineX1, nLineY1, nLineX2, nLineY2
				);
		}
		break;
		// B
		case	8:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_MIDDLE<2, 1, 6, 4, 5, JF_LT>(
				nR2
				, nBoxR, nBoxB, nBoxL, nBoxT
				, nLineX1, nLineY1, nLineX2, nLineY2
				);
		}
		break;
		// R
		case	2:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_MIDDLE<4, 8, 5, 1, 9, JF_GT>(
				nR2
				, nBoxT ,nBoxR ,nBoxB ,nBoxL
				, nLineY1 , nLineX1 ,nLineY2 ,nLineX2
				);
		}
		break;
		// BL
		case	1:
		{
			return	WHBOX_CHECK_INTERSECTION_WITH_LINE_MIDDLE<8 ,4 ,10, 2, 6, JF_GT>(
				nR2
				, nBoxB ,nBoxL ,nBoxT ,nBoxR
				, nLineY1 , nLineX1 ,nLineY2 ,nLineX2
				);
		}
		break;
		// ʣ�¾Ͳ�������
		default:
			assert(0);
			return	false;
		break;
	}
}

}		// EOF namespace n_whcmn
