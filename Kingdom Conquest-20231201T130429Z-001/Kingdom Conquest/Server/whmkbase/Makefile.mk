####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Prj.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : ֱ�Ӹ���Ŀ¼�µ��������ļ��������make
##              : �����Makefile_$(ConfigurationName).mk����������$(ConfigurationName)��ȡֵӦ����debug����release
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# �ж������Sln-mk/Sln.mk����make solution
# ��Ȼ��ʱĿ¼����Ҳ������Prj.mk������������Ǹ������̹��õģ�����Ҫ�����̵�Ŀ¼�������make����
ifneq ($(wildcard Sln-mk/Sln.mk),)
include $(PMKBASE)/Makefile_Sln.mk

else
# �ж������Prj.mk����make project
ifneq ($(wildcard Prj.mk),)
PRJ_FILE	:= Prj.mk
include $(PMKBASE)/Makefile_Prj.mk

else
ifneq ($(wildcard Prj-mk/Prj.mk),)
PRJ_FILE	:= Prj-mk/Prj.mk
include $(PMKBASE)/Makefile_Prj.mk

else
# û��ָ���ļ���ʲôҲ��������

endif
endif
endif
