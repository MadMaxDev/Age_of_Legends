include ../../Sln-mk/PrjDLL.mk
InstallTargets	:= XLP4Web/
include	$(PMKBASE)/cmn/whmultiinstall.mk
MYDEPLIB		:= TinyXML cryptlib
WHTEMPVAR		:= LPGamePlay4Web_Lib GameUtilityLib PNGS WHNET WHCMN 
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB		:= $(MYSYSLIB) -lz
