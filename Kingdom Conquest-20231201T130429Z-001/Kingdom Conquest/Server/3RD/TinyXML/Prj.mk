# ISWHAT��ȡֵ�������£�LIB EXE DLL
ISWHAT		:= LIB
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= .
IntDir		:= ../temp/$(ConfigurationName)/$(ProjectName)
OutDir		:= ../lib
