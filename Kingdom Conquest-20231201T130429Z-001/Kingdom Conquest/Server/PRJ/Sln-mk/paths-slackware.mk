# ����ļ����ڹ���Ŀ¼�»������ϵ�ĳ��Ŀ¼������make����Ŀ¼���Ͽ�ʼ�ң��ҵ��ĸ��Ͱ����ĸ�
# ����incĿ¼
# ע�ⲻ����~�����û���Ŀ¼�����·��gcc���ϡ�
VER_EXT		:= _cur
WORKDIR		:= WORK$(VER_EXT)
PRJDIR		:= PRJ
MYSQLLIBPATH:= /usr/local/lib/mysql
WHTEMPVAR	:= 3RD $(PRJDIR)/ENGINE $(PRJDIR)/GAME $(PRJDIR)/TTY
WHTEMPVAR	:= $(addprefix -I$(HOME)/$(WORKDIR)/,$(WHTEMPVAR))
INCPATH		:= -I/usr/local/include/mysql $(INCPATH) $(WHTEMPVAR)
# ����libĿ¼
WHTEMPVAR	:= 3RD/lib $(PRJDIR)/lib
WHTEMPVAR	:= $(addprefix -L$(HOME)/$(WORKDIR)/,$(WHTEMPVAR))
LIBPATH		:= $(LIBPATH) $(WHTEMPVAR)

# ��װ��Ŀ¼
INSTALLROOT	:= /ITC/GameSvr$(VER_EXT)/bin
#INSTALLROOT	:= /ITC/GameSvr$(VER_EXT)_tst/bin

