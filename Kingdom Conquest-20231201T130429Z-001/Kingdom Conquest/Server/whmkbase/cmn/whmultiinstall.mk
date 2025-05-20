####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whmultiinstall.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 一个文件多个目的地的install。
##              : 这个应该是在Prj.mk文件中被包含的，所以需要用OutDir和ProjectName推算出TargetOutFile（但是不能用这个名字）
##              : Prj.mk中还需要定义InstallTargets，里面是基于INSTALLROOT的相对目录or文件名(注意：如果字串以'/'结尾则表示目录，否则表示文件)。
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

ifneq ($(INSTALLROOT_$(ConfigurationName)),)
MyInstDir		:= $(INSTALLROOT_$(ConfigurationName))
else
MyInstDir		:= $(INSTALLROOT)
endif
# 注意：如果这里也用TargetOutFile作临时变量会和后面生成的TargetOutFile重复
whmultiinstall__TargetOutFile	:= $(OutDir)/$(ProjectName)$(CfgFileTag)
ifeq ($(ISWHAT),DLL)
# 是DLL
whmultiinstall__TargetOutFile	:= $(whmultiinstall__TargetOutFile).so
endif

install:
# 向各个目的地拷贝文件
# 并先确认目标目录存在
	@for it in $(InstallTargets); do					\
		$(SURECP) $(whmultiinstall__TargetOutFile) $(MyInstDir)/$$it;	\
	done

