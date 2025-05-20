include ../../Sln-mk/PrjEXENEEDDLL.mk
MonoOutFile		:= ServerBattle.so
MonoCodeSrcDir	:= $(HOME)/$(WORKDIR)/mono/ServerBattle
include $(PMKBASE)/cmn/whoneinstall_with_mono.mk
INCPATH			:= $(INCPATH) -I/opt/mono/include/mono-2.0
LIBPATH			:= $(LIBPATH) -L/opt/mono/lib -lmono-2.0  -lm -lrt  -ldl -lpthread
MYDEPLIB		:= cryptlib
WHTEMPVAR		:= DIA WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB		:= $(MYSYSLIB) $(MYSQLLIBPATH)/libmysqlclient_r.a -lz