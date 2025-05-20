####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Sln.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 针对Solution，即一组工程(多个目录)的通用make文件
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# 当前目录下应该有Sln-mk/Sln.mk
# 里面有：
# PRJS，表示所有需要编译的工程。whmakeindir.mk中需要用到PRJS来进入各个目录进行编译。
# 
include Sln-mk/Sln.mk

.PHONY: all dirs dep cleanobj cleanint cleanout clean veryclean install

TARGET := all
include $(PMKBASE)/cmn/whmakeindir.mk

TARGET := dirs
include $(PMKBASE)/cmn/whmakeindir.mk

TARGET := dep
include $(PMKBASE)/cmn/whmakeindir.mk

# .o文件的清除
TARGET := cleanobj
include $(PMKBASE)/cmn/whmakeindir.mk

# 临时文件的清除
TARGET := cleanint 
include $(PMKBASE)/cmn/whmakeindir.mk

# 结果文件的清除(即编译结果)
TARGET := cleanout
include $(PMKBASE)/cmn/whmakeindir.mk

# 上面清除的综合
TARGET := clean
include $(PMKBASE)/cmn/whmakeindir.mk

# 删除目录的完全清除
TARGET := veryclean
include $(PMKBASE)/cmn/whmakeindir.mk

# 把结果拷贝到安装目录中去
TARGET := install
include $(PMKBASE)/cmn/whmakeindir.mk

