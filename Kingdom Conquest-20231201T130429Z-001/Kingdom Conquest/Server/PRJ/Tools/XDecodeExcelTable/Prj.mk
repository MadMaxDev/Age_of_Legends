include ../../Sln-mk/PrjEXENEEDDLL.mk
include $(PMKBASE)/cmn/whoneinstall.mk
#MYDEPLIB	:= PNGS WHNET WHCMN cryptlib
MYDEPLIB		:= cryptlib
WHTEMPVAR		:= Common GameUtilityLib PNGS WHNET WHCMN
MYDEPLIB_ABS	:= $(addprefix $(HOME)/$(WORKDIR)/$(PRJDIR)/lib/lib, $(WHTEMPVAR))
MYSYSLIB_TMP	:= libreadline.a libhistory.a
MYSYSLIB        := $(addprefix ../../../3RD/lib/, $(MYSYSLIB_TMP)) -lncurses
