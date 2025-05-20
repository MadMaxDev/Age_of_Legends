include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XCAAFS4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
MYDEPLIB		:= TinyXML cryptlib
WHTEMPVAR		:= CAAFS4Web_Lib PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB		:= $(MYSYSLIB) -lz
