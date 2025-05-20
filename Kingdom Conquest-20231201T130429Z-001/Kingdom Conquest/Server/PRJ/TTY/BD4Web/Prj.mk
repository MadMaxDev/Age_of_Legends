include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XGS4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
MYDEPLIB		:= TinyXML cryptlib
WHTEMPVAR		:= BD4Web_Lib PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB		:= $(MYSYSLIB) -lz
