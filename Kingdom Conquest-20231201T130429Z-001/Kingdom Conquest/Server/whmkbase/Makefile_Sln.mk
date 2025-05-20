####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Sln.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : ���Solution����һ�鹤��(���Ŀ¼)��ͨ��make�ļ�
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# ��ǰĿ¼��Ӧ����Sln-mk/Sln.mk
# �����У�
# PRJS����ʾ������Ҫ����Ĺ��̡�whmakeindir.mk����Ҫ�õ�PRJS���������Ŀ¼���б��롣
# 
include Sln-mk/Sln.mk

.PHONY: all dirs dep cleanobj cleanint cleanout clean veryclean install

TARGET := all
include $(PMKBASE)/cmn/whmakeindir.mk

TARGET := dirs
include $(PMKBASE)/cmn/whmakeindir.mk

TARGET := dep
include $(PMKBASE)/cmn/whmakeindir.mk

# .o�ļ������
TARGET := cleanobj
include $(PMKBASE)/cmn/whmakeindir.mk

# ��ʱ�ļ������
TARGET := cleanint 
include $(PMKBASE)/cmn/whmakeindir.mk

# ����ļ������(��������)
TARGET := cleanout
include $(PMKBASE)/cmn/whmakeindir.mk

# ����������ۺ�
TARGET := clean
include $(PMKBASE)/cmn/whmakeindir.mk

# ɾ��Ŀ¼����ȫ���
TARGET := veryclean
include $(PMKBASE)/cmn/whmakeindir.mk

# �ѽ����������װĿ¼��ȥ
TARGET := install
include $(PMKBASE)/cmn/whmakeindir.mk

