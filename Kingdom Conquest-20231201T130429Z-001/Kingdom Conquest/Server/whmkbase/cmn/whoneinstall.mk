####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whoneinstall.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : �򵥵ĵ����ļ���install��ֻ������ļ���������װĿ¼���Լ���������Ӧ��Ŀ¼�м��ɡ�
##              : ���Ӧ������Prj.mk�ļ��б������ģ���ΪTargetOutFile����installĿ¼���ʹ�õ����Բ���Ҫ��OutDir��ProjectName�����TargetOutFile��ֱ��ʹ�ü��ɡ�
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

ifneq ($(INSTALLROOT_$(ConfigurationName)),)
MyInstDir		:= $(INSTALLROOT_$(ConfigurationName))/$(ProjectName)
else
MyInstDir		:= $(INSTALLROOT)/$(ProjectName)
endif

install:
# ȷ��Ŀ��Ŀ¼����
ifeq ($(wildcard $(MyInstDir)),)
	$(MKDIR) -p $(MyInstDir)
endif
# �����ļ�(EXE��DLL(so)�ļ�)
	$(CP) -f $(TargetOutFile)* $(MyInstDir)

