####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Prj.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 直接根据目录下的引导性文件进行相关make
##              : 这个由Makefile_$(ConfigurationName).mk包含，其中$(ConfigurationName)的取值应该是debug或者release
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# 判断如果有Sln-mk/Sln.mk就用make solution
# 虽然此时目录下面也可能有Prj.mk，但是这可能是各个工程公用的，还是要到工程的目录下面才能make工程
ifneq ($(wildcard Sln-mk/Sln.mk),)
include $(PMKBASE)/Makefile_Sln.mk

else
# 判断如果有Prj.mk就用make project
ifneq ($(wildcard Prj.mk),)
PRJ_FILE	:= Prj.mk
include $(PMKBASE)/Makefile_Prj.mk

else
ifneq ($(wildcard Prj-mk/Prj.mk),)
PRJ_FILE	:= Prj-mk/Prj.mk
include $(PMKBASE)/Makefile_Prj.mk

else
# 没有指定文件就什么也不做即可

endif
endif
endif
