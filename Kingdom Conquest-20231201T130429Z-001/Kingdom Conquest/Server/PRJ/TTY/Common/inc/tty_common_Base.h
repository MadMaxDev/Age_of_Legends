// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 无
// File         : tty_common_Base.h
// Creator      : Wei Hua (魏华)
// Comment      : 所有tty_common_xxx.h都需要包含的头文件，里面是一些基本定义
// CreationDate : 2006-02-24
// Change LOG   : 2006-06-08 明确指定结尾为4字节对齐
//              : 2008-06-06 修改用户数据版本，避免服务器宕掉

#ifndef	__TTY_COMMON_BASE_H__
#define	__TTY_COMMON_BASE_H__

#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whdataobj.h>
#include <GameUtilityLib/inc/DMtype.h>

using namespace n_whcmn;
typedef	whallocationobjwithonemem<>	TTY_AOONE_T;

// 所有需要跟数据库同步的数据的版本（和可支持的最低版本）
enum
{
	TTY_DATA_VER	= 11,
	TTY_DATA_VERMIN	= 11,
};

// 需要放在列表中的结构都需要从这个结构继承
// 这样写虽然可能会造成一定的麻烦（比如不能直接用等号或memcpy对结构赋值进行赋值，因为那样会造成nStatus被改变），但是因为不想多生成一个专门存方Status的list，所以这样做是最简单的做法。
#pragma pack(push, old_pack_num, 4)
struct	TTY_LISTUNIT_BASE_T
{
private:
	// 取值为WHDATAOBJ_LISTTYPE_UNIT_STATUS_XXX，定义在whdataobj_def.h中
	unsigned char	nStatus;
public:
	TTY_LISTUNIT_BASE_T()
		: nStatus(0)
	{
	}
	inline unsigned char	GetStatus() const
	{
		return	nStatus;
	}
	inline unsigned char	GetStatusLow() const	// 增加这个主要是因为&的优先级比比较要底，所以用这个函数包装一下就不用写那么多括号了
	{
		return	nStatus & 0x0F;
	}
	inline void	SetStatus(unsigned char nVal)
	{
		nStatus	= nVal;
	}
	inline void	SetStatusBit(unsigned char nBitVal)
	{
		nStatus	|= nBitVal;
	}
	inline void	ClearStatusBit(unsigned char nBitVal)
	{
		nBitVal	&= nStatus;
		if( nBitVal != 0 )
		{
			nStatus	^= nBitVal;
		}
	}
};
#pragma pack(pop, old_pack_num)

// 重新设置一下分配管理器
#define	TTY_RESET_AOMGR(pObj, pAM)	pObj->SetBase(pObj->_pBase, pAM)

#endif
