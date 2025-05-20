include ../../Sln-mk/PrjEXE.mk
include $(PMKBASE)/cmn/whoneinstall.mk
#MYDEPLIB	:= WHNET WHCMN
WHTEMPVAR		:= WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
