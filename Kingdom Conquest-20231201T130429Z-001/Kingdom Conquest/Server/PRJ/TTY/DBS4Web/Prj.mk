include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XDBS4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
WHTEMPVAR		:= DBS4Web_Lib DIA PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB        := $(MYSYSLIB) $(MYSQLLIBPATH)/libmysqlclient_r.a -lz
