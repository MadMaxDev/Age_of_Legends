# ISWHAT的取值定义如下：LIB EXE DLL
ISWHAT		:= LIB
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= .
IntDir		:= ../temp/$(ConfigurationName)/$(ProjectName)
OutDir		:= ../lib
