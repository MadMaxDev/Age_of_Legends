// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataobj_def.h
// Creator      : Wei Hua (魏华)
// Comment      : 用于为逻辑层提供简单的属性获取和设置方法，同时隐藏底层的数据存取细节
//               （包括如何从数据源初始化对象；如何在共享内存中申请对象；如何在发生宕机事件后从共享内存中恢复数据并重新存储；如何进行增量存储等）。
//                想法：可能会采用自动生成代码。
// CreationDate : 2006-02-14
// ChangeLOG    : 2006-02-22 WHDATAOBJ_CMD_T从unsigned short改为unsigned char

#ifndef	__WHDATAOBJ_DEF_H__
#define	__WHDATAOBJ_DEF_H__

namespace n_whcmn
{

// 指令类型定义
typedef	unsigned char						WHDATAOBJ_CMD_T;
typedef	unsigned short						WHDATAOBJ_NAMEID_T;

// 和数据库通讯的操作指令(即上面定义的类型WHDATAOBJ_CMD_T)
enum
{
	// 版本校验
	// 后面跟WHDATAOBJ_NAMEID_T vermin 和 WHDATAOBJ_NAMEID_T ver
	WHDATAOBJ_CMD_VERSION					= 0x00,
	// 固定变量被更改
	// 后面跟NameID和数据部分(数据长度接收方应该根据NameID可以知道)
	WHDATAOBJ_CMD_VAR_MODIFY				= 0x01,
	// 标记变量（用于让DB知道是那一段的数据）
	WHDATAOBJ_CMD_MARKER					= 0x02,
	// 列表更改开始（后面应该跟具体条目：修改、添加、删除）
	// 后面跟NameID
	WHDATAOBJ_CMD_LIST_MODIFY_BEGIN			= 0x10,
	// 列表更改结束
	// 后面什么也不跟
	WHDATAOBJ_CMD_LIST_MODIFY_END			= 0x11,
	// 列表元素被修改
	// 后面数据部分(数据长度接收方应该根据list的NameID可以知道)
	WHDATAOBJ_CMD_LIST_ITEM_MODIFY			= 0x12,
	// 列表元素被添加
	// 后面数据部分(数据长度接收方应该根据list的NameID可以知道)
	WHDATAOBJ_CMD_LIST_ITEM_ADD				= 0x13,
	// 列表元素被删除
	// 后面ITEM的ID(应该是64位整数)
	WHDATAOBJ_CMD_LIST_ITEM_DEL				= 0x14,
	// 数组更改开始
	// 后面跟NameID
	WHDATAOBJ_CMD_ARR_MODIFY_BEGIN			= 0x20,
	// 数组更改结束
	// 后面什么也不跟
	WHDATAOBJ_CMD_ARR_MODIFY_END			= 0x21,
	// 单一数组元素更改
	// 后面跟vn2型偏移(这个相对于上一个MODIFY结尾的偏移，初始的上一个为-1)。
	// 然后是一个数值(由数组元素类型决定)
	WHDATAOBJ_CMD_ARR_ITEM_MODIFY1			= 0x22,
	// 连续多数组元素更改
	// 后面跟vn2型偏移(这个相对于上一个MODIFY结尾的偏移，初始的上一个为-1)。
	// 后面跟vn2型个数N。
	// 然后是N个数值(由数组元素类型决定)
	WHDATAOBJ_CMD_ARR_ITEM_MODIFYN			= 0x23,
	// 本对象结束（这个在一个stream中含有多个数据对象的时候使用）
	// 后面什么也没有
	// 注：对于单对象流，最后可以写也可以不写这个。
	WHDATAOBJ_CMD_ENDOFOBJ					= 0xFF,
};

// 列表条目的状态取值
enum
{
	// STATUS的取值在0x0~0xF之间
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN		= 0x00,
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD		= 0x01,					// 刚刚加入的
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL		= 0x02,					// 刚刚删除的
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED	= 0x03,					// 不是加入的但是是更新了的
	// 标志位
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK	= 0x80,					// 表示元素被锁定，一般是修改过的元素正在被存盘的过程中才被设置这个。
};

}		// EOF namespace n_whcmn

#endif
