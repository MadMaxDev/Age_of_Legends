####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whoneinstall.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 简单的单个文件的install。只把输出文件拷贝到安装目录下自己工程名对应的目录中即可。
##              : 这个应该是在Prj.mk文件中被包含的，因为TargetOutFile是在install目录表后使用的所以不需要用OutDir和ProjectName推算出TargetOutFile。直接使用即可。
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

ifneq ($(INSTALLROOT_$(ConfigurationName)),)
MyInstDir		:= $(INSTALLROOT_$(ConfigurationName))/$(ProjectName)
else
MyInstDir		:= $(INSTALLROOT)/$(ProjectName)
endif

install:
# 确认目标目录存在
ifeq ($(wildcard $(MyInstDir)),)
	$(MKDIR) -p $(MyInstDir)
endif
# 拷贝文件(EXE或DLL(so)文件)
	$(CP) -f $(TargetOutFile)* $(MyInstDir)

