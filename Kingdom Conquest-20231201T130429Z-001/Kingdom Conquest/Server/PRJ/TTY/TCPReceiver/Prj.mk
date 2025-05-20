include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XDBS4Web/ XLP4Web/ XGS4Web/ XGDB4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
#MYDEPLIB	:= PNGS WHNET WHCMN
WHTEMPVAR		:= PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
