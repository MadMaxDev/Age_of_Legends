# ISWHAT的取值定义如下：LIB EXE DLL
ISWHAT		:= LIB
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= .
IntDir		:= ../temp/$(ConfigurationName)/$(ProjectName)
OutDir		:= ../lib
# 把-Wall去掉(因为cryptlib的warning太多了)
CCFlag		:= $(filter-out -Wall,$(CCFlag)) -msse2
#CCFlag		:= $(filter-out -Wall,$(CCFlag))

