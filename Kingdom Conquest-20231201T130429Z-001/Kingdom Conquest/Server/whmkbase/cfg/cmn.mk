# ������Ҫ��һЩȫ�ֶ���
PMKDEFINEs	:= -DLINUX -fpermissive #-DSTACK_DIRECTION=1 -DSIZEOF_INT=4
# cpp�ļ���׺�������Ҫ�����ڱ����cpp��׺���ļ�������ϰ�����⡣
CPPEXT		:= cpp
# һ�������ļ���
AR			:= ar
RANLIB		:= ranlib
CC			:= g++ -m32
# �����RH9�Ļ���û��--preserve-root���������
RM			:= rm --preserve-root
CP			:= cp
SURECP		:= sh $(PMKBASE)/cmn/whsurecp.sh
MKDIR		:= mkdir

# make depʱ��Ҫ��ѡ��
DEPFlag		:= -MM -MT
# ���ӿ���Ҫ��ѡ��
ARFlag		:= crs
# ����DLL��Ҫ��obj�ı���ѡ��
SOCCFlag	:= -fPIC
# ����DLL�ı���ѡ��
SOLNFlag	:= -shared

# �����ļ����ϼ����ļ���
SEARCHINCLVL:= 4
