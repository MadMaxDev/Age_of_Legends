####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whmultiinstall.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : һ���ļ����Ŀ�ĵص�install��
##              : ���Ӧ������Prj.mk�ļ��б������ģ�������Ҫ��OutDir��ProjectName�����TargetOutFile�����ǲ�����������֣�
##              : Prj.mk�л���Ҫ����InstallTargets�������ǻ���INSTALLROOT�����Ŀ¼or�ļ���(ע�⣺����ִ���'/'��β���ʾĿ¼�������ʾ�ļ�)��
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

ifneq ($(INSTALLROOT_$(ConfigurationName)),)
MyInstDir		:= $(INSTALLROOT_$(ConfigurationName))
else
MyInstDir		:= $(INSTALLROOT)
endif
# ע�⣺�������Ҳ��TargetOutFile����ʱ������ͺ������ɵ�TargetOutFile�ظ�
whmultiinstall__TargetOutFile	:= $(OutDir)/$(ProjectName)$(CfgFileTag)
ifeq ($(ISWHAT),DLL)
# ��DLL
whmultiinstall__TargetOutFile	:= $(whmultiinstall__TargetOutFile).so
endif

install:
# �����Ŀ�ĵؿ����ļ�
# ����ȷ��Ŀ��Ŀ¼����
	@for it in $(InstallTargets); do					\
		$(SURECP) $(whmultiinstall__TargetOutFile) $(MyInstDir)/$$it;	\
	done

