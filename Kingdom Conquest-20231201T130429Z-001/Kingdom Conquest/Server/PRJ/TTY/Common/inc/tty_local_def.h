// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// File         : tty_local_def.h
// Creator      : liuyi
// Comment      : ���������������ĺ꣬ͷ�ļ���lib��
// CreationDate : 2009-05-14
// Change LOG   : 

#ifndef	__TTY_LOCAL_DEF_H__
#define	__TTY_LOCAL_DEF_H__


// ������һ���汾��ֻӦ������һ������������ע������

/////////////////////////////////////////////////////////////////////////////
// ��½Ѱ����صĺ�

#define LOCAL_CN

#ifdef LOCAL_CN
		#define ANTIBOT_TX			// ��Ѷ�����
		#define RS_TX				// ��Ѷ��QQ״̬֪ͨ�ӿ�
		#define TQOS_TX				// ��Ѷ��TQOS��Ϣ�ϱ�ϵͳ
		#define	CROSS_TX			// ��Ѷ��Crossϵͳ
		#define TCLS_TX				// ��Ѷ��TCLS��½ϵͳ
        #define TALK_STRS     "|%d|%s|%s|%s|%d|%s|%s"			// ��Ѷ����ʹ�á�|����Ϊ�ָ���
		#define TALK_STRD     "|%d|%s|%s|%s|%d|%d|%s"			// ��Ѷ����ʹ�á�|����Ϊ�ָ���
		#define PRIMSG_STR		"|%s|%d|0x%"INT64PRFX"x|%s|%s|%s|%s|%d|%d" //�ʼ�ʹ�á�|����Ϊ��־�ָ���
		#define AHMSG_STR      "|%s|||%s|%s||||%d|0x%"INT64PRFX"x" 
		#define RENAME_STR		"|%s||0x%"INT64PRFX"x|%s|%s||||%d|%d"
        //#define TDIRTY_TX
#endif

#ifndef LOCAL_CN
//Ĭ�ϵ�������־��ʽ����Ѷ���ر�DEFINE
#define TALK_STRS     "%d,%s,%s,%s,%d,%s,%s"			// ����ʹ�á�,����Ϊ�ָ���
#define TALK_STRD     "%d,%s,%s,%s,%d,%d,%s"			// ����ʹ�á�,����Ϊ�ָ���  
#define PRIMSG_STR		"%s,%d,0x%"INT64PRFX"x,%s,%s,%s,%s,%d,%d" //�ʼ�ʹ�á�������Ϊ��־�ָ���
#define AHMSG_STR      "%s,,,%s,%s,,,,%d,0x%"INT64PRFX"x" 
#define RENAME_STR		"%s,,0x%"INT64PRFX"x,%s,%s,,,,%d,%d"
#endif
/////////////////////////////////////////////////////////////////////////////
// ̨��Ѱ����صĺ�

//#define LOCAL_TW

	#ifdef LOCAL_TW
		#define APEX_XX				// APEX�����
	#endif

/////////////////////////////////////////////////////////////////////////////
// ���Ѱ����صĺ�

//#define LOCAL_HK

	#ifdef LOCAL_HK
		#define APEX_XX				// APEX�����
	#endif

/////////////////////////////////////////////////////////////////////////////
// ����Ѱ����صĺ�

//#define LOCAL_KR

	#ifdef LOCAL_KR
		#define X_TRAP_CJ			// ���������
		#define VER_KR_LOGIN		// ������¼��ؽӿ�
		#define WEBSHOP_CJ			// �����̳�IE���棨��ҪVER_KR_LOGIN��ص�֧�֣�
	#endif

/////////////////////////////////////////////////////////////////////////////
// ��������Ѱ����صĺ�

//#define LOCAL_MY

	#ifdef LOCAL_MY
		#define APEX_XX				// APEX�����
	#endif

/////////////////////////////////////////////////////////////////////////////
// ŷ��Ѱ����صĺ�

//#define LOCAL_US
	#ifdef LOCAL_US
		#define SOCIALITY_MAKEFRIENDS //�����Ӻ��ѹ���
	#endif

////////////////////////////////////////////////////////////////////////////

#endif	// __TTY_LOCAL_DEF_H__
