####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whsearch_inc_file_4.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : �ӵ�ǰĿ¼��ʼ���ϲ���SEARCHINCFILE������ļ���֮��ĳ�����ھ�ֹͣ����include֮
##              : 8������
## CreationDate : 2006-04-04
## ChangeLog    :
####################################################################

#
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
SEARCHINCFILE := ../$(SEARCHINCFILE)
ifeq ($(wildcard $(SEARCHINCFILE)),)
# û�ҵ�
SEARCHINCFILE :=
endif
endif
endif
endif
endif
endif
endif
endif

ifneq ($(wildcard $(SEARCHINCFILE)),)
include $(SEARCHINCFILE)
endif

