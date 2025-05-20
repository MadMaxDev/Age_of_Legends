include ../../Sln-mk/PrjEXENEEDDLL.mk
InstallTargets	:= $(ProjectName)/ XDBS4Web/XDBS4Web$(CfgFileTag) XCAAFS4Web/XCAAFS4Web$(CfgFileTag) XCLS4Web/XCLS4Web$(CfgFileTag) XLP4Web/XLP4Web$(CfgFileTag) XGS4Web/XGS4Web$(CfgFileTag) XGDB4Web/XGDB4Web$(CfgFileTag)
include	$(PMKBASE)/cmn/whmultiinstall.mk
#MYDEPLIB	:= PNGS WHNET WHCMN cryptlib
MYDEPLIB		:= cryptlib
WHTEMPVAR		:= PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
