# 程序需要的一些全局定义
PMKDEFINEs	:= -DLINUX -fpermissive #-DSTACK_DIRECTION=1 -DSIZEOF_INT=4
# cpp文件后缀，这个主要是用于编译非cpp后缀的文件。个人习惯问题。
CPPEXT		:= cpp
# 一个工具文件名
AR			:= ar
RANLIB		:= ranlib
CC			:= g++ -m32
# 如果是RH9的话就没有--preserve-root这个参数了
RM			:= rm --preserve-root
CP			:= cp
SURECP		:= sh $(PMKBASE)/cmn/whsurecp.sh
MKDIR		:= mkdir

# make dep时需要的选项
DEPFlag		:= -MM -MT
# 连接库需要的选项
ARFlag		:= crs
# 编译DLL需要的obj的编译选项
SOCCFlag	:= -fPIC
# 连接DLL的编译选项
SOLNFlag	:= -shared

# 包含文件向上检索的级别
SEARCHINCLVL:= 4
