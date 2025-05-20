include ../../Sln-mk/PrjEXENEEDDLL.mk
InstallTargets	:= $(ProjectName)/
include	$(PMKBASE)/cmn/whmultiinstall.mk
#MYDEPLIB	:= PNGS WHNET WHCMN cryptlib
MYDEPLIB		:= cryptlib GameUtilityLib WHNET WHCMN 
WHTEMPVAR		:= DIA PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB        := $(MYSYSLIB) $(MYSQLLIBPATH)/libmysqlclient_r.a -lz
