# ����ļ����ڹ���Ŀ¼�»������ϵ�ĳ��Ŀ¼������make����Ŀ¼���Ͽ�ʼ�ң��ҵ��ĸ��Ͱ����ĸ�
# ����incĿ¼
# ע�ⲻ����~�����û���Ŀ¼�����·��gcc���ϡ�
WHTEMPVAR	:= 3RD PRJ/ENGINE PRJ/GAME PRJ/TTY
WHTEMPVAR	:= $(addprefix -I$(HOME)/WORK/,$(WHTEMPVAR))
INCPATH		:= $(INCPATH) $(WHTEMPVAR)
# ����libĿ¼
WHTEMPVAR	:= 3RD/lib PRJ/lib
WHTEMPVAR	:= $(addprefix -L$(HOME)/WORK/,$(WHTEMPVAR))
LIBPATH		:= $(LIBPATH) $(WHTEMPVAR)

# ��װ��Ŀ¼
INSTALLROOT	:= /ITC/GameSvr0

