####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whsearch_inc_file_4.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 从当前目录开始向上查找SEARCHINCFILE代表的文件，之后某级存在就停止，并include之
##              : 8级检索
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
# 没找到
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

