// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whbox.h
// Creator      : Wei Hua (魏华)
// Comment      : 和二维矩形相关的应用
//              : 坐标系为右下为正方向，角度正方向使用左手定则（即X轴到Y轴的方向）
// 判断点和线段的关系（二进制）（01和10刚好相互取反）
// 01 00 10
// 即十进制的
// 1 0 2
// 其中0表示在线段上（包括端点）
// 判断点和矩形的关系（二进制）（Y坐标关系在先）
// 0101 0100 0110
// 0001 0000 0010
// 1001 1000 1010
// 即十进制的
// 5 4 6
// 1 0 2
// 9 8 10
// 四个方向的关系转换（通过旋转矩阵得到）
// LT作为左上角
//  5 4 6
//  1 0 2
//  9 8 10
// RB作为左上角
// 10 8 9
//  2 0 1
//  6 4 5
// TR作为左上角
//  6 2 10
//  4 0 8
//  5 1 9
// BL作为左上角
//  9 1 5
//  8 0 4
// 10 2 6
// CreationDate : 2005-12-08
// ChangeLOG    : 

#ifndef	__WHBOX_H__
#define	__WHBOX_H__

namespace n_whcmn
{

// 判断点和线段的关系（二进制）（01和10刚好相互取反）
// 01 00 10
// 即十进制的
// 1 0 2
// 其中0表示在线段上（包括端点）
inline unsigned int	whline_check_point_relation(int nLineL, int nLineR, int nPoint)
{
	if( (nPoint - nLineL) < 0 )			return	1;
	else if( (nPoint - nLineR) > 0 )	return	2;
	else								return	0;
}

// 差积结果，作为二维向量来说，差积结果就是Z的部分
inline int	whbox_smp_cross_product(int nX1, int nY1, int nX2, int nY2)
{
	return	nX1*nY2 - nY1*nX2;
}
// 连续三点的折线判断折向（差积为正标明向正方向折）
// 点循序为1,2,3
// 差积为(2-1)x(3-2)
inline int	whbox_3pointline_cross_product(int nX1, int nY1, int nX2, int nY2, int nX3, int nY3)
{
	return	whbox_smp_cross_product(nX2-nX1, nY2-nY1, nX3-nX2, nY3-nY2);
}

// 判断点和矩形的关系（二进制）（Y坐标关系在先）
// 0101 0100 0110
// 0001 0000 0010
// 1001 1000 1010
// 即十进制的
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
// 判断线段是否和指定矩形有交集（包括线段整个都在矩形内部的情况）
bool	whbox_check_intersection_with_line(
		int nBoxL, int nBoxT, int nBoxR, int nBoxB
		, int nLineX1, int nLineY1, int nLineX2, int nLineY2
		);

}		// EOF namespace n_whcmn

#endif	// EOF __WHBOX_H__

