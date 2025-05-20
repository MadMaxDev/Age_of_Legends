include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XLP4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
WHTEMPVAR		:= GSDB4Web_Lib DIA PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
