# ISWHAT的取值定义如下：LIB EXE DLL
ISWHAT		:= DLL
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= ./src
# 在temp和out目录中多一级上层目录
UPPERDIR	:= $(notdir $(patsubst %/,%,$(dir $(CURDIR))))
IntDir		:= ../../temp/$(ConfigurationName)/$(UPPERDIR)/$(ProjectName)
OutDir		:= ../../bin/$(ConfigurationName)/$(UPPERDIR)/$(ProjectName)
