# �Ǵ�gprof��ģʽ
GPUSED					:= 1
# ����ģʽ��
ConfigurationName		:= gprelease
# �������ָñ���ģʽ���ļ����
# (����debug����˵������Ŀ���ļ���׺�ͻ���._do)
CfgFileTag				:= _p
# ����ѡ��
CCFlag					:= -O2 -Wall -pg -static -D_DEBUG -Wno-strict-aliasing
#CCFlag					:= -finput-charset=gb2312 -O2 -Wall -pg -static -D_DEBUG -Wno-strict-aliasing
# ����ѡ��
LNFlag					:= -lpthread
