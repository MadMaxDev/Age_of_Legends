// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataobj_def.h
// Creator      : Wei Hua (κ��)
// Comment      : ����Ϊ�߼����ṩ�򵥵����Ի�ȡ�����÷�����ͬʱ���صײ�����ݴ�ȡϸ��
//               ��������δ�����Դ��ʼ����������ڹ����ڴ��������������ڷ���崻��¼���ӹ����ڴ��лָ����ݲ����´洢����ν��������洢�ȣ���
//                �뷨�����ܻ�����Զ����ɴ��롣
// CreationDate : 2006-02-14
// ChangeLOG    : 2006-02-22 WHDATAOBJ_CMD_T��unsigned short��Ϊunsigned char

#ifndef	__WHDATAOBJ_DEF_H__
#define	__WHDATAOBJ_DEF_H__

namespace n_whcmn
{

// ָ�����Ͷ���
typedef	unsigned char						WHDATAOBJ_CMD_T;
typedef	unsigned short						WHDATAOBJ_NAMEID_T;

// �����ݿ�ͨѶ�Ĳ���ָ��(�����涨�������WHDATAOBJ_CMD_T)
enum
{
	// �汾У��
	// �����WHDATAOBJ_NAMEID_T vermin �� WHDATAOBJ_NAMEID_T ver
	WHDATAOBJ_CMD_VERSION					= 0x00,
	// �̶�����������
	// �����NameID�����ݲ���(���ݳ��Ƚ��շ�Ӧ�ø���NameID����֪��)
	WHDATAOBJ_CMD_VAR_MODIFY				= 0x01,
	// ��Ǳ�����������DB֪������һ�ε����ݣ�
	WHDATAOBJ_CMD_MARKER					= 0x02,
	// �б���Ŀ�ʼ������Ӧ�ø�������Ŀ���޸ġ���ӡ�ɾ����
	// �����NameID
	WHDATAOBJ_CMD_LIST_MODIFY_BEGIN			= 0x10,
	// �б���Ľ���
	// ����ʲôҲ����
	WHDATAOBJ_CMD_LIST_MODIFY_END			= 0x11,
	// �б�Ԫ�ر��޸�
	// �������ݲ���(���ݳ��Ƚ��շ�Ӧ�ø���list��NameID����֪��)
	WHDATAOBJ_CMD_LIST_ITEM_MODIFY			= 0x12,
	// �б�Ԫ�ر����
	// �������ݲ���(���ݳ��Ƚ��շ�Ӧ�ø���list��NameID����֪��)
	WHDATAOBJ_CMD_LIST_ITEM_ADD				= 0x13,
	// �б�Ԫ�ر�ɾ��
	// ����ITEM��ID(Ӧ����64λ����)
	WHDATAOBJ_CMD_LIST_ITEM_DEL				= 0x14,
	// ������Ŀ�ʼ
	// �����NameID
	WHDATAOBJ_CMD_ARR_MODIFY_BEGIN			= 0x20,
	// ������Ľ���
	// ����ʲôҲ����
	WHDATAOBJ_CMD_ARR_MODIFY_END			= 0x21,
	// ��һ����Ԫ�ظ���
	// �����vn2��ƫ��(����������һ��MODIFY��β��ƫ�ƣ���ʼ����һ��Ϊ-1)��
	// Ȼ����һ����ֵ(������Ԫ�����;���)
	WHDATAOBJ_CMD_ARR_ITEM_MODIFY1			= 0x22,
	// ����������Ԫ�ظ���
	// �����vn2��ƫ��(����������һ��MODIFY��β��ƫ�ƣ���ʼ����һ��Ϊ-1)��
	// �����vn2�͸���N��
	// Ȼ����N����ֵ(������Ԫ�����;���)
	WHDATAOBJ_CMD_ARR_ITEM_MODIFYN			= 0x23,
	// ����������������һ��stream�к��ж�����ݶ����ʱ��ʹ�ã�
	// ����ʲôҲû��
	// ע�����ڵ���������������дҲ���Բ�д�����
	WHDATAOBJ_CMD_ENDOFOBJ					= 0xFF,
};

// �б���Ŀ��״̬ȡֵ
enum
{
	// STATUS��ȡֵ��0x0~0xF֮��
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN		= 0x00,
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD		= 0x01,					// �ոռ����
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL		= 0x02,					// �ո�ɾ����
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED	= 0x03,					// ���Ǽ���ĵ����Ǹ����˵�
	// ��־λ
	WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK	= 0x80,					// ��ʾԪ�ر�������һ�����޸Ĺ���Ԫ�����ڱ����̵Ĺ����вű����������
};

}		// EOF namespace n_whcmn

#endif
